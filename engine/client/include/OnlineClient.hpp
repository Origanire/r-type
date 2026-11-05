#pragma once

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "ClientConnection.hpp"
#include "DebugMetrics.hpp"
#include "GameView.hpp"
#include "Match.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "RoomListMessage.hpp"
#include "RoomUpdateMessage.hpp"
#include "SnapshotMessage.hpp"

namespace engine::client {

class OnlineClient
{
public:
    static constexpr std::chrono::seconds ROOM_LIST_INTERVAL{2};
    static constexpr std::chrono::seconds ROOM_SILENCE_LIMIT{4};
    static constexpr const char *MENU_SCENE = "menu";
    static constexpr const char *LOBBY_SCENE = "lobby";
    static constexpr const char *ROOM_SCENE = "room";
    static constexpr const char *GAME_SCENE = "game";
    static constexpr const char *YES = "yes";
    static constexpr const char *NO = "no";
    static constexpr const char *RULE_PREFIX = "rule.";

    OnlineClient(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
        const std::string &game, const network::NetworkSettings &settings, std::optional<std::string> address);

    void run();

private:
    using Time = std::chrono::steady_clock::time_point;

    void updateNetwork(Time now);
    void handleActions();
    void handleAction(const std::string &action);
    void handleEvent(const network::Packet &event, Time now);
    void handleRejection(std::uint8_t reason);
    void createRoom();
    std::vector<std::uint8_t> ruleValues();
    void joinOrAsk(std::uint16_t roomId);
    void openPopup(std::uint16_t roomId);
    void closePopup();
    bool popupOpen();
    void connect();
    void disconnect(const std::string &errorKey);
    void enterScene(const std::string &scene);
    void showRoomList(const network::RoomListMessage &list);
    void showMembers(const network::RoomUpdateMessage &update);
    void applySnapshot(const network::SnapshotMessage &snapshot);
    void showWorld(const network::SnapshotMessage &snapshot);
    void showPlayer(std::size_t slot, const network::PlayerStatus &player);
    static std::string ratio(std::uint8_t percent);
    static std::string phaseName(std::uint8_t gameState);
    std::uint8_t buttons() const;
    std::string field(const std::string &name);
    static bool startsWith(const std::string &text, const std::string &prefix);
    static ecs::Match::Phase toPhase(std::uint8_t gameState);

    GameView m_view;
    network::NetworkSettings m_settings;
    std::unique_ptr<network::ClientConnection> m_network;
    std::string m_address;
    std::optional<std::uint8_t> m_slot;
    std::unordered_set<std::uint16_t> m_lockedRooms;
    std::uint16_t m_pendingRoom;
    Time m_lastRoomList;
    Time m_lastRoomUpdate;
    DebugMetrics m_metrics;
};

} // namespace engine::client
