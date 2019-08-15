#ifndef LUNARWATCHER_ADVLAND
#define LUNARWATCHER_ADVLAND

#include "Poco/Exception.h"
#include "Poco/Net/Context.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/HTTPCookie.h"
#include "Poco/Net/HTTPMessage.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/NameValueCollection.h"
#include "Poco/StreamCopier.h"
#include "nlohmann/json.hpp"

#include "game/Player.hpp"
#include "game/PlayerSkeleton.hpp"
#include "meta/Typedefs.hpp"
#include "objects/GameData.hpp"
#include "objects/Server.hpp"

#include <algorithm>
#include <iostream>
#include <istream>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <utility>

#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>

namespace advland {

using namespace Poco;
using namespace Poco::Net;

class AdvLandClient {
private:
    static auto inline const mLogger = spdlog::stdout_color_mt("AdvLandClient");
    HTTPSClientSession session;

    // This is the session cookie. It's used with some calls, and is essential for base
    // post-login auth operations.
    std::string sessionCookie;
    std::string userAuth;

    // The user ID. Not to be confused with character IDs.
    std::string userId;

    // Mirror of the in-game G variable
    GameData data;
    // vector<pair<id, username>>
    std::vector<std::pair<std::string, std::string>> characters;
    std::vector<ServerCluster> serverClusters;

    std::vector<std::shared_ptr<Player>> bots;

    void login(std::string& email, std::string& password);
    void collectGameData();
    void collectCharacters();
    void collectServers();
    void validateSession();

    void parseCharacters(nlohmann::json& data);

    void postRequest(std::stringstream& out, HTTPResponse& response, std::string apiEndpoint, std::string arguments,
                     bool auth, const std::vector<CookiePair>& formData = {});

public:
    AdvLandClient(std::string email, std::string password);
    virtual ~AdvLandClient();

    void addPlayer(std::string name, Server& server, PlayerSkeleton& skeleton) {
        for (auto& [id, username] : characters) {
            if (username == name) {
                std::shared_ptr<Player> bot = std::make_shared<Player>(
                    id, server.getIp() + ":" + std::to_string(server.getPort()), *this, skeleton);
                this->bots.push_back(bot);
                return;
            }
        }
    }

    void startBlocking() {
        for (auto& player : bots) {
            player->start();
        }
    }

    ServerCluster* getServerCluster(std::string identifier);
    Server* getServerInCluster(std::string clusterIdentifier, std::string serverIdentifier);

    std::string& getUserId() { return userId; }
    std::string& getAuthToken() { return userAuth; }
};

} // namespace advland

#endif
