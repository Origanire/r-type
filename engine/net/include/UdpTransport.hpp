#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Datagram.hpp"
#include "Endpoint.hpp"

namespace engine::net {

class UdpTransport
{
public:
    static constexpr std::size_t MAX_DATAGRAM_SIZE = 1200;

    explicit UdpTransport(std::uint16_t port = 0);
    ~UdpTransport();
    UdpTransport(const UdpTransport &) = delete;
    UdpTransport &operator=(const UdpTransport &) = delete;

    void send(const Endpoint &peer, std::vector<std::uint8_t> bytes);
    std::vector<Datagram> poll();
    void checkHealth() const;
    static Endpoint numericEndpoint(const std::string &address, std::uint16_t port);

private:
    class Impl;

    std::unique_ptr<Impl> m_impl;
};

} // namespace engine::net
