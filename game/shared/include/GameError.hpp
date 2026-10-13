#pragma once

#include "EngineError.hpp"

namespace rtype {

class GameError : public engine::EngineError
{
public:
    using engine::EngineError::EngineError;
};

} // namespace rtype
