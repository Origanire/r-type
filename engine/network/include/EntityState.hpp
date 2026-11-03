#pragma once

#include <cstddef>
#include <cstdint>

#include "ByteReader.hpp"
#include "ByteWriter.hpp"

namespace engine::network {

class EntityState
{
public:
    static constexpr std::size_t SIZE = 7;

    EntityState(std::uint16_t id, std::uint8_t kind, std::int16_t x, std::int16_t y);

    std::uint16_t id() const;
    std::uint8_t kind() const;
    std::int16_t x() const;
    std::int16_t y() const;

    void write(engine::net::ByteWriter &writer) const;
    static EntityState read(engine::net::ByteReader &reader);

private:
    std::uint16_t m_id;
    std::uint8_t m_kind;
    std::int16_t m_x;
    std::int16_t m_y;
};

} // namespace engine::network
