#pragma once
#include <mutex>

#include "install.h"
#include "process.h"
namespace cane {
inline std::string executableName(const std::string& name) {
#ifdef _WIN32
    return name + ".exe";
#else
    return name;
#endif
}
struct LauncherBackend {
    UserPaths locations = userPaths();
    fs::path tools = executableDirectory();
    json config = json::object();
    LauncherBackend() {
        for (auto& p :
             {locations.data.parent_path(), locations.user, locations.config, locations.logs})
            fs::create_directories(p);
        {
            FileLock migrationLock(locations.config / "profile-migration.lock");
            importDirectoryIfEmpty(runtimeStorage(locations) / "Saves", locations.user / "Saves");
            importDirectoryIfEmpty(legacyLogDirectory(locations), locations.logs);
            importDirectoryIfEmpty(executableDirectory() / "Saves", locations.user / "Saves");
        }
        fs::create_directories(locations.user / "Saves");
        auto file = locations.config / "launcher.json";
        if (fs::exists(file)) {
            try {
                config = readJson(file);
                if (!config.is_object()) throw std::runtime_error("Invalid launcher configuration");
                for (auto key : {"runtime_dir", "iso_path"})
                    if (config.contains(key) && !config.at(key).is_string())
                        throw std::runtime_error("Invalid launcher configuration field");
            } catch (...) {
                auto backup = file;
                backup += ".broken";
                std::error_code ec;
                fs::rename(file, backup, ec);
                config = json::object();
            }
        }
        if (config.contains("runtime_dir"))
            tools = locations.portable ? runtimeStorage(locations) / "runtime" / "bin"
                                       : path(config.at("runtime_dir").get<std::string>());
        // Recover activation interrupted between its two renames.
        for (const auto& target : {locations.data, runtimeStorage(locations) / "runtime"}) {
            auto previous = target;
            previous += ".previous";
            if (!fs::exists(target) && fs::exists(previous)) {
                try {
                    InstallationTransaction recovery(target);
                } catch (...) {
                }
            }
        }
    }
    fs::path binary(const std::string& name) const {
        auto p = tools / executableName(name);
        if (!fs::is_regular_file(p))
            throw std::runtime_error("Missing bundled program: " + utf8(p));
        return p;
    }
    bool installed() const {
        try {
            auto m = readJson(locations.data / "installation.json");
            return m.at("schema") == 1 && m.at("asset_version") == assetVersion &&
                   m.at("disc_sha256") == discFingerprint &&
                   fs::is_regular_file(binary("Sly1-Release"));
        } catch (...) {
            return false;
        }
    }
    std::string rememberedIso() const {
        if (config.contains("iso_path")) return config.at("iso_path").get<std::string>();
        try {
            return readJson(locations.data / "installation.json").at("iso_path").get<std::string>();
        } catch (...) {
            return {};
        }
    }
    void install(const std::string& iso, const std::atomic<bool>& cancel, const LogFunction& log) {
        try {
            checkedProcess({utf8(binary("cane-extractor")), "--iso", iso, "--output",
                            utf8(locations.data), "--non-interactive", "--progress=jsonl"},
                           tools, cancel, log);
        } catch (...) {
            // The extractor may be terminated without running destructors.
            // Acquire its released lock before discarding incomplete staging.
            try {
                InstallationTransaction cleanup(locations.data);
            } catch (...) {
            }
            throw;
        }
        config["iso_path"] = iso;
        writeJson(locations.config / "launcher.json", config);
    }
    void play(const std::atomic<bool>& cancel, const LogFunction& log) {
        if (!installed())
            throw std::runtime_error("Install the game with this launcher version first");
        checkedProcess({utf8(binary("Sly1-Release")), "--data-dir", utf8(locations.data),
                        "--user-dir", utf8(locations.user)},
                       tools, cancel, log);
    }
    void buildSource(const fs::path& checkout, const std::atomic<bool>& cancel,
                     const LogFunction& log) {
        auto source = fs::absolute(checkout);
        if (!fs::is_regular_file(source / "CMakeLists.txt") ||
            !fs::is_directory(source / "Sly1-Release"))
            throw std::runtime_error(
                "Select a ProjectCane checkout containing the portable CMake build");
        fs::create_directories(runtimeStorage(locations));
        FileLock buildLock(runtimeStorage(locations) / "source-build.lock");
        auto build = runtimeStorage(locations) / "source-build";
        std::ifstream cache(build / "CMakeCache.txt");
        std::string line;
        while (std::getline(cache, line)) {
            constexpr const char* prefix = "CMAKE_HOME_DIRECTORY:INTERNAL=";
            if (line.starts_with(prefix) &&
                fs::weakly_canonical(path(line.substr(std::char_traits<char>::length(prefix)))) !=
                    fs::weakly_canonical(source)) {
                cache.close();
                fs::remove_all(build);
                break;
            }
        }
        try {
            checkedProcess({"cmake", "--version"}, source, cancel, log);
        } catch (...) {
            if (cancel) throw;
            throw std::runtime_error(
                "CMake is missing or cannot run. Install CMake 3.22+ and add it to PATH, then "
                "restart the launcher.");
        }
        std::vector<std::string> configure = {"cmake",
                                              "-S",
                                              utf8(source),
                                              "-B",
                                              utf8(build),
                                              "-DCMAKE_BUILD_TYPE=Release",
                                              "-DCANE_BUILD_LAUNCHER=OFF",
                                              "-DBUILD_TESTING=OFF"};
        try {
            checkedProcess(configure, source, cancel, log);
        } catch (...) {
            if (cancel) throw;
#ifdef _WIN32
            throw std::runtime_error(
                "Configuration failed. Install Visual Studio 2022 with Desktop development with "
                "C++, or check the build log for missing dependencies.");
#else
            throw std::runtime_error(
                "Configuration failed. Install a C++20 compiler, make/Ninja, and the development "
                "packages listed in README.md. See the build log for the exact missing tool or "
                "library.");
#endif
        }
        checkedProcess({"cmake", "--build", utf8(build), "--config", "Release", "--parallel", "2"},
                       source, cancel, log);
        auto runtime = runtimeStorage(locations) / "runtime";
        InstallationTransaction transaction(runtime);
        checkedProcess({"cmake", "--install", utf8(build), "--config", "Release", "--prefix",
                        utf8(transaction.directory())},
                       source, cancel, log);
        for (auto name : {"cane-extractor", "Sly1-Release"})
            if (!fs::is_regular_file(transaction.directory() / "bin" / executableName(name)))
                throw std::runtime_error("Build did not install required programs");
        transaction.commit();
        tools = runtime / "bin";
        config["runtime_dir"] =
            utf8(locations.portable ? fs::relative(tools, locations.config) : tools);
        writeJson(locations.config / "launcher.json", config);
    }
};
}  // namespace cane
