#pragma once
#include <filesystem>
#include <fcntl.h>
#include <unistd.h>

namespace CaneIo
{
inline bool AtomicReplace(const std::filesystem::path& temporary, const std::filesystem::path& path)
{
    const int descriptor = open(temporary.c_str(), O_RDONLY);
    if (descriptor >= 0) { fsync(descriptor); close(descriptor); }
    std::error_code error;
    std::filesystem::rename(temporary, path, error);

    if (error && std::filesystem::is_regular_file(path))
    {
        const std::filesystem::path backup = path.string() + ".previous";
        std::filesystem::remove(backup, error);
        error.clear();
        std::filesystem::rename(path, backup, error);
        if (error) return false;
        std::filesystem::rename(temporary, path, error);
        if (error)
        {
            std::error_code rollback;
            std::filesystem::rename(backup, path, rollback);
            return false;
        }
        std::filesystem::remove(backup, error);
        error.clear();
    }
    return !error;
}

inline std::filesystem::path RecoverFile(const std::filesystem::path& path)
{
    std::error_code error;
    const std::filesystem::path backup = path.string() + ".previous";
    if (!std::filesystem::is_regular_file(path, error) && std::filesystem::is_regular_file(backup, error))
        std::filesystem::rename(backup, path, error);
    return path;
}
}
