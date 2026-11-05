#include "RoomUpdateMessage.hpp"

#include <cstddef>
#include <utility>

namespace engine::network {

RoomUpdateMessage::RoomUpdateMessage(std::uint8_t ownerSlot, std::vector<bool> occupied)
    : m_ownerSlot(ownerSlot), m_occupied(std::move(occupied))
{
}

std::uint8_t RoomUpdateMessage::ownerSlot() const
{
    return m_ownerSlot;
}

const std::vector<bool> &RoomUpdateMessage::occupied() const
{
    return m_occupied;
}

bool RoomUpdateMessage::isValid(const NetworkSettings &settings) const
{
    return m_occupied.size() == settings.maxPlayers() && m_ownerSlot < m_occupied.size() && m_occupied[m_ownerSlot];
}

void RoomUpdateMessage::write(net::ByteWriter &writer) const
{
    writer.writeU8(m_ownerSlot);
    writer.writeU8(static_cast<std::uint8_t>(m_occupied.size()));
    for (bool occupied : m_occupied)
        writer.writeU8(occupied ? 1 : 0);
}

RoomUpdateMessage RoomUpdateMessage::read(net::ByteReader &reader)
{
    const std::uint8_t ownerSlot = reader.readU8();
    const std::size_t slotCount = reader.readU8();
    std::vector<bool> occupied;
    occupied.reserve(slotCount);
    for (std::size_t i = 0; i < slotCount; ++i)
        occupied.push_back(reader.readU8() != 0);
    return RoomUpdateMessage(ownerSlot, std::move(occupied));
}

} // namespace engine::network
