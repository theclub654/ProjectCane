#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef __SWITCH__
#include "switch/file_io.h"
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <shellapi.h>
#else
#include <csignal>
#include <unistd.h>
#endif

namespace cane
{
namespace fs = std::filesystem;

inline fs::path g_dataOverride;
inline fs::path g_userOverride;

inline std::string Utf8(const fs::path& value)
{
    const auto text = value.u8string();
    return { text.begin(), text.end() };
}

inline fs::path PathFromUtf8(const std::string& value)
{
    return fs::path(std::u8string(value.begin(), value.end()));
}

inline fs::path ExecutableDirectory()
{
#ifdef _WIN32
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(nullptr, path.data(),
        static_cast<DWORD>(path.size()));
    if (length != 0 && length < path.size())
    {
        path.resize(length);
        return fs::path(path).parent_path();
    }
#elif !defined(__SWITCH__)
    std::vector<char> path(4096);
    const ssize_t length = readlink("/proc/self/exe", path.data(), path.size());
    if (length > 0 && static_cast<size_t>(length) < path.size())
        return fs::path(std::string(path.data(), length)).parent_path();
#endif
    return fs::current_path();
}

inline fs::path DataDirectory()
{
    if (!g_dataOverride.empty())
        return g_dataOverride;

#if !defined(_WIN32) && !defined(__SWITCH__)
    // AppImage runs the real executable from a temporary, read-only mount.
    // The APPIMAGE variable points at the user-visible image, beside which the
    // extracted Worlds and Cutscenes directories are stored.
    if (const char* appImage = std::getenv("APPIMAGE"); appImage != nullptr && *appImage != '\0')
        return fs::path(appImage).parent_path();
#endif

    return ExecutableDirectory();
}

inline fs::path UserDirectory()
{
    if (!g_userOverride.empty())
        return g_userOverride;

#if defined(_WIN32) || defined(__SWITCH__)
    return ExecutableDirectory();
#else
    // ExecutableDirectory() is read-only for AppImages. Follow the XDG base
    // directory convention so saves and settings remain writable and persist
    // when the AppImage is replaced.
    if (const char* xdgDataHome = std::getenv("XDG_DATA_HOME");
        xdgDataHome != nullptr && *xdgDataHome != '\0')
    {
        return fs::path(xdgDataHome) / "ProjectCane";
    }
    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0')
        return fs::path(home) / ".local" / "share" / "ProjectCane";

    return fs::current_path() / ".projectcane";
#endif
}

inline std::vector<std::string> Arguments(int argc, char** argv)
{
    std::vector<std::string> result;
#ifdef _WIN32
    int count = 0;
    wchar_t** args = CommandLineToArgvW(GetCommandLineW(), &count);
    if (args == nullptr)
        throw std::runtime_error("Cannot read command line");
    for (int i = 1; i < count; ++i)
        result.push_back(Utf8(fs::path(args[i])));
    LocalFree(args);
#else
    for (int i = 1; i < argc; ++i)
        result.emplace_back(argv[i]);
#endif
    return result;
}

inline void ConfigureRuntime(const std::vector<std::string>& args)
{
    for (size_t i = 0; i < args.size(); ++i)
    {
        if (args[i] != "--data-dir" && args[i] != "--user-dir")
            throw std::runtime_error("Unknown argument: " + args[i]);
        const std::string option = args[i];
        if (++i == args.size())
            throw std::runtime_error("Missing path for " + option);
        const fs::path value = fs::absolute(PathFromUtf8(args[i]));
        if (option == "--data-dir")
            g_dataOverride = value;
        else
            g_userOverride = value;
    }
    fs::create_directories(UserDirectory());
}

inline bool AtomicReplace(const fs::path& temporary, const fs::path& destination)
{
#ifdef __SWITCH__
    return CaneIo::AtomicReplace(temporary, destination);
#elif defined(_WIN32)
    return MoveFileExW(temporary.c_str(), destination.c_str(),
        MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
#else
    std::error_code error;
    fs::rename(temporary, destination, error);
    return !error;
#endif
}

inline bool DebugMode()
{
#ifdef __SWITCH__
    return false;
#else
    const fs::path iniPath = UserDirectory() / "Debug Mode.ini";
    std::ifstream input(iniPath);
    if (!input)
    {
        std::ofstream(iniPath) << "[Debug]\nDebug Mode=0\n";
        return false;
    }

    std::string line;
    while (std::getline(input, line))
    {
        const size_t equals = line.find('=');
        if (equals == std::string::npos)
            continue;
        std::string key = line.substr(0, equals);
        key.erase(std::remove_if(key.begin(), key.end(),
            [](unsigned char ch) { return std::isspace(ch) != 0; }), key.end());
        if (key != "DebugMode")
            continue;
        std::string value = line.substr(equals + 1);
        value.erase(std::remove_if(value.begin(), value.end(),
            [](unsigned char ch) { return std::isspace(ch) != 0; }), value.end());
        std::transform(value.begin(), value.end(), value.begin(),
            [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
        return value == "true" || std::strtol(value.c_str(), nullptr, 10) != 0;
    }
    return false;
#endif
}

inline int CompareInsensitive(const char* left, const char* right)
{
    while (*left != '\0' && *right != '\0')
    {
        const int difference = std::tolower(static_cast<unsigned char>(*left++)) -
            std::tolower(static_cast<unsigned char>(*right++));
        if (difference != 0)
            return difference;
    }
    return static_cast<unsigned char>(*left) - static_cast<unsigned char>(*right);
}
}

#if !defined(_WIN32) && !defined(__SWITCH__)
#define _stricmp cane::CompareInsensitive
#define __debugbreak() raise(SIGTRAP)
#endif
