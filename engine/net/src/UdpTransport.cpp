#include "UdpTransport.hpp"

#include <array>
#include <atomic>
#include <chrono>
#include <exception>
#include <mutex>
#include <string>
#include <thread>
#include <utility>

#include <asio.hpp>

#include "NetError.hpp"

namespace engine::net {

namespace {

constexpr std::size_t MAX_QUEUE = 256;
constexpr std::chrono::milliseconds FLUSH_TIMEOUT{200};

} // namespace

struct UdpTransport::Impl
{
    asio::io_context io;
    asio::ip::udp::socket socket;
    std::array<std::uint8_t, MAX_DATAGRAM_SIZE + 1> buffer{};
    asio::ip::udp::endpoint sender;
    std::mutex mutex;
    std::vector<Datagram> incoming;
    std::exception_ptr failure;
    std::atomic_size_t pending{0};
    std::thread worker;

    explicit Impl(std::uint16_t port) : socket(io, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
    {
        receive();
        worker = std::thread([this] { runIo(); });
    }

    ~Impl()
    {
        const auto deadline = std::chrono::steady_clock::now() + FLUSH_TIMEOUT;
        while (pending.load() > 0 && std::chrono::steady_clock::now() < deadline)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        io.stop();
        if (worker.joinable())
            worker.join();
    }

    void runIo()
    {
        try {
            io.run();
        } catch (const std::exception &error) {
            std::lock_guard lock(mutex);
            failure = std::make_exception_ptr(NetError(std::string("UdpTransport: ") + error.what()));
        } catch (...) {
            std::lock_guard lock(mutex);
            failure = std::make_exception_ptr(NetError("UdpTransport: unknown network failure"));
        }
    }

    void receive()
    {
        socket.async_receive_from(asio::buffer(buffer), sender, [this](const asio::error_code &error, std::size_t size) {
            if (!error && size <= MAX_DATAGRAM_SIZE)
                store(size);
            if (error != asio::error::operation_aborted)
                receive();
        });
    }

    void store(std::size_t size)
    {
        std::lock_guard lock(mutex);
        if (incoming.size() >= MAX_QUEUE)
            return;
        Endpoint from{sender.address().to_string(), sender.port()};
        incoming.push_back({std::move(from), std::vector<std::uint8_t>(buffer.begin(), buffer.begin() + size)});
    }
};

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
    return {ip.to_string(), port};
}

void UdpTransport::send(const Endpoint &peer, std::vector<std::uint8_t> bytes)
{
    checkHealth();
    if (bytes.size() > MAX_DATAGRAM_SIZE)
        throw NetError("UdpTransport: datagram exceeds the maximum size");
    const asio::ip::udp::endpoint destination(asio::ip::make_address_v4(peer.address), peer.port);
    if (m_impl->pending.fetch_add(1) >= MAX_QUEUE) {
        m_impl->pending.fetch_sub(1);
        return;
    }
    auto data = std::make_shared<std::vector<std::uint8_t>>(std::move(bytes));
    Impl *impl = m_impl.get();
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
