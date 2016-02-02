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

#include <scope/localization.h>
#include <scope/preview.h>
#include <api/client.h>
#include <api/comment.h>

#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/VariantBuilder.h>

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

Preview::Preview(const sc::Result &result, const sc::ActionMetadata &metadata,
                std::shared_ptr<sc::OnlineAccountClient> oa_client) :
    sc::PreviewQueryBase(result, metadata),
    client_(oa_client) {
}

void Preview::cancelled() {
}

void Preview::run(sc::PreviewReplyProxy const& reply) {
    try {
        auto const res = result();

        sc::PreviewWidgetList widgets;
        std::vector<std::string> ids;
        // Support three different column layouts
        sc::ColumnLayout layout1col(1), layout2col(2), layout3col(3);

        string mode = res["mode"].get_string();
        if (mode == _("user")) {
            ids = std::vector<std::string>{ "header", "art", "statistics", "description", "actions"};
            sc::PreviewWidget header("header", "header");
            header.add_attribute_mapping("title", "title");
            widgets.emplace_back(header);

            string artwork_url= res["art"].get_string();
            boost::replace_all(artwork_url, "large", "t500x500");
            sc::PreviewWidget art("art", "image");
            art.add_attribute_value("source", sc::Variant(artwork_url));
            widgets.emplace_back(art);

            sc::PreviewWidget statistics("statistics", "header");
            statistics.add_attribute_value("title", sc::Variant(
                    res["track-count"].get_string() + "  " +
                    res["followers-count"].get_string() + "  " +
                    res["followings-count"].get_string()));
            widgets.emplace_back(statistics);

            sc::PreviewWidget description("description", "text");
            description.add_attribute_value("text", sc::Variant(
                    res["permalink-url"].get_string() + "<br>" +
                    res["bio"].get_string()));
            widgets.emplace_back(description);

            std::string userid  = res["id"].get_string();
            sc::VariantBuilder builder;
            sc::PreviewWidget actions("actions", "actions");
            {
                string permalink_url = res["permalink-url"].get_string();
                builder.add_tuple({
                      {"id", sc::Variant("view")},
                      {"label", sc::Variant(_("View in browser"))},
                      {"uri", sc::Variant(permalink_url)}
                  });
                sc::CannedQuery new_query(SCOPE_NAME);
                new_query.set_department_id("userid:" + userid);
                builder.add_tuple({
                      {"id", sc::Variant("usertracks")},
                      {"label", sc::Variant(_("Get user tracks"))},
                      {"uri", sc::Variant(new_query.to_uri())}
                  });
            }
            actions.add_attribute_value("actions", builder.end());
            widgets.emplace_back(actions);
        } else {
            ids = std::vector<std::string> { "header", "art", "statistics", "trackinfo", "tracks", "description", "actions"};

            sc::PreviewWidget header("header", "header");
            header.add_attribute_mapping("title", "title");
            header.add_attribute_mapping("subtitle", "username");
            widgets.emplace_back(header);

            //load big track thubmail
            string artwork_url= res["art"].get_string();
            boost::replace_all(artwork_url, "large", "t500x500");
            sc::PreviewWidget art("art", "image");
            art.add_attribute_value("source", sc::Variant(artwork_url));
            widgets.emplace_back(art);

            sc::PreviewWidget statistics("statistics", "header");
            statistics.add_attribute_value("title", sc::Variant(
                    res["playback-count"].get_string() + "  " +
                    res["likes-count"].get_string() + "  " +
                    res["repost-count"].get_string() + "  " +
                    res["favoritings-count"].get_string() + "  " +
                    res["comment-count"].get_string()));
            widgets.emplace_back(statistics);

            std::string trackid = res["id"].get_string();
            std::string userid  = res["userid"].get_string();

            sc::PreviewWidget tracks("tracks", "audio");
            {
                if (res["streamable"].get_bool()) {
                    sc::VariantBuilder builder;
                    builder.add_tuple({
                          {"title", sc::Variant(res.title())},
                          {"source", sc::Variant(res["stream-url"])},
                          {"length", res["duration"]}
                      });
                    tracks.add_attribute_value("tracks", builder.end());
                    widgets.emplace_back(tracks);
                }
            }

            if (!client_.authenticated()) {
                ids.emplace_back("tips-headerid");
                sc::PreviewWidget w_tips(ids.at(ids.size() - 1), "text");
                w_tips.add_attribute_value("text", sc::Variant(_("Please login to post a comment  ")));
                widgets.emplace_back(w_tips);

                ids.emplace_back("login-actionId");
                sc::PreviewWidget w_action(ids.at(ids.size() - 1), "actions");
                sc::VariantBuilder builder;
                builder.add_tuple({
                      {"id", sc::Variant(_("open"))},
                      {"label", sc::Variant(_("Login to soundcloud"))},
                  });

                sc::OnlineAccountClient oa_client(SCOPE_NAME, "sharing", SCOPE_ACCOUNTS_NAME);

                oa_client.register_account_login_item(w_action,
                                                      sc::OnlineAccountClient::InvalidateResults,
                                                      sc::OnlineAccountClient::DoNothing);

                w_action.add_attribute_value("actions", builder.end());
                widgets.emplace_back(w_action);
            }

            sc::PreviewWidget trackinfo("trackinfo", "table");
            trackinfo.add_attribute_mapping("values", "trackinfo");
            widgets.emplace_back(trackinfo);

            sc::PreviewWidget description("description", "text");
            description.add_attribute_mapping("text", "description");
            widgets.emplace_back(description);

            if (client_.authenticated()) {
                ids.emplace_back("comment-inputid");
                sc::PreviewWidget w_commentInput(ids.at(ids.size() - 1), "comment-input");
                w_commentInput.add_attribute_value("submit-label", sc::Variant(_("Post")));
                widgets.emplace_back(w_commentInput);
            }

            sc::VariantBuilder builder;
            sc::PreviewWidget actions("actions", "actions");
            {
                string purchase_url = res["purchase-url"].get_string();
                if (!purchase_url.empty()) {
                    builder.add_tuple({
                          {"id", sc::Variant("buy")},
                          {"label", sc::Variant(_("Buy"))},
                          {"uri", sc::Variant(purchase_url)}
                      });
                }
                string video_url = res["video-url"].get_string();
                if (!video_url.empty()) {
                    builder.add_tuple({
                          {"id", sc::Variant("video")},
                          {"label", sc::Variant(_("Watch video"))},
                          {"uri", sc::Variant(video_url)}
                      });
                }
                {
                    builder.add_tuple({
                          {"id", sc::Variant("play")},
                          {"label", sc::Variant(_("Play in browser"))}
                      });
                }
                if (client_.authenticated()) {
                    sc::CannedQuery new_query(SCOPE_NAME);
                    new_query.set_department_id("userid:" + userid);
                    builder.add_tuple({
                          {"id", sc::Variant("usertracks")},
                          {"label", sc::Variant(_("Get user tracks"))},
                          {"uri", sc::Variant(new_query.to_uri())}
                      });

                    future<bool> like_future = client_.is_fav_track(trackid);
                    auto status = get_or_throw(like_future);
                    cout << "is fav stats: " << status << endl;
                    if (status == true) {
                        builder.add_tuple({
                              {"id", sc::Variant("deletelike")},
                              {"label", sc::Variant(_("Remove 'Like'"))}
                          });
                    } else {
                        builder.add_tuple({
                              {"id", sc::Variant("like")},
                              {"label", sc::Variant(_("Like"))}
                          });
                    }

                    future<bool> follow_future = client_.is_user_follower(userid);
                    status = get_or_throw(follow_future);
                    cout << "is users follower: " << status << endl;
                    if (status == true) {
                        builder.add_tuple({
                              {"id", sc::Variant("unfollow")},
                              {"label", sc::Variant(_("Unfollow"))}
                          });
                    } else {
                        builder.add_tuple({
                              {"id", sc::Variant("follow")},
                              {"label", sc::Variant(_("Follow"))}
                          });
                    }
                }
            }
            actions.add_attribute_value("actions", builder.end());
            widgets.emplace_back(actions);

            future<deque<Comment>> comment_future;
            comment_future = client_.track_comments(trackid);

            int index = 0;
            for (const auto &comment : get_or_throw(comment_future)) {
                std::string id = "commentId_"+ std::to_string(index++);
                ids.emplace_back(id);

                sc::PreviewWidget w_comment(id, "comment");
                w_comment.add_attribute_value("comment", sc::Variant(comment.body()));
                w_comment.add_attribute_value("author", sc::Variant(comment.title()));
                w_comment.add_attribute_value("source", sc::Variant(comment.artwork()));
                w_comment.add_attribute_value("subtitle", sc::Variant(comment.created_at()));
                widgets.emplace_back(w_comment);
            }
        }

        layout1col.add_column(ids);
        reply->register_layout( { layout1col }); //, layout2col, layout3col
        reply->push(widgets);
    }catch (domain_error &e) {
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}
