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

#ifndef API_CONFIG_H_
#define API_CONFIG_H_

#include <memory>
#include <string>

namespace api {

struct Config {
    typedef std::shared_ptr<Config> Ptr;

    std::string client_id { "eadbbc8380aa72be1412e2abe5f8e4ca" };

    std::string apiroot { "https://api.soundcloud.com" };

    std::string user_agent { "unity-scope-soundcloud 0.1" };

    std::string access_token { };

    bool authenticated = false;
};

}

#endif /* API_CONFIG_H_ */

