#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <string>

#include "DataNode.hpp"

namespace engine::network {

class NetworkSettings
{
public:
    static constexpr int MAX_PORT = 65535;
    static constexpr int MAX_PLAYER_LIMIT = 255;
    static constexpr int MAX_ENTITY_LIMIT = 255;
    static constexpr int MAX_COUNT = 1000000;

    static NetworkSettings fromGame(const scene::DataNode &game);

    std::uint16_t defaultPort() const;
    std::size_t maxPlayers() const;
    std::size_t maxEntities() const;
    int tickRate() const;
    std::uint32_t snapshotEveryTicks() const;
    std::chrono::milliseconds inputInterval() const;
    std::chrono::milliseconds inputReleaseDelay() const;
    std::chrono::milliseconds heartbeatInterval() const;
    std::chrono::milliseconds connectRetryInterval() const;
    unsigned connectAttempts() const;
    std::chrono::milliseconds sessionTimeout() const;

private:
    NetworkSettings(const scene::DataNode &game, const scene::DataNode &network);

    void checkPacketFits(const std::string &source) const;
    static int readBounded(const scene::DataNode &node, const std::string &key, int maximum);
    static std::chrono::milliseconds readDuration(const scene::DataNode &node, const std::string &key);

    std::uint16_t m_defaultPort;
    std::size_t m_maxPlayers;
    std::size_t m_maxEntities;
    int m_tickRate;
    std::uint32_t m_snapshotEveryTicks;
    std::chrono::milliseconds m_inputInterval;
    std::chrono::milliseconds m_inputReleaseDelay;
    std::chrono::milliseconds m_heartbeatInterval;
    std::chrono::milliseconds m_connectRetryInterval;
    unsigned m_connectAttempts;
    std::chrono::milliseconds m_sessionTimeout;
};

} // namespace engine::network
