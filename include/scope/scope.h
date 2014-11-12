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

#ifndef SCOPE_SCOPE_H_
#define SCOPE_SCOPE_H_

#include <mutex>
#include <condition_variable>

#include <api/config.h>

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/OnlineAccountClient.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/ReplyProxyFwd.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/PreviewQueryBase.h>

namespace scope {

/**
 * Defines the lifecycle of scope plugin, and acts as a factory
 * for Query and Preview objects.
 *
 * Note that the #preview and #search methods are each called on
 * different threads, so some form of interlocking is required
 * if shared data structures are used.
 */
class Scope: public unity::scopes::ScopeBase {
public:
    /**
     * Called once at startup
     */
    void start(std::string const&) override;

    /**
     * Called at shutdown
     */
    void stop() override;

    /**
     * Called each time a new preview is requested
     */
    unity::scopes::PreviewQueryBase::UPtr preview(const unity::scopes::Result&,
                                                  const unity::scopes::ActionMetadata&) override;

    /**
     * Called each time a new query is requested
     */
    unity::scopes::SearchQueryBase::UPtr search(
            unity::scopes::CannedQuery const& q,
            unity::scopes::SearchMetadata const&) override;

protected:
    std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client_;

    std::mutex config_mutex_;
    std::condition_variable config_cond_;
    api::Config::Ptr config_;

    void init_config();
    void update_config();
};

}

#endif // SCOPE_SCOPE_H_

