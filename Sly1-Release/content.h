#pragma once

#include <filesystem>
#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>

enum class CONTENT_REGION
{
    Unknown,
    NtscU,
    Pal,
    NtscJ
};

enum class CONTENT_LANGUAGE
{
    English,
    French,
    German,
    Spanish,
    Italian,
    Japanese
};

struct CONTENT_PACKAGE
{
    CONTENT_REGION region = CONTENT_REGION::Unknown;
    CONTENT_LANGUAGE language = CONTENT_LANGUAGE::English;
    CONTENT_LANGUAGE preferredLanguage = CONTENT_LANGUAGE::English;
    std::filesystem::path root;
    std::filesystem::path manifestPath;
    std::string serial;
    std::uint32_t languageMask = 1;
    CONTENT_LANGUAGE defaultLanguage = CONTENT_LANGUAGE::English;
    CONTENT_LANGUAGE voiceLanguage = CONTENT_LANGUAGE::English;
    CONTENT_LANGUAGE defaultVoiceLanguage = CONTENT_LANGUAGE::English;
    std::uint32_t voiceLanguageMask = 1;
    bool manifestLoaded = false;
};

// Detects the extraction package containing hint. Detection is lazy and may be
// called repeatedly as the debug level picker moves between extraction roots.
void RefreshContentPackage(const std::filesystem::path& hint = {});

const CONTENT_PACKAGE& ContentPackage();
bool FPalContent();
// True only for the European PAL package. Unlike FPalContent(), this excludes
// NTSC-J, which shares PAL gameplay code but uses a 60 Hz video standard.
bool FPalVideoStandard();
bool FJapaneseContent();
bool FLocalizedContent();
bool FContentLanguageAvailable(CONTENT_LANGUAGE language);
void ConfigureContentLanguages(std::uint32_t languageMask, CONTENT_LANGUAGE defaultLanguage);
const char* ContentLanguageDirectory();
const char* ContentAudioLanguageDirectory();
CONTENT_LANGUAGE ContentVoiceLanguage();
void SetContentLanguage(CONTENT_LANGUAGE language);
// Language-menu changes are transactional because selecting a language reloads
// the active level. A failed reload must leave the old payload and its language
// selection paired together.
void BeginContentLanguageChange(CONTENT_LANGUAGE language);
void FinishContentLanguageChange(bool committed);
bool LoadContentLocalization(const std::filesystem::path& levelPath);
bool LoadContentLocalization(const std::uint8_t* data, std::size_t size);
void ClearContentLocalization();
const char* ContentLocalizedText(std::size_t index);
std::shared_ptr<char[]> ContentLocalizedTextAlloc(std::int16_t index);
const char* ContentUiText(const char* english, const char* french,
    const char* german, const char* spanish, const char* italian);
