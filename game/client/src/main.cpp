#include <exception>
#include <filesystem>
#include <iostream>

#include "Client.hpp"
#include "EngineError.hpp"

namespace {

std::filesystem::path findAssets(const char *programPath)
{
    std::filesystem::path besideProgram = std::filesystem::absolute(programPath).parent_path() / "assets";
    if (std::filesystem::exists(besideProgram))
        return besideProgram;
    return std::filesystem::current_path() / "assets";
}

} // namespace

int main(int, char **argv)
{
    try {
        rtype::client::Client client(findAssets(argv[0]));
        client.run();
    } catch (const engine::EngineError &error) {
        std::cerr << "r-type_client: " << error.what() << std::endl;
        return 84;
    } catch (const std::exception &error) {
        std::cerr << "r-type_client: unexpected error: " << error.what() << std::endl;
        return 84;
    }
    return 0;
}
