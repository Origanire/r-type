#pragma once

#include "ByteReader.hpp"
#include "ByteWriter.hpp"
#include "NetworkSettings.hpp"
#include "Protocol.hpp"

namespace engine::network {

class ConnectMessage
{
public:
    static constexpr MessageType TYPE = MessageType::Connect;

    bool isValid(const NetworkSettings &settings) const;
    void write(engine::net::ByteWriter &writer) const;
    static ConnectMessage read(engine::net::ByteReader &reader);
};

} // namespace engine::network
