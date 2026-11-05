#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "IGameSession.hpp"

namespace engine::network {

class GameRoom
{
public:
    static constexpr std::size_t OWNER_SLOT = 0;

    GameRoom(std::uint16_t id, std::string name, std::string code, std::size_t capacity,
        std::unique_ptr<IGameSession> session);

    std::uint16_t id() const;
    const std::string &name() const;
    bool locked() const;
    bool acceptsCode(const std::string &code) const;
    bool playing() const;
    void setPlaying(bool playing);
    IGameSession &session();

    std::size_t capacity() const;
    std::size_t playerCount() const;
    std::optional<std::size_t> freeSlot() const;
    void seat(std::size_t slot, std::size_t client);
    void unseat(std::size_t slot);
    std::optional<std::size_t> clientAt(std::size_t slot) const;
    std::optional<std::size_t> slotOf(std::size_t client) const;
    std::vector<bool> occupiedSlots() const;
    std::vector<std::size_t> clients() const;

    std::size_t owner() const;

private:
    std::uint16_t m_id;
    std::string m_name;
    std::string m_code;
    bool m_playing;
    std::unique_ptr<IGameSession> m_session;
    std::vector<std::optional<std::size_t>> m_seats;
};

} // namespace engine::network
