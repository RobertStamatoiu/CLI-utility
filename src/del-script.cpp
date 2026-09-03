#include "..\include\sys.hpp"
#include "../include/term.hpp"

#include <string>
#include <vector>

namespace
{
    constexpr std::string_view VERSION = "0.1";
}

int main(int argc, const char* argv[])
{
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty() || args[0] == "--help")
    {
        if (args.size() > 1)
            return sys::INVALID_INPUT_ERROR;
        term::println(
            "Usage: del <path> [paths...] [--recursive]\n"
            "Deletes files, empty directories, or directories with --recursive.",
            term::style::info);
        return sys::SUCCESS;
    }
    if (args[0] == "--version")
    {
        if (args.size() != 1) return sys::INVALID_INPUT_ERROR;
        term::println("version: v" + std::string(VERSION), term::style::info);
        return sys::SUCCESS;
    }

    const bool recursive = !args.empty() && args.back() == "--recursive";
    const std::size_t count = args.size() - (recursive ? 1 : 0);
    if (count == 0)
        return sys::INVALID_INPUT_ERROR;

    for (std::size_t i = 0; i < count; ++i)
    {
        const fs::path path = args[i];
        std::error_code error;
        if (!fs::exists(path, error))
        {
            term::println("Path does not exist: " + path.string(), term::style::error);
            return sys::FILE_DOESNT_EXIST;
        }
        const auto removed = recursive ? fs::remove_all(path, error)
                                       : (fs::remove(path, error) ? 1 : 0);
        if (error || removed == 0)
        {
            term::println("Couldn't delete: " + path.string(), term::style::error);
            return error ? sys::DELETE_ERROR : sys::DIRECTORY_NOT_EMPTY;
        }
        term::println("Deleted \"" + path.string() + "\".", term::style::success);
    }
    return sys::SUCCESS;
}
