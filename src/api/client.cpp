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

#include <api/client.h>
#include <api/track.h>
#include <api/comment.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/algorithm/string.hpp>
#include <core/net/error.h>
#include <core/net/http/client.h>
#include <core/net/http/content_type.h>
#include <core/net/http/response.h>
#include <json/json.h>

#include <algorithm>

namespace http = core::net::http;
namespace io = boost::iostreams;
namespace json = Json;
namespace net = core::net;

using namespace api;
using namespace std;

namespace {

template<typename T>
static deque<T> get_typed_list(const string &filter, const json::Value &root) {
    deque<T> results;
    for (json::ArrayIndex index = 0; index < root.size(); ++index) {
        json::Value item = root[index];

        string kind = item["kind"].asString();

        if (kind == filter) {
            results.emplace_back(T(item));
        }
    }
    return results;
}

template<typename T>
static deque<T> get_typed_activity_list(const string &filter, const json::Value &root) {
    deque<T> results;
    json::Value collection = root["collection"];
    for (json::ArrayIndex index = 0; index < collection.size(); ++index) {
        json::Value item = collection[index];

        string activity_type = item["type"].asString();

        if (activity_type == filter) {
            results.emplace_back(T(item["origin"]));
        }
    }
    return results;
}

template<typename T>
static T get_typed_authuser_info(const string &filter, const json::Value &root) {
    T results((json::Value()));

    string kind = root["kind"].asString();

    if (kind == filter) {
        return T(root);
    }
    return results;
}

template<typename T>
static T is_successful(const json::Value &root) {
    T results = (boost::algorithm::contains(root["status"].asString(), "201")
                  || boost::algorithm::contains(root["status"].asString(), "200")
                  || root["id"].asUInt() > 0);
    return results;
}

}

class Client::Priv {
public:
    Priv(std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client) :
            client_(http::make_client()), worker_ { [this]() {client_->run();} },
            oa_client_(oa_client), cancelled_(false) {
    }

    ~Priv() {
        client_->stop();
        if (worker_.joinable()) {
            worker_.join();
        }
    }

    std::shared_ptr<core::net::http::Client> client_;

    std::thread worker_;

    Config config_;
    std::mutex config_mutex_;

    std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client_;

    std::atomic<bool> cancelled_;

    void get(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            http::Request::Handler &handler) {
        std::lock_guard<std::mutex> lock(config_mutex_);
        http::Request::Configuration configuration = net_config(path, parameters);
        configuration.header.add("User-Agent", config_.user_agent + " (gzip)");
        configuration.header.add("Accept-Encoding", "gzip");

        auto request = client_->head(configuration);
        request->async_execute(handler);
    }

    void post(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            const std::string &postmsg,
            const std::string &content_type,
            http::Request::Handler &handler) {
        std::lock_guard<std::mutex> lock(config_mutex_);
        http::Request::Configuration configuration = net_config(path, parameters);
        configuration.header.add("User-Agent", config_.user_agent);
        configuration.header.add("Content-Type", content_type);
      
        auto request = client_->post(configuration, postmsg, content_type);
        request->async_execute(handler);
    }
    
    void put(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            const std::string &postmsg,
            http::Request::Handler &handler) {
        std::lock_guard<std::mutex> lock(config_mutex_);
        http::Request::Configuration configuration = net_config(path, parameters);
        configuration.header.add("User-Agent", config_.user_agent);
        std::istringstream is(postmsg);

        auto request = client_->put(configuration, is, postmsg.length());
        request->async_execute(handler);
    }

    void del(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            http::Request::Handler &handler) {
        std::lock_guard<std::mutex> lock(config_mutex_);
        http::Request::Configuration configuration = net_config(path, parameters);
        configuration.header.add("User-Agent", config_.user_agent);
        configuration.header.add("X-HTTP-Method-Override", "DELETE");

        auto request = client_->post(configuration, "", "");
        request->async_execute(handler);
    }

    http::Request::Configuration net_config(const net::Uri::Path &path,
                                            const net::Uri::QueryParameters &parameters) {
        update_config();

        http::Request::Configuration configuration;
        net::Uri::QueryParameters complete_parameters(parameters);
        if (config_.authenticated) {
            complete_parameters.emplace_back(
                "oauth_token", config_.access_token);
        } else {
            complete_parameters.emplace_back("client_id", config_.client_id);
        }

        net::Uri uri = net::make_uri(config_.apiroot, path,
                complete_parameters);
        configuration.uri = client_->uri_to_string(uri);

		return configuration;
	}

    http::Request::Progress::Next progress_report(
            const http::Request::Progress&) {
        return cancelled_ ?
                http::Request::Progress::Next::abort_operation :
                http::Request::Progress::Next::continue_operation;
    }

    template<typename T>
    future<T> async_get(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            const function<T(const json::Value &root)> &func) {
        auto prom = make_shared<promise<T>>();

        http::Request::Handler handler;
        handler.on_progress(
                bind(&Client::Priv::progress_report, this, placeholders::_1));
        handler.on_error([prom](const net::Error& e)
        {
            prom->set_exception(make_exception_ptr(e));
        });
        handler.on_response(
                [prom,func](const http::Response& response)
                {
                    string decompressed;

                    if(!response.body.empty()) {
                        try {
                            io::filtering_ostream os;
                            os.push(io::gzip_decompressor());
                            os.push(io::back_inserter(decompressed));
                            os << response.body;
                            boost::iostreams::close(os);
                        } catch(io::gzip_error &e) {
                            prom->set_exception(make_exception_ptr(e));
                            return;
                        }
                    }

                    json::Value root;
                    json::Reader reader;
                    reader.parse(decompressed, root);

                    //Soundcloud api return 404 if track is not in auth user's favorite list
                    //or auth user is not following one certain user.
//                    if (response.status != http::Status::ok) {
//                        prom->set_exception(make_exception_ptr(domain_error(root["error"].asString())));
//                    } else {
                        prom->set_value(func(root));
//                    }
                });

        get(path, parameters, handler);

        return prom->get_future();
    }

    template<typename T>
    future<T> async_post(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            const std::string &postmsg,
            const std::string &content_type,
            const function<T(const json::Value &root)> &func) {
        auto prom = make_shared<promise<T>>();

        http::Request::Handler handler;
        handler.on_progress(
                bind(&Client::Priv::progress_report, this, placeholders::_1));
        handler.on_error([prom](const net::Error& e)
        {
            prom->set_exception(make_exception_ptr(e));
        });
        handler.on_response(
                [prom,func](const http::Response& response)
                {
                    json::Value root;
                    json::Reader reader;
                    reader.parse(response.body, root);

                    if (response.status != http::Status::ok && 
						response.status != http::Status::created) {
                        prom->set_exception(make_exception_ptr(domain_error(root["error"].asString())));
                    } else {
                        prom->set_value(func(root));
                    }
                });

        post(path, parameters, postmsg, content_type, handler);

        return prom->get_future();
    }
    
    template<typename T>
    future<T> async_put(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            const std::string &msg,
            const function<T(const json::Value &root)> &func) {
        auto prom = make_shared<promise<T>>();

        http::Request::Handler handler;
        handler.on_progress(
                bind(&Client::Priv::progress_report, this, placeholders::_1));
        handler.on_error([prom](const net::Error& e)
        {
            prom->set_exception(make_exception_ptr(e));
        });
        handler.on_response(
                [prom,func](const http::Response& response)
                {		  
                    json::Value root;
                    json::Reader reader;
                    reader.parse(response.body, root);

                    if (response.status != http::Status::created &&
                        response.status != http::Status::ok) {
                        prom->set_exception(make_exception_ptr(domain_error(root["error"].asString())));
                    } else {
                        prom->set_value(func(root));
                    }
                });

        put(path, parameters, msg, handler);

        return prom->get_future();
    }

    template<typename T>
    future<T> async_del(const net::Uri::Path &path,
            const net::Uri::QueryParameters &parameters,
            const function<T(const json::Value &root)> &func) {
        auto prom = make_shared<promise<T>>();

        http::Request::Handler handler;
        handler.on_progress(
                bind(&Client::Priv::progress_report, this, placeholders::_1));
        handler.on_error([prom](const net::Error& e)
        {
            prom->set_exception(make_exception_ptr(e));
        });
        handler.on_response(
                [prom,func](const http::Response& response)
                {
                    json::Value root;
                    json::Reader reader;
                    reader.parse(response.body, root);

                    if (response.status != http::Status::created &&
                            response.status != http::Status::ok) {
                        prom->set_exception(make_exception_ptr(domain_error(root["error"].asString())));
                    } else {
                        prom->set_value(func(root));
                    }
                });

        del(path, parameters, handler);

        return prom->get_future();
    }

    std::string client_id() {
        std::lock_guard<std::mutex> lock(config_mutex_);
        update_config();
        return config_.client_id;
    }

    bool authenticated() {
        std::lock_guard<std::mutex> lock(config_mutex_);
        update_config();
        return config_.authenticated;
    }

    void update_config() {
        config_ = Config();

        if (getenv("NETWORK_SCOPE_APIROOT")) {
            config_.apiroot = getenv("NETWORK_SCOPE_APIROOT");
        }

        if (getenv("SOUNDCLOUD_SCOPE_IGNORE_ACCOUNTS") != nullptr) {
            return;
        }

        /// TODO: The code commented out below should be uncommented as soon as
        /// OnlineAccountClient::refresh_service_statuses() is fixed (Bug #1398813).
        /// For now we have to re-instantiate a new OnlineAccountClient each time.

        ///if (oa_client_ == nullptr) {
            oa_client_.reset(new unity::scopes::OnlineAccountClient(
                SCOPE_NAME, "sharing", SCOPE_ACCOUNTS_NAME));
        ///} else {
        ///    oa_client_->refresh_service_statuses();
        ///}

        for (auto const& status : oa_client_->get_service_statuses()) {
            if (status.service_authenticated) {
                config_.authenticated = true;
                config_.access_token = status.access_token;
                config_.client_id = status.client_id;
                break;
            }
        }

        if (!config_.authenticated) {
            std::cerr << "SoundCloud scope is unauthenticated" << std::endl;
        } else {
            std::cerr << "SoundCloud scope is authenticated" << std::endl;
        }
    }
};

Client::Client(std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client) :
        p(new Priv(oa_client)) {
}

future<deque<Track>> Client::search_tracks(const std::deque<std::pair<SP, std::string>> &parameters) {
    bool sort = false;
    net::Uri::QueryParameters params;
    for(const auto &p: parameters) {
        switch(p.first){
        case SP::genre:
            params.emplace_back(make_pair("genres", p.second));
            break;
        case SP::limit:
            params.emplace_back(make_pair("limit", p.second));
            break;
        case SP::order:
            sort = true;
            break;
        case SP::query:
            params.emplace_back(make_pair("q", p.second));
            break;
        }
    }

    return p->async_get<deque<Track>>( { "tracks.json" }, params,
            [sort](const json::Value &root) {
                auto results = get_typed_list<Track>("track", root);
                // Unfortunately SoundCloud doesn't support ordering by hotness any more
                // See excuse on developer blog: https://developers.soundcloud.com/blog/removing-hotness-param
                if (sort) {
                    stable_sort(results.begin(), results.end(), [](const Track &i, const Track &j) {
                                return i.playback_count() > j.playback_count();
                            });
                }
                return results;
            });
}

future<deque<Track>> Client::stream_tracks(int limit) {
    net::Uri::QueryParameters params;
    if (limit > 0) {
        params.emplace_back("limit", std::to_string(limit));
    }
    return p->async_get<deque<Track>>(
        { "me", "activities", "tracks", "affiliated.json" }, params,
        [](const json::Value &root) {
            return get_typed_activity_list<Track>("track", root);
        });
}

future<deque<Comment>> Client::track_comments(const std::string &trackid) {
    net::Uri::QueryParameters params;

    return p->async_get<deque<Comment>>( 
        { "tracks", trackid, "comments.json"}, params,
        [](const json::Value &root) {
            auto results = get_typed_list<Comment>("comment", root);
            return results;
        });
}

future<bool> Client::post_comment(const std::string &trackid,
                                  const std::string &postmsg) {
    net::Uri::QueryParameters params;

    string postbody = "<comment><body>"+ postmsg + "</body></comment>";
    std::string content_type = "application/xml";
    return p->async_post<bool>(
        { "tracks", trackid, "comments.json"}, params, postbody, content_type,
        [](const json::Value &root) {
            auto results = is_successful<bool>(root);
            return results;
    });
}

std::future<std::deque<Track> > Client::favorite_tracks()
{
    net::Uri::QueryParameters params;

    return p->async_get<deque<Track>>(
        { "me", "favorites.json"}, params,
        [](const json::Value &root) {
            return get_typed_list<Track>("track", root);
    });
}

std::future<std::deque<Track> > Client::get_user_tracks(const string &userid,
                                                        int limit)
{
    net::Uri::QueryParameters params;
    if (limit > 0) {
        params.emplace_back("limit", std::to_string(limit));
    }
    return p->async_get<deque<Track>>(
        { "users", userid, "tracks.json"}, params,
        [](const json::Value &root) {
            return get_typed_list<Track>("track", root);
        });
}

future<bool> Client::is_fav_track(const std::string &trackid) {
    net::Uri::QueryParameters params;

    return p->async_get<bool>(
        { "me", "favorites", trackid}, params,
        [](const json::Value &root) {
            auto results = is_successful<bool>(root);
            return results;
        });
}

future<bool> Client::like_track(const std::string &trackid) {
    net::Uri::QueryParameters params;

    return p->async_put<bool>(
        { "me", "favorites", trackid}, params, "",
        [](const json::Value &root) {
            auto results = is_successful<bool>(root);
            return results;
    });
}

future<bool> Client::delete_like_track(const std::string &trackid) {
    net::Uri::QueryParameters params;

    return p->async_del<bool>(
        { "me", "favorites", trackid}, params,
        [](const json::Value &root) {
            auto results = is_successful<bool>(root);
            return results;
    });
}

std::future<bool> Client::is_user_follower(const string &userid) {
    net::Uri::QueryParameters params;

    return p->async_get<bool>(
        { "me", "followings", userid}, params,
        [](const json::Value &root) {
            auto results = is_successful<bool>(root);
            return results;
    });
}

future<bool> Client::follow_user(const std::string &userid) {
    net::Uri::QueryParameters params;

    return p->async_put<bool>(
        { "me", "followings", userid}, params, "",
        [](const json::Value &root) {
            auto results = is_successful<bool>(root);
            return results;
    });
}

std::future<bool> Client::unfollow_user(const string &userid)
{
    net::Uri::QueryParameters params;

    return p->async_del<bool>(
        { "me", "followings", userid}, params,
        [](const json::Value &root) {
            auto results = is_successful<bool>(root);
            return results;
    });
}

std::future<User> Client::get_authuser_info()
{
    net::Uri::QueryParameters params;

    return p->async_get<User>(
        { "me" }, params,
        [](const json::Value &root) {
            auto results = get_typed_authuser_info<User>("user", root);
            return results;
    });
}

std::future<User> Client::get_user_info(const string &userid)
{
    net::Uri::QueryParameters params;

    return p->async_get<User>(
        { "users", userid}, params,
        [](const json::Value &root) {
            auto results = get_typed_authuser_info<User>("user", root);
            return results;
    });
}

void Client::cancel() {
    p->cancelled_ = true;
}

std::string Client::client_id() {
    return p->client_id();
}

bool Client::authenticated() {
    return p->authenticated();
}

