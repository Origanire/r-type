#include "OnlineClient.hpp"

#include <cstddef>
#include <utility>

#include "Action.hpp"
#include "Match.hpp"

namespace engine::client {

OnlineClient::OnlineClient(const std::filesystem::path &assetsDirectory,
    const std::filesystem::path &dataDirectory,
    const std::string &game, net::Endpoint server, const network::NetworkSettings &settings)
    : m_view(assetsDirectory, dataDirectory, game), m_network(std::move(server), settings)
{
}

void OnlineClient::run()
{
    while (m_view.isOpen()) {
        m_view.processEvents();
        m_network.update(buttons());
        for (const network::Packet &event : m_network.takeEvents())
            handleEvent(event);
        if (auto snapshot = m_network.takeSnapshot())
            applySnapshot(*snapshot);
        m_view.frame();
    }
    m_view.close();
    m_network.disconnect();
}

std::uint8_t OnlineClient::buttons() const
{
    using input::Action;

    std::uint8_t bits = 0;
    if (m_view.isPressed(Action::Up))
        bits |= network::Protocol::BUTTON_UP;
    if (m_view.isPressed(Action::Down))
        bits |= network::Protocol::BUTTON_DOWN;
    if (m_view.isPressed(Action::Left))
        bits |= network::Protocol::BUTTON_LEFT;
    if (m_view.isPressed(Action::Right))
        bits |= network::Protocol::BUTTON_RIGHT;
    if (m_view.isPressed(Action::Fire))
        bits |= network::Protocol::BUTTON_FIRE;
    return bits;
}

void OnlineClient::handleEvent(const network::Packet &event)
{
    if (event.holds<network::EntityDestroyedMessage>())
        m_view.destroyEntity(event.as<network::EntityDestroyedMessage>().entityId());
}

void OnlineClient::applySnapshot(const network::SnapshotMessage &snapshot)
{
    m_view.beginSync();
    for (const network::EntityState &entity : snapshot.entities())
        m_view.syncEntity(entity.id(), entity.kind(), static_cast<float>(entity.x()), static_cast<float>(entity.y()));
    m_view.endSync();
    m_view.setPhase(toPhase(snapshot.gameState()));
    for (std::size_t slot = 0; slot < snapshot.players().size(); ++slot) {
        const network::PlayerStatus &player = snapshot.players()[slot];
        if (player.state() == network::Protocol::SLOT_EMPTY)
            m_view.removePlayer(slot);
        else
            m_view.setPlayer(slot, player.lives(), player.score(), player.avatarId());
    }
}

ecs::Match::Phase OnlineClient::toPhase(std::uint8_t gameState)
{
    if (gameState == network::Protocol::GAME_PLAYING)
        return ecs::Match::Phase::Playing;
    if (gameState == network::Protocol::GAME_OVER)
        return ecs::Match::Phase::Over;
    return ecs::Match::Phase::Waiting;
}

} // namespace engine::client
