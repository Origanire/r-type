#pragma once

#include <cstddef>
#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"

namespace engine::network {

class PlayerStatus
{
public:
    static constexpr std::size_t SIZE = 8;

    PlayerStatus();
    PlayerStatus(std::uint8_t state, std::uint8_t lives, std::uint32_t score, std::uint16_t avatarId);

    std::uint8_t state() const;
    std::uint8_t lives() const;
    std::uint32_t score() const;
    std::uint16_t avatarId() const;

    void write(engine::net::ByteWriter &writer) const;
    static PlayerStatus read(engine::net::ByteReader &reader);

private:
    std::uint8_t m_state;
    std::uint8_t m_lives;
    std::uint32_t m_score;
    std::uint16_t m_avatarId;
};

} // namespace engine::network
