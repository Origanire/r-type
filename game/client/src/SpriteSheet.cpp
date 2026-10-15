#include "SpriteSheet.hpp"

namespace rtype::client {

engine::render::IntRect SpriteSheet::frame(int column, int row) const
{
    return {column * frameWidth, row * frameHeight, frameWidth, frameHeight};
}

} // namespace rtype::client
