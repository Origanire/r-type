#include "Client.hpp"

#include <cstddef>
#include <utility>

#include "Action.hpp"
#include "Match.hpp"

namespace rtype::client {

namespace {

engine::ecs::Match::Phase toPhase(std::uint8_t gameState)
{
    if (gameState == protocol::GAME_PLAYING)
        return engine::ecs::Match::Phase::Playing;
    if (gameState == protocol::GAME_OVER)
        return engine::ecs::Match::Phase::Over;
    return engine::ecs::Match::Phase::Waiting;
}

} // namespace

Client::Client(const std::filesystem::path &assetsDirectory, const std::filesystem::path &dataDirectory,
    const std::string &game, engine::net::Endpoint server)
    : m_view(assetsDirectory, dataDirectory, game), m_network(std::move(server))
{
}

void Client::run()
{
    while (m_view.isOpen()) {
        m_view.processEvents();
        m_network.update(buttons());
        for (const protocol::Packet &event : m_network.takeEvents())
            handleEvent(event);
        if (auto snapshot = m_network.takeSnapshot())
            applySnapshot(*snapshot);
        m_view.frame();
    }
    m_view.close();
    m_network.disconnect();
}

std::uint8_t Client::buttons() const
{
    using engine::input::Action;

    std::uint8_t bits = 0;
    if (m_view.isPressed(Action::Up))
        bits |= protocol::BUTTON_UP;
    if (m_view.isPressed(Action::Down))
        bits |= protocol::BUTTON_DOWN;
    if (m_view.isPressed(Action::Left))
        bits |= protocol::BUTTON_LEFT;
    if (m_view.isPressed(Action::Right))
        bits |= protocol::BUTTON_RIGHT;
    if (m_view.isPressed(Action::Fire))
        bits |= protocol::BUTTON_FIRE;
    return bits;
}

void Client::handleEvent(const protocol::Packet &event)
{
    if (event.type == protocol::MessageType::EntityDestroyed)
        m_view.destroyEntity(event.entityId);
}

void Client::applySnapshot(const protocol::WorldSnapshot &snapshot)
{
    m_view.beginSync();
    for (const protocol::NetworkEntity &entity : snapshot.entities)
        m_view.syncEntity(entity.id, entity.kind, static_cast<float>(entity.x), static_cast<float>(entity.y));
    m_view.endSync();
    m_view.setPhase(toPhase(snapshot.gameState));
    for (std::size_t slot = 0; slot < snapshot.players.size(); ++slot) {
        const protocol::PlayerState &player = snapshot.players[slot];
        if (player.state == protocol::SLOT_EMPTY)
            m_view.removePlayer(slot);
        else
            m_view.setPlayer(slot, player.lives, player.score, player.entityId);
    }
}

} // namespace rtype::client
