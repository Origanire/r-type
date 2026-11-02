#pragma once

#include <string>
#include <vector>

#include "Color.hpp"
#include "DataNode.hpp"
#include "IntRect.hpp"

namespace engine::scene {

IntRect readRect(const DataNode &node, const std::string &key);
Color readColor(const DataNode &values);
Color readColor(const DataNode &node, const std::string &key);
std::vector<Color> readColors(const DataNode &node, const std::string &key);

} // namespace engine::scene
