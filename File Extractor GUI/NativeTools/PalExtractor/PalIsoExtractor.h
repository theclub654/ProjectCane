#pragma once
#include <cstddef>
#include <filesystem>
#include <functional>
#include <string>
#include <utility>

namespace cane::pal
{
enum class DiscRegion { NtscU, Pal, Japanese };
struct ExtractionProgress {
    std::string stage;
    std::filesystem::path file;
    std::size_t completed = 0, total = 0;
};

enum class PalLanguageSlot : std::size_t { English, French, German, Spanish, Italian };

class PalIsoExtractor {
public:
    using ProgressCallback = std::function<void(const ExtractionProgress&)>;
    explicit PalIsoExtractor(ProgressCallback progress = {}) : progress_(std::move(progress)) {}
    static DiscRegion DetectRegion(const std::filesystem::path& isoPath);
    void Extract(const std::filesystem::path& isoPath,
        const std::filesystem::path& outputDirectory,
        PalLanguageSlot language) const;
    void ExtractAllLanguages(const std::filesystem::path& isoPath,
        const std::filesystem::path& outputDirectory) const;
    void ExtractDetectedRegion(const std::filesystem::path& isoPath,
        const std::filesystem::path& outputDirectory) const;
private:
    ProgressCallback progress_;
};

}
