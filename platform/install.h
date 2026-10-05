#pragma once
#include <array>
#include <cerrno>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <system_error>

#include "picosha2.h"
#include "platform.h"
#ifndef _WIN32
#include <fcntl.h>
#include <sys/file.h>
#endif
namespace cane {
using json = nlohmann::json;
inline constexpr int assetVersion = 1;
inline constexpr const char* discFingerprint =
    "d4ba3ce30047ffb2a825ef13d37a48c4df63aa57977157a194f66bd0b859796c";
inline std::string hashFile(const fs::path& file, const std::function<bool()>& cancelled = {}) {
    std::ifstream in(file, std::ios::binary);
    if (!in) throw std::runtime_error("Cannot read: " + utf8(file));
    picosha2::hash256_one_by_one hash;
    std::array<char, 65536> buffer;
    while (in) {
        if (cancelled && cancelled()) throw std::runtime_error("Cancelled");
        in.read(buffer.data(), buffer.size());
        hash.process(buffer.begin(), buffer.begin() + in.gcount());
    }
    if (!in.eof()) throw std::runtime_error("Read failed: " + utf8(file));
    hash.finish();
    return picosha2::get_hash_hex_string(hash);
}
inline json readJson(const fs::path& file) {
    std::ifstream in(file);
    if (!in) throw std::runtime_error("Cannot open: " + utf8(file));
    json value;
    in >> value;
    return value;
}
inline void writeJson(const fs::path& file, const json& value) {
    fs::create_directories(file.parent_path());
    auto temporary = file;
    temporary += ".tmp";
    std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
    out << value.dump(2) << '\n';
    out.flush();
    if (!out) throw std::runtime_error("Cannot write: " + utf8(file));
    out.close();
    if (!atomicReplace(temporary, file)) throw std::runtime_error("Cannot replace: " + utf8(file));
}
inline uint64_t processId() {
#ifdef _WIN32
    return GetCurrentProcessId();
#else
    return getpid();
#endif
}
inline bool processAlive(uint64_t pid) {
#ifdef _WIN32
    auto h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, static_cast<DWORD>(pid));
    if (!h) return GetLastError() == ERROR_ACCESS_DENIED;
    DWORD code = 0;
    auto alive = GetExitCodeProcess(h, &code) && code == STILL_ACTIVE;
    CloseHandle(h);
    return alive;
#else
    return pid > 0 && (kill(static_cast<pid_t>(pid), 0) == 0 || errno == EPERM);
#endif
}
// OS locks are released even when an installer is killed. The empty lock
// file stays in place on POSIX so competing processes always lock one inode.
class FileLock {
#ifdef _WIN32
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#else
    int handle_ = -1;
#endif
   public:
    explicit FileLock(const fs::path& file) {
#ifdef _WIN32
        handle_ = CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
        if (handle_ == INVALID_HANDLE_VALUE)
            throw std::runtime_error(
                "Another installation is in progress, or its lock cannot be opened");
#else
        handle_ = open(file.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
        if (handle_ < 0) throw std::runtime_error("Cannot open installation lock");
        if (flock(handle_, LOCK_EX | LOCK_NB) != 0) {
            close(handle_);
            handle_ = -1;
            throw std::runtime_error("Another installation is in progress");
        }
#endif
    }
    FileLock(const FileLock&) = delete;
    ~FileLock() {
#ifdef _WIN32
        if (handle_ != INVALID_HANDLE_VALUE) CloseHandle(handle_);
#else
        if (handle_ >= 0) {
            flock(handle_, LOCK_UN);
            close(handle_);
        }
#endif
    }
};
class InstallationTransaction {
    fs::path target_, stage_, previous_, lock_;
    std::unique_ptr<FileLock> guard_;
    bool committed_ = false;

   public:
    explicit InstallationTransaction(fs::path target)
        : target_(fs::absolute(target)), stage_(target_), previous_(target_), lock_(target_) {
        stage_ += ".staging";
        previous_ += ".previous";
        lock_ += ".lock";
        fs::create_directories(target_.parent_path());
        guard_ = std::make_unique<FileLock>(lock_);
        if (!fs::exists(target_) && fs::exists(previous_)) fs::rename(previous_, target_);
        if (fs::exists(target_) && fs::exists(previous_)) fs::remove_all(previous_);
        fs::remove_all(stage_);
        fs::create_directory(stage_);
    }
    InstallationTransaction(const InstallationTransaction&) = delete;
    const fs::path& directory() const { return stage_; }
    void commit() {
        if (fs::exists(target_)) fs::rename(target_, previous_);
        try {
            fs::rename(stage_, target_);
        } catch (...) {
            if (fs::exists(previous_)) fs::rename(previous_, target_);
            throw;
        }
        committed_ = true;
        std::error_code ec;
        fs::remove_all(previous_, ec);
    }
    ~InstallationTransaction() {
        std::error_code ec;
        if (!committed_) fs::remove_all(stage_, ec);
    }
};
inline fs::path safeAssetPath(const fs::path& root, const std::string& name) {
    auto relative = path(name);
    if (relative.empty() || relative.is_absolute() || relative.has_root_name())
        throw std::runtime_error("Invalid manifest path");
    for (const auto& part : relative)
        if (part == ".." || part == ".") throw std::runtime_error("Invalid manifest path");
    auto candidate = root;
    for (const auto& part : relative) {
        candidate /= part;
        if (fs::is_symlink(candidate)) throw std::runtime_error("Symlink in installation: " + name);
    }
    return candidate;
}
inline void verifyInstallation(
    const fs::path& root,
    const std::function<void(size_t, size_t, const std::string&)>& progress = {},
    const std::function<bool()>& cancelled = {}) {
    auto manifest = readJson(root / "installation.json");
    if (manifest.at("schema") != 1 || manifest.at("asset_version") != assetVersion ||
        manifest.at("disc_sha256") != discFingerprint)
        throw std::runtime_error("Assets need reinstalling with this version of ProjectCane");
    auto& files = manifest.at("files");
    if (!files.is_array() || files.empty())
        throw std::runtime_error("Installation inventory is empty");
    size_t index = 0;
    for (const auto& entry : files) {
        if (cancelled && cancelled()) throw std::runtime_error("Cancelled");
        auto name = entry.at("path").get<std::string>();
        auto file = safeAssetPath(root, name);
        if (!fs::is_regular_file(file) || fs::file_size(file) != entry.at("size").get<uint64_t>() ||
            hashFile(file, cancelled) != entry.at("sha256").get<std::string>())
            throw std::runtime_error("Missing or damaged asset: " + name);
        if (progress) progress(++index, files.size(), name);
    }
}
inline json makeManifest(
    const fs::path& root, const fs::path& iso,
    const std::function<void(size_t, size_t, const std::string&)>& progress = {}) {
    std::vector<fs::path> files;
    for (const char* directory : {"Worlds", "Cutscenes"})
        for (auto& e : fs::recursive_directory_iterator(root / directory))
            if (e.is_regular_file()) files.push_back(e.path());
    std::sort(files.begin(), files.end());
    json manifest = {{"schema", 1},
                     {"asset_version", assetVersion},
                     {"extractor_version", "0.1.0"},
                     {"disc", "SCUS_971.98"},
                     {"disc_sha256", discFingerprint},
                     {"iso_path", utf8(fs::absolute(iso))},
                     {"files", json::array()}};
    size_t levels = 0, movies = 0, banks = 0, music = 0, index = 0;
    for (auto& file : files) {
        auto name = utf8(fs::relative(file, root));
        manifest["files"].push_back(
            {{"path", name}, {"size", fs::file_size(file)}, {"sha256", hashFile(file)}});
        if (file.extension() == ".brx") ++levels;
        if (file.extension() == ".m2v") {
            ++movies;
            if (!fs::is_regular_file(fs::path(file).replace_extension(".wav")))
                throw std::runtime_error("Missing cutscene audio: " + name);
        }
        if (file.extension() == ".bnk") ++banks;
        if (file.extension() == ".mus") ++music;
        if (progress) progress(++index, files.size(), name);
    }
    if (levels != 46 || movies != 18 || banks != 7 || music != 38)
        throw std::runtime_error(
            "Incomplete extraction: expected 46 levels, 18 video/audio pairs, 7 sound banks and 38 "
            "music groups");
    manifest["inventory"] = {
        {"levels", levels}, {"cutscenes", movies}, {"sound_banks", banks}, {"music_groups", music}};
    return manifest;
}
struct UserPaths {
    fs::path data, user, config, logs;
    bool portable = false;
};
inline fs::path environmentDirectory(const char* key, const fs::path& fallback) {
#ifdef _WIN32
    auto wideKey = path(key).wstring();
    auto value = _wgetenv(wideKey.c_str());
    return value && *value ? fs::path(value) : fallback;
#else
    auto value = std::getenv(key);
    return value && *value ? path(value) : fallback;
#endif
}
inline UserPaths userPaths(const fs::path& executableDir = executableDirectory()) {
    for (const auto& root : {executableDir.parent_path(), executableDir}) {
        if (fs::is_regular_file(root / "portable.txt")) {
            auto base = root / "data";
            return {base / "installations" / "sly1", base / "user", base / "config", base / "logs",
                    true};
        }
    }
#ifdef _WIN32
    auto base = environmentDirectory("LOCALAPPDATA", userDirectory()) / "ProjectCane";
    return {base / "installations" / "sly1", base / "user", base / "config", base / "logs"};
#else
    auto home = environmentDirectory("HOME", userDirectory());
    auto base = environmentDirectory("XDG_DATA_HOME", home / ".local/share") / "projectcane";
    auto config = environmentDirectory("XDG_CONFIG_HOME", home / ".config") / "projectcane";
    return {base / "installations" / "sly1", config, config, config / "logs"};
#endif
}
// Large source builds and runtime artifacts remain alongside the game data.
inline fs::path runtimeStorage(const UserPaths& locations) {
    return locations.data.parent_path().parent_path() / "user";
}
inline fs::path legacyLogDirectory(const UserPaths& locations) {
    if (locations.portable) return locations.logs;
#ifdef _WIN32
    return locations.logs;
#else
    auto home = environmentDirectory("HOME", userDirectory());
    return environmentDirectory("XDG_STATE_HOME", home / ".local/state") / "projectcane/logs";
#endif
}
// Stage imports before activation, leaving the original and any newer profile intact.
inline bool importDirectoryIfEmpty(const fs::path& source, const fs::path& destination) {
    if (source == destination || !fs::is_directory(source) ||
        (fs::exists(destination) && !fs::is_empty(destination)))
        return false;
    InstallationTransaction transaction(destination);
    if (fs::exists(destination) && !fs::is_empty(destination)) return false;
    fs::copy(source, transaction.directory(), fs::copy_options::recursive);
    transaction.commit();
    return true;
}
}  // namespace cane
