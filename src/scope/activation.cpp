/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 *         Gary Wang  <gary.wang@canonical.com>
 */

#include <scope/activation.h>
#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/ActionMetadata.h>

#include <iostream>

namespace sc = unity::scopes;

using namespace std;
using namespace scope;
using namespace api;

template<typename T>
static T get_or_throw(future<T> &f) {
    if (f.wait_for(std::chrono::seconds(10)) != future_status::ready) {
        throw domain_error("HTTP request timeout");
    }
    return f.get();
}

Activation::Activation(const sc::Result &result,
               const sc::ActionMetadata &metadata,
               std::string const& action_id,
               std::shared_ptr<sc::OnlineAccountClient> oa_client) : 
    sc::ActivationQueryBase(result, metadata), 
    action_id_(action_id),
    client_(oa_client) {
}

sc::ActivationResponse Activation::activate() {
    try {
        string trackid = result()["id"].get_string();
        string userid = result()["userid"].get_string();

        if (action_id_ == "commented") {
            string comments = action_metadata().scope_data().get_dict()["comment"].get_string();
            future<bool> post_future = client_.post_comment(trackid, comments);
            auto status = get_or_throw(post_future);
            cout<< "auth user post a comment: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "like") {
            future<bool> like_future = client_.like_track(trackid);
            auto status = get_or_throw(like_future);
            cout<< "auth user likes track: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "deletelike") {
            future<bool> ret_future = client_.delete_like_track(trackid);
            auto status = get_or_throw(ret_future);
            cout<< "auth user delete a like track: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "follow") {
            future<bool> follow_future = client_.follow_user(userid);
            auto status = get_or_throw(follow_future);
            cout<< "auth user follow user: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        } else if (action_id_ == "unfollow") {
            future<bool> unfollow_future = client_.unfollow_user(userid);
            auto status = get_or_throw(unfollow_future);
            cout<< "auth user unfollow user: " << status << endl;

            return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
        }
    }catch (domain_error &e) {
        cerr << e.what() << endl;
        return sc::ActivationResponse(sc::ActivationResponse::Status::ShowPreview);
    }
    return sc::ActivationResponse(sc::ActivationResponse::Status::NotHandled);
}
