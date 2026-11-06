#include "NetworkSettings.hpp"

#include "NetError.hpp"
#include "Protocol.hpp"
#include "RoomInfo.hpp"
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
      m_sessionTimeout(readDuration(network, "sessionTimeoutMs")),
      m_maxClients(static_cast<std::size_t>(readBounded(network, "maxClients", MAX_CLIENT_LIMIT))),
      m_maxRooms(static_cast<std::size_t>(readBounded(network, "maxRooms", MAX_ROOM_LIMIT))),
      m_roomNameMaxLength(static_cast<std::size_t>(readBounded(network, "roomNameMaxLength", MAX_TEXT_LIMIT))),
      m_roomCodeMaxLength(static_cast<std::size_t>(readBounded(network, "roomCodeMaxLength", MAX_TEXT_LIMIT))),
      m_maxJoinAttempts(static_cast<unsigned>(readBounded(network, "maxJoinAttempts", MAX_COUNT))),
      m_worldValueCount(game.has("worldValues") ? game.child("worldValues").items().size() : 0),
      m_playerValueCount(game.has("playerValues") ? game.child("playerValues").items().size() : 0),
      m_ruleCount(game.has("rules") ? game.child("rules").items().size() : 0),
      m_eventSoundCount(game.has("eventSounds") ? game.child("eventSounds").items().size() : 0),
      m_consoleRule(findRule(game))
{
}

std::size_t NetworkSettings::maxClients() const
{
    return m_maxClients;
}

std::size_t NetworkSettings::maxRooms() const
{
    return m_maxRooms;
}

std::size_t NetworkSettings::roomNameMaxLength() const
{
    return m_roomNameMaxLength;
}

std::size_t NetworkSettings::roomCodeMaxLength() const
{
    return m_roomCodeMaxLength;
}

std::size_t NetworkSettings::playerValueCount() const
{
    return m_playerValueCount;
}

std::size_t NetworkSettings::ruleCount() const
{
    return m_ruleCount;
}

std::optional<std::size_t> NetworkSettings::consoleRule() const
{
    return m_consoleRule;
}

std::optional<std::size_t> NetworkSettings::findRule(const scene::DataNode &game)
{
    if (!game.has("consoleRule") || !game.has("rules"))
        return std::nullopt;
    const std::string name = game.getString("consoleRule");
    const auto rules = game.child("rules").items();
    for (std::size_t index = 0; index < rules.size(); ++index) {
        if (rules[index].getString("name") == name)
            return index;
    }
    return std::nullopt;
}

std::size_t NetworkSettings::eventSoundCount() const
{
    return m_eventSoundCount;
}

std::size_t NetworkSettings::worldValueCount() const
{
    return m_worldValueCount;
}

unsigned NetworkSettings::maxJoinAttempts() const
{
    return m_maxJoinAttempts;
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
    if (Protocol::HEADER_SIZE + SnapshotMessage::sizeFor(m_maxPlayers, m_maxEntities, m_worldValueCount, m_playerValueCount) > Protocol::MAX_PACKET_BYTES)
        throw net::NetError("NetworkSettings: " + source + ": maxPlayers and maxEntities do not fit in one packet");
    const std::size_t roomList = 1 + m_maxRooms * RoomInfo::sizeFor(m_roomNameMaxLength);
    if (Protocol::HEADER_SIZE + roomList > Protocol::MAX_PACKET_BYTES)
        throw net::NetError("NetworkSettings: " + source + ": maxRooms and roomNameMaxLength do not fit in one packet");
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
