#include "Packet.hpp"

namespace engine::network {

MessageType Packet::type() const
{
    return m_type;
}

std::uint8_t Packet::version() const
{
    return m_version;
}

std::uint32_t Packet::sequence() const
{
    return m_sequence;
}

void Packet::setSequence(std::uint32_t sequence)
{
    m_sequence = sequence;
}

} // namespace engine::network
