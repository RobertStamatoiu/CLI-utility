#include <string>
#include <vector>

#include "./src/mkproj-script.hpp"
#include "./src/pysetup-script.hpp"
#include "./src/touch-script.hpp"

#include "./include/term.hpp"
#include "./include/sys.hpp"

int main(int argc, const char* argv[]){
    if(argc < 2){
        term::println("Usage: dev <command> [args...]", term::style::error);
        return sys::GENERIC_ERROR;
    } else if (argc == 2 && (argv[1] == "--help" || argv[1] == "-h" || argv[1] == "-?")){
        std::string helpMenu = R"(Usage: dev <command> [args...]
Commands:
        * mkproj ProjectName [template] --- creates a full projects with folders like src/, include/ and test/ and can be templated for specific langauges
        * pysetup ProjectName [template?] [modules?] --- sets up a fully python environment alongside preinstalled modules bundles in templates
        * touch FileName [FileContents?] [--override?] --- creates a file or appends to a file with the given name the FileContents, or overwrites previous contents
    Tip: you can run dev <command> --help for more detailed help and guides on their uses
Others:
        * --version: shows the current installed version of your dev
        * --help / -h / -?: shows this help message)";
        term::println(helpMenu, term::style::info);
        return sys::SUCCESS;
    } else {
        std::vector<std::string> varg(argv + 2, argv + argc);
        if(argv[1] == "pysetup"){
            return PysetupCLI(varg);
        } else if (varg[1] == "mkproj"){
            return MkprojCLI(varg);
        } else if (varg[1] == "touch"){
            return TouchCLI(varg);
        } else {
            term::println("Unrecognised command: \"" + std::string(varg[1]) + "\"! Try dev --help", term::style::error);
            return sys::INVALID_INPUT_ERROR;
        }
    }
    

}