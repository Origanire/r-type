#pragma once

#include <cstdint>
#include <vector>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class RoomUpdateMessage
{
public:
    static constexpr MessageType TYPE = MessageType::RoomUpdate;

    RoomUpdateMessage(std::uint8_t ownerSlot, std::vector<bool> occupied);

    std::uint8_t ownerSlot() const;
    const std::vector<bool> &occupied() const;

    bool isValid(const NetworkSettings &settings) const;
    void write(net::ByteWriter &writer) const;
    static RoomUpdateMessage read(net::ByteReader &reader);

private:
    std::uint8_t m_ownerSlot;
    std::vector<bool> m_occupied;
};

} // namespace engine::network
