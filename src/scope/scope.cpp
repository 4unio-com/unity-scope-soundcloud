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

#include <scope/localization.h>
#include <scope/preview.h>
#include <scope/query.h>
#include <scope/scope.h>

#include <iostream>
#include <sstream>
#include <fstream>

namespace sc = unity::scopes;
using namespace std;
using namespace api;
using namespace scope;

void Scope::init_config() {
    config_ = make_shared<Config>();
    // Under test we set a different API root
    char *apiroot = getenv("NETWORK_SCOPE_APIROOT");
    if (apiroot) {
        config_->apiroot = apiroot;
    }
}

void Scope::update_config() {
    std::lock_guard<std::mutex> lock(config_mutex_);
    init_config();

    for (auto const& status: oa_client_->get_service_statuses()) {
        if (status.service_authenticated) {
            config_->authenticated = true;
            config_->access_token = status.access_token;
            config_->client_id = status.client_id;
            break;
        }
    }

    if (config_->authenticated) {
        cerr << "SoundCloud scope is authenticated" << endl;
    } else {
        cerr << "SoundCloud scope is unauthenticated" << endl;
    }
    config_cond_.notify_all();
}

void Scope::start(string const&) {
    config_ = make_shared<Config>();
    config_->directory = scope_directory();

    setlocale(LC_ALL, "");
    string translation_directory = ScopeBase::scope_directory()
            + "/../share/locale/";
    bindtextdomain(GETTEXT_PACKAGE, translation_directory.c_str());

    if (getenv("SOUNDCLOUD_SCOPE_IGNORE_ACCOUNTS") == nullptr) {
        oa_client_.reset(new sc::OnlineAccountClient(
            SCOPE_NAME, "sharing", ACCOUNTS_NAME));
        oa_client_->set_service_update_callback(
            [this](sc::OnlineAccountClient::ServiceStatus const &) {
                this->update_config();
            });

        ///! TODO: We should only be waiting here if we know that
        ///        there is at least one SoundCloud account enabled.
        ///        OnlineAccountClient needs to expose some
        ///        functionality for us to determine that.

        // Allow 1 second for the callback to initialize config_
        std::unique_lock<std::mutex> lock(config_mutex_);
        config_cond_.wait_for(lock, std::chrono::seconds(1), [this] { return config_ != nullptr; });
    }

    if (config_ == nullptr) {
        init_config();
        cerr << "SoundCloud scope is unauthenticated" << endl;
    }
}

void Scope::stop() {
}

sc::SearchQueryBase::UPtr Scope::search(const sc::CannedQuery &query,
                                        const sc::SearchMetadata &metadata) {
    return sc::SearchQueryBase::UPtr(new Query(query, metadata, config_));
}

sc::PreviewQueryBase::UPtr Scope::preview(sc::Result const& result,
                                          sc::ActionMetadata const& metadata) {
    return sc::PreviewQueryBase::UPtr(new Preview(result, metadata));
}

#define EXPORT __attribute__ ((visibility ("default")))

// These functions define the entry points for the scope plugin
extern "C" {

EXPORT
unity::scopes::ScopeBase*
// cppcheck-suppress unusedFunction
UNITY_SCOPE_CREATE_FUNCTION() {
    return new Scope();
}

EXPORT
void
// cppcheck-suppress unusedFunction
UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base) {
    delete scope_base;
}

}

