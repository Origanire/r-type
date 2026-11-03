#include "NetworkSettings.hpp"

#include "NetError.hpp"
#include "Protocol.hpp"
#include "SnapshotMessage.hpp"

namespace engine::network {

NetworkSettings NetworkSettings::fromGame(const scene::DataNode &game)
{
    const NetworkSettings settings(game, game.child("network"));
    settings.checkPacketFits(game.path());
    return settings;
}

NetworkSettings::NetworkSettings(const scene::DataNode &game, const scene::DataNode &network)
    : m_defaultPort(static_cast<std::uint16_t>(readBounded(network, "defaultPort", MAX_PORT))),
      m_maxPlayers(static_cast<std::size_t>(readBounded(network, "maxPlayers", MAX_PLAYER_LIMIT))),
      m_maxEntities(static_cast<std::size_t>(readBounded(network, "maxEntities", MAX_ENTITY_LIMIT))),
      m_tickRate(readBounded(game, "tickRate", MAX_COUNT)),
      m_snapshotEveryTicks(static_cast<std::uint32_t>(readBounded(network, "snapshotEveryTicks", MAX_COUNT))),
      m_inputInterval(readDuration(network, "inputIntervalMs")),
      m_inputReleaseDelay(readDuration(network, "inputReleaseDelayMs")),
      m_heartbeatInterval(readDuration(network, "heartbeatIntervalMs")),
      m_connectRetryInterval(readDuration(network, "connectRetryIntervalMs")),
      m_connectAttempts(static_cast<unsigned>(readBounded(network, "connectAttempts", MAX_COUNT))),
      m_sessionTimeout(readDuration(network, "sessionTimeoutMs"))
{
}

std::uint16_t NetworkSettings::defaultPort() const
{
    return m_defaultPort;
}

std::size_t NetworkSettings::maxPlayers() const
{
    return m_maxPlayers;
}

std::size_t NetworkSettings::maxEntities() const
{
    return m_maxEntities;
}

int NetworkSettings::tickRate() const
{
    return m_tickRate;
}

std::uint32_t NetworkSettings::snapshotEveryTicks() const
{
    return m_snapshotEveryTicks;
}

std::chrono::milliseconds NetworkSettings::inputInterval() const
{
    return m_inputInterval;
}

std::chrono::milliseconds NetworkSettings::inputReleaseDelay() const
{
    return m_inputReleaseDelay;
}

std::chrono::milliseconds NetworkSettings::heartbeatInterval() const
{
    return m_heartbeatInterval;
}

std::chrono::milliseconds NetworkSettings::connectRetryInterval() const
{
    return m_connectRetryInterval;
}

unsigned NetworkSettings::connectAttempts() const
{
    return m_connectAttempts;
}

std::chrono::milliseconds NetworkSettings::sessionTimeout() const
{
    return m_sessionTimeout;
}

void NetworkSettings::checkPacketFits(const std::string &source) const
{
    if (Protocol::HEADER_SIZE + SnapshotMessage::sizeFor(m_maxPlayers, m_maxEntities) > Protocol::MAX_PACKET_BYTES)
        throw net::NetError("NetworkSettings: " + source + ": maxPlayers and maxEntities do not fit in one packet");
}

int NetworkSettings::readBounded(const scene::DataNode &node, const std::string &key, int maximum)
{
    const int value = node.getInt(key);
    if (value <= 0 || value > maximum)
        throw net::NetError("NetworkSettings: " + node.path() + "." + key + " must be between 1 and "
            + std::to_string(maximum));
    return value;
}

std::chrono::milliseconds NetworkSettings::readDuration(const scene::DataNode &node, const std::string &key)
{
    return std::chrono::milliseconds(readBounded(node, key, MAX_COUNT));
}

} // namespace engine::network
