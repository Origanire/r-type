#pragma once

#include <filesystem>

namespace rtype {

std::filesystem::path findAssets(const char *programPath);
std::filesystem::path findData(const char *programPath);

} // namespace rtype
