#pragma once
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <filesystem>
#include <string>

namespace CaneVag
{
struct Request
{
    std::filesystem::path dialogDirectory, executableDirectory, workingDirectory, direct;
    uint32_t identity = 0, sector = 0;
    bool encrypted = false, valid = false;

    std::string Key() const
    {
        std::string key = dialogDirectory.generic_string() + '\0' + executableDirectory.generic_string() + '\0' +
            workingDirectory.generic_string() + '\0';
        return key + (encrypted ? std::to_string(identity) + ':' + std::to_string(sector) : direct.generic_string());
    }
};

inline bool IsReadablePath(const char* value)
{
    if (!value || !value[0]) return false;
    for (unsigned i = 0; i < 32; ++i)
    {
        if (!value[i]) return i != 0;
        const auto ch = static_cast<unsigned char>(value[i]);
        if (ch < 0x20 || ch > 0x7e) return false;
    }
    return true;
}

inline Request MakeRequest(const char* value, const std::filesystem::path& level,
    const std::filesystem::path& executable, const std::filesystem::path& working, bool descriptor = false)
{
    Request request;
    if (!value) return request;
    request.valid = true;
    if (!level.empty()) request.dialogDirectory = level.parent_path() / "Dialog";
    request.executableDirectory = executable;
    request.workingDirectory = working;
    if (!descriptor && IsReadablePath(value))
    {
        request.direct = value;
        request.direct.replace_extension(".wav");
    }
    else
    {
        uint32_t words[8];
        std::memcpy(words, value, sizeof(words));
        request.identity = words[2] ^ words[6];
        request.sector = words[0] ^ words[5];
        request.encrypted = true;
    }
    return request;
}

template<class Find, class Commentary>
std::filesystem::path Resolve(const Request& request, Find&& find, Commentary&& commentary)
{
    if (!request.valid) return {};
    std::wstring names[2];
    unsigned count = 1;
    std::error_code error;
    if (!request.encrypted)
    {
        if (std::filesystem::is_regular_file(request.direct, error)) return request.direct;
        names[0] = request.direct.filename().wstring();
    }
    else
    {
        char name[64];
        std::snprintf(name, sizeof(name), "VAG_%04X_%06X.wav", request.identity, request.sector);
        names[0] = std::filesystem::path(name).wstring();
        std::snprintf(name, sizeof(name), "VAG_%04X.wav", request.identity);
        names[1] = std::filesystem::path(name).wstring();
        count = 2;
    }
    if (!request.dialogDirectory.empty())
        for (unsigned i = 0; i < count; ++i)
        {
            auto local = request.dialogDirectory / names[i];
            if (std::filesystem::is_regular_file(local, error)) return local;
        }
    for (unsigned i = 0; i < count; ++i)
        if (auto found = find(names[i]); !found.empty()) return found;
    return request.encrypted ? commentary(request.identity, request.sector) : std::filesystem::path{};
}
}
