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

#include <boost/algorithm/string.hpp>
#include <api/comment.h>

#include <json/json.h>

namespace json = Json;
using namespace api;
using namespace std;

Comment::Comment(const json::Value &data) :
        user_(data["user"]) {
    body_ = data["body"].asString();
    created_at_ = data["created_at"].asString();
    
    std::vector<std::string> created_time;
    boost::split(created_time, created_at_, boost::is_any_of(" "));
    created_at_ = created_time[0];

    id_ = data["id"].asUInt();
}

const unsigned int & Comment::id() const {
    return id_; 
}

const string & Comment::body() const {
    return body_;
}

const string & Comment::title() const {
    return user_.title();
}

const string & Comment::artwork() const {
    return user_.artwork();
}

const string & Comment::created_at() const {
    return created_at_;
}

const User & Comment::user() const {
    return user_;
}

Resource::Kind Comment::kind() const {
    return Resource::Kind::comment;
}

std::string Comment::kind_str() const {
    return "comment";
}
