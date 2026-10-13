#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace engine::net {

struct Endpoint
{
    std::string address;
    std::uint16_t port;
    bool operator==(const Endpoint &) const = default;
};

struct Datagram
{
    Endpoint sender;
    std::vector<std::uint8_t> bytes;
};

// One I/O thread, bounded receive/send queues. Asio stays inside the engine.
class UdpTransport
{
public:
    explicit UdpTransport(std::uint16_t port = 0);
    ~UdpTransport();
    UdpTransport(const UdpTransport &) = delete;
    UdpTransport &operator=(const UdpTransport &) = delete;

    void send(const Endpoint &peer, std::vector<std::uint8_t> bytes);
    std::vector<Datagram> poll();
    void checkHealth() const;
    static Endpoint numericEndpoint(const std::string &address, std::uint16_t port);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace engine::net
