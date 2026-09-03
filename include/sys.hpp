#pragma once

/**
 * @file sys.hpp
 * @brief Shared Windows system helpers and command-line conventions.
 *
 * The header is intentionally self-contained for the CLI programs.  Process
 * failures use the documented status codes below; child-process exit codes
 * are returned as 100 + child_exit_code by exec().
 */

#include <windows.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include ".\term.hpp"

namespace fs = std::filesystem;

namespace sys
{
    /** ANSI/VT100 escape sequences used by CLI output and terminal control. */
    namespace ansi
    {
        inline constexpr std::string_view ESC = "\x1b";
        inline constexpr std::string_view RESET = "\x1b[0m";
        inline constexpr std::string_view BOLD = "\x1b[1m";
        inline constexpr std::string_view DIM = "\x1b[2m";
        inline constexpr std::string_view ITALIC = "\x1b[3m";
        inline constexpr std::string_view UNDERLINE = "\x1b[4m";
        inline constexpr std::string_view SGR_BLINK = "\x1b[5m";
        inline constexpr std::string_view SGR_RAPID_BLINK = "\x1b[6m";
        inline constexpr std::string_view INVERSE = "\x1b[7m";
        inline constexpr std::string_view HIDDEN = "\x1b[8m";
        inline constexpr std::string_view STRIKETHROUGH = "\x1b[9m";
        inline constexpr std::string_view DOUBLE_UNDERLINE = "\x1b[21m";
        inline constexpr std::string_view OVERLINE = "\x1b[53m";
        inline constexpr std::string_view CLEAR_SCREEN = "\x1b[2J";
        inline constexpr std::string_view CLEAR_LINE = "\x1b[2K";
        inline constexpr std::string_view CURSOR_HOME = "\x1b[H";
        inline constexpr std::string_view CURSOR_HIDE = "\x1b[?25l";
        inline constexpr std::string_view CURSOR_SHOW = "\x1b[?25h";

        inline std::string cursor_position(int row, int column)
        {
            return "\x1b[" + std::to_string(row) + ";" +
                   std::to_string(column) + "H";
        }

        inline std::string foreground(int red, int green, int blue)
        {
            return "\x1b[38;2;" + std::to_string(red) + ";" +
                   std::to_string(green) + ";" + std::to_string(blue) + "m";
        }

        inline std::string background(int red, int green, int blue)
        {
            return "\x1b[48;2;" + std::to_string(red) + ";" +
                   std::to_string(green) + ";" + std::to_string(blue) + "m";
        }
    }

    inline constexpr int SUCCESS = 0;

    // Filesystem and I/O.
    inline constexpr int FILE_ALREADY_EXISTS = 1;
    inline constexpr int FILE_DOESNT_EXIST = 2;
    inline constexpr int DIRECTORY_NOT_FOUND = 3;
    inline constexpr int DIRECTORY_NOT_EMPTY = 4;
    inline constexpr int INVALID_PATH = 5;
    inline constexpr int PATH_TOO_LONG = 6;
    inline constexpr int FILE_TOO_LARGE = 7;
    inline constexpr int DISK_FULL = 8;
    inline constexpr int OPEN_FILE_ERROR = 9;
    inline constexpr int CLOSE_FILE_ERROR = 10;
    inline constexpr int READ_ERROR = 11;
    inline constexpr int WRITE_ERROR = 12;
    inline constexpr int DELETE_ERROR = 13;
    inline constexpr int RENAME_ERROR = 14;
    inline constexpr int CREATE_DIRECTORY_ERROR = 15;

    // Permissions, handles, and arguments.
    inline constexpr int ACCESS_DENIED = 16;
    inline constexpr int HANDLE_ERROR = 17;
    inline constexpr int INVALID_PARAMETER_ERROR = 18;

    // Processes and executables.
    inline constexpr int PROCESS_LAUNCH_ERROR = 19;
    inline constexpr int CREATE_PROCESS_ERROR = 20;
    inline constexpr int PROCESS_NOT_FOUND = 21;
    inline constexpr int PROCESS_TERMINATION_ERROR = 22;
    inline constexpr int WAIT_ERROR = 23;
    inline constexpr int GET_EXIT_CODE_ERROR = 24;
    inline constexpr int EXECUTABLE_NOT_FOUND = 25;
    inline constexpr int EXE_LAUNCH_ERROR = 26;
    inline constexpr int MODULE_NOT_FOUND = 27;

    // Pipes and runtime.
    inline constexpr int CREATE_PIPE_ERROR = 28;
    inline constexpr int SET_HANDLE_INFO_ERROR = 29;
    inline constexpr int PIPE_READ_ERROR = 30;
    inline constexpr int PIPE_WRITE_ERROR = 31;
    inline constexpr int PIPE_DISCONNECTED = 32;
    inline constexpr int MEMORY_ERROR = 33;
    inline constexpr int BUFFER_ERROR = 34;
    inline constexpr int ENVIRONMENT_ERROR = 35;
    inline constexpr int ENCODING_ERROR = 36;
    inline constexpr int TIMEOUT_ERROR = 37;
    inline constexpr int GENERIC_ERROR = 38;
    inline constexpr int INVALID_INPUT_ERROR = 39;
    inline constexpr int NOT_DIRECTORY = 40;

    inline bool succeeded(int result)
    {
        return result == SUCCESS || (result >= 100 && result < 200);
    }

    struct execOptions
    {
        bool captureStdOut = false;
        bool captureStdErr = false;
        bool hideOutput = true;
        bool searchPATH = false;
    };

    struct execReturnType
    {
        int returnCode;
        std::optional<std::string> output;
    };

    namespace detail
    {
        inline std::wstring toWide(std::string_view input)
        {
            if (input.empty())
                return {};
            const int size = MultiByteToWideChar(
                CP_UTF8, 0, input.data(), static_cast<int>(input.size()),
                nullptr, 0);
            if (size <= 0)
                return {};
            std::wstring result(size, L'\0');
            if (MultiByteToWideChar(
                    CP_UTF8, 0, input.data(), static_cast<int>(input.size()),
                    result.data(), size) <= 0)
                return {};
            return result;
        }

        inline std::wstring quoteArgument(const std::wstring& argument)
        {
            if (argument.empty())
                return L"\"\"";

            const bool needsQuotes = argument.find_first_of(L" \t\"") !=
                                     std::wstring::npos;
            if (!needsQuotes)
                return argument;

            std::wstring result = L"\"";
            std::size_t backslashes = 0;
            for (wchar_t character : argument)
            {
                if (character == L'\\')
                {
                    ++backslashes;
                }
                else if (character == L'"')
                {
                    result.append(backslashes * 2 + 1, L'\\');
                    result += L'"';
                    backslashes = 0;
                }
                else
                {
                    result.append(backslashes, L'\\');
                    backslashes = 0;
                    result += character;
                }
            }
            result.append(backslashes * 2, L'\\');
            result += L'"';
            return result;
        }

        inline int mapWindowsError(DWORD error)
        {
            switch (error)
            {
            case ERROR_FILE_NOT_FOUND: return FILE_DOESNT_EXIST;
            case ERROR_PATH_NOT_FOUND: return DIRECTORY_NOT_FOUND;
            case ERROR_FILE_EXISTS:
            case ERROR_ALREADY_EXISTS: return FILE_ALREADY_EXISTS;
            case ERROR_INVALID_NAME:
            case ERROR_INVALID_DRIVE: return INVALID_PATH;
            case ERROR_FILENAME_EXCED_RANGE: return PATH_TOO_LONG;
            case ERROR_DISK_FULL:
            case ERROR_HANDLE_DISK_FULL: return DISK_FULL;
            case ERROR_FILE_TOO_LARGE: return FILE_TOO_LARGE;
            case ERROR_DIR_NOT_EMPTY: return DIRECTORY_NOT_EMPTY;
            case ERROR_ACCESS_DENIED:
            case ERROR_WRITE_PROTECT: return ACCESS_DENIED;
            case ERROR_OPEN_FAILED: return OPEN_FILE_ERROR;
            case ERROR_READ_FAULT:
            case ERROR_CRC:
            case ERROR_IO_DEVICE: return READ_ERROR;
            case ERROR_WRITE_FAULT:
            case ERROR_CANNOT_MAKE: return WRITE_ERROR;
            case ERROR_BAD_EXE_FORMAT:
            case ERROR_EXE_MACHINE_TYPE_MISMATCH: return EXE_LAUNCH_ERROR;
            case ERROR_DLL_NOT_FOUND:
            case ERROR_MOD_NOT_FOUND: return MODULE_NOT_FOUND;
            case ERROR_PROC_NOT_FOUND: return PROCESS_NOT_FOUND;
            case ERROR_PROCESS_ABORTED: return PROCESS_TERMINATION_ERROR;
            case ERROR_CREATE_FAILED: return CREATE_PROCESS_ERROR;
            case ERROR_INVALID_HANDLE: return HANDLE_ERROR;
            case ERROR_INVALID_PARAMETER:
            case ERROR_INVALID_ACCESS: return INVALID_PARAMETER_ERROR;
            case ERROR_PIPE_NOT_CONNECTED:
            case ERROR_BROKEN_PIPE: return PIPE_DISCONNECTED;
            case ERROR_PIPE_BUSY:
            case ERROR_NO_DATA: return PIPE_READ_ERROR;
            case ERROR_PIPE_LISTENING: return WAIT_ERROR;
            case ERROR_NOT_ENOUGH_MEMORY:
            case ERROR_OUTOFMEMORY: return MEMORY_ERROR;
            case ERROR_INSUFFICIENT_BUFFER:
            case ERROR_BUFFER_OVERFLOW: return BUFFER_ERROR;
            case ERROR_ENVVAR_NOT_FOUND:
            case ERROR_BAD_ENVIRONMENT: return ENVIRONMENT_ERROR;
            case WAIT_TIMEOUT:
            case ERROR_TIMEOUT: return TIMEOUT_ERROR;
            default: return GENERIC_ERROR;
            }
        }

        inline std::wstring commandLine(
            const std::variant<fs::path, std::string, const char*>& executable,
            const std::variant<std::vector<std::string>,
                               std::vector<const char*>>& arguments,
            std::wstring& applicationName,
            bool searchPATH)
        {
            std::wstring executableName;
            if (std::holds_alternative<fs::path>(executable))
            {
                fs::path path = std::get<fs::path>(executable);
                if (path.is_absolute() || !searchPATH)
                {
                    path = path.is_absolute() ? path : fs::absolute(path);
                    applicationName = path.wstring();
                    executableName = path.wstring();
                }
                else
                {
                    applicationName.clear();
                    executableName = path.filename().wstring();
                }
            }
            else
            {
                const std::string value =
                    std::holds_alternative<std::string>(executable)
                        ? std::get<std::string>(executable)
                        : std::get<const char*>(executable);
                executableName = toWide(value);
                if (searchPATH)
                    applicationName.clear();
                else
                    applicationName = executableName;
            }

            std::wstring result = quoteArgument(executableName);
            if (std::holds_alternative<std::vector<std::string>>(arguments))
            {
                for (const auto& argument :
                     std::get<std::vector<std::string>>(arguments))
                    result += L" " + quoteArgument(toWide(argument));
            }
            else
            {
                for (const auto* argument :
                     std::get<std::vector<const char*>>(arguments))
                    result += L" " + quoteArgument(toWide(argument ? argument : ""));
            }
            return result;
        }
    }

    inline execReturnType exec(
        std::variant<fs::path, std::string, const char*> executable,
        std::variant<std::vector<std::string>, std::vector<const char*>> arguments,
        execOptions options = {})
    {
        std::wstring applicationName;
        std::wstring command = detail::commandLine(
            executable, arguments, applicationName, options.searchPATH);
        std::vector<wchar_t> commandBuffer(command.begin(), command.end());
        commandBuffer.push_back(L'\0');

        STARTUPINFOW startup{};
        PROCESS_INFORMATION process{};
        startup.cb = sizeof(startup);

        HANDLE readHandle = nullptr;
        HANDLE writeHandle = nullptr;
        BOOL inheritHandles = FALSE;
        if (options.captureStdOut || options.captureStdErr)
        {
            SECURITY_ATTRIBUTES attributes{};
            attributes.nLength = sizeof(attributes);
            attributes.bInheritHandle = TRUE;
            if (!CreatePipe(&readHandle, &writeHandle, &attributes, 0))
                return {CREATE_PIPE_ERROR, std::nullopt};
            if (!SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0))
            {
                CloseHandle(readHandle);
                CloseHandle(writeHandle);
                return {SET_HANDLE_INFO_ERROR, std::nullopt};
            }
            startup.dwFlags |= STARTF_USESTDHANDLES;
            startup.hStdOutput = writeHandle;
            startup.hStdError = writeHandle;
            inheritHandles = TRUE;
        }

        if (!CreateProcessW(
                applicationName.empty() ? nullptr : applicationName.c_str(),
                commandBuffer.data(), nullptr, nullptr, inheritHandles, 0,
                nullptr, fs::current_path().c_str(), &startup, &process))
        {
            const int result = detail::mapWindowsError(GetLastError());
            if (writeHandle) CloseHandle(writeHandle);
            if (readHandle) CloseHandle(readHandle);
            return {result, std::nullopt};
        }
        if (writeHandle)
            CloseHandle(writeHandle);

        std::string output;
        if (readHandle)
        {
            char buffer[4096];
            DWORD bytesRead = 0;
            while (ReadFile(readHandle, buffer, sizeof(buffer), &bytesRead, nullptr))
            {
                if (bytesRead == 0) break;
                output.append(buffer, bytesRead);
            }
            CloseHandle(readHandle);
        }

        if (WaitForSingleObject(process.hProcess, INFINITE) == WAIT_FAILED)
        {
            const int result = detail::mapWindowsError(GetLastError());
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
            return {result, std::nullopt};
        }

        DWORD exitCode = 0;
        if (!GetExitCodeProcess(process.hProcess, &exitCode))
        {
            const int result = detail::mapWindowsError(GetLastError());
            CloseHandle(process.hThread);
            CloseHandle(process.hProcess);
            return {result, std::nullopt};
        }
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
        return {
            static_cast<int>(exitCode) + 100,
            readHandle ? std::optional<std::string>(std::move(output))
                       : std::nullopt};
    }

    /** Create a file, or append/replace its contents when it already exists. */
    inline int touch(
        const fs::path& path,
        const std::string& contents = {},
        bool overwrite = false)
    {
        if (fs::exists(path))
        {
            if (!fs::is_regular_file(path))
                return INVALID_PATH;
            if (contents.empty())
                return SUCCESS;
            std::ofstream file(
                path, overwrite ? std::ios::out | std::ios::trunc
                                : std::ios::out | std::ios::app);
            if (!file)
                return OPEN_FILE_ERROR;
            if (!overwrite)
                file << '\n';
            file << contents;
            return file ? SUCCESS : WRITE_ERROR;
        }

        std::ofstream file(path);
        if (!file)
            return OPEN_FILE_ERROR;
        if (!contents.empty())
            file << contents;
        return file ? SUCCESS : WRITE_ERROR;
    }

    inline int touch(
        const std::string& path,
        const std::string& contents = {},
        bool overwrite = false)
    {
        return touch(fs::path(path), contents, overwrite);
    }
}
