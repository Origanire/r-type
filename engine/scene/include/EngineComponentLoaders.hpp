#pragma once

#include "ComponentFactory.hpp"
#include "LayerTable.hpp"

namespace engine::scene {

void addEngineComponentLoaders(ComponentFactory &factory, const LayerTable &layers);

} // namespace engine::scene
