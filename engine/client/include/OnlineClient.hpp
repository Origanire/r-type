#pragma once

#include <cstdint>
#include <filesystem>
#include <string>

#include "ClientConnection.hpp"
#include "Endpoint.hpp"
#include "GameView.hpp"
#include "Match.hpp"
#include "NetworkSettings.hpp"
#include "Packet.hpp"
#include "Protocol.hpp"
#include "SnapshotMessage.hpp"

namespace engine::client {

class OnlineClient
{
public:
    OnlineClient(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
        const std::string &game, net::Endpoint server, const network::NetworkSettings &settings);

    void run();

private:
    std::uint8_t buttons() const;
    void handleEvent(const network::Packet &event);
    void applySnapshot(const network::SnapshotMessage &snapshot);
    static ecs::Match::Phase toPhase(std::uint8_t gameState);

    GameView m_view;
    network::ClientConnection m_network;
};

} // namespace engine::client
