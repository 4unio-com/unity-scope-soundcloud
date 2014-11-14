#include <scope/scope.h>

#include <core/posix/exec.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/SearchReplyProxyFwd.h>
#include <unity/scopes/Variant.h>
#include <unity/scopes/testing/Category.h>
#include <unity/scopes/testing/MockSearchReply.h>
#include <unity/scopes/testing/TypedScopeFixture.h>

using namespace std;
using namespace testing;
using namespace scope;

namespace posix = core::posix;
namespace sc = unity::scopes;
namespace sct = unity::scopes::testing;

namespace unity {
namespace scopes {

void PrintTo(const CategorisedResult& res, ::std::ostream* os) {
  *os << "CategorisedResult: " << Variant(res.serialize()).serialize_json();
}

}
}

/**
 * Keep the tests in an anonymous namespace
 */
namespace {

/**
 * Custom matcher to check the properties of search results
 */
MATCHER_P2(ResultProp, prop, value, "") {
    if (arg.contains(prop)) {
        *result_listener << "result[" << prop << "] is " << arg[prop].serialize_json();
    } else {
        *result_listener << "result[" << prop << "] is not set";
    }
    return arg.contains(prop) && arg[prop] == sc::Variant(value);
}

/**
 * Custom matcher to check the presence of departments
 */
MATCHER_P(IsDepartment, department, "") {
    return arg->serialize() == department->serialize();
}

typedef sct::TypedScopeFixture<Scope> TypedScopeFixtureScope;

class TestScope: public TypedScopeFixtureScope {
protected:
    void SetUp() override
    {
        // Start up Python-based fake OpenWeatherMap server
        fake_server_ = posix::exec("/usr/bin/python3", { FAKE_SERVER }, { },
                                   posix::StandardStream::stdout);

        // Check it's running
        ASSERT_GT(fake_server_.pid(), 0);
        string port;
        // The server will print out the random port it is using
        fake_server_.cout() >> port;
        // Check we have a port
        ASSERT_FALSE(port.empty());

        // Build up the API root
        string apiroot = "http://127.0.0.1:" + port;
        // Override the API root that the scope will use
        setenv("NETWORK_SCOPE_APIROOT", apiroot.c_str(), true);

        setenv("SOUNDCLOUD_SCOPE_IGNORE_ACCOUNTS", "true", true);

        // Do the parent SetUp
        TypedScopeFixture::set_scope_directory(TEST_SCOPE_DIRECTORY);
        TypedScopeFixtureScope::SetUp();
    }

    /**
     * Start by assuming the server is invalid
     */
    posix::ChildProcess fake_server_ = posix::ChildProcess::invalid();
};

TEST_F(TestScope, empty_search_string) {
    const sc::CategoryRenderer renderer;
    NiceMock<sct::MockSearchReply> reply;

    // Build a query with an empty search string
    sc::CannedQuery query(SCOPE_NAME, "", "");

    EXPECT_CALL(reply, register_departments(_)).WillOnce(Return());

    // Expect the "explore" category
    EXPECT_CALL(reply, register_category("explore", "Explore", "", _))
            .WillOnce(Return(make_shared<sct::Category>("explore", "Explore", "", renderer)));

    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "More To Me - The SHBC Youth Choir - 1975"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000087979234-7cep7m-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Mc Mona Desamor  ("),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000073185202-uf8hf2-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Kompa Zouk Mix 11.14.2014 By DJ Seven"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111350-7i4ahu-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Roacks Feat. Dj Diego @Presents  Episode 01"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111348-5raywd-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Guillermo Santis-Short beat 2004-2006"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111346-83dnq0-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "lastchild-tak pernah ternilai"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000114345711-6hlhnb-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Fading"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111342-wdq1mu-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Supernova.mp3"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111339-ftgujh-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Abduction Harvest Practice Vox"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111338-ynvx0r-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "whos fault"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111340-whcqkm-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Sakima's Voice"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111337-q8jkbh-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Posted Up Instrumental 2"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000109811572-y2buso-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "clear"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111335-egypfk-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Fuck Em"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000097111336-xyssft-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "12. Freq36 - Magical Desert"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000071660061-2d3qop-large.jpg")
        )))).WillOnce(Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_GB", "phone");

    // Create a query object
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);

    // Run the search
    search_query->run(reply_proxy);

    // Google Mock will make assertions when the mocks are destructed.
}

TEST_F(TestScope, search) {
    const sc::CategoryRenderer renderer;
    NiceMock<sct::MockSearchReply> reply;

    // Build a query with a non-empty search string
    sc::CannedQuery query(SCOPE_NAME, "hermitude", "");

    // Expect the search category
    EXPECT_CALL(reply, register_category("search", "", "", _)).Times(1)
            .WillOnce(Return(make_shared<sct::Category>("search", "", "", renderer)));

    // With one result
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - HyperParadise (Flume Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000024685089-qb8n2m-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Ukiyo"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000076220875-r5sdzw-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude x Flume - Hyperparadise (GANZ Flip) - Now available on iTunes"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000070790362-35yfc9-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Get In My Life"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000011278619-3z5yua-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - Hyper Paradise (Flume Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000025119833-mq4j9w-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "HyperParadise"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000095874960-cpz1te-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Let You Go"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000095874960-cpz1te-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - Ukiyo"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000076815278-8bf40i-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Rufus Du Sol - Modest Life (Hermitude Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000095874960-cpz1te-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Say My Name (feat. Zyra) (Hermitude Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000091815486-sfr3qd-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Speak of the Devil"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000011278563-er3f8j-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "The Presets - Ghosts (Hermitude Trapped in Heaven Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000029601833-4g7k65-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Holdin On (Hermitude Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000079007030-gjh17s-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - HyperParadise (Flume Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000034663576-h073vm-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude x Flume - Hyperparadise (GANZ Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000070974633-6nml9p-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "DJ NOIZ 2014 REMIX- HERMITUDE VS ONLY THAT REAL"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000114354320-6m3j4w-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - Speak of the Devil"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000011277470-yyse8r-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - HyperParadise (Flume Remix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000024582389-bf84nk-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - HyperParadise"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000015281854-2pvkgf-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Boiler Room Sydney - Hermitude"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000072662285-21x0ab-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude ft. Rick Ross - HyperHustlin' (V\xc3\x84RT mix)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000039684445-1whny5-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - Get Free (Major Lazer Cover)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000031151618-5yvw8a-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - Get In My Life"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000007310241-5y1ucb-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - The Villian"),
        ResultProp("art", "https://i1.sndcdn.com/avatars-000023174642-ytiuqw-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "PARADISE (Ganz x Flume x Hermitude x Juicy J x Katy Perry)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000074416243-my109m-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude x Flume - Hyperparadise (GANZ Flip)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000070631497-xd90tg-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Flume x Hermitude - Hyperparadise (GANZ Flip)"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000076794027-uofhyo-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude x Flume - Hyperparadise (GANZ Flip) [REPOST]"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000071377156-dcd59c-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Hermitude - The Villain"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000063897781-dnnjds-large.jpg")
        )))).WillOnce(Return(true));
    EXPECT_CALL(reply, push(Matcher<sc::CategorisedResult const&>(AllOf(
        ResultProp("title", "Life In Color Mix"),
        ResultProp("art", "https://i1.sndcdn.com/artworks-000035495213-nlveml-large.jpg")
        )))).WillOnce(Return(true));

    sc::SearchReplyProxy reply_proxy(&reply, [](sc::SearchReply*) {}); // note: this is a std::shared_ptr with empty deleter
    sc::SearchMetadata meta_data("en_GB", "phone");

    // Create a query object
    auto search_query = scope->search(query, meta_data);
    ASSERT_NE(nullptr, search_query);

    // Run the search
    search_query->run(reply_proxy);

    // Google Mock will make assertions when the mocks are destructed.
}

} // namespace

