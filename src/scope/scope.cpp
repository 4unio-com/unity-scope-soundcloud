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

#include <scope/localization.h>
#include <scope/preview.h>
#include <scope/query.h>
#include <scope/scope.h>
#include <scope/activation.h>

namespace sc = unity::scopes;
using namespace std;
using namespace api;
using namespace scope;

void Scope::start(string const&) {
    setlocale(LC_ALL, "");
    string translation_directory = ScopeBase::scope_directory()
            + "/../share/locale/";
    bindtextdomain(GETTEXT_PACKAGE, translation_directory.c_str());

    if (getenv("SOUNDCLOUD_SCOPE_IGNORE_ACCOUNTS") == nullptr) {
        oa_client_.reset(new sc::OnlineAccountClient(
            SCOPE_NAME, "sharing", SCOPE_ACCOUNTS_NAME));
    }
}

void Scope::stop() {
}

sc::SearchQueryBase::UPtr Scope::search(const sc::CannedQuery &query,
                                        const sc::SearchMetadata &metadata) {
    return sc::SearchQueryBase::UPtr(new Query(query, metadata, oa_client_));
}

sc::PreviewQueryBase::UPtr Scope::preview(sc::Result const& result,
                                          sc::ActionMetadata const& metadata) {
    return sc::PreviewQueryBase::UPtr(new Preview(result, metadata, oa_client_));
}

sc::ActivationQueryBase::UPtr Scope::perform_action(const sc::Result &result,
                                                 const sc::ActionMetadata &metadata,
                                                 const std::string &widget_id,
                                                 const std::string &action_id) {
    return sc::ActivationQueryBase::UPtr(new Activation(result, metadata, action_id, oa_client_));
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

