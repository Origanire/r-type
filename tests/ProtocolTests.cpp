#include <iostream>
#include <random>
#include <stdexcept>

#include "RtypeProtocol.hpp"

using namespace rtype::protocol;

void require(bool value, const char *message)
{
    if (!value)
        throw std::runtime_error(message);
}

int main()
{
    try {
        Packet connect;
        const std::vector<std::uint8_t> expected{0x52, 0x54, 1, 1, 0, 0, 0, 0, 0, 0};
        require(encode(connect) == expected, "CONNECT golden bytes");
        Packet input; input.type = MessageType::Input; input.token = 0x1A2B3C4D; input.sequence = 42; input.buttons = 0x18;
        const std::vector<std::uint8_t> inputBytes{0x52, 0x54, 1, 2, 0, 0, 0, 42, 0, 5, 0x1A, 0x2B, 0x3C, 0x4D, 0x18};
        require(encode(input) == inputBytes, "INPUT golden bytes");
        for (const auto type : {MessageType::Connect, MessageType::Input, MessageType::Heartbeat,
                 MessageType::Disconnect, MessageType::Accept, MessageType::Reject, MessageType::Snapshot,
                 MessageType::HeartbeatAck, MessageType::EntityDestroyed, MessageType::PlayerJoined,
                 MessageType::PlayerKilled, MessageType::PlayerLeft, MessageType::ServerShutdown}) {
            Packet packet; packet.type = type; packet.token = 1; packet.reason = 1; packet.entityId = 1;
            packet.timestamp = 0x12345678; packet.sequence = 0xFFFFFFFE;
            packet.snapshot.entities.push_back({1, 1, -32768, 32767});
            packet.snapshot.players[0] = {1, 3, 1500, 1};
            const auto bytes = encode(packet);
            const auto side = (static_cast<unsigned>(type) & 0x80) ? Receiver::Client : Receiver::Server;
            const auto decoded = decode(bytes, side);
            require(decoded && encode(*decoded) == bytes, "Round trip");
            require(!decode(bytes, side == Receiver::Client ? Receiver::Server : Receiver::Client), "Direction check");
            for (std::size_t n = 0; n < bytes.size(); ++n)
                require(!decode(std::span(bytes).first(n), side), "Truncated packet accepted");
            auto extra = bytes; extra.push_back(0);
            require(!decode(extra, side), "Trailing byte accepted");
        }
        auto broken = inputBytes; broken.back() = 0x80;
        require(!decode(broken, Receiver::Server), "Reserved input bits");
        broken = inputBytes; broken[0] = 0;
        require(!decode(broken, Receiver::Server), "Invalid magic");
        broken = inputBytes; broken[2] = 2;
        require(!decode(broken, Receiver::Server), "Invalid version");
        broken = expected; broken[2] = 2;
        require(decode(broken, Receiver::Server).has_value(), "CONNECT version must reach REJECT handler");
        require(isNewer(0, 0xFFFFFFFF) && !isNewer(0xFFFFFFFF, 0) && !isNewer(5, 5), "Sequence wrap");
        Packet snapshot; snapshot.type = MessageType::Snapshot;
        for (std::uint16_t i = 1; i <= MAX_ENTITIES; ++i)
            snapshot.snapshot.entities.push_back({i, 250, -1, 1});
        require(encode(snapshot).size() == 1098, "Maximum snapshot size");
        require(decode(encode(snapshot), Receiver::Client).has_value(), "Unknown entity kind must be tolerated");
        auto malformedSnapshot = encode(snapshot);
        malformedSnapshot[37 + 10] = 151;
        require(!decode(malformedSnapshot, Receiver::Client), "Incoming entity count limit");
        malformedSnapshot = encode(snapshot);
        malformedSnapshot[38 + 10 + 7] = 0;
        malformedSnapshot[38 + 10 + 8] = 1;
        require(!decode(malformedSnapshot, Receiver::Client), "Incoming duplicate IDs");
        malformedSnapshot = encode(snapshot);
        malformedSnapshot[15] = 1;
        malformedSnapshot[16] = 3;
        malformedSnapshot[21] = 0;
        malformedSnapshot[22] = 200;
        require(!decode(malformedSnapshot, Receiver::Client), "Alive player references missing ship");
        snapshot.snapshot.entities.push_back({151, 1, 0, 0});
        bool rejected = false;
        try { encode(snapshot); } catch (const std::runtime_error &) { rejected = true; }
        require(rejected, "Too many outgoing entities");
        snapshot.snapshot.entities.pop_back();
        snapshot.snapshot.entities[1].id = 1;
        rejected = false;
        try { encode(snapshot); } catch (const std::runtime_error &) { rejected = true; }
        require(rejected, "Duplicate entity IDs");
        std::mt19937 random(42);
        for (unsigned i = 0; i < 10000; ++i) {
            std::vector<std::uint8_t> noise(random() % 1300);
            for (auto &byte : noise) byte = static_cast<std::uint8_t>(random());
            decode(noise, Receiver::Client); decode(noise, Receiver::Server);
        }
        std::cout << "Protocol tests passed\n";
    } catch (const std::exception &error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
