#include "UdpTransport.hpp"

#include <array>
#include <atomic>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

#include <asio.hpp>

namespace engine::net {

struct UdpTransport::Impl
{
    static constexpr std::size_t MAX_QUEUE = 256;
    asio::io_context io;
    asio::ip::udp::socket socket;
    std::array<std::uint8_t, 1201> buffer{};
    asio::ip::udp::endpoint sender;
    std::mutex mutex;
    std::vector<Datagram> incoming;
    std::exception_ptr failure;
    std::atomic_size_t pending{0};
    std::thread worker;

    explicit Impl(std::uint16_t port) : socket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
    {
        receive();
        worker = std::thread([this] {
            try {
                io.run();
            } catch (...) {
                std::lock_guard lock(mutex);
                failure = std::current_exception();
            }
        });
    }

    ~Impl()
    {
        io.stop();
        if (worker.joinable())
            worker.join();
    }

    void receive()
    {
        socket.async_receive_from(asio::buffer(buffer), sender, [this](const asio::error_code &error, std::size_t size) {
            if (!error && size <= 1200) {
                std::lock_guard lock(mutex);
                if (incoming.size() < MAX_QUEUE)
                    incoming.push_back({{sender.address().to_string(), sender.port()},
                        std::vector<std::uint8_t>(buffer.begin(), buffer.begin() + size)});
            }
            if (error != asio::error::operation_aborted)
                receive();
        });
    }
};

UdpTransport::UdpTransport(std::uint16_t port) : m_impl(std::make_unique<Impl>(port)) {}
UdpTransport::~UdpTransport() = default;

Endpoint UdpTransport::numericEndpoint(const std::string &address, std::uint16_t port)
{
    const auto ip = asio::ip::make_address_v4(address);
    return {ip.to_string(), port};
}

void UdpTransport::send(const Endpoint &peer, std::vector<std::uint8_t> bytes)
{
    checkHealth();
    if (bytes.size() > 1200)
        throw std::runtime_error("UDP datagram exceeds 1200 bytes");
    const auto destination = asio::ip::udp::endpoint(asio::ip::make_address_v4(peer.address), peer.port);
    if (m_impl->pending.fetch_add(1) >= Impl::MAX_QUEUE) {
        m_impl->pending.fetch_sub(1);
        return;
    }
    auto data = std::make_shared<std::vector<std::uint8_t>>(std::move(bytes));
    auto *impl = m_impl.get();
    asio::post(impl->io, [impl, destination, data] {
        impl->socket.async_send_to(asio::buffer(*data), destination,
            [impl, data](const asio::error_code &, std::size_t) { impl->pending.fetch_sub(1); });
    });
}

std::vector<Datagram> UdpTransport::poll()
{
    checkHealth();
    std::lock_guard lock(m_impl->mutex);
    std::vector<Datagram> result;
    result.swap(m_impl->incoming);
    return result;
}

void UdpTransport::checkHealth() const
{
    std::lock_guard lock(m_impl->mutex);
    if (m_impl->failure)
        std::rethrow_exception(m_impl->failure);
}

} // namespace engine::net
