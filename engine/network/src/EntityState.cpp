#include "EntityState.hpp"

namespace engine::network {

EntityState::EntityState(std::uint16_t id, std::uint8_t kind, std::int16_t x, std::int16_t y)
    : m_id(id), m_kind(kind), m_x(x), m_y(y)
{
}

std::uint16_t EntityState::id() const
{
    return m_id;
}

std::uint8_t EntityState::kind() const
{
    return m_kind;
}

std::int16_t EntityState::x() const
{
    return m_x;
}

std::int16_t EntityState::y() const
{
    return m_y;
}

void EntityState::write(engine::net::ByteWriter &writer) const
{
    writer.writeU16(m_id);
    writer.writeU8(m_kind);
    writer.writeI16(m_x);
    writer.writeI16(m_y);
}

EntityState EntityState::read(engine::net::ByteReader &reader)
{
    const std::uint16_t id = reader.readU16();
    const std::uint8_t kind = reader.readU8();
    const std::int16_t x = reader.readI16();
    const std::int16_t y = reader.readI16();
    return EntityState(id, kind, x, y);
}

} // namespace engine::network
