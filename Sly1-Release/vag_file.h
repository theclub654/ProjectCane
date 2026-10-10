#pragma once
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <string>

namespace CaneVag
{
struct Request
{
    std::filesystem::path levelDirectory, executableDirectory, workingDirectory, direct;
    std::string language;
    bool localized = false, valid = false;

    std::string Key() const
    {
        return levelDirectory.generic_string() + '\0' + executableDirectory.generic_string() + '\0' +
            workingDirectory.generic_string() + '\0' + language + '\0' + direct.generic_string();
    }
};

template<class Find, class Commentary>
std::filesystem::path Resolve(const Request& request, Find&& find, Commentary&& commentary)
{
    if (!request.valid) return {};
    std::error_code error;
    if (!request.localized && std::filesystem::is_regular_file(request.direct, error)) return request.direct;
    auto level = request.levelDirectory;
    if (request.localized && level.filename() != request.language) level /= request.language;
    for (const char* directory : {"Dialogs", "Commentary", "Dialog"})
    {
        const auto local = level / directory / request.direct.filename();
        if (std::filesystem::is_regular_file(local, error)) return local;
    }
    if (request.localized) return {};
    if (auto found = find(request.direct.filename().wstring()); !found.empty()) return found;
    const auto filename = request.direct.filename().string();
    unsigned identity = 0, sector = 0;
    int length = 0;
    if (std::sscanf(filename.c_str(), "VAG_%x_%x.wav%n", &identity, &sector, &length) == 2 &&
        length == static_cast<int>(filename.size()))
    {
        char shortName[32];
        std::snprintf(shortName, sizeof(shortName), "VAG_%04X.wav", identity);
        if (auto found = find(std::filesystem::path(shortName).wstring()); !found.empty()) return found;
        return commentary(identity, sector, UINT32_MAX);
    }
    if (std::sscanf(filename.c_str(), "Commentary_%x.wav%n", &identity, &length) == 1 &&
        length == static_cast<int>(filename.size())) return commentary(0, 0, identity);
    return {};
}
}
