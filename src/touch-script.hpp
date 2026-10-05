#pragma once

#include "..\include\sys.hpp"
#include "../include/term.hpp"

#include <string>
#include <vector>

namespace
{
    constexpr std::string_view TOUCH_VERSION = "0.4";
}

int TouchCLI(std::vector<std::string> args)
{
    if (args.empty() || args.size() > 3)
    {
        term::println(
            "Usage: touch <file-name> [file-contents] [--overwrite]",
            term::style::error);
        return sys::INVALID_INPUT_ERROR;
    }
    if (args[0] == "--help")
    {
        if (args.size() != 1)
            return sys::INVALID_INPUT_ERROR;
        term::println(
            "Usage: touch <file-name> [file-contents] [--overwrite]\n"
            "Creates a file, appends contents by default, or replaces them with --overwrite.",
            term::style::info);
        return sys::SUCCESS;
    }
    if (args[0] == "--version")
    {
        if (args.size() != 1)
            return sys::INVALID_INPUT_ERROR;
        term::println("version: v" + std::string(TOUCH_VERSION), term::style::info);
        return sys::SUCCESS;
    }

    const bool overwrite = args.size() == 3 && args[2] == "--overwrite";
    if (args.size() == 3 && !overwrite)
    {
        term::println("Unrecognised flag: " + args[2], term::style::error);
        return sys::INVALID_INPUT_ERROR;
    }
    const std::string contents = args.size() >= 2 ? args[1] : "";
    const int result = sys::touch(args[0], contents, overwrite);
    if (result != sys::SUCCESS)
    {
        term::println("Couldn't create file \"" + args[0] + "\".", term::style::error);
        term::println("Returned with exit code: " + std::to_string(result), term::style::info);
        return result;
    }
    term::println("Successfully created file \"" + args[0] + "\"!", term::style::success);
    return sys::SUCCESS;
}
