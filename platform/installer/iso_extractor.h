#pragma once
#include <cstddef>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>

namespace CaneExtract
{
struct Progress
{
    std::string stage;
    std::string file;
    size_t completed = 0;
    size_t total = 0;
};
using ProgressCallback = std::function<bool(const Progress&)>;
class Cancelled : public std::runtime_error
{
public:
    Cancelled() : std::runtime_error("Extraction cancelled") {}
};
bool NeedsRebuild(const std::filesystem::path& directory);
void RebuildGame(const std::filesystem::path& directory, const ProgressCallback& callback = {});
bool IsReady(const std::filesystem::path& directory);
void ExtractGame(const std::filesystem::path& iso, const std::filesystem::path& directory,
    const ProgressCallback& callback = {});
}
