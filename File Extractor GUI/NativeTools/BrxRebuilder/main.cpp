#include "BrxWalker.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
template <typename Character>
int Run(int argc, Character** argv)
{
    BrxSourceRegion region = BrxSourceRegion::Auto;
    if (argc >= 3) {
        const std::filesystem::path regionArgument(argv[2]);
        const auto value = regionArgument.string();
        if (value == "pal") region = BrxSourceRegion::Pal;
        else if (value == "japanese") region = BrxSourceRegion::Japanese;
        else if (value == "ntsc") region = BrxSourceRegion::NtscU;
        else throw std::runtime_error("region must be ntsc, pal, or japanese");
    }
    BrxWalker walker(StringEncoding::Narrow, region);
    if (argc == 4) walker.LoadLevelIdentityMap(std::filesystem::path(argv[3]));
    const std::filesystem::path input(argv[1]);
    if (!std::filesystem::is_regular_file(input) || input.extension() != ".brx")
        throw std::runtime_error("input must be an existing .brx file");
    walker.WalkFile(input);
    return 0;
}
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    if (argc < 2 || argc > 4) {
        std::wcerr << L"Usage: Sly1BrxRebuilder <level.brx> [ntsc|pal|japanese] [level-identity-map]\n";
        return 2;
    }

    try {
        return Run(argc, argv);
    }
    catch (const std::exception& exception) {
        std::cerr << "Failed: " << exception.what() << '\n';
        return 1;
    }
}
#else
int main(int argc, char** argv)
{
    if (argc < 2 || argc > 4) {
        std::cerr << "Usage: Sly1BrxRebuilder <level.brx> [ntsc|pal|japanese] [level-identity-map]\n";
        return 2;
    }

    try {
        return Run(argc, argv);
    }
    catch (const std::exception& exception) {
        std::cerr << "Failed: " << exception.what() << '\n';
        return 1;
    }
}
#endif
