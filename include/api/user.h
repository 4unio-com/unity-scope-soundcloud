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

#ifndef API_USER_H_
#define API_USER_H_

#include <api/resource.h>

#include <memory>
#include <string>

namespace Json {
class Value;
}

namespace api {

class User: public Resource {
public:
    typedef std::shared_ptr<User> Ptr;

    User(const Json::Value &data);

    virtual ~User() = default;

    const std::string & title() const override;

    const std::string & artwork() const override;

    const unsigned int & id() const override;

    const std::string & permalink_url() const;

    unsigned int track_count() const;

    unsigned int followers_count() const;

    unsigned int followings_count() const;

    const std::string & bio() const;

    Kind kind() const override;

    std::string kind_str() const override;

protected:
    std::string title_;

    unsigned int id_;

    std::string artwork_;

    std::string permalink_;

    unsigned int track_count_;

    unsigned int followers_count_;

    unsigned int followings_count_;

    std::string bio_;
};

}

#endif // API_USER_H_
