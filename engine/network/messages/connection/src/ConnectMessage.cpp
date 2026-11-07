#include "ConnectMessage.hpp"

namespace engine::network {

bool ConnectMessage::isValid(const NetworkSettings &) const
{
    return true;
}

void ConnectMessage::write(engine::net::ByteWriter &) const
{
}

ConnectMessage ConnectMessage::read(engine::net::ByteReader &)
{
    return ConnectMessage();
}

} // namespace engine::network
