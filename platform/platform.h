#pragma once

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

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
#ifdef __APPLE__
#include <climits>
#include <cstdint>

// Declared here rather than via <mach-o/dyld.h>, which defines a global
// enum { FALSE, TRUE } that collides with audio_linux.h through the PCH.
extern "C" int _NSGetExecutablePath(char* buf, uint32_t* bufsize);
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
#elif defined(__APPLE__)
    // macOS has no /proc. _NSGetExecutablePath reports the required size when
    // the buffer is too small, and may return a path containing symlinks or
    // "..", so resolve it before taking the parent.
    uint32_t size = 4096;
    std::vector<char> path(size);
    if (_NSGetExecutablePath(path.data(), &size) == -1)
    {
        path.resize(size);
        if (_NSGetExecutablePath(path.data(), &size) == -1)
            return fs::current_path();
    }
    char resolved[PATH_MAX];
    if (realpath(path.data(), resolved) != nullptr)
        return fs::path(resolved).parent_path();
#else
    std::vector<char> path(4096);
    const ssize_t length = readlink("/proc/self/exe", path.data(), path.size());
    if (length > 0 && static_cast<size_t>(length) < path.size())
        return fs::path(std::string(path.data(), length)).parent_path();
#endif
    return fs::current_path();
}
#ifdef __APPLE__

// Application Support is the macOS home for per-user app data; the app
// bundle itself may be read-only or replaced on update. XDG_DATA_HOME is a
// Linux convention and is deliberately ignored here. This is the default
// folder; --user-dir overrides it for saves and settings only.
inline fs::path DefaultUserDirectory()
{
    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0')
        return fs::path(home) / "Library" / "Application Support" / "ProjectCane";

    return fs::current_path() / ".projectcane";
}

// The <X>.app folder when the executable runs from <X>.app/Contents/MacOS,
// otherwise empty.
inline fs::path AppBundleDirectory(const fs::path& executableDirectory)
{
    const fs::path contents = executableDirectory.parent_path();
    const fs::path bundle = contents.parent_path();
    if (executableDirectory.filename() == "MacOS" && contents.filename() == "Contents" &&
        bundle.extension() == ".app")
        return bundle;
    return {};
}

// A folder has game data when its Worlds is a directory with at least one
// entry other than Finder's .DS_Store. An empty Worlds (for example one
// created by hand before extracting) is not data.
inline bool HasGameData(const fs::path& directory)
{
    std::error_code error;
    for (fs::directory_iterator entry(directory / "Worlds", error), end;
         !error && entry != end; entry.increment(error))
    {
        if (entry->path().filename() != ".DS_Store")
            return true;
    }
    return false;
}

// Inside ProjectCane.app the executable directory is Contents/MacOS, where
// game data never is. Look in Application Support first, then beside the
// .app. An empty result means the executable is not in a bundle or neither
// place has data; the caller then falls back as on other platforms.
inline fs::path BundleDataDirectory(const fs::path& applicationSupport,
    const fs::path& executableDirectory)
{
    const fs::path bundle = AppBundleDirectory(executableDirectory);
    if (bundle.empty())
        return {};
    if (HasGameData(applicationSupport))
        return applicationSupport;
    if (HasGameData(bundle.parent_path()))
        return bundle.parent_path();
    return {};
}
#endif

inline fs::path DataDirectory()
{
    if (!g_dataOverride.empty())
        return g_dataOverride;
#ifdef __APPLE__

    if (fs::path bundleData = BundleDataDirectory(DefaultUserDirectory(), ExecutableDirectory());
        !bundleData.empty())
        return bundleData;
#endif

#ifndef _WIN32
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

#ifdef _WIN32
    return ExecutableDirectory();
#elif defined(__APPLE__)
    return DefaultUserDirectory();
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
#ifdef _WIN32
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

#ifndef _WIN32
#define _stricmp cane::CompareInsensitive
#define __debugbreak() raise(SIGTRAP)
#endif
