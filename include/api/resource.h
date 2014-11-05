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

#ifndef API_RESOURCE_H_
#define API_RESOURCE_H_

#include <memory>
#include <string>

namespace api {

class Resource {
public:
    enum class Kind {
        track, user
    };

    typedef std::shared_ptr<Resource> Ptr;

    Resource() = default;

    virtual ~Resource() = default;

    virtual const std::string & title() const = 0;

    virtual const std::string & artwork() const = 0;

    virtual const unsigned int & id() const = 0;

    virtual Kind kind() const = 0;

    virtual std::string kind_str() const = 0;
};

}

#endif // API_RESOURCE_H_
