#ifndef LUNARWATCHER_ADVLAND
#define LUNARWATCHER_ADVLAND

#include <cpr/cpr.h>
#include <cpr/session.h>

#include "nlohmann/json.hpp"

#include "meta/Typedefs.hpp"
#include "movement/MapProcessing.hpp"
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
class Player;
class PlayerSkeleton;
class AdvLandClient {
private:
    static auto inline const mLogger = spdlog::stdout_color_mt("AdvLandClient");
    // This is the session cookie. It's used with some calls, and is essential for base
    // post-login auth operations.
    std::string sessionCookie;
    std::string userAuth;

    // The user ID. Not to be confused with character IDs.
    std::string userId;
    static bool running;

    // Mirror of the in-game G variable
    GameData data;
    // vector<pair<id, username>>
    std::vector<std::pair<std::string, std::string>> characters;
    std::vector<ServerCluster> serverClusters;

    std::vector<std::shared_ptr<Player>> bots;
    MapProcessor mapProcessor;

    std::thread runner;

    void login(const std::string& email, const std::string& password);
    void collectGameData();
    void collectCharacters();
    void collectServers();
    void validateSession();

    void parseCharacters(nlohmann::json& data);

    cpr::Response postRequest(std::string apiEndpoint, std::string arguments, bool auth,
                              const cpr::Payload& formData = {});

    void processInternals();
    void construct(const nlohmann::json& email, const nlohmann::json& password);

public:
    AdvLandClient();
    AdvLandClient(const std::string& credentialFileLocation);
    AdvLandClient(const nlohmann::json& email, const nlohmann::json& password);
    virtual ~AdvLandClient();

    void addPlayer(const std::string& name, Server& server, PlayerSkeleton& skeleton);

    /**
     * Starts the bot in blocking mode. What this implies for you as the developer is that you don't need to manually
     * keep the main thread alive. Note that any code placed after this function will not be called until the bot shuts
     * down
     */
    void startBlocking();
    /**
     * Starts the bot async. If you use this, make sure you keep the main thread busy. A while loop with an execution
     * timeout and a decent exit condition is usually enough.
     */
    void startAsync();

    std::optional<ServerCluster> getServerCluster(std::string identifier);
    std::optional<Server> getServerInCluster(std::string clusterIdentifier, std::string serverIdentifier);

    std::string& getUserId() { return userId; }
    std::string& getAuthToken() { return userAuth; }
    GameData& getData() { return data; }
    MapProcessor& getMapProcessor() { return mapProcessor; }
    bool isLocalPlayer(std::string username);
    void dispatchLocalCm(std::string to, const nlohmann::json& message, std::string from);
    /**
     * This kills all the bot connections, as well as threads created by the client and players.
     * This does NOT kill threads created by the developer in a PlayerSkeleton.
     *
     * Note that calling this incapacitates the client
     */
    void kill();

    /**
     * This method reflects the current run state. If this is true, threads can continue.
     * It's advised to use this as a the condition for blocking while-loops in threads.
     *
     *
     */
    static bool canRun();
};
template <typename... Ts> cpr::Response Get(Ts&&... ts) {
    cpr::Session session;
    cpr::priv::set_option(session, CPR_FWD(ts)...);
#ifdef _WIN32
    // TODO: Figure out Windows cert verification
    session.SetVerifySsl(false);
#endif
    
    return session.Get();
}

template <typename... Ts> cpr::Response Post(Ts&&... ts) {
    cpr::Session session;
    cpr::priv::set_option(session, CPR_FWD(ts)...);
#ifdef _WIN32
    session.SetVerifySsl(false);
#endif
    return session.Post();
}


} // namespace advland

#endif
