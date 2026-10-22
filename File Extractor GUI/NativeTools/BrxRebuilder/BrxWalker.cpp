#include "BrxWalker.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

BrxWalker::BrxWalker(StringEncoding encoding, BrxSourceRegion region)
    : encoding_(encoding), region_(region) {}

void BrxWalker::WalkFile(const std::filesystem::path& path) {
    WalkFileInternal(path);
}

void BrxWalker::LoadLevelIdentityMap(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("could not open PAL level identity map " + path.string());
    std::uint32_t identity = 0;
    std::string filename;
    while (input >> identity >> std::quoted(filename)) {
        const auto [iterator, inserted] = levelIdentityMap_.emplace(identity, filename);
        if (!inserted && iterator->second != filename)
            throw std::runtime_error("conflicting PAL level identity " + std::to_string(identity));
    }
    if (!input.eof()) throw std::runtime_error("invalid PAL level identity map " + path.string());
    if (levelIdentityMap_.empty()) throw std::runtime_error("PAL level identity map is empty");
}

void BrxWalker::WalkFileInternal(const std::filesystem::path& path) {
    stats_ = {};
    pal_ = false;
    palLocalizationStrings_.clear();
    referenceLocalizationStrings_.clear();
    rebuildOperations_.clear();
    CBinaryInputStream stream(path,encoding_);
    const auto begin=stream.GetOffset(); stream.S32Read(); stream.S32Read();
    RecordReplacement(begin, stream.GetOffset(), {
        'P', 'C', 'B', 'R',
        17, 0, 0, 0
    });
    Trace("CTransition::Execute header",begin,stream.GetOffset());
    LoadSwFromBrx(stream);
    if(stream.GetOffset()!=stream.GetSize())
        throw std::runtime_error("BRX walk ended before EOF at offset "+
            std::to_string(stream.GetOffset())+" of "+std::to_string(stream.GetSize()));
    const auto outputPath = WriteRebuiltFile(path);
    std::wcout << L"Rebuilt file: " << outputPath.wstring() << L'\n';
}

void BrxWalker::LoadSwFromBrx(CBinaryInputStream& stream) {
    const auto begin=stream.GetOffset();
    const auto debugFlagBegin = stream.GetOffset();
    const bool debugInfo = stream.U8Read() != 0;
    RecordReplacement(debugFlagBegin, stream.GetOffset(), {});
    ConvertSoundBank(stream);
    const auto nameTableBegin = stream.GetOffset();
    RemoveNameTableFromBrx(stream, debugInfo);
    RecordReplacement(nameTableBegin, stream.GetOffset(), {});
    pal_ = region_ == BrxSourceRegion::Pal || region_ == BrxSourceRegion::Japanese ||
        (region_ == BrxSourceRegion::Auto && HasPalLocalizationTable(stream));
    if (pal_) {
        if (region_ == BrxSourceRegion::Japanese)
            RemoveJapaneseLocalizationTables(stream);
        else
            RemovePalLocalizationTable(stream);
        // PAL's leading localization table is UTF-16, but strings embedded in
        // the remaining BRX object data are mixed: localized text is UTF-16
        // while identifiers and option strings can still be narrow. Detect
        // each subsequent string independently so narrow values such as
        // "back_of_van" do not consume twice their serialized byte length.
        stream.SetStringEncoding(StringEncoding::Auto);
    }
    LoadSwSpliceFromBrx(stream); LoadOptionsFromBrx(stream);
    ConvertAudioDescriptor(stream); ReadAndConvertString(stream); ConvertMusicDescriptor(stream);
    if (pal_) {
        // PAL BRX files carry fourteen additional fixed-size localization
        // references after the music descriptor. The selected PAL level is
        // already language-specific, and PCBRX resolves localized dialogue
        // and ASEG VAG descriptors to filenames while walking the objects, so
        // these retail-only fields are not consumed by LoadSwFromBrx.
        const auto localizationAssetsBegin = stream.GetOffset();
        constexpr std::size_t localizationAssetCount = 14;
        constexpr std::size_t localizationAssetSize = 32;
        if (stream.Remaining() < localizationAssetCount * localizationAssetSize)
            throw std::runtime_error("truncated PAL fixed localization asset table");
        stream.Skip(localizationAssetCount * localizationAssetSize);
        RecordReplacement(localizationAssetsBegin, stream.GetOffset(), {});
    }
    LoadShadersFromBrx(stream); LoadSwObjectsFromBrx(stream);
    const auto alignmentBegin = stream.GetOffset();
    stream.Align(16);
    RecordAlignment(alignmentBegin, stream.GetOffset(), 16);
    LoadTexturesFromBrx(stream);
    Trace("LoadSwFromBrx",begin,stream.GetOffset());
}

std::string BrxWalker::ReadAndConvertString(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    std::string text = stream.ReadStringSw();
    if (!pal_)
        return text;
    if (text.size() > 0xffff)
        throw std::runtime_error("converted PAL UTF-8 string exceeds the BRX string limit");

    std::vector<std::uint8_t> replacement;
    replacement.reserve(sizeof(std::uint16_t) + text.size());
    const auto length = static_cast<std::uint16_t>(text.size());
    replacement.push_back(static_cast<std::uint8_t>(length));
    replacement.push_back(static_cast<std::uint8_t>(length >> 8));
    replacement.insert(replacement.end(), text.begin(), text.end());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
    return text;
}

std::string BrxWalker::ReadAndConvertUtf16String(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    std::string text = stream.ReadUtf16StringSw();
    if (text.size() > 0xffff)
        throw std::runtime_error("converted Japanese UTF-8 string exceeds the BRX string limit");
    const auto length = static_cast<std::uint16_t>(text.size());
    std::vector<std::uint8_t> replacement;
    replacement.reserve(sizeof(length) + text.size());
    replacement.push_back(static_cast<std::uint8_t>(length));
    replacement.push_back(static_cast<std::uint8_t>(length >> 8));
    replacement.insert(replacement.end(), text.begin(), text.end());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
    return text;
}

void BrxWalker::RemoveNameTableFromBrx(CBinaryInputStream& stream, bool debugInfo) {
    const std::uint32_t count = stream.U32Read();
    if (!debugInfo || count <= 1)
        return;

    for (std::uint32_t i = 1; i < count; ++i) {
        const std::uint16_t byteCount = stream.U16Read();
        if (byteCount > stream.Remaining())
            throw std::runtime_error("BRX debug-name table exceeds the input file");
        stream.Skip(byteCount);
    }
}

bool BrxWalker::HasPalLocalizationTable(CBinaryInputStream& stream) {
    const auto saved = stream.GetOffset();
    if (stream.Remaining() < 6)
        return false;

    const std::uint16_t count = stream.U16Read();
    const std::uint16_t characters = stream.U16Read();
    bool plausible = count > 0 && count < 4096 && characters > 0 && characters < 4096 &&
        static_cast<std::size_t>(characters) * 2 <= stream.Remaining();
    const std::uint16_t sample = (std::min)(characters, static_cast<std::uint16_t>(4));
    for (std::uint16_t i = 0; plausible && i < sample; ++i) {
        const std::uint16_t character = stream.U16Read();
        plausible = character >= 0x20 && character != 0xffff;
    }
    stream.Seek(saved);
    return plausible;
}

void BrxWalker::RemovePalLocalizationTable(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    const std::uint16_t count = stream.U16Read();
    palLocalizationStrings_.clear();
    palLocalizationStrings_.reserve(count);
    const StringEncoding previousEncoding = stream.GetStringEncoding();
    stream.SetStringEncoding(StringEncoding::Utf16Le);
    for (std::uint16_t i = 0; i < count; ++i) {
        palLocalizationStrings_.push_back(stream.ReadStringSw());
    }
    stream.SetStringEncoding(previousEncoding);
    RecordReplacement(begin, stream.GetOffset(), {});
}

void BrxWalker::RemoveJapaneseLocalizationTables(CBinaryInputStream& stream) {
    RemovePalLocalizationTable(stream);
    const auto uiStrings = palLocalizationStrings_;
    RemovePalLocalizationTable(stream);
    referenceLocalizationStrings_ = palLocalizationStrings_;
    palLocalizationStrings_ = uiStrings;
    // SCPS stores its Japanese UI table first and its serialized event
    // reference table second. Keep the UI table in PLOC and use the second
    // table only while expanding references into canonical inline strings.
}

void BrxWalker::ConvertLocalizationReference(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    const std::int16_t index = stream.S16Read();
    std::string text;
    if (index >= 0) {
        const auto& strings = IsJapanese() && !referenceLocalizationStrings_.empty()
            ? referenceLocalizationStrings_ : palLocalizationStrings_;
        if (static_cast<std::size_t>(index) >= strings.size())
            throw std::runtime_error("PAL localization index " +
                std::to_string(index) + " is outside the level string table");
        text = strings[static_cast<std::size_t>(index)];
    }
    if (text.size() > 0xffff)
        throw std::runtime_error("PAL localized subtitle exceeds the PCBRX string limit");

    const auto length = static_cast<std::uint16_t>(text.size());
    std::vector<std::uint8_t> replacement;
    replacement.reserve(sizeof(length) + text.size());
    replacement.push_back(static_cast<std::uint8_t>(length));
    replacement.push_back(static_cast<std::uint8_t>(length >> 8));
    replacement.insert(replacement.end(), text.begin(), text.end());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
}

void BrxWalker::ConvertAudioDescriptor(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    std::array<std::uint32_t, 8> words{};
    for (std::uint32_t& word : words)
        word = stream.U32Read();

    std::array<char, 32> filename{};
    const std::uint32_t sector = words[0] ^ words[5];
    const std::uint32_t size = words[1] ^ words[7];
    const std::uint32_t identity = words[2] ^ words[6];
    if (sector != 0 && size != 0) {
        const int length = std::snprintf(filename.data(), filename.size(),
            "VAG_%04X_%06X.wav", identity, sector);
        if (length < 0 || static_cast<std::size_t>(length) >= filename.size())
            throw std::runtime_error("audio filename does not fit the fixed 32-byte field");
    }

    std::vector<std::uint8_t> replacement(filename.size());
    std::memcpy(replacement.data(), filename.data(), filename.size());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
}

void BrxWalker::ConvertCutsceneDescriptor(CBinaryInputStream& stream) {
    constexpr std::uint32_t startupSplashSector = 0x000C8F03;
    constexpr std::array<std::uint32_t, 18> identities = {
        0x340, 0x341, 0x33C, 0x33F, 0x345, 0x346, 0x33D, 0x33E,
        0x347, 0x348, 0x342, 0x343, 0x336, 0x33A, 0x33B, 0x338,
        0x344, 0x344
    };
    constexpr std::array<std::uint32_t, 17> palIdentities = {
        0x11, 0x16, 0x18, 0x13, 0x0D, 0x0A, 0x10, 0x0E,
        0x12, 0x17, 0x07, 0x02, 0x19, 0x0F, 0x0C, 0x09,
        0x08 // special descriptor immediately before the ordinary table
    };
    constexpr std::array<std::uint32_t, 18> japaneseIdentities = {
        0x10, 0x16, 0x0C, 0x09, 0x0F, 0x0D, 0x13, 0x17,
        0x07, 0x02, 0x1A, 0x0E, 0x18, 0x0B, 0x11, 0x19,
        0x12, // special descriptor at table - 0x20
        0x08  // play-once startup descriptor at table - 0x40
    };

    const auto begin = stream.GetOffset();
    std::array<std::uint32_t, 8> words{};
    bool present = false;
    for (std::uint32_t& word : words) {
        word = stream.U32Read();
        present |= word != 0;
    }

    std::array<char, 32> filename{};
    if (present) {
        const std::uint32_t sector = words[0] ^ words[5];
        const std::uint32_t identity = words[2] ^ words[6];
        std::size_t cutscene = identities.size();
        if (region_ == BrxSourceRegion::Japanese) {
            const auto match = std::find(japaneseIdentities.begin(),
                japaneseIdentities.end(), identity);
            if (match != japaneseIdentities.end())
                cutscene = static_cast<std::size_t>(match - japaneseIdentities.begin());
        } else if (pal_) {
            // PAL uses non-sequential identities. Preserve the executable's
            // exact table order, which is also the extractor's output order.
            const auto match = std::find(palIdentities.begin(), palIdentities.end(), identity);
            if (match != palIdentities.end())
                cutscene = static_cast<std::size_t>(match - palIdentities.begin());
        } else if (sector == startupSplashSector && identity == 0x344) {
            cutscene = 17;
        } else {
            const auto match = std::find(identities.begin(), identities.end() - 1,
                identity);
            if (match != identities.end() - 1)
                cutscene = static_cast<std::size_t>(match - identities.begin());
        }
        if (cutscene == identities.size()) {
            std::ostringstream error;
            error << "unknown " << (region_ == BrxSourceRegion::Japanese
                    ? "Japanese" : pal_ ? "PAL" : "NTSC")
                  << " cutscene descriptor identity=0x" << std::hex << identity
                  << " sector=0x" << sector << " at offset 0x" << begin;
            throw std::runtime_error(error.str());
        }

        const int length = std::snprintf(filename.data(), filename.size(),
            "Cutscene_%02zu.m2v", cutscene);
        if (length < 0 || static_cast<std::size_t>(length) >= filename.size())
            throw std::runtime_error("cutscene filename does not fit the fixed 32-byte field");
    }

    std::vector<std::uint8_t> replacement(filename.size());
    std::memcpy(replacement.data(), filename.data(), filename.size());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
}

void BrxWalker::ConvertLevelDescriptor(CBinaryInputStream& stream) {
    struct LevelName {
        std::uint32_t searchKey;
        const char* filename;
    };
    constexpr LevelName levels[] = {
        {21, "Splash.brx"}, {10, "Paris.brx"}, {9, "The Hideout.brx"},
        {37, "A Stealthy Approach.brx"}, {38, "Prowling the Grounds.brx"},
        {33, "High Class Heist.brx"}, {36, "Into the Machine.brx"},
        {32, "A Cunning Disguise.brx"}, {31, "The Fire Down Below.brx"},
        {39, "Treasure in the Depths.brx"}, {40, "The Gunboat Graveyard.brx"},
        {34, "The Eye of the Storm.brx"}, {12, "A Rocky Start.brx"},
        {15, "Muggshot's Turf.brx"}, {14, "Boneyard Casino.brx"},
        {18, "Murray's Big Gamble.brx"}, {19, "At the Dog Track.brx"},
        {16, "Two to Tango.brx"}, {17, "Straight to the Top.brx"},
        {20, "Back Alley Heist.brx"}, {13, "Last Call.brx"},
        {41, "The Dread Swamp Path.brx"}, {45, "The Swamp's Dark Center.brx"},
        {49, "The Lair of the Beast.brx"}, {44, "A Grave Undertaking.brx"},
        {47, "Piranha Lake.brx"}, {48, "Descent into Danger.brx"},
        {46, "A Ghastly Voyage.brx"}, {43, "Down Home Cooking.brx"},
        {42, "A Deadly Dance.brx"}, {22, "A Perilous Ascent.brx"},
        {25, "Inside the Stronghold.brx"}, {27, "Flaming Temple of Flame.brx"},
        {23, "The Unseen Foe.brx"}, {28, "The King of the Hill.brx"},
        {30, "Rapid Fire Assault.brx"}, {26, "Duel by the Dragon.brx"},
        {29, "A Desperate Race.brx"}, {24, "Flame Fu!.brx"},
        {8, "A Hazardous Path.brx"}, {7, "Burning Rubber.brx"},
        {6, "A Daring Rescue.brx"}, {2, "Bentley Comes Through.brx"},
        {5, "A Temporary Truce.brx"}, {4, "Sinking Peril.brx"},
        {3, "A Strange Reunion.brx"}
    };
    constexpr std::array<const char*, 46> palLevels = {
        "Splash.brx", "Paris.brx", "The Hideout.brx",
        "A Stealthy Approach.brx", "Prowling the Grounds.brx", "High Class Heist.brx",
        "Into the Machine.brx", "A Cunning Disguise.brx", "The Fire Down Below.brx",
        "Treasure in the Depths.brx", "The Gunboat Graveyard.brx", "The Eye of the Storm.brx",
        "A Rocky Start.brx", "Muggshot's Turf.brx", "Boneyard Casino.brx",
        "Murray's Big Gamble.brx", "At the Dog Track.brx", "Two to Tango.brx",
        "Straight to the Top.brx", "Back Alley Heist.brx", "Last Call.brx",
        "The Dread Swamp Path.brx", "The Swamp's Dark Center.brx", "The Lair of the Beast.brx",
        "A Grave Undertaking.brx", "Piranha Lake.brx", "Descent into Danger.brx",
        "A Ghastly Voyage.brx", "Down Home Cooking.brx", "A Deadly Dance.brx",
        "A Perilous Ascent.brx", "Inside the Stronghold.brx", "Flaming Temple of Flame.brx",
        "The Unseen Foe.brx", "The King of the Hill.brx", "Rapid Fire Assault.brx",
        "Duel by the Dragon.brx", "A Desperate Race.brx", "Flame Fu!.brx",
        "A Hazardous Path.brx", "Burning Rubber.brx", "A Daring Rescue.brx",
        "Bentley Comes Through.brx", "A Temporary Truce.brx", "Sinking Peril.brx",
        "A Strange Reunion.brx"
    };

    // SCPS keeps one level-table record per level, with separate Japanese and
    // English asset descriptors in that record. Serialized level references
    // use the descriptor identity (not the record's sequential search key).
    // The identities are not level-order sequential after the first six
    // entries, so an arithmetic conversion silently routes transitions to the
    // wrong world (for example, Japanese Paris identity 31 became index 3).
    constexpr std::array<std::uint32_t, 46> japaneseLevelIdentities = {
        29, 31, 33, 35, 37, 39, 49, 43, 41, 47, 45, 51,
        53, 55, 57, 65, 67, 59, 61, 63, 69, 71, 73, 77,
        75, 83, 79, 81, 85, 87, 89, 91, 97, 93, 99, 103,
        95, 101, 105, 107, 109, 111, 113, 115, 117, 119
    };
    constexpr std::array<std::uint32_t, 46> englishLevelIdentities = {
        28, 30, 32, 34, 36, 38, 48, 42, 40, 46, 44, 50,
        52, 54, 56, 64, 66, 58, 60, 62, 68, 70, 72, 76,
        74, 82, 78, 80, 84, 86, 88, 90, 96, 92, 98, 102,
        94, 100, 104, 106, 108, 110, 112, 114, 116, 118
    };

    const auto begin = stream.GetOffset();
    std::array<std::uint32_t, 8> words{};
    bool present = false;
    for (std::uint32_t& word : words) {
        word = stream.U32Read();
        present |= word != 0;
    }

    std::array<char, 32> filename{};
    if (present) {
        const std::uint32_t searchKey = words[2] ^ words[6];
        const char* selectedFilename = nullptr;
        if (region_ == BrxSourceRegion::Japanese) {
            for (std::size_t level = 0; level < palLevels.size(); ++level) {
                if (searchKey == japaneseLevelIdentities[level] ||
                    searchKey == englishLevelIdentities[level]) {
                    selectedFilename = palLevels[level];
                    break;
                }
            }
        } else if (pal_ && !levelIdentityMap_.empty()) {
            const auto match = levelIdentityMap_.find(searchKey);
            if (match != levelIdentityMap_.end()) selectedFilename = match->second.c_str();
        } else if (pal_ && searchKey >= 27 && searchKey < 27 + palLevels.size() * 5) {
            // Compatibility fallback for direct/manual use without an ISO
            // identity map. The extractor always supplies the authoritative
            // mapping because PAL descriptor identities are not level-order.
            selectedFilename = palLevels[(searchKey - 27) / 5];
        } else {
            const auto match = std::find_if(std::begin(levels), std::end(levels),
                [searchKey](const LevelName& level) {
                    return level.searchKey == searchKey;
                });
            if (match != std::end(levels))
                selectedFilename = match->filename;
        }
        if (selectedFilename == nullptr)
            throw std::runtime_error("unknown level search key " +
                std::to_string(searchKey) + " at offset " + std::to_string(begin));
        const std::size_t length = std::strlen(selectedFilename);
        if (length >= filename.size())
            throw std::runtime_error("level filename does not fit the fixed 32-byte field");
        std::memcpy(filename.data(), selectedFilename, length);
    }

    std::vector<std::uint8_t> replacement(filename.size());
    std::memcpy(replacement.data(), filename.data(), filename.size());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
}

void BrxWalker::ConvertMusicDescriptor(CBinaryInputStream& stream) {
    const auto begin = stream.GetOffset();
    std::array<std::uint32_t, 8> words{};
    for (std::uint32_t& word : words)
        word = stream.U32Read();

    std::array<char, 32> filename{};
    const std::uint32_t size = words[1] ^ words[7];
    const std::uint32_t identity = words[2] ^ words[6];
    if (size != 0) {
        const int length = std::snprintf(filename.data(), filename.size(),
            "MUSID_%02X.mus", identity);
        if (length < 0 || static_cast<std::size_t>(length) >= filename.size())
            throw std::runtime_error("music filename does not fit the fixed 32-byte field");
    }

    std::vector<std::uint8_t> replacement(filename.size());
    std::memcpy(replacement.data(), filename.data(), filename.size());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
}

void BrxWalker::ConvertSoundBank(CBinaryInputStream& stream) {
    constexpr std::array<const char*, 7> bankNames = {
        "Common.bnk", "Intro.bnk", "Underwater.bnk", "Muggshot.bnk",
        "Voodoo.bnk", "Snow.bnk", "Clockwerk.bnk"
    };

    const auto begin = stream.GetOffset();
    const std::int16_t bank = stream.S16Read();
    std::array<char, 32> filename{};
    if (bank != -1) {
        if (bank < 0 || bank >= static_cast<std::int16_t>(bankNames.size()))
            throw std::runtime_error("unknown sound-bank index " +
                std::to_string(bank) + " at offset " + std::to_string(begin));
        const std::size_t length = std::strlen(bankNames[bank]);
        std::memcpy(filename.data(), bankNames[bank], length);
    }

    std::vector<std::uint8_t> replacement(filename.size());
    std::memcpy(replacement.data(), filename.data(), filename.size());
    RecordReplacement(begin, stream.GetOffset(), std::move(replacement));
}

void BrxWalker::RecordAlignment(std::size_t begin, std::size_t end,
    std::size_t alignment) {
    rebuildOperations_.push_back({begin, end, alignment, {}});
}

void BrxWalker::RecordReplacement(std::size_t begin, std::size_t end,
    std::vector<std::uint8_t> replacement) {
    rebuildOperations_.push_back({begin, end, 0, std::move(replacement)});
}

std::filesystem::path BrxWalker::WriteRebuiltFile(
    const std::filesystem::path& sourcePath) {
    std::ifstream input(sourcePath, std::ios::binary);
    if (!input)
        throw std::runtime_error("could not reopen source BRX for rebuilding");
    input.seekg(0, std::ios::end);
    const auto fileSize = static_cast<std::size_t>(input.tellg());
    input.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> source(fileSize);
    if (fileSize != 0)
        input.read(reinterpret_cast<char*>(source.data()), static_cast<std::streamsize>(fileSize));
    if (!input && fileSize != 0)
        throw std::runtime_error("could not read complete source BRX for rebuilding");

    std::sort(rebuildOperations_.begin(), rebuildOperations_.end(),
        [](const RebuildOperation& left, const RebuildOperation& right) {
            return left.begin < right.begin;
        });

    std::vector<std::uint8_t> rebuilt;
    rebuilt.reserve(source.size());
    std::size_t sourceCursor = 0;
    for (const RebuildOperation& operation : rebuildOperations_) {
        if (operation.begin < sourceCursor || operation.end < operation.begin ||
            operation.end > source.size())
            throw std::runtime_error("overlapping or invalid rebuild operation");
        rebuilt.insert(rebuilt.end(), source.begin() + sourceCursor,
            source.begin() + operation.begin);
        sourceCursor = operation.end;
        if (operation.alignment != 0) {
            const auto remainder = rebuilt.size() % operation.alignment;
            if (remainder != 0)
                rebuilt.insert(rebuilt.end(), operation.alignment - remainder, 0);
        } else {
            rebuilt.insert(rebuilt.end(), operation.replacement.begin(),
                operation.replacement.end());
        }
    }
    rebuilt.insert(rebuilt.end(), source.begin() + sourceCursor, source.end());

    // Keep PAL's per-level UI/subtitle table available to the PC runtime.
    if (pal_) {
        std::vector<std::uint8_t> localization;
        const auto appendU32 = [&localization](std::uint32_t value) {
            for (int shift = 0; shift < 32; shift += 8)
                localization.push_back(static_cast<std::uint8_t>(value >> shift));
        };
        appendU32(static_cast<std::uint32_t>(palLocalizationStrings_.size()));
        for (const std::string& text : palLocalizationStrings_) {
            appendU32(static_cast<std::uint32_t>(text.size()));
            localization.insert(localization.end(), text.begin(), text.end());
        }
        rebuilt.insert(rebuilt.end(), localization.begin(), localization.end());
        constexpr std::uint32_t localizationMagic = 0x434f4c50; // "PLOC"
        const auto appendRebuiltU32 = [&rebuilt](std::uint32_t value) {
            for (int shift = 0; shift < 32; shift += 8)
                rebuilt.push_back(static_cast<std::uint8_t>(value >> shift));
        };
        appendRebuiltU32(localizationMagic);
        appendRebuiltU32(static_cast<std::uint32_t>(localization.size()));
    }

    auto outputPath = sourcePath;
    outputPath.replace_filename(sourcePath.stem().wstring() + L"_Rebuilt.brx");
    std::ofstream output(outputPath, std::ios::binary | std::ios::trunc);
    if (!output)
        throw std::runtime_error("could not create rebuilt BRX output");
    if (!rebuilt.empty())
        output.write(reinterpret_cast<const char*>(rebuilt.data()),
            static_cast<std::streamsize>(rebuilt.size()));
    if (!output)
        throw std::runtime_error("failed while writing rebuilt BRX output");
    return outputPath;
}

void BrxWalker::Trace(const char* name,std::size_t begin,std::size_t end) const {
    std::cout<<std::left<<std::setw(36)<<name<<"  0x"<<std::hex<<begin
        <<"..0x"<<end<<std::dec<<" ("<<(end-begin)<<" bytes)\n";
}
