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
 */

#include <api/client.h>
#include <api/track.h>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
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
        configuration.header.add("User-Agent", config_.user_agent + " (gzip)");
        configuration.header.add("Accept-Encoding", "gzip");

        auto request = client_->head(configuration);
        request->async_execute(handler);
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

                    if (response.status != http::Status::ok) {
                        prom->set_exception(make_exception_ptr(domain_error(root["error"].asString())));
                    } else {
                        prom->set_value(func(root));
                    }
                });

        get(path, parameters, handler);

        return prom->get_future();
    }

    bool authenticated() {
        std::lock_guard<std::mutex> lock(config_mutex_);
        update_config();
        return config_.authenticated;
    }

    void update_config() {
        if (getenv("YOUTUBE_SCOPE_APIROOT")) {
            config_.apiroot = getenv("YOUTUBE_SCOPE_APIROOT");
        }

        if (getenv("YOUTUBE_SCOPE_IGNORE_ACCOUNTS") != nullptr) {
            return;
        }

        /// TODO: The code commented out below should be uncommented as soon as
        /// OnlineAccountClient::refresh_service_statuses() is fixed (Bug #1398813).
        /// For now we have to re-instantiate a new OnlineAccountClient each time.

        ///if (oa_client_ == nullptr) {
            oa_client_.reset(
                    new unity::scopes::OnlineAccountClient(SCOPE_INSTALL_NAME,
                            "sharing", "google"));
        ///} else {
        ///    oa_client_->refresh_service_statuses();
        ///}

        for (auto const& status : oa_client_->get_service_statuses()) {
            if (status.service_authenticated) {
                config_.authenticated = true;
                config_.access_token = status.access_token;
                config_.client_id = status.client_id;
                config_.client_secret = status.client_secret;
                break;
            }
        }

        if (!config_.authenticated) {
            config_.access_token = "";
            config_.client_id = "";
            config_.client_secret = "";
            std::cerr << "YouTube scope is unauthenticated" << std::endl;
        } else {
            std::cerr << "YouTube scope is authenticated" << std::endl;
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

void Client::cancel() {
    p->cancelled_ = true;
}

bool Client::authenticated() {
    return p->authenticated();
}

