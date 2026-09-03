#include "..\include\sys.hpp"
#include "../include/term.hpp"

#include <algorithm>
#include <string>
#include <vector>

namespace
{
    constexpr std::string_view VERSION = "1.1";

    bool contains(const std::vector<std::string>& values, std::string_view value)
    {
        return std::find(values.begin(), values.end(), value) != values.end();
    }
}

int main(int argc, const char* argv[])
{
    const std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty())
    {
        term::println("Usage: mkproj <name> [--cpp] [--python] [--rust] [--web]",
                      term::style::error);
        return sys::INVALID_INPUT_ERROR;
    }
    if (args[0] == "--version")
    {
        if (args.size() != 1) return sys::INVALID_INPUT_ERROR;
        term::println("version: v" + std::string(VERSION), term::style::info);
        return sys::SUCCESS;
    }
    if (args[0] == "--help")
    {
        if (args.size() != 1) return sys::INVALID_INPUT_ERROR;
        term::println(
            "Usage: mkproj <name> [language flags]\n"
            "Flags: --cpp, --python, --rust, --web",
            term::style::info);
        return sys::SUCCESS;
    }

    const fs::path project = args[0];
    if (fs::exists(project))
    {
        term::println("Project already exists: " + project.string(), term::style::error);
        return sys::FILE_ALREADY_EXISTS;
    }
    std::error_code error;
    for (const auto& directory : {project, project / "src", project / "lib",
                                  project / "test"})
    {
        if (!fs::create_directory(directory, error) && error)
        {
            term::println("Couldn't create " + directory.string(), term::style::error);
            return sys::CREATE_DIRECTORY_ERROR;
        }
    }

    if (contains(args, "--cpp"))
    {
        fs::create_directory(project / "includes", error);
        const std::string cmake =
            "cmake_minimum_required(VERSION 3.20)\n\n"
            "project(MyProject VERSION 1.0)\n\n"
            "set(CMAKE_CXX_STANDARD 20)\n"
            "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n\n"
            "add_executable(MyProject src/main.cpp)\n";
        const std::string main = "#include <iostream>\n\nint main() {\n    std::cout << \"Hello, World!\\n\";\n}\n";
        if (sys::touch(project / "CMakeLists.txt", cmake, true) != sys::SUCCESS ||
            sys::touch(project / "src" / "main.cpp", main, true) != sys::SUCCESS)
            return sys::WRITE_ERROR;
    }
    if (contains(args, "--python"))
    {
        const std::vector<const char*> command = {
            args[0].c_str(), "--basic-preset"};
        const auto result = sys::exec("pysetup", command, {.searchPATH = true});
        if (!sys::succeeded(result.returnCode))
            return result.returnCode;
        const std::string main =
            "def main():\n    print(\"Hello, World!\")\n\n"
            "if __name__ == \"__main__\":\n    main()\n";
        if (sys::touch(project / "src" / "main.py", main, true) != sys::SUCCESS)
            return sys::WRITE_ERROR;
    }
    if (contains(args, "--rust"))
    {
        if (sys::touch(project / "Cargo.toml",
                       "[package]\nname = \"my-project\"\nversion = \"0.1.0\"\nedition = \"2021\"\n",
                       true) != sys::SUCCESS ||
            sys::touch(project / "src" / "main.rs",
                       "fn main() {\n    println!(\"Hello, world!\");\n}\n", true) != sys::SUCCESS)
            return sys::WRITE_ERROR;
    }
    if (contains(args, "--web"))
    {
        if (sys::touch(project / "index.html",
                       "<!doctype html>\n<html lang=\"en\">\n<head>\n"
                       "  <meta charset=\"UTF-8\">\n  <title>My Project</title>\n"
                       "  <link rel=\"stylesheet\" href=\"src/style.css\">\n</head>\n"
                       "<body><script src=\"src/script.js\"></script></body>\n</html>\n",
                       true) != sys::SUCCESS ||
            sys::touch(project / "src" / "script.js", "console.log(\"Hello, World!\");\n", true) != sys::SUCCESS ||
            sys::touch(project / "src" / "style.css", "* { box-sizing: border-box; }\n", true) != sys::SUCCESS)
            return sys::WRITE_ERROR;
    }
    term::println("Successfully created project \"" + project.string() + "\"!",
                  term::style::success);
    return sys::SUCCESS;
}
