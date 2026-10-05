# CLI

`CLI` is a Windows command-line toolkit written in C++20. Its commands are
compiled together into one executable, `dev.exe`.

## Project structure

```text
CLI/
├── archive/                 Historical/local executable archives (ignored by Git)
├── bin/                     Build output and local executables (ignored by Git)
├── include/
│   ├── json.hpp             nlohmann JSON single-header dependency
│   ├── sys.hpp              Windows filesystem and process helpers
│   └── term.hpp             Terminal output, colors, and styles
├── src/
│   ├── mkproj-script.hpp    Project scaffolding command
│   ├── pysetup-script.hpp   Python environment setup command
│   └── touch-script.hpp     File creation and editing command
├── dev.cpp                 Program entry point and command dispatcher
├── build.bat               Build using g++ from Command Prompt
├── build.ps1               Build using g++ from PowerShell
├── .gitignore
└── README.md
```

The command implementations are header files included by `dev.cpp`; they are
not separate programs to compile. `term.hpp` includes `json.hpp`, and the
commands use the shared helpers in `sys.hpp` and `term.hpp`.

## Commands

The `dev` executable provides:

- `mkproj` — create a project skeleton, with optional language templates.
- `pysetup` — set up a Python environment and optionally install package presets.
- `touch` — create a file, append contents, or replace contents with
  `--overwrite`.

Use `dev <command> --help` for command-specific usage.

## Build

Install a Windows-compatible `g++` compiler that supports C++20 and make sure
it is available on `PATH`. From this directory, run either:

```powershell
.\build.ps1
```

or from Command Prompt:

```bat
build.bat
```

Both scripts compile `dev.cpp` (including its header dependencies) into
`bin\dev.exe`. They do not separately compile the command headers or build
additional executables.
