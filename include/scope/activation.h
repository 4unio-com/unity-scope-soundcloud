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

#ifndef SCOPE_ACTIVATIOIN_H_
#define SCOPE_ACTIVATIOIN_H_

#include <api/client.h>

#include <unity/scopes/ActivationQueryBase.h>

namespace unity {
namespace scopes {
class Result;
}
}

namespace scope {

/**
 * Represents an individual action request.
 *
 * Each time a action is performed in the UI a new Action
 * object is created.
 */
class Activation : public unity::scopes::ActivationQueryBase
{
public:
    Activation(const unity::scopes::Result &result,
           const unity::scopes::ActionMetadata & metadata,
           std::string const& action_id,
           std::shared_ptr<unity::scopes::OnlineAccountClient> oa_client);

    ~Activation() = default;

     /**
     * Trigger the action object with action id.
     */
     virtual unity::scopes::ActivationResponse activate() override;

private:
    std::string const action_id_;
    
    api::Client client_;
};

}

#endif // SCOPE_ACTIVATION_H_
