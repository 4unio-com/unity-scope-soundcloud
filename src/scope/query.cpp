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

#include <boost/algorithm/string/trim.hpp>

#include <scope/localization.h>
#include <scope/query.h>

#include <unity/scopes/Annotation.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/OnlineAccountClient.h>
#include <unity/scopes/QueryBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/VariantBuilder.h>

#include <chrono>
#include <iomanip>
#include <sstream>
#include <ctime>

namespace sc = unity::scopes;
namespace alg = boost::algorithm;

using namespace std;
using namespace std::chrono;
using namespace api;
using namespace scope;

namespace {

const static string SEARCH_CATEGORY_TEMPLATE =
        R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "overlay": true,
    "card-background": "color:///#000000"
  },
  "components": {
    "title": "title",
    "art" : {
      "field": "waveform",
      "aspect-ratio": 4.0
    },
    "subtitle": "username",
    "mascot": "art",
    "attributes": {
      "field": "attributes",
      "max-count": 3
    }
  }
}
)";

const static string SEARCH_CATEGORY_LOGIN_NAG = R"(
{
  "schema-version": 1,
  "template": {
    "category-layout": "grid",
    "card-size": "large",
    "card-background": "color:///#ff4200"
  },
  "components": {
    "title": "title",
    "background": "background",
    "art" : {
      "aspect-ratio": 100.0
    }
  }
}
)";
// unconfuse emacs: "

static const vector<string> AUDIO_DEPARTMENT_IDS { "Audiobooks", "Business",
        "Comedy", "Entertainment", "Learning", "News & Politics",
        "Religion & Spirituality", "Science", "Sports", "Storytelling",
        "Technology" };

static const vector<string> AUDIO_DEPARTMENT_NAMES { _("Audiobooks"), _(
        "Business"), _("Comedy"), _("Entertainment"), _("Learning"), _(
        "News & Politics"), _("Religion & Spirituality"), _("Science"), _(
        "Sports"), _("Storytelling"), _("Technology") };

static const vector<string> MUSIC_DEPARTMENT_IDS { "Popular Music",
        "Alternative Rock", "Ambient", "Classical", "Country", "Dance",
        "Deep House", "Disco", "Drum & Bass", "Dubstep", "Electro",
        "Electronic", "Folk", "Hardcore Techno", "Hip Hop", "House",
        "Indie Rock", "Jazz", "Latin", "Metal", "Minimal Techno", "Piano",
        "Pop", "Progressive House", "Punk", "R&B", "Rap", "Reggae", "Rock",
        "Singer-Songwriter", "Soul", "Tech House", "Techno", "Trance", "Trap",
        "Trip Hop", "World" };

static const vector<string> MUSIC_DEPARTMENT_NAMES { _("Home"), _(
        "Alternative Rock"), _("Ambient"), _("Classical"), _("Country"), _(
        "Dance"), _("Deep House"), _("Disco"), _("Drum & Bass"), _("Dubstep"),
        _("Electro"), _("Electronic"), _("Folk"), _("Hardcore Techno"), _(
                "Hip Hop"), _("House"), _("Indie Rock"), _("Jazz"), _("Latin"),
        _("Metal"), _("Minimal Techno"), _("Piano"), _("Pop"), _(
                "Progressive House"), _("Punk"), _("R&B"), _("Rap"), _(
                "Reggae"), _("Rock"), _("Singer-Songwriter"), _("Soul"), _(
                "Tech House"), _("Techno"), _("Trance"), _("Trap"), _(
                "Trip Hop"), _("World") };

template<typename T>
static T get_or_throw(future<T> &f) {
    if (f.wait_for(std::chrono::seconds(10)) != future_status::ready) {
        throw domain_error("HTTP request timeout");
    }
    return f.get();
}

static sc::Department::SPtr create_departments(const sc::CannedQuery &query) {
    sc::Department::SPtr root_department = sc::Department::create("", query,
            MUSIC_DEPARTMENT_NAMES.front());
    for (size_t i = 1; i < MUSIC_DEPARTMENT_IDS.size(); ++i) {
        sc::Department::SPtr dept = sc::Department::create(
                MUSIC_DEPARTMENT_IDS[i], query, MUSIC_DEPARTMENT_NAMES[i]);
        root_department->add_subdepartment(dept);
    }
    for (size_t i = 0; i < AUDIO_DEPARTMENT_IDS.size(); ++i) {
        sc::Department::SPtr dept = sc::Department::create(
                AUDIO_DEPARTMENT_IDS[i], query, AUDIO_DEPARTMENT_NAMES[i]);
        root_department->add_subdepartment(dept);
    }
    return root_department;
}

static string department_to_category(const string &department) {
    string id = department;
    if (id.empty()) {
        id = MUSIC_DEPARTMENT_IDS.front();
    }
    return id;
}

static string format_fixed(unsigned int i) {
    std::stringstream ss;
    ss.imbue(std::locale(""));
    ss << std::fixed << i;
    return ss.str();
}

static string format_time(unsigned int t) {
    milliseconds ms(t);

    hours   hh = duration_cast<hours>(ms);
    minutes mm = duration_cast<minutes>(ms % chrono::hours(1));
    seconds ss = duration_cast<chrono::seconds>(ms % chrono::minutes(1));

    stringstream result;

    if (hh.count() > 0) {
        result << hh.count() << ":" << setw(2) << setfill('0') << mm.count()
                << ":" << setw(2) << setfill('0') << ss.count();
    } else if (mm.count() > 0) {
        result << mm.count() << ":" << setw(2) << setfill('0') << ss.count();
    } else if (ss.count() > 0) {
        result << "0:" << setw(2) << setfill('0') << ss.count();
    }

    return result.str();
}

}

Query::Query(const sc::CannedQuery &query, const sc::SearchMetadata &metadata,
             std::shared_ptr<sc::OnlineAccountClient> oa_client) :
        sc::SearchQueryBase(query, metadata),
        client_(oa_client) {
}

void Query::cancelled() {
    client_.cancel();
}

void Query::run(sc::SearchReplyProxy const& reply) {
    try {
        const sc::CannedQuery &query(sc::SearchQueryBase::query());
        string query_string = alg::trim_copy(query.query_string());

        reply->register_departments(create_departments(query));

        // Avoid blocking on HTTP requests at this point

        sc::Category::SCPtr first_cat;
        future<deque<Track>> stream_future;
        bool reading_stream = false;
        if (query_string.empty() && query.department_id().empty()) {
            if (client_.authenticated()) {
                first_cat = reply->register_category(
                    "stream", _("Stream"), "",
                    sc::CategoryRenderer(SEARCH_CATEGORY_TEMPLATE));
                stream_future = client_.stream_tracks(30);
                reading_stream = true;
            } else {
                add_login_nag(reply);
            }
        }

        sc::Category::SCPtr second_cat;
        future<deque<Track>> tracks_future;
        if (query_string.empty()) {
            second_cat = reply->register_category("explore", _("Explore"), "",
                    sc::CategoryRenderer(SEARCH_CATEGORY_TEMPLATE));
            tracks_future = client_.search_tracks({
                { SP::query, query_string },
                { SP::limit, "15" },
                { SP::genre, department_to_category(query.department_id()) },
                { SP::order, "hotness" }
            });
        } else {
            second_cat = reply->register_category("search", "", "",
                    sc::CategoryRenderer(SEARCH_CATEGORY_TEMPLATE));
            tracks_future = client_.search_tracks( {
                { SP::query, query_string },
                { SP::limit, "30" }
            });
        }

        // Now we come to wait for the results

        if (reading_stream) {
            for (const auto &track : get_or_throw(stream_future)) {
                if (!push_track(reply, first_cat, track)) {
                    return;
                }
            }
        }

        for (const auto &track : get_or_throw(tracks_future)) {
            if (!push_track(reply, second_cat, track)) {
                return;
            }
        }

    } catch (domain_error &e) {
        cerr << e.what() << endl;
        reply->error(current_exception());
    }
}

bool Query::push_track(const sc::SearchReplyProxy &reply,
                       const sc::Category::SCPtr &category,
                       const Track &track) {
    sc::CategorisedResult res(category);

    res.set_uri(track.permalink_url());
    res.set_title(track.title());
    if (!track.artwork().empty()) {
        res.set_art(track.artwork());
    } else {
        res.set_art(track.user().artwork());
    }

    res["label"] = track.label_name();
    res["streamable"] = track.streamable();
    res["stream-url"] = track.stream_url() + "?client_id=" + client_.client_id();
    res["purchase-url"] = track.purchase_url();
    res["video-url"] = track.video_url();
    res["waveform"] = track.waveform();
    res["username"] = track.user().title();
    res["description"] = track.description();

    string duration = format_time(track.duration());
    string playback_count = u8"\u25B6 " + format_fixed(track.playback_count());
    string favoritings_count = u8"\u2665 " + format_fixed(track.favoritings_count());
    res["duration"] = (int) (track.duration() / 1000);
    res["playback-count"] = playback_count;
    res["favoritings-count"] = favoritings_count;

    sc::VariantBuilder builder;
    builder.add_tuple({{"value", sc::Variant(duration)}});
    builder.add_tuple({{"value", sc::Variant(playback_count)}});
    builder.add_tuple({{"value", sc::Variant(favoritings_count)}});
    res["attributes"] = builder.end();

    return reply->push(res);
}

void Query::add_login_nag(const sc::SearchReplyProxy &reply) {
    if (getenv("SOUNDCLOUD_SCOPE_IGNORE_ACCOUNTS")) {
        return;
    }

    sc::CategoryRenderer rdr(SEARCH_CATEGORY_LOGIN_NAG);
    auto cat = reply->register_category("soundcloud_login_nag", "", "", rdr);

    sc::CategorisedResult res(cat);
    res.set_title(_("Log-in to SoundCloud"));

    sc::OnlineAccountClient oa_client(SCOPE_NAME, "sharing", SCOPE_ACCOUNTS_NAME);
    oa_client.register_account_login_item(res,
                                          query(),
                                          sc::OnlineAccountClient::InvalidateResults,
                                          sc::OnlineAccountClient::DoNothing);

    reply->push(res);
}
