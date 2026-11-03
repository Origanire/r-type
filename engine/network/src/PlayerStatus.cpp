#include "PlayerStatus.hpp"

#include "Protocol.hpp"

namespace engine::network {

PlayerStatus::PlayerStatus() : m_state(Protocol::SLOT_EMPTY), m_lives(0), m_score(0), m_avatarId(0)
{
}

PlayerStatus::PlayerStatus(std::uint8_t state, std::uint8_t lives, std::uint32_t score, std::uint16_t avatarId)
    : m_state(state), m_lives(lives), m_score(score), m_avatarId(avatarId)
{
}

std::uint8_t PlayerStatus::state() const
{
    return m_state;
}

std::uint8_t PlayerStatus::lives() const
{
    return m_lives;
}

std::uint32_t PlayerStatus::score() const
{
    return m_score;
}

std::uint16_t PlayerStatus::avatarId() const
{
    return m_avatarId;
}

void PlayerStatus::write(engine::net::ByteWriter &writer) const
{
    writer.writeU8(m_state);
    writer.writeU8(m_lives);
    writer.writeU32(m_score);
    writer.writeU16(m_avatarId);
}

PlayerStatus PlayerStatus::read(engine::net::ByteReader &reader)
{
    const std::uint8_t state = reader.readU8();
    const std::uint8_t lives = reader.readU8();
    const std::uint32_t score = reader.readU32();
    const std::uint16_t avatarId = reader.readU16();
    return PlayerStatus(state, lives, score, avatarId);
}

} // namespace engine::network
