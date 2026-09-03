# CLI

Small Windows command-line utilities written in C++20.

## Utilities

| Command | Source | Purpose |
| --- | --- | --- |
| `touch` | `src/touch-script.cpp` | Creates files, appends contents, or replaces contents with `--overwrite`. |
| `mkproj` | `src/mkproj-script.cpp` | Creates a project skeleton for C++, Python, Rust, web, or Node.js projects. |
| `pysetup` | `src/pysetup-script.cpp` | Creates or reuses a Python virtual environment and installs selected package presets. |
| `del` | `src/del-script.cpp` | Deletes files and empty directories, or recursively deletes directories with `--recursive`. |

Each utility supports `--help` and `--version` where applicable. Programs return
the status constants defined in `include/sys.hpp`; child-process results are
represented as `100 + child_exit_code`.

For Node.js projects, `mkproj <name> --node` runs `cmd.exe` through
`sys::exec()` to execute `npm init -y` and `npm install cowsay` in the new
project directory. It then creates `package-load.json` and `index.js`.
The generated project uses CommonJS and is immediately usable with
`npm start` or `node index.js`.

## Headers

### `include/sys.hpp`

Provides the shared Windows system layer:

- documented filesystem and process status codes;
- ANSI escape-sequence helpers;
- UTF-8 to UTF-16 conversion and Windows command-line quoting;
- `sys::exec()` for launching child processes;
- `sys::touch()` for creating, appending to, and replacing files.

### `include/term.hpp`

Provides the `term::` output API:

- `term::print()` and `term::println()` for styled output;
- RGB, HSV, and hexadecimal color construction;
- composable text styles;
- semantic styles such as `term::style::success` and `term::style::error`;
- built-in `Light` and `Dark` themes loaded with
  `term::style::load("Light")` or `term::style::load("Dark")`.

### `include/json.hpp`

Vendored nlohmann JSON single-header library used by `pysetup` to parse
`pip list --format=json` output. Its upstream license and notices are retained
in the file.

## Building

From a Windows PowerShell prompt with MSYS2 UCRT64 installed:

```powershell
$cxx = "C:\msys64\ucrt64\bin\g++.exe"
New-Item -ItemType Directory -Force .build | Out-Null
& $cxx -std=c++20 -O2 -static -I include src\touch-script.cpp -o .build\touch.exe
& $cxx -std=c++20 -O2 -static -I include src\mkproj-script.cpp -o .build\mkproj.exe
& $cxx -std=c++20 -O2 -static -I include src\pysetup-script.cpp -o .build\pysetup.exe
& $cxx -std=c++20 -O2 -static -I include src\del-script.cpp -o .build\del.exe
```

All source files explicitly include `../include/term.hpp` and
`../include/sys.hpp`. `pysetup-script.cpp` additionally includes
`../include/json.hpp`.

## Distribution

The current executables are stored in `dist/` and mirrored in the workspace
`bin/` directory. Historical versioned executables are kept under `archive/`.
The archive is intentionally excluded from Git by `.gitignore`; it remains
available locally for version tracking and rollback.

## Layout

```text
CLI/
├── include/       Shared headers
├── src/           Utility implementations
├── dist/          Current distributable executables
├── archive/       Local historical executables (not committed)
├── README.md      Project documentation
└── .gitignore     Git exclusions
```
