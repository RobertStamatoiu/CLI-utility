#include <algorithm>
#include <array>
// #include <cassert>
// #include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstdint>
// #include <exception>
#include <filesystem>
#include <fstream>
// #include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
// #include <map>
// #include <memory>
// #include <optional>
// #include <queue>
#include <ranges>
// #include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
// #include <unordered_map>
#include <unordered_set>
// #include <utility>
// #include <variant>
#include <vector>
#include <process.h>

#include "..\include\sys.hpp"
#include "../include/term.hpp"
#include "..\include\json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

template <typename T>

void append(std::vector<T> &dst, const std::vector<T> &src)
{
    dst.insert(dst.end(), src.begin(), src.end());
}

// packages for the presets

std::vector<const char *> math_preset = {
    "numpy",
    "scipy",
    "simpy",
    "matplotlib",
    "sympy"};
std::vector<const char *> llm_preset = {
    "torch",
    "transformers",
    "datasets",
    "tokenisers",
    "accelerate",
    "safetensors",
    "huggingface-hub"};
std::vector<const char *> visual_preset = {
    "matplotlib",
    "pycame-ce",
    "pillow",
    "opencv-python",
    "imageio",
    "cairosvg",
    "manim"};
std::vector<const char *> dev_tools_preset = {
    "pytest",
    "ruff",
    "mypy",
    "pre-commit",
    "python-dotenv",
    "ipython",
    "rich"};
std::vector<const char *> web_preset = {
    "requests",
    "httpx",
    "beautifulsoup4",
    "fastapi",
    "uvicorn",
    "jinja2",
    "websockets",
    "python-dotenv"};

std::string version = "0.2.2";
namespace help
{
    std::string version = R"(Usage:
        pysetup --version

Description: The "--version" flag will give you the current version of pysetup you have installed

Examples:       * version: v0.1.12
                * version: v0.2.1
                * version: v1.0.2
                * version: v1.0.2 Xmas edition )";

    std::string math_preset = R"(Usage:
        pysetup <directory-name> --math-preset
        
Description: The math-preset is a module bundle containing multiple math-related modules desgined to make your workflow easier to follow

Modules:
                * numpy
                * scitpy
                * simpy
                * matplotlib
                * sympy)";
    std::string llm_preset = R"(Usage:
        pysetup <directory-name> --llm-preset
        
Description: This preset contains a multitude of AI focused modules, skipping you the pain of installing them manually

Modules:
                * torch
                * transformers
                * datasets
                * tokenisers
                * accelerate
                * safetensors
                * huggingface-hub)";
    std::string visual_preset = R"(Usage:
        pysetup <directory-name> --visual-preset
        
Description: The visual preset is designed for poeple working with images, graphs, videos, and other highly visual workflows

Modules:
                * matplotlib
                * pygame-ce
                * pillow
                * manim
                * imageio
                * cairosvg
                * opencv-python)";
    std::string dev_tools_preset = R"(Usage:
        pysetup <directory-name> --dev-tools-preset
        
Description: The dev tools pack is meant primarily for debugging a praticularly nasty program, or for more hands on control over your python program and routine

Modules: 
                * pytest
                * ruff
                * mypy
                * pre-commit
                * python-dotenv
                * ipython
                * rich)";
    std::string web_preset = R"(Usage:
        pysetup <directory-name> --web-preset
        
Description: The web packages is meant for web and browser, work, and can be complemented with javascript

Modules: 
                * requests
                * httpx
                * fastapi
                * unicorv
                * beautifulsoup4
                * jinja2
                * websockets
                * python-dotenv)";
    std::string basic_preset = R"(Usage:
        pysetup <directory-name> --basic-preset
        
Description: The basic preset is the bedrock on which all other presets are build. it doesn't install any modules, but upgrades the critical ones

Modules:
                * pip (uograded)
                * wheel (upgraded)
                * setuptools (upgraded))";
    std::string modules = R"(UsageL
        pysetup <directory-name> --modules <module-names>
        
Description: followed by a list of modules, or just one, it installs those specific modules and their dependencies)";
}

int main(int argc, const char *argv[])
{
    std::vector<std::string> varg(argv + 1, argv + argc);
    if (varg.empty())
    {
        term::println("Usage: pysetup <directory-name> <options>", term::style::error);
        return sys::INVALID_INPUT_ERROR;
    }
    if (varg[0] == "--help")
    {
        if (varg.size() != 1)
        {
            term::println("Usage: pysetup --help", term::style::error);
            return sys::INVALID_INPUT_ERROR;
        }
        std::string message = R"(Usage:
        pysetup <directory-name> <options>

Functionality:
        The `pysetup` command takes as argumends a directory name and various flags and arguments in the scope of setting up that directory for your python workflow.
        It checks to see if a python `venv` already exists under common names such as ".venv", ".env" and so on, validates that it is a usable virtual environment, checks whether the modules we chose are already installed and only installs what is needed, and in the case of failure, deleted the venv created from the command, as to preserve integrity
        
Details:
        <directory-name>: the name of the directory you wish to setup. it can take both relative and absolute paths
        <options>:        various flags that allow you to configure you venv around your work,
                          Examples:     * --basic-preset:          this preset upgrades critical modules and allows for full customisability. WARNING: this is 
                                                                   automatically chosen for all presets 

                                        * --math-preset:           designed for math lovers, it installs a variety of math related modules, including, but not limited 
                                                                   to, numpy, scipy, matplotlib, and many more

                                        * --llm-preset:            this preset installs various AI- and LLM- focused modules, for your workflow with AI

                                        * --visual-preset:         this modules was designed for people working with more visual input and/or output, and installs modules 
                                                                   such as pillow, matplotlib, cairosvg, and more

                                        * --dev-tools-preset:      the dev-tools preset is designed for the people that love more `hands-on` work, and installs modules 
                                                                   such as mypy, python-dotenv, and many more
                                                                     
                                        * --web-preset:            this preset is designed for the people that work in web developement or are simpli pasioned by this. it 
                                                                   installs modules such as requests, httpx, and many others

                                        * --modules:               followed by a list of module names, it will install thos emodules and dependencies automatically, along 
                                                                   with critical module upgrades

                                        * --help:                  this flag can come either as the first argument, in which case will create this help menu, or after 
                                                                   another flag, case in which it will show information and tips about that flags

                                        * --version:               used as the first argument, it will show the current installed version of `pysetup` installed )";
        term::println(message, term::style::info);
        return sys::SUCCESS;
    }
    else if (varg[0] == "--version")
    {
        if (varg.size() > 2)
        {
            term::println("Usage: pysetup --version", term::style::error);
            term::println("Tip: try running pysetup --help or pysetup --version --help", term::style::info);
            return sys::INVALID_INPUT_ERROR;
        }
        else if (varg.size() == 2)
        {
            if (varg[1] == "--help")
            {
                term::println(help::version, term::style::info);
                return sys::SUCCESS;
            }
            else
            {
                term::println("\"" + varg[1] + "\" is not recognised as an option", term::style::error);
                return sys::INVALID_INPUT_ERROR;
            }
        }
        else
        {
            term::println("version: " + version, term::style::info);
            return sys::SUCCESS;
        }
    }
    else if (varg.size() > 1 && varg[1] == "--help")
    {
        // this will handle all cases when there is a flag and then "--help" after
        if (varg.size() != 2)
        {
            term::println("Usage: pysetup <flagg> --help", term::style::error);
            term::println("Tip: try tipying pysetup --help", term::style::info);
            return sys::INVALID_INPUT_ERROR;
        }
        if (varg[0] == "--math-preset")
        {
            term::println(help::math_preset, term::style::info);
            return sys::SUCCESS;
        }
        else if (varg[0] == "--llm-preset")
        {
            term::println(help::llm_preset, term::style::info);
            return sys::SUCCESS;
        }
        else if (varg[0] == "--visual-preset")
        {
            term::println(help::visual_preset, term::style::info);
            return sys::SUCCESS;
        }
        else if (varg[0] == "--dev-tools-preset")
        {
            term::println(help::dev_tools_preset, term::style::info);
            return sys::SUCCESS;
        }
        else if (varg[0] == "--web-preset")
        {
            term::println(help::web_preset, term::style::info);
            return sys::SUCCESS;
        }
        else if (varg[0] == "--basic-preset")
        {
            term::println(help::basic_preset, term::style::info);
            return sys::SUCCESS;
        }
        else if (varg[0] == "--modules")
        {
            term::println(help::modules, term::style::info);
            return sys::SUCCESS;
        }
        else
        {
            term::println("\"" + varg[0] + "\" is not recognised as a flag", term::style::error);
            return sys::INVALID_INPUT_ERROR;
        }
    }
    // lets check to see if the directory is valid and if a venv exists

    fs::path directory = varg[0];
    if (!fs::exists(directory))
    {
        term::println("\"" + directory.string() + "\" is not a valid directory!", term::style::error);
        return sys::DIRECTORY_NOT_FOUND;
    }
    else if (!fs::is_directory(directory))
    {
        term::println("\"" + directory.string() + "\" is not a valid directory!", term::style::error);
        return sys::NOT_DIRECTORY;
    }
    term::println("Checking for a venv...", term::style::info);
    std::array<const char *, 4> envs = {".venv", "venv", ".env", "env"};
    fs::path env;
    for (auto candidate : envs)
    {
        fs::path possible = directory / candidate;
        if (fs::exists(possible))
        {
            if (fs::exists(possible / "Scripts" / "python.exe"))
            {
                env = possible;
                break;
            }
            else
            {
                continue;
            }
        }
        else
        {
            continue;
        }
    }
    if (env.empty())
    {
        term::println("Coudln't find an already existing venv. Creating one...", term::style::info);
        env = directory / ".venv";
        std::string envStr = env.string();
        std::vector<const char *> args = {"-m", "venv", envStr.c_str()};
        auto res = sys::exec("python", args, {.searchPATH = true});
        if (res.returnCode == sys::SUCCESS || res.returnCode - 100 == sys::SUCCESS)
        {
            term::println("Successfully created venv at" + directory.string() + "!", term::style::success);
        }
        else
        {
            term::println("An unexpected error occured while initializing \"python\" executable...", term::style::error);
            term::println("Program returned with return code: " + std::to_string(res.returnCode), term::style::info);
            return res.returnCode;
        }
    }
    else
    {
        term::println("Found viable venv at: " + env.string(), term::style::info);
    }
    term::println("Upgrading critical modules...", term::style::info);
    auto args = {"-m", "pip", "install", "--upgrade", "pip", "wheel", "setuptools"};
    auto res = sys::exec((env / "Scripts" / "python.exe"), args, {.searchPATH = false});
    if (res.returnCode == sys::SUCCESS || res.returnCode - 100 == sys::SUCCESS)
    {
        term::println("Successfully upgraded the following modules: pip wheel setuptools", term::style::success);
    }
    else
    {
        term::println("An unexpected error occured while upgrading modules...", term::style::error);
        term::println("Returned with return code: " + std::to_string(res.returnCode), term::style::info);
        return res.returnCode;
    }
    // now lets see what modules are already installed
    args = {"-m", "pip", "list", "--format=json"};
    res = sys::exec((env / "Scripts" / "python.exe"), args, {.captureStdOut = true, .searchPATH = false});
    std::string json_object;
    if (res.returnCode == sys::SUCCESS || res.returnCode - 100 == sys::SUCCESS)
    {
        json_object = res.StandardOut.value();
    }
    else
    {
        term::println("An unexpected error occured while fwetching installed modules...", term::style::error);
        term::println("Returned with return code: " + std::to_string(res.returnCode), term::style::info);
        return res.returnCode;
    }

    json pip_data = json::parse(json_object);

    std::vector<const char *> packages;
    for (auto arg : varg)
    {
        if (arg == "--math-preset")
            append(packages, math_preset);
        else if (arg == "--llm-preset")
            append(packages, llm_preset);
        else if (arg == "--visual-preset")
            append(packages, visual_preset);
        else if (arg == "--dev-tools-preset")
            append(packages, dev_tools_preset);
        else if (arg == "--web-preset")
            append(packages, web_preset);
        else if (arg == "--modules")
        {
            int index = std::find(varg.begin(), varg.end(), "--modules") - varg.begin();
            for (size_t i = index; i < varg.size(); i++)
            {
                if (!varg[i].starts_with("--"))
                {
                    packages.push_back(varg[i].c_str());
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            continue;
        }
    }

    packages.erase(
        std::remove_if(packages.begin(), packages.end(), [&pip_data](const char *pkg_name)
                       {
            // Căutăm în array-ul JSON dacă există vreun obiect cu acest "name"
            return std::any_of(pip_data.begin(), pip_data.end(), [pkg_name](const json& item) {
                // nlohmann::json știe să compare direct un obiect json cu un const char*
                return item.contains("name") && item["name"] == pkg_name;
            }); }),
        packages.end());
    if (!packages.empty())
    {
        std::vector<const char *> command = {"-m", "pip", "install"};
        command.insert(command.end(), packages.begin(), packages.end());
        auto res = sys::exec(env / "Scripts" / "python.exe", command, {.searchPATH = false});
        if (res.returnCode == sys::SUCCESS || res.returnCode - 100 == sys::SUCCESS)
        {
            term::println("Successfully installed all required modules...", term::style::success);
            return 0;
        }
        else
        {
            term::println("An unexpected error occured while installing final packages...", term::style::error);
            term::println("Returned with return code: " + std::to_string(res.returnCode), term::style::info);
            return res.returnCode;
        }
    }
    else
    {
        term::println("No modules left to install...", term::style::info);
        return 0;
    }
}
