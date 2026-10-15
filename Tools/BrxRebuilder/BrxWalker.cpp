#include "BrxWalker.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>

BrxWalker::BrxWalker(StringEncoding encoding) : encoding_(encoding) {}

void BrxWalker::WalkFile(const std::filesystem::path& path) {
    stats_ = {};
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
    stream.U8Read();
    RecordReplacement(debugFlagBegin, stream.GetOffset(), {});
    ConvertSoundBank(stream);
    const auto nameTableBegin = stream.GetOffset();
    stream.U32Read();
    RecordReplacement(nameTableBegin, stream.GetOffset(), {});
    LoadSwSpliceFromBrx(stream); LoadOptionsFromBrx(stream);
    ConvertAudioDescriptor(stream); stream.ReadStringSw(); ConvertMusicDescriptor(stream);
    LoadShadersFromBrx(stream); LoadSwObjectsFromBrx(stream);
    const auto alignmentBegin = stream.GetOffset();
    stream.Align(16);
    RecordAlignment(alignmentBegin, stream.GetOffset(), 16);
    LoadTexturesFromBrx(stream);
    Trace("LoadSwFromBrx",begin,stream.GetOffset());
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
        if (sector == startupSplashSector && identity == 0x344) {
            cutscene = 17;
        } else {
            const auto match = std::find(identities.begin(), identities.end() - 1,
                identity);
            if (match != identities.end() - 1)
                cutscene = static_cast<std::size_t>(match - identities.begin());
        }
        if (cutscene == identities.size())
            throw std::runtime_error("unknown cutscene descriptor at offset " +
                std::to_string(begin));

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
        const auto match = std::find_if(std::begin(levels), std::end(levels),
            [searchKey](const LevelName& level) {
                return level.searchKey == searchKey;
            });
        if (match == std::end(levels))
            throw std::runtime_error("unknown level search key " +
                std::to_string(searchKey) + " at offset " + std::to_string(begin));
        const std::size_t length = std::strlen(match->filename);
        if (length >= filename.size())
            throw std::runtime_error("level filename does not fit the fixed 32-byte field");
        std::memcpy(filename.data(), match->filename, length);
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
