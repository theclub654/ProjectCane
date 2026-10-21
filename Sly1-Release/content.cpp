#include "content.h"

#include "../platform/platform.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <cstring>
#include <utility>
#include <vector>
#include <nlohmann/json.hpp>

namespace
{
struct PendingLanguageChange
{
    bool active = false;
    CONTENT_LANGUAGE language = CONTENT_LANGUAGE::English;
    CONTENT_LANGUAGE preferredLanguage = CONTENT_LANGUAGE::English;
    CONTENT_LANGUAGE voiceLanguage = CONTENT_LANGUAGE::English;
};

PendingLanguageChange s_pendingLanguageChange;
constexpr const char* kManifestName = "extraction_manifest.json";
constexpr const char* kSettingsName = "content_settings.json";
CONTENT_PACKAGE s_contentPackage;
std::vector<std::string> s_localizedText;

const char* LanguageName(CONTENT_LANGUAGE language)
{
    switch (language)
    {
        case CONTENT_LANGUAGE::French: return "French";
        case CONTENT_LANGUAGE::German: return "German";
        case CONTENT_LANGUAGE::Spanish: return "Spanish";
        case CONTENT_LANGUAGE::Italian: return "Italian";
        case CONTENT_LANGUAGE::Japanese: return "Japanese";
        default: return "English";
    }
}
CONTENT_LANGUAGE ParseLanguage(const std::string& value)
{
    for (int index = 0; index <= static_cast<int>(CONTENT_LANGUAGE::Japanese); ++index)
    {
        const auto language = static_cast<CONTENT_LANGUAGE>(index);
        if (value == LanguageName(language))
            return language;
    }
    return CONTENT_LANGUAGE::English;
}

bool TryParseLanguage(const std::string& value, CONTENT_LANGUAGE& language)
{
    for (int index = 0; index <= static_cast<int>(CONTENT_LANGUAGE::Japanese); ++index)
    {
        const auto candidate = static_cast<CONTENT_LANGUAGE>(index);
        if (value == LanguageName(candidate))
        {
            language = candidate;
            return true;
        }
    }
    return false;
}

std::uint32_t LanguageBit(CONTENT_LANGUAGE language)
{
    const int index = static_cast<int>(language);
    return index >= 0 && index <= static_cast<int>(CONTENT_LANGUAGE::Japanese)
        ? 1u << index : 0u;
}

std::filesystem::path SettingsPath()
{
    return cane::UserDirectory() / kSettingsName;
}

CONTENT_LANGUAGE LoadSavedLanguage(const std::string& serial,
    CONTENT_LANGUAGE fallback)
{
    try
    {
        std::ifstream input(SettingsPath());
        if (!input)
            return fallback;
        const nlohmann::json settings = nlohmann::json::parse(input);
        return ParseLanguage(settings.value("languages", nlohmann::json::object())
            .value(serial, std::string(LanguageName(fallback))));
    }
    catch (const std::exception&)
    {
        return fallback;
    }
}

void SaveLanguage(const std::string& serial, CONTENT_LANGUAGE language)
{
    if (serial.empty())
        return;

    nlohmann::json settings = nlohmann::json::object();
    try
    {
        std::ifstream input(SettingsPath());
        if (input)
            settings = nlohmann::json::parse(input);
    }
    catch (const std::exception&)
    {
        settings = nlohmann::json::object();
    }

    settings["languages"][serial] = LanguageName(language);
    const std::filesystem::path temporary = SettingsPath().wstring() + L".tmp";
    std::ofstream output(temporary, std::ios::trunc);
    if (!output)
        return;
    output << settings.dump(2) << '\n';
    output.close();

    std::error_code error;
    std::filesystem::remove(SettingsPath(), error);
    error.clear();
    std::filesystem::rename(temporary, SettingsPath(), error);
}

std::filesystem::path FindManifestAbove(std::filesystem::path path)
{
    std::error_code error;
    if (std::filesystem::is_regular_file(path, error))
        path = path.parent_path();

    while (!path.empty())
    {
        const std::filesystem::path manifest = path / kManifestName;
        if (std::filesystem::is_regular_file(manifest, error))
            return manifest;

        if (path == path.root_path())
            break;
        path = path.parent_path();
    }
    return {};
}

std::filesystem::path FindManifest(const std::filesystem::path& hint)
{
    const std::array candidates =
    {
        hint,
        cane::DataDirectory(),
        std::filesystem::current_path()
    };

    for (const std::filesystem::path& candidate : candidates)
    {
        if (const std::filesystem::path manifest = FindManifestAbove(candidate);
            !manifest.empty())
            return manifest;

        std::error_code error;
        const std::filesystem::path child = candidate / kManifestName;
        if (std::filesystem::is_regular_file(child, error))
            return child;
    }
    return {};
}

CONTENT_REGION ParseRegion(const std::string& value)
{
    std::string normalized = value;
    std::transform(normalized.begin(), normalized.end(), normalized.begin(),
        [](unsigned char character) { return static_cast<char>(std::toupper(character)); });
    if (normalized == "PAL")
        return CONTENT_REGION::Pal;
    if (normalized == "NTSC-U")
        return CONTENT_REGION::NtscU;
    if (normalized == "NTSC-J")
        return CONTENT_REGION::NtscJ;
    return CONTENT_REGION::Unknown;
}
}

void RefreshContentPackage(const std::filesystem::path& hint)
{
    const std::filesystem::path manifestPath = FindManifest(hint);
    if (manifestPath.empty())
        return;
    if (s_contentPackage.manifestLoaded)
    {
        std::error_code error;
        if (std::filesystem::equivalent(
            manifestPath, s_contentPackage.manifestPath, error) && !error)
            return;
    }

    try
    {
        std::ifstream input(manifestPath);
        const nlohmann::json manifest = nlohmann::json::parse(input);
        if (manifest.value("schemaVersion", 0) != 1 ||
            manifest.value("levelFormat", std::string()) != "PCBRX" ||
            manifest.value("levelFormatVersion", 19) < 17 ||
            manifest.value("levelFormatVersion", 19) > 19)
        {
            std::printf("CONTENT MANIFEST ERROR: unsupported manifest '%s'\n",
                cane::Utf8(manifestPath).c_str());
            return;
        }

        const CONTENT_REGION region = ParseRegion(
            manifest.value("region", std::string()));
        if (region == CONTENT_REGION::Unknown)
        {
            std::printf("CONTENT MANIFEST ERROR: unknown region in '%s'\n",
                cane::Utf8(manifestPath).c_str());
            return;
        }

        s_contentPackage = {};
        s_pendingLanguageChange = {};
        s_contentPackage.region = region;
        s_contentPackage.root = manifestPath.parent_path();
        s_contentPackage.manifestPath = manifestPath;
        s_contentPackage.serial = manifest.value("serial", std::string());
        s_contentPackage.languageMask = 0;
        if (const auto languages = manifest.find("languages");
            languages != manifest.end() && languages->is_array())
        {
            for (const auto& entry : *languages)
            {
                if (!entry.is_string())
                    continue;
                CONTENT_LANGUAGE language{};
                if (TryParseLanguage(entry.get<std::string>(), language))
                    s_contentPackage.languageMask |= LanguageBit(language);
            }
        }
        if (s_contentPackage.languageMask == 0)
            s_contentPackage.languageMask = LanguageBit(CONTENT_LANGUAGE::English);
        s_contentPackage.defaultLanguage = region == CONTENT_REGION::NtscJ
            ? CONTENT_LANGUAGE::Japanese : CONTENT_LANGUAGE::English;
        s_contentPackage.voiceLanguageMask = 0;
        if (const auto voices = manifest.find("voiceLanguages");
            voices != manifest.end() && voices->is_array())
        {
            for (const auto& entry : *voices)
            {
                if (!entry.is_string())
                    continue;
                CONTENT_LANGUAGE language{};
                if (TryParseLanguage(entry.get<std::string>(), language))
                    s_contentPackage.voiceLanguageMask |= LanguageBit(language);
            }
        }
        if (s_contentPackage.voiceLanguageMask == 0)
            s_contentPackage.voiceLanguageMask = s_contentPackage.languageMask;
        s_contentPackage.defaultVoiceLanguage = region == CONTENT_REGION::NtscJ
            ? CONTENT_LANGUAGE::Japanese : s_contentPackage.defaultLanguage;
        const CONTENT_LANGUAGE savedLanguage = LoadSavedLanguage(
            s_contentPackage.serial, s_contentPackage.defaultVoiceLanguage);
        s_contentPackage.preferredLanguage = savedLanguage;
        s_contentPackage.language = region == CONTENT_REGION::NtscJ
            ? CONTENT_LANGUAGE::Japanese
            : (s_contentPackage.languageMask & LanguageBit(savedLanguage)) != 0
                ? savedLanguage : s_contentPackage.defaultLanguage;
        s_contentPackage.voiceLanguage =
            (s_contentPackage.voiceLanguageMask & LanguageBit(savedLanguage)) != 0
                ? savedLanguage : s_contentPackage.defaultVoiceLanguage;
        s_contentPackage.manifestLoaded = true;

        std::printf("Content package: %s (%s) at %s\n",
            manifest.value("region", std::string()).c_str(),
            s_contentPackage.serial.c_str(),
            cane::Utf8(s_contentPackage.root).c_str());
    }
    catch (const std::exception& exception)
    {
        std::printf("CONTENT MANIFEST ERROR: %s\n", exception.what());
    }
}

const CONTENT_PACKAGE& ContentPackage()
{
    return s_contentPackage;
}

bool FPalContent()
{
    // The Japanese executable is derived from the PAL gameplay branch even
    // though its video standard/serial is NTSC-J.
    return s_contentPackage.region == CONTENT_REGION::Pal ||
        s_contentPackage.region == CONTENT_REGION::NtscJ;
}

bool FPalVideoStandard()
{
    return s_contentPackage.region == CONTENT_REGION::Pal;
}

bool FJapaneseContent()
{
    return s_contentPackage.region == CONTENT_REGION::NtscJ;
}

bool FLocalizedContent()
{
    if (FJapaneseContent())
        return true;
    const std::uint32_t mask = s_contentPackage.languageMask;
    return mask != 0 && (mask & (mask - 1)) != 0;
}

bool FContentLanguageAvailable(CONTENT_LANGUAGE language)
{
    const std::uint32_t mask = FJapaneseContent()
        ? s_contentPackage.voiceLanguageMask : s_contentPackage.languageMask;
    return (mask & LanguageBit(language)) != 0;
}

void ConfigureContentLanguages(std::uint32_t languageMask, CONTENT_LANGUAGE defaultLanguage)
{
    if (FJapaneseContent())
    {
        // SCPS PCBRX IDs are voice variants: 0=Japanese, 1=English.
        s_contentPackage.voiceLanguageMask = 0;
        if ((languageMask & 1u) != 0)
            s_contentPackage.voiceLanguageMask |= LanguageBit(CONTENT_LANGUAGE::Japanese);
        if ((languageMask & 2u) != 0)
            s_contentPackage.voiceLanguageMask |= LanguageBit(CONTENT_LANGUAGE::English);
        s_contentPackage.defaultVoiceLanguage = defaultLanguage == CONTENT_LANGUAGE::English
            ? CONTENT_LANGUAGE::Japanese : CONTENT_LANGUAGE::English;
        if ((s_contentPackage.voiceLanguageMask &
            LanguageBit(s_contentPackage.voiceLanguage)) == 0)
            s_contentPackage.voiceLanguage = s_contentPackage.defaultVoiceLanguage;
        s_contentPackage.language = CONTENT_LANGUAGE::Japanese;
        return;
    }
    constexpr std::uint32_t validLanguages = (1u << 6) - 1;
    languageMask &= validLanguages;
    if (languageMask == 0)
        languageMask = LanguageBit(CONTENT_LANGUAGE::English);
    if ((languageMask & LanguageBit(defaultLanguage)) == 0)
    {
        for (int index = 0; index <= static_cast<int>(CONTENT_LANGUAGE::Japanese); ++index)
        {
            const auto candidate = static_cast<CONTENT_LANGUAGE>(index);
            if ((languageMask & LanguageBit(candidate)) != 0)
            {
                defaultLanguage = candidate;
                break;
            }
        }
    }
    s_contentPackage.languageMask = languageMask;
    s_contentPackage.defaultLanguage = defaultLanguage;
    s_contentPackage.language = FContentLanguageAvailable(s_contentPackage.preferredLanguage)
        ? s_contentPackage.preferredLanguage : defaultLanguage;
}

const char* ContentLanguageDirectory()
{
    return LanguageName(s_contentPackage.language);
}

const char* ContentAudioLanguageDirectory()
{
    return LanguageName(FJapaneseContent()
        ? s_contentPackage.voiceLanguage : s_contentPackage.language);
}

CONTENT_LANGUAGE ContentVoiceLanguage()
{
    return FJapaneseContent() ? s_contentPackage.voiceLanguage : s_contentPackage.language;
}

void SetContentLanguage(CONTENT_LANGUAGE language)
{
    if (FJapaneseContent())
    {
        if ((s_contentPackage.voiceLanguageMask & LanguageBit(language)) == 0)
            language = s_contentPackage.defaultVoiceLanguage;
        s_contentPackage.voiceLanguage = language;
        s_contentPackage.preferredLanguage = language;
        SaveLanguage(s_contentPackage.serial, language);
        return;
    }
    if (!FContentLanguageAvailable(language))
        language = s_contentPackage.defaultLanguage;
    s_contentPackage.language = language;
    s_contentPackage.preferredLanguage = language;
    if (FLocalizedContent())
        SaveLanguage(s_contentPackage.serial, s_contentPackage.language);
}

void BeginContentLanguageChange(CONTENT_LANGUAGE language)
{
    if (!s_pendingLanguageChange.active)
    {
        s_pendingLanguageChange.active = true;
        s_pendingLanguageChange.language = s_contentPackage.language;
        s_pendingLanguageChange.preferredLanguage = s_contentPackage.preferredLanguage;
        s_pendingLanguageChange.voiceLanguage = s_contentPackage.voiceLanguage;
    }

    if (FJapaneseContent())
    {
        if ((s_contentPackage.voiceLanguageMask & LanguageBit(language)) == 0)
            language = s_contentPackage.defaultVoiceLanguage;
        s_contentPackage.voiceLanguage = language;
        s_contentPackage.preferredLanguage = language;
        return;
    }

    if (!FContentLanguageAvailable(language))
        language = s_contentPackage.defaultLanguage;
    s_contentPackage.language = language;
    s_contentPackage.preferredLanguage = language;
}

void FinishContentLanguageChange(bool committed)
{
    if (!s_pendingLanguageChange.active)
        return;

    if (!committed)
    {
        s_contentPackage.language = s_pendingLanguageChange.language;
        s_contentPackage.preferredLanguage = s_pendingLanguageChange.preferredLanguage;
        s_contentPackage.voiceLanguage = s_pendingLanguageChange.voiceLanguage;
    }
    else if (FJapaneseContent())
    {
        SaveLanguage(s_contentPackage.serial, s_contentPackage.voiceLanguage);
    }
    else if (FLocalizedContent())
    {
        SaveLanguage(s_contentPackage.serial, s_contentPackage.language);
    }

    s_pendingLanguageChange = {};
}

bool LoadContentLocalization(const std::filesystem::path& levelPath)
{
    constexpr std::uint32_t singleLocalizationMagic = 0x434f4c50; // "PLOC"
    std::ifstream input(levelPath, std::ios::binary);
    if (!input)
        return false;

    input.seekg(0, std::ios::end);
    const std::streamoff fileSize = input.tellg();
    if (fileSize < 8)
        return false;
    input.seekg(fileSize - 8);
    std::uint32_t magic = 0;
    std::uint32_t payloadSize = 0;
    input.read(reinterpret_cast<char*>(&magic), sizeof(magic));
    input.read(reinterpret_cast<char*>(&payloadSize), sizeof(payloadSize));
    if (!input || magic != singleLocalizationMagic || payloadSize > fileSize - 8)
        return false;

    input.seekg(fileSize - 8 - payloadSize);
    std::uint32_t count = 0;
    input.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!input || payloadSize < sizeof(count) || count > 4096)
        return false;

    std::uint64_t remaining = payloadSize - sizeof(count);

    std::vector<std::string> strings;
    strings.reserve(count);
    for (std::uint32_t index = 0; index < count; ++index)
    {
        std::uint32_t length = 0;
        input.read(reinterpret_cast<char*>(&length), sizeof(length));
        if (!input || remaining < sizeof(length))
            return false;
        remaining -= sizeof(length);
        if (length > remaining)
            return false;
        std::string text(length, '\0');
        input.read(text.data(), static_cast<std::streamsize>(length));
        if (!input)
            return false;
        remaining -= length;
        strings.push_back(std::move(text));
    }
    if (remaining != 0)
        return false;
    s_localizedText = std::move(strings);
    return true;
}

bool LoadContentLocalization(const std::uint8_t* data, std::size_t size)
{
    if (data == nullptr || size < 8)
        return false;

    constexpr std::uint32_t localizationMagic = 0x434f4c50; // "PLOC"
    const auto readU32 = [](const std::uint8_t* source) {
        return static_cast<std::uint32_t>(source[0]) |
            (static_cast<std::uint32_t>(source[1]) << 8) |
            (static_cast<std::uint32_t>(source[2]) << 16) |
            (static_cast<std::uint32_t>(source[3]) << 24);
    };

    const std::uint32_t magic = readU32(data + size - 8);
    const std::uint32_t payloadSize = readU32(data + size - 4);
    if (magic != localizationMagic || payloadSize > size - 8)
        return false;

    const std::uint8_t* cursor = data + size - 8 - payloadSize;
    const std::uint8_t* const end = data + size - 8;
    if (static_cast<std::size_t>(end - cursor) < 4)
        return false;
    const std::uint32_t count = readU32(cursor);
    cursor += 4;
    if (count > 4096)
        return false;

    std::vector<std::string> strings;
    strings.reserve(count);
    for (std::uint32_t index = 0; index < count; ++index)
    {
        if (static_cast<std::size_t>(end - cursor) < 4)
            return false;
        const std::uint32_t length = readU32(cursor);
        cursor += 4;
        if (length > static_cast<std::size_t>(end - cursor))
            return false;
        strings.emplace_back(reinterpret_cast<const char*>(cursor), length);
        cursor += length;
    }
    if (cursor != end)
        return false;
    s_localizedText = std::move(strings);
    return true;
}

void ClearContentLocalization()
{
    s_localizedText.clear();
}

std::shared_ptr<char[]> ContentLocalizedTextAlloc(std::int16_t index)
{
    const char* text = index >= 0
        ? ContentLocalizedText(static_cast<std::size_t>(index)) : nullptr;
    const std::size_t length = text != nullptr ? std::strlen(text) : 0;
    std::shared_ptr<char[]> result(new char[length + 1]());
    if (length != 0)
        std::memcpy(result.get(), text, length);
    return result;
}

const char* ContentLocalizedText(std::size_t index)
{
    return index < s_localizedText.size() ? s_localizedText[index].c_str() : nullptr;
}

const char* ContentUiText(const char* english, const char* french,
    const char* german, const char* spanish, const char* italian)
{
    if (!FContentLanguageAvailable(s_contentPackage.language))
        return english;

    switch (ContentPackage().language)
    {
    case CONTENT_LANGUAGE::French: return french;
    case CONTENT_LANGUAGE::German: return german;
    case CONTENT_LANGUAGE::Spanish: return spanish;
    case CONTENT_LANGUAGE::Italian: return italian;
    case CONTENT_LANGUAGE::Japanese: return english;
    default: return english;
    }
}

