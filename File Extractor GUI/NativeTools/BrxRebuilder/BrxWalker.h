#pragma once

#include "BinaryInputStream.h"

#include <cstddef>
#include <cstdint>
#include <array>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <vector>

struct GeometrySummary { std::uint16_t positions{}, surfaces{}, edges{}; };
struct WalkStats {
    std::uint64_t objects{};
    std::uint64_t options{};
    std::uint64_t geometries{};
    std::uint64_t bspNodes{};
    std::uint64_t stripVertices{};
    std::uint64_t triangles{};
};

enum class BrxSourceRegion { Auto, NtscU, Pal, Japanese };

class BrxWalker {
public:
    explicit BrxWalker(StringEncoding encoding, BrxSourceRegion region = BrxSourceRegion::Auto);
    void LoadLevelIdentityMap(const std::filesystem::path& path);
    void WalkFile(const std::filesystem::path& path);
    [[nodiscard]] const WalkStats& Stats() const { return stats_; }
    GeometrySummary ReadGeom(CBinaryInputStream& stream);
    void ReadBspc(CBinaryInputStream& stream, const GeometrySummary& geometry);
    void LoadOptionsFromBrx(CBinaryInputStream& stream);
    void LoadSwSpliceFromBrx(CBinaryInputStream& stream, bool debugInfo = false);
    void ReadSerializedPair(CBinaryInputStream& stream, bool debugInfo, unsigned depth = 0);
    void LoadShadersFromBrx(CBinaryInputStream& stream);
    void LoadTexturesFromBrx(CBinaryInputStream& stream);
    void LoadSwObjectsFromBrx(CBinaryInputStream& stream, unsigned depth = 0);
    void ConvertAudioDescriptor(CBinaryInputStream& stream);
    void ConvertCutsceneDescriptor(CBinaryInputStream& stream);
    void ConvertLevelDescriptor(CBinaryInputStream& stream);
    void ConvertMusicDescriptor(CBinaryInputStream& stream);
    void ConvertSoundBank(CBinaryInputStream& stream);
    std::string ReadAndConvertString(CBinaryInputStream& stream);
    std::string ReadAndConvertUtf16String(CBinaryInputStream& stream);
    void ConvertLocalizationReference(CBinaryInputStream& stream);
    [[nodiscard]] bool IsPal() const { return pal_; }
    [[nodiscard]] bool IsJapanese() const { return region_ == BrxSourceRegion::Japanese; }

private:
    void WalkFileInternal(const std::filesystem::path& path);
    struct RebuildOperation {
        std::size_t begin{};
        std::size_t end{};
        std::size_t alignment{};
        std::vector<std::uint8_t> replacement;
    };
    void RecordAlignment(std::size_t begin, std::size_t end, std::size_t alignment);
    void RecordReplacement(std::size_t begin, std::size_t end,
        std::vector<std::uint8_t> replacement);
    std::filesystem::path WriteRebuiltFile(const std::filesystem::path& sourcePath);
    std::size_t lastOptionListStart_{};
    std::size_t lastOptionListEnd_{};
    std::size_t lastOptionTerminator_{};
    std::size_t lastOptionStart_{};
    std::size_t lastOptionEnd_{};
    std::int16_t lastOptionId_{-1};
    bool haveLastOption_{};
    void LoadSwFromBrx(CBinaryInputStream& stream);
    void RemoveNameTableFromBrx(CBinaryInputStream& stream, bool debugInfo);
    bool HasPalLocalizationTable(CBinaryInputStream& stream);
    void RemovePalLocalizationTable(CBinaryInputStream& stream);
    void RemoveJapaneseLocalizationTables(CBinaryInputStream& stream);
    void DispatchObject(CBinaryInputStream& stream, std::int16_t cid,
        std::int16_t oid, std::int16_t splice, unsigned depth);
    void LoadLo(CBinaryInputStream& stream);
    void LoadAlo(CBinaryInputStream& stream, unsigned depth);
    void LoadSo(CBinaryInputStream& stream, unsigned depth);
    void ReadEmitColors(CBinaryInputStream& stream, std::uint16_t count);
    std::uint8_t LoadGlobset(CBinaryInputStream& stream);
    void LoadAlox(CBinaryInputStream& stream);
    void Trace(const char* name, std::size_t begin, std::size_t end) const;
    StringEncoding encoding_;
    BrxSourceRegion region_;
    bool pal_{};
    std::unordered_map<std::uint32_t, std::string> levelIdentityMap_;
    std::vector<std::string> palLocalizationStrings_;
    std::vector<std::string> referenceLocalizationStrings_;
    WalkStats stats_{};
    std::vector<RebuildOperation> rebuildOperations_;
    struct SourceClut {
        std::uint32_t grfzon{};
        std::uint16_t colorCount{}, colorSize{};
        std::uint32_t offset{};
    };
    struct SourceBitmap {
        std::uint32_t width{}, height{}, grfzon{}, size{}, offset{};
        std::int8_t psm{}, cgsRow{};
        std::uint16_t cgsPixels{};
    };
    struct PendingTexture {
        std::size_t begin{}, end{};
        std::int16_t oid{}, flags{};
        std::uint8_t shaderKind{};
        std::vector<std::uint16_t> bitmaps;
        std::vector<std::uint16_t> cluts;
    };
    struct PendingFont {
        std::size_t referenceBegin{};
        std::uint16_t bitmap{};
        std::int16_t clut{};
    };
    std::vector<SourceClut> sourceCluts_;
    std::vector<SourceBitmap> sourceBitmaps_;
    std::vector<PendingTexture> pendingTextures_;
    std::vector<PendingFont> pendingFonts_;
    std::size_t textureTablesBegin_{};
    std::size_t textureTablesEnd_{};
    std::vector<std::array<float, 4>> shaderColors_;
    bool captureEmitOrigin_{};
    bool emitOriginCaptured_{};
    std::int16_t emitOriginKind_{};
};
