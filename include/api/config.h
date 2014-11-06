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

    std::string directory {};
};

}

#endif /* API_CONFIG_H_ */

