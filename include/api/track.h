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

#ifndef API_TRACK_H_
#define API_TRACK_H_

#include <api/user.h>

#include <memory>
#include <string>

namespace Json {
class Value;
}

namespace api {

class Track: public Resource {
public:
    typedef std::shared_ptr<Track> Ptr;

    Track(const Json::Value &data);

    virtual ~Track() = default;

    const unsigned int & id() const override;

    const std::string & title() const override;

    const std::string & artwork() const override;

    const std::string & waveform() const;

    const std::string & description() const;

    const std::string & uri() const;

    const std::string & label_name() const;

    unsigned int duration() const;

    const std::string & license() const;

    const std::string & created_at() const;

    bool streamable() const;

    bool downloadable() const;

    const std::string & permalink_url() const;

    const std::string & purchase_url() const;

    const std::string & stream_url() const;

    const std::string & download_url() const;

    const std::string & video_url() const;

    unsigned int playback_count() const;

    unsigned int favoritings_count() const;

    unsigned int comment_count() const;

    unsigned int repost_count() const;

    unsigned int likes_count() const;

    const std::string & genre() const;

    const std::string & original_format() const;

    const User & user() const;

    Kind kind() const override;

    std::string kind_str() const override;

protected:
    unsigned int id_;

    std::string title_;

    std::string description_;

    std::string artwork_;

    std::string waveform_;

    std::string uri_;

    std::string label_name_;

    unsigned int duration_;

    std::string license_;

    std::string created_at_;

    bool streamable_;

    bool downloadable_;

    std::string permalink_url_;

    std::string purchase_url_;

    std::string stream_url_;

    std::string download_url_;

    std::string video_url_;

    unsigned int playback_count_;

    unsigned int favoritings_count_;

    unsigned int comment_count_;

    unsigned int repost_count_;

    unsigned int likes_count_;

    std::string genre_;

    std::string original_format_;

    User user_;
};

}

#endif // API_TRACK_H_
