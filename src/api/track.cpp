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

#include <api/track.h>

#include <json/json.h>

namespace json = Json;
using namespace api;
using namespace std;

Track::Track(const json::Value &data) :
        user_(data["user"]) {
    title_ = data["title"].asString();
    description_ = data["description"].asString();

    id_ = data["id"].asUInt();
    artwork_ = data["artwork_url"].asString();
    waveform_ = data["waveform_url"].asString();
}

const string & Track::title() const {
    return title_;
}

const unsigned int & Track::id() const {
    return id_;
}

const string & Track::artwork() const {
    return artwork_;
}

const string & Track::waveform() const {
    return waveform_;
}

const string & Track::description() const {
    return description_;
}

const User & Track::user() const {
    return user_;
}

Resource::Kind Track::kind() const {
    return Resource::Kind::track;
}

std::string Track::kind_str() const {
    return "track";
}
