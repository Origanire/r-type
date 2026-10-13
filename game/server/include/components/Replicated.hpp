#pragma once

#include <cstdint>

namespace rtype::server {

struct Replicated
{
    std::uint16_t networkId = 0;
    std::uint8_t kind = 0;
};

} // namespace rtype::server
