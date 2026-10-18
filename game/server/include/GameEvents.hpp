#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "RtypeProtocol.hpp"

namespace rtype::server {

class GameEvents
{
public:
    void entityDestroyed(std::uint16_t networkId);
    void playerKilled(std::size_t slot);
    std::vector<protocol::Packet> take();

private:
    std::vector<protocol::Packet> m_events;
};

} // namespace rtype::server
