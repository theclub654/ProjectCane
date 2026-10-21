#pragma once
#include <cstddef>
#include <filesystem>

enum class PalLanguageSlot : std::size_t { English, French, German, Spanish, Italian };

class PalIsoExtractor {
public:
    void Extract(const std::filesystem::path& isoPath,
        const std::filesystem::path& outputDirectory,
        PalLanguageSlot language) const;
    void ExtractAllLanguages(const std::filesystem::path& isoPath,
        const std::filesystem::path& outputDirectory) const;
    void ExtractDetectedRegion(const std::filesystem::path& isoPath,
        const std::filesystem::path& outputDirectory) const;
};
