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
    id_ = data["id"].asUInt();
    title_ = data["title"].asString();
    description_ = data["description"].asString();
    label_name_ = data["label_name"].asString();
    duration_ = data["duration"].asUInt();
    license_ = data["license"].asString();
    created_at_ = data["created_at"].asString();

    playback_count_ = data["playback_count"].asUInt();
    favoritings_count_ = data["favoritings_count"].asUInt();

    artwork_ = data["artwork_url"].asString();
    waveform_ = data["waveform_url"].asString();

    streamable_ = data["streamable"].asBool();
    downloadable_ = data["downloadable"].asBool();

    permalink_url_ = data["permalink_url"].asString();
    purchase_url_ = data["purchase_url"].asString();
    stream_url_ = data["stream_url"].asString();
    download_url_ = data["download_url"].asString();
    video_url_ = data["video_url"].asString();
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

const std::string & Track::uri() const {
    return uri_;
}

const std::string & Track::label_name() const {
    return label_name_;
}

unsigned int Track::duration() const {
    return duration_;
}

const std::string & Track::license() const {
    return license_;
}

const std::string & Track::created_at() const {
    return created_at_;
}

bool Track::streamable() const {
    return streamable_;
}

bool Track::downloadable() const {
    return downloadable_;
}

const std::string & Track::permalink_url() const {
    return permalink_url_;
}

const std::string & Track::purchase_url() const {
    return purchase_url_;
}

const std::string & Track::stream_url() const {
    return stream_url_;
}

const std::string & Track::download_url() const {
    return download_url_;
}

const std::string & Track::video_url() const {
    return video_url_;
}

unsigned int Track::playback_count() const {
    return playback_count_;
}

unsigned int Track::favoritings_count() const {
    return favoritings_count_;
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
