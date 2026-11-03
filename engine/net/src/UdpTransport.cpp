#include "UdpTransport.hpp"

#include <utility>

#include "NetError.hpp"
#include "UdpTransportImpl.hpp"

namespace engine::net {

UdpTransport::UdpTransport(std::uint16_t port)
{
    try {
        m_impl = std::make_unique<Impl>(port);
    } catch (const asio::system_error &error) {
        throw NetError("UdpTransport: cannot open UDP port " + std::to_string(port) + ": " + error.what());
    }
}

UdpTransport::~UdpTransport() = default;

Endpoint UdpTransport::numericEndpoint(const std::string &address, std::uint16_t port)
{
    asio::error_code error;
    const auto ip = asio::ip::make_address_v4(address, error);
    if (error)
        throw NetError("UdpTransport: invalid IPv4 address " + address);
    return Endpoint(ip.to_string(), port);
}

void UdpTransport::send(const Endpoint &peer, std::vector<std::uint8_t> bytes)
{
    checkHealth();
    if (bytes.size() > MAX_DATAGRAM_SIZE)
        throw NetError("UdpTransport: datagram exceeds the maximum size");
    m_impl->send(peer, std::move(bytes));
}

std::vector<Datagram> UdpTransport::poll()
{
    checkHealth();
    return m_impl->takeIncoming();
}

void UdpTransport::checkHealth() const
{
    m_impl->checkHealth();
}

} // namespace engine::net
