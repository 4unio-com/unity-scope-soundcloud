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

#ifndef API_COMMENT_H_
#define API_COMMENT_H_

#include <api/user.h>

#include <memory>
#include <string>

namespace Json {
class Value;
}

namespace api {

class Comment: public Resource {
public:
    typedef std::shared_ptr<Comment> Ptr;

    Comment(const Json::Value &data);

    virtual ~Comment() = default;

    const unsigned int & id() const override;

    const std::string & title() const override;

    const std::string & artwork() const override;

    const std::string & body() const;

    const std::string & created_at() const;
    
    const User & user() const;

    Kind kind() const override;

    std::string kind_str() const override;

protected:
    std::string body_;

    std::string created_at_;
    
    unsigned int id_;
    
    User user_;
};

}

#endif // API_COMMENT_H_
