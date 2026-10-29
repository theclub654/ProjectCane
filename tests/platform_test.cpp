// Plain test executable for platform/platform.h. No framework: each failing
// CHECK is printed and the process exits non-zero if any check failed.
//
// Never call ConfigureRuntime or DebugMode here; they create real
// directories and files in the user's profile.

#include "platform.h"

#include <cstdio>
#include <cstdlib>
#include <optional>
#include <random>
#include <string>
#include <system_error>

namespace
{
void SetEnv(const char* name, const std::string& value)
{
#ifdef _WIN32
    _putenv_s(name, value.c_str());
#else
    setenv(name, value.c_str(), 1);
#endif
}

void UnsetEnv(const char* name)
{
#ifdef _WIN32
    _putenv_s(name, "");
#else
    unsetenv(name);
#endif
}

int g_failures = 0;
int g_checks = 0;

#define CHECK(condition) Check((condition), #condition, __FILE__, __LINE__)
#define CHECK_EQ(left, right) CheckEqual((left), (right), #left, #right, __FILE__, __LINE__)

void Check(bool condition, const char* expression, const char* file, int line)
{
    ++g_checks;
    if (condition)
        return;
    ++g_failures;
    std::fprintf(stderr, "%s:%d: CHECK failed: %s\n", file, line, expression);
}

void CheckEqual(const cane::fs::path& left, const cane::fs::path& right,
    const char* leftText, const char* rightText, const char* file, int line)
{
    ++g_checks;
    if (left == right)
        return;
    ++g_failures;
    std::fprintf(stderr, "%s:%d: CHECK_EQ failed: %s == %s\n    left:  %s\n    right: %s\n",
        file, line, leftText, rightText, cane::Utf8(left).c_str(), cane::Utf8(right).c_str());
}

// Restores an environment variable to its original state on scope exit.
class EnvGuard
{
public:
    explicit EnvGuard(const char* name) : m_name(name)
    {
        if (const char* value = std::getenv(name); value != nullptr)
            m_value = value;
    }
    ~EnvGuard()
    {
        if (m_value)
            SetEnv(m_name.c_str(), *m_value);
        else
            UnsetEnv(m_name.c_str());
    }
    EnvGuard(const EnvGuard&) = delete;
    EnvGuard& operator=(const EnvGuard&) = delete;

private:
    std::string m_name;
    std::optional<std::string> m_value;
};

// Restores the working directory and the platform overrides on scope exit.
class StateGuard
{
public:
    StateGuard()
        : m_cwd(cane::fs::current_path()),
          m_data(cane::g_dataOverride),
          m_user(cane::g_userOverride)
    {
    }
    ~StateGuard()
    {
        std::error_code error;
        cane::fs::current_path(m_cwd, error);
        cane::g_dataOverride = m_data;
        cane::g_userOverride = m_user;
    }
    StateGuard(const StateGuard&) = delete;
    StateGuard& operator=(const StateGuard&) = delete;

private:
    cane::fs::path m_cwd;
    cane::fs::path m_data;
    cane::fs::path m_user;
};

// A fresh, empty directory removed on scope exit.
class TempDir
{
public:
    TempDir()
    {
        std::random_device random;
        for (int attempt = 0; attempt < 100; ++attempt)
        {
            const cane::fs::path candidate = cane::fs::temp_directory_path() /
                ("cane_platform_test." + std::to_string(random()));
            if (cane::fs::create_directory(candidate))
            {
                // canonical() resolves macOS's /var -> /private/var symlink so
                // comparisons against current_path() are exact.
                m_path = cane::fs::canonical(candidate);
                return;
            }
        }
        throw std::runtime_error("Cannot create temporary directory");
    }
    ~TempDir()
    {
        std::error_code error;
        cane::fs::remove_all(m_path, error);
    }
    TempDir(const TempDir&) = delete;
    TempDir& operator=(const TempDir&) = delete;
    const cane::fs::path& path() const { return m_path; }

private:
    cane::fs::path m_path;
};

// (a) ExecutableDirectory() must locate the binary, not fall back to cwd.
void TestExecutableDirectoryIgnoresWorkingDirectory()
{
    StateGuard state;
    TempDir temp;
    cane::fs::current_path(temp.path());

    const cane::fs::path executableDirectory = cane::ExecutableDirectory();
    CHECK(executableDirectory != cane::fs::current_path());
    CHECK(executableDirectory.is_absolute());
#ifdef _WIN32
    CHECK(cane::fs::exists(executableDirectory / "cane_platform_tests.exe"));
#else
    CHECK(cane::fs::exists(executableDirectory / "cane_platform_tests"));
#endif
}

// (b) UserDirectory() resolves from HOME, and falls back to cwd without it.
void TestUserDirectoryFromHome()
{
    StateGuard state;
    EnvGuard home("HOME");
    EnvGuard xdg("XDG_DATA_HOME");
    TempDir temp;
    cane::fs::current_path(temp.path());
    cane::g_userOverride.clear();

    const cane::fs::path fakeHome = temp.path() / "home";
    SetEnv("HOME", cane::Utf8(fakeHome));
    UnsetEnv("XDG_DATA_HOME");
#if defined(_WIN32)
    // Windows keeps user data beside the executable.
    CHECK_EQ(cane::UserDirectory(), cane::ExecutableDirectory());
#elif defined(__APPLE__)
    CHECK_EQ(cane::UserDirectory(),
        fakeHome / "Library" / "Application Support" / "ProjectCane");

    // XDG_DATA_HOME is a Linux convention and must not redirect macOS saves.
    SetEnv("XDG_DATA_HOME", cane::Utf8(temp.path() / "xdg"));
    CHECK_EQ(cane::UserDirectory(),
        fakeHome / "Library" / "Application Support" / "ProjectCane");
    UnsetEnv("XDG_DATA_HOME");
#else
    CHECK_EQ(cane::UserDirectory(), fakeHome / ".local" / "share" / "ProjectCane");
#endif

#ifndef _WIN32
    SetEnv("HOME", "");
    CHECK_EQ(cane::UserDirectory(), cane::fs::current_path() / ".projectcane");

    UnsetEnv("HOME");
    CHECK_EQ(cane::UserDirectory(), cane::fs::current_path() / ".projectcane");
#endif
}

// (c) Command-line overrides still take precedence.
void TestOverridesWin()
{
    StateGuard state;
    EnvGuard home("HOME");
    TempDir temp;

    SetEnv("HOME", cane::Utf8(temp.path() / "home"));
    cane::g_userOverride = temp.path() / "user";
    cane::g_dataOverride = temp.path() / "data";
    CHECK_EQ(cane::UserDirectory(), temp.path() / "user");
    CHECK_EQ(cane::DataDirectory(), temp.path() / "data");
}
#ifdef __APPLE__

// A Worlds folder with one entry: the minimum that counts as game data.
void MakeWorldsEntry(const cane::fs::path& directory)
{
    cane::fs::create_directories(directory / "Worlds");
    std::ofstream(directory / "Worlds" / "level.pcbrx") << "x";
}

// (d) Inside an .app the executable directory is Contents/MacOS, where game
// data never is. Without --data-dir, data is looked up in Application
// Support, then beside the .app, then the executable directory as before.
void TestMacBundleDataDirectory()
{
    StateGuard state;
    EnvGuard home("HOME");
    TempDir temp;
    cane::g_userOverride.clear();
    cane::g_dataOverride.clear();
    SetEnv("HOME", cane::Utf8(temp.path() / "home"));

    // ConfigureRuntime always creates Application Support, so the "no data
    // there" case is an existing empty directory, not a missing one.
    const cane::fs::path support = cane::UserDirectory();
    CHECK_EQ(support, temp.path() / "home" / "Library" / "Application Support" / "ProjectCane");
    cane::fs::create_directories(support);

    const cane::fs::path games = temp.path() / "Games";
    const cane::fs::path bundleMacOS = games / "X.app" / "Contents" / "MacOS";
    cane::fs::create_directories(bundleMacOS);

    // Neither location has Worlds: no bundle answer, so the caller falls back.
    CHECK_EQ(cane::BundleDataDirectory(support, bundleMacOS), cane::fs::path());

    // Beside the .app only (Application Support has no Worlds).
    MakeWorldsEntry(games);
    CHECK_EQ(cane::BundleDataDirectory(support, bundleMacOS), games);

    // Application Support wins over beside the .app.
    MakeWorldsEntry(support);
    CHECK_EQ(cane::BundleDataDirectory(support, bundleMacOS), support);

    // Beside-the-.app applies only to <X>.app/Contents/MacOS layouts.
    cane::fs::remove_all(support / "Worlds");
    const cane::fs::path plainBin = games / "build" / "bin";
    cane::fs::create_directories(plainBin);
    MakeWorldsEntry(games / "build");
    CHECK_EQ(cane::BundleDataDirectory(support, plainBin), cane::fs::path());
    const cane::fs::path notApp = games / "X" / "Contents" / "MacOS";
    cane::fs::create_directories(notApp);
    CHECK_EQ(cane::BundleDataDirectory(support, notApp), cane::fs::path());

    // A Worlds file, not a directory, is not data.
    const cane::fs::path other = temp.path() / "Other";
    const cane::fs::path otherMacOS = other / "Y.app" / "Contents" / "MacOS";
    cane::fs::create_directories(otherMacOS);
    std::ofstream(other / "Worlds") << "not a directory";
    CHECK_EQ(cane::BundleDataDirectory(support, otherMacOS), cane::fs::path());

    // Symlinked Worlds (the usual way to point at extracted data) counts.
    const cane::fs::path linked = temp.path() / "Linked";
    const cane::fs::path linkedMacOS = linked / "Z.app" / "Contents" / "MacOS";
    cane::fs::create_directories(linkedMacOS);
    cane::fs::create_directory_symlink(games / "Worlds", linked / "Worlds");
    CHECK_EQ(cane::BundleDataDirectory(support, linkedMacOS), linked);

    // A symlinked Worlds in Application Support counts too.
    cane::fs::create_directory_symlink(games / "Worlds", support / "Worlds");
    CHECK_EQ(cane::BundleDataDirectory(support, otherMacOS), support);

    // End to end through DataDirectory(): this test binary is not in a
    // bundle, so Application Support is not consulted (see (f)).
    CHECK_EQ(cane::DataDirectory(), cane::ExecutableDirectory());

    // An explicit --data-dir still wins over Application Support.
    cane::g_dataOverride = temp.path() / "data";
    CHECK_EQ(cane::DataDirectory(), temp.path() / "data");
}

// (e) An empty Worlds folder, or one holding only Finder's .DS_Store, is not
// data: it must not hide the data beside the .app.
void TestMacEmptyWorldsIsNoData()
{
    StateGuard state;
    TempDir temp;
    const cane::fs::path support = temp.path() / "support";
    const cane::fs::path games = temp.path() / "Games";
    const cane::fs::path bundleMacOS = games / "X.app" / "Contents" / "MacOS";
    cane::fs::create_directories(bundleMacOS);
    MakeWorldsEntry(games);

    cane::fs::create_directories(support / "Worlds");
    CHECK_EQ(cane::BundleDataDirectory(support, bundleMacOS), games);
    std::ofstream(support / "Worlds" / ".DS_Store") << "finder";
    CHECK_EQ(cane::BundleDataDirectory(support, bundleMacOS), games);
    MakeWorldsEntry(support);
    CHECK_EQ(cane::BundleDataDirectory(support, bundleMacOS), support);

    // The same rule, used by main.cpp's missing-data alert.
    const cane::fs::path empty = temp.path() / "empty";
    CHECK(!cane::HasGameData(empty));
    cane::fs::create_directories(empty / "Worlds");
    CHECK(!cane::HasGameData(empty));
    std::ofstream(empty / "Worlds" / ".DS_Store") << "finder";
    CHECK(!cane::HasGameData(empty));
    CHECK(cane::HasGameData(games));
    CHECK_EQ(cane::AppBundleDirectory(bundleMacOS), games / "X.app");
    CHECK_EQ(cane::AppBundleDirectory(games / "build" / "bin"), cane::fs::path());

    // With no data anywhere there is no bundle answer.
    cane::fs::remove_all(support / "Worlds");
    cane::fs::remove_all(games / "Worlds");
    cane::fs::create_directories(games / "Worlds");
    CHECK_EQ(cane::BundleDataDirectory(support, bundleMacOS), cane::fs::path());
}

// (f) A run outside a bundle (dev build, tests) returns exactly what master
// returns: the executable directory, even when Application Support has data.
void TestMacNonBundleIgnoresApplicationSupport()
{
    StateGuard state;
    EnvGuard home("HOME");
    EnvGuard appImage("APPIMAGE");
    TempDir temp;
    cane::g_userOverride.clear();
    cane::g_dataOverride.clear();
    UnsetEnv("APPIMAGE");
    SetEnv("HOME", cane::Utf8(temp.path() / "home"));
    MakeWorldsEntry(cane::UserDirectory());

    CHECK_EQ(cane::DataDirectory(), cane::ExecutableDirectory());
    const cane::fs::path plainBin = temp.path() / "build" / "bin";
    cane::fs::create_directories(plainBin);
    CHECK_EQ(cane::BundleDataDirectory(cane::UserDirectory(), plainBin), cane::fs::path());
}

// Runs a copy of this test binary placed inside <bundle>/Contents/MacOS and
// returns the data directory it reports (see --print-data-directory in main).
std::string BundledDataDirectory(const cane::fs::path& bundle, const std::string& extraArguments)
{
    const cane::fs::path macOS = bundle / "Contents" / "MacOS";
    cane::fs::create_directories(macOS);
    const cane::fs::path copy = macOS / "cane_platform_tests";
    if (!cane::fs::exists(copy))
        cane::fs::copy_file(cane::ExecutableDirectory() / "cane_platform_tests", copy);
    const std::string command = "'" + cane::Utf8(copy) + "' --print-data-directory " + extraArguments;
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe == nullptr)
        return "<popen failed>";
    std::string output;
    char buffer[4096];
    while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr)
        output += buffer;
    if (pclose(pipe) != 0)
        return "<child failed: " + output + ">";
    while (!output.empty() && output.back() == '\n')
        output.pop_back();
    return output;
}

// (g) End to end in a real bundle layout: --user-dir moves saves and settings
// but not the data lookup, which uses the default Application Support folder.
void TestMacBundledRunDataLookup()
{
    StateGuard state;
    EnvGuard home("HOME");
    EnvGuard appImage("APPIMAGE");
    TempDir temp;
    UnsetEnv("APPIMAGE");
    const cane::fs::path fakeHome = temp.path() / "home";
    SetEnv("HOME", cane::Utf8(fakeHome));
    const cane::fs::path support = fakeHome / "Library" / "Application Support" / "ProjectCane";
    const cane::fs::path games = temp.path() / "Some Games";
    const cane::fs::path bundle = games / "Renamed Cane.app";
    const cane::fs::path custom = temp.path() / "custom user";

    // Nothing anywhere: the executable directory, as before.
    CHECK_EQ(cane::PathFromUtf8(BundledDataDirectory(bundle, "")), bundle / "Contents" / "MacOS");

    // Beside the .app.
    MakeWorldsEntry(games);
    CHECK_EQ(cane::PathFromUtf8(BundledDataDirectory(bundle, "")), games);

    // --user-dir pointing at a folder with data does not move the lookup.
    MakeWorldsEntry(custom);
    CHECK_EQ(cane::PathFromUtf8(BundledDataDirectory(bundle,
        "--user-dir '" + cane::Utf8(custom) + "'")), games);

    // Application Support wins, with or without --user-dir.
    MakeWorldsEntry(support);
    CHECK_EQ(cane::PathFromUtf8(BundledDataDirectory(bundle, "")), support);
    CHECK_EQ(cane::PathFromUtf8(BundledDataDirectory(bundle,
        "--user-dir '" + cane::Utf8(custom) + "'")), support);
}
#endif
}

#ifdef __APPLE__
int main(int argc, char** argv)
{
    // Child mode for TestMacBundledRunDataLookup: report the data directory
    // as seen from wherever this copy of the binary lives. Only --user-dir
    // is accepted; ConfigureRuntime is not called (see the note at the top).
    if (argc >= 2 && std::string(argv[1]) == "--print-data-directory")
    {
        if (argc == 4 && std::string(argv[2]) == "--user-dir")
            cane::g_userOverride = cane::fs::absolute(cane::PathFromUtf8(argv[3]));
        std::printf("%s\n", cane::Utf8(cane::DataDirectory()).c_str());
        return 0;
    }
#else
int main()
{
#endif
    TestExecutableDirectoryIgnoresWorkingDirectory();
    TestUserDirectoryFromHome();
    TestOverridesWin();
#ifdef __APPLE__
    TestMacBundleDataDirectory();
    TestMacEmptyWorldsIsNoData();
    TestMacNonBundleIgnoresApplicationSupport();
    TestMacBundledRunDataLookup();
#endif

    if (g_failures != 0)
    {
        std::fprintf(stderr, "%d of %d checks failed\n", g_failures, g_checks);
        return 1;
    }
    std::printf("All %d checks passed\n", g_checks);
    return 0;
}
