#pragma once

#include "BinaryInputStream.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

struct GeometrySummary { std::uint16_t positions{}, surfaces{}, edges{}; };
struct WalkStats { std::uint64_t objects{}, options{}, geometries{}, bspNodes{}; };
struct AudioDescriptor {
    std::uint32_t sector{}, size{}, identity{};
};

enum class BrxRegion { Auto, Pal, Japanese };

class BrxWalker {
public:
    explicit BrxWalker(StringEncoding encoding, BrxRegion region = BrxRegion::Auto);
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
    void ReadAudioDescriptor(CBinaryInputStream& stream);
    [[nodiscard]] const std::vector<AudioDescriptor>& AudioDescriptors() const { return audioDescriptors_; }
    [[nodiscard]] const std::vector<AudioDescriptor>& CommentaryDescriptors() const { return commentaryDescriptors_; }
    [[nodiscard]] const std::vector<AudioDescriptor>& MusicDescriptors() const { return musicDescriptors_; }

private:
    std::size_t lastOptionListStart_{};
    std::size_t lastOptionListEnd_{};
    std::size_t lastOptionTerminator_{};
    std::size_t lastOptionStart_{};
    std::size_t lastOptionEnd_{};
    std::int16_t lastOptionId_{-1};
    bool haveLastOption_{};
    void LoadSwFromBrx(CBinaryInputStream& stream);
    void LoadNameTableFromBrx(CBinaryInputStream& stream, bool debugInfo);
    bool HasPalLocalizationTable(CBinaryInputStream& stream);
    void LoadPalLocalizationTable(CBinaryInputStream& stream);
    void LoadJapaneseLocalizationTable(CBinaryInputStream& stream);
    void DispatchObject(CBinaryInputStream& stream, std::int16_t cid,
        std::int16_t oid, std::int16_t splice, unsigned depth);
    void LoadLo(CBinaryInputStream& stream);
    void LoadAlo(CBinaryInputStream& stream, unsigned depth);
    void LoadSo(CBinaryInputStream& stream, unsigned depth);
    std::uint8_t LoadGlobset(CBinaryInputStream& stream);
    void LoadAlox(CBinaryInputStream& stream);
    void Trace(const char* name, std::size_t begin, std::size_t end) const;
    StringEncoding encoding_;
    BrxRegion region_{};
    bool pal_{};
    WalkStats stats_{};
    struct DataBlock { std::uint32_t offset{}, size{}; };
    std::vector<DataBlock> clutBlocks_;
    std::vector<DataBlock> bitmapBlocks_;
    std::vector<AudioDescriptor> audioDescriptors_;
    std::vector<AudioDescriptor> commentaryDescriptors_;
    std::vector<AudioDescriptor> musicDescriptors_;
    bool captureEmitOrigin_{};
    bool emitOriginCaptured_{};
    std::int16_t emitOriginKind_{};
};
