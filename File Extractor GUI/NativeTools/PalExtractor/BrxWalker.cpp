#include "BrxWalker.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <stdexcept>

BrxWalker::BrxWalker(StringEncoding encoding, BrxRegion region)
    : encoding_(encoding), region_(region) {}

void BrxWalker::WalkFile(const std::filesystem::path& path) {
    pal_ = false;
    audioDescriptors_.clear();
    commentaryDescriptors_.clear();
    musicDescriptors_.clear();
    CBinaryInputStream stream(path,encoding_);
    const auto begin=stream.GetOffset(); stream.S32Read(); stream.S32Read();
    Trace("CTransition::Execute header",begin,stream.GetOffset());
    LoadSwFromBrx(stream);
    if(stream.GetOffset()!=stream.GetSize())
        throw std::runtime_error("BRX walk ended before EOF at offset "+
            std::to_string(stream.GetOffset())+" of "+std::to_string(stream.GetSize()));
}

void BrxWalker::LoadSwFromBrx(CBinaryInputStream& stream) {
    const auto begin=stream.GetOffset();
    const bool debugInfo = stream.U8Read() != 0;
    stream.S16Read();
    LoadNameTableFromBrx(stream, debugInfo);
    pal_ = region_ == BrxRegion::Pal || region_ == BrxRegion::Japanese ||
        (region_ == BrxRegion::Auto && HasPalLocalizationTable(stream));
    if (region_ == BrxRegion::Japanese) LoadJapaneseLocalizationTable(stream);
    else if (pal_) LoadPalLocalizationTable(stream);
    LoadSwSpliceFromBrx(stream); LoadOptionsFromBrx(stream);
    {
        std::uint32_t words[8]{};
        for (auto& word : words) word = stream.U32Read();
        const AudioDescriptor commentary{
            words[0] ^ words[5], words[1] ^ words[7], words[2] ^ words[6]
        };
        if (commentary.sector != 0 && commentary.size != 0)
            commentaryDescriptors_.push_back(commentary);
    }
    stream.ReadStringSw();
    {
        std::uint32_t words[8]{};
        for (auto& word : words) word = stream.U32Read();
        const AudioDescriptor descriptor{
            words[0] ^ words[5], words[1] ^ words[7], words[2] ^ words[6]
        };
        if (descriptor.sector != 0 && descriptor.size != 0)
            musicDescriptors_.push_back(descriptor);
    }
    if (pal_) {
        const auto localizedAssetsBegin = stream.GetOffset();
        for (int i = 0; i < 14; ++i) stream.ReadFixedString32();
        Trace(region_ == BrxRegion::Japanese ? "Japanese fixed localization assets" :
            "PAL fixed localization assets", localizedAssetsBegin, stream.GetOffset());
    }
    LoadShadersFromBrx(stream); LoadSwObjectsFromBrx(stream);
    stream.Align(16); LoadTexturesFromBrx(stream);
    Trace("LoadSwFromBrx",begin,stream.GetOffset());
}

void BrxWalker::ReadAudioDescriptor(CBinaryInputStream& stream) {
    std::uint32_t words[8]{};
    for (auto& word : words) word = stream.U32Read();
    const AudioDescriptor descriptor{
        words[0] ^ words[5],
        words[1] ^ words[7],
        words[2] ^ words[6]
    };
    if (descriptor.sector == 0 || descriptor.size == 0) return;
    const auto duplicate = std::find_if(audioDescriptors_.begin(), audioDescriptors_.end(),
        [&](const AudioDescriptor& value) {
            return value.sector == descriptor.sector && value.size == descriptor.size &&
                value.identity == descriptor.identity;
        });
    if (duplicate == audioDescriptors_.end()) audioDescriptors_.push_back(descriptor);
}

void BrxWalker::LoadNameTableFromBrx(CBinaryInputStream& stream, bool debugInfo) {
    const auto begin = stream.GetOffset();
    const auto count = stream.U32Read();
    if (debugInfo && count > 1) {
        for (std::uint32_t i = 1; i < count; ++i) {
            const auto byteCount = stream.U16Read();
            stream.Skip(byteCount);
        }
    }
    Trace("LoadNameTableFromBrx", begin, stream.GetOffset());
}

bool BrxWalker::HasPalLocalizationTable(CBinaryInputStream& stream) {
    const auto saved = stream.GetOffset();
    if (stream.Remaining() < 6) return false;
    const auto count = stream.U16Read();
    const auto characters = stream.U16Read();
    bool plausible = count > 0 && count < 4096 && characters > 0 && characters < 4096 &&
        static_cast<std::size_t>(characters) * 2 <= stream.Remaining();
    const auto sample = std::min<std::uint16_t>(characters, 4);
    for (std::uint16_t i = 0; plausible && i < sample; ++i) {
        const auto character = stream.U16Read();
        plausible = character >= 0x20 && character != 0xFFFF;
    }
    stream.Seek(saved);
    return plausible;
}

void BrxWalker::LoadPalLocalizationTable(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    const auto count = stream.U16Read();
    for (std::uint16_t i = 0; i < count; ++i) {
        const auto characters = stream.U16Read();
        if (static_cast<std::size_t>(characters) * 2 > stream.Remaining())
            throw std::runtime_error("PAL UTF-16 string exceeds the file at index " + std::to_string(i));
        stream.Skip(static_cast<std::size_t>(characters) * 2);
    }
    Trace("PAL localization/subtitle table", begin, stream.GetOffset());
}

void BrxWalker::LoadJapaneseLocalizationTable(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    for (int tableIndex = 0; tableIndex < 2; ++tableIndex) {
        const auto count = stream.U16Read();
        if (count >= 4096)
            throw std::runtime_error("Implausible Japanese localization string count " +
                std::to_string(count) + " in table " + std::to_string(tableIndex));
        for (std::uint16_t i = 0; i < count; ++i) {
            const auto characters = stream.U16Read();
            if (static_cast<std::size_t>(characters) * 2 > stream.Remaining())
                throw std::runtime_error("Japanese UTF-16 string exceeds the file at index " +
                    std::to_string(i) + " in table " + std::to_string(tableIndex));
            stream.Skip(static_cast<std::size_t>(characters) * 2);
        }
    }
    Trace("Japanese localization tables", begin, stream.GetOffset());
}

void BrxWalker::Trace(const char* name,std::size_t begin,std::size_t end) const {
    std::cout<<std::left<<std::setw(36)<<name<<"  0x"<<std::hex<<begin
        <<"..0x"<<end<<std::dec<<" ("<<(end-begin)<<" bytes)\n";
}
