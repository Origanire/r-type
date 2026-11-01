#include "AssetPath.hpp"

namespace rtype {

namespace {

std::filesystem::path findDirectory(
    const char *programPath, const std::filesystem::path &besideName, const std::filesystem::path &sourcePath)
{
    std::filesystem::path besideProgram = std::filesystem::absolute(programPath).parent_path() / besideName;
    if (std::filesystem::exists(besideProgram))
        return besideProgram;
    return std::filesystem::current_path() / sourcePath;
}

} // namespace

std::filesystem::path findAssets(const char *programPath)
{
    return findDirectory(programPath, "assets", "assets");
}

std::filesystem::path findData(const char *programPath)
{
    return findDirectory(programPath, "data", std::filesystem::path("game") / "data");
}

} // namespace rtype
