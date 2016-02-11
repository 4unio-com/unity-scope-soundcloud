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

#include <api/user.h>

#include <json/json.h>

namespace json = Json;
using namespace api;
using namespace std;

User::User(const json::Value &data) {
    title_ = data["username"].asString();
    id_ = data["id"].asUInt();
    artwork_ = data["avatar_url"].asString();
    permalink_ = data["permalink_url"].asString();
    track_count_ = data["track_count"].asUInt();
    followers_count_ = data["followers_count"].asUInt();
    followings_count_ = data["followings_count"].asUInt();
    bio_ = data["description"].asString();
}

const string & User::title() const {
    return title_;
}

const unsigned int & User::id() const {
    return id_;
}

const string &User::permalink_url() const
{
    return permalink_;
}

const unsigned int &User::track_count() const
{
    return track_count_;
}

const unsigned int &User::followers_count() const
{
    return followers_count_;
}

const unsigned int &User::followings_count() const
{
    return followings_count_;
}

const string &User::bio() const
{
    return bio_;
}

const string & User::artwork() const {
    return artwork_;
}

Resource::Kind User::kind() const {
    return Resource::Kind::user;
}

std::string User::kind_str() const {
    return "user";
}
