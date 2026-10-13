#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace engine::net {

class ByteWriter
{
public:
    explicit ByteWriter(std::size_t maxSize);

    void writeU8(std::uint8_t value);
    void writeU16(std::uint16_t value);
    void writeU32(std::uint32_t value);
    void writeI16(std::int16_t value);
    void patchU16(std::size_t offset, std::uint16_t value);

    std::size_t size() const;
    std::vector<std::uint8_t> release();

private:
    void writeBigEndian(std::uint32_t value, std::size_t byteCount);

    std::size_t m_maxSize;
    std::vector<std::uint8_t> m_bytes;
};

} // namespace engine::net
