#include "BrxWalker.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
int Run(const std::filesystem::path& input)
{
    if (!std::filesystem::is_regular_file(input) || input.extension() != ".brx")
        throw std::runtime_error("input must be an existing .brx file");

    BrxWalker walker(StringEncoding::Narrow);
    walker.WalkFile(input);
    return 0;
}
}

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    if (argc != 2) {
        std::wcerr << L"Usage: Sly1BrxRebuilder <decompressed-level.brx>\n";
        return 2;
    }

    try {
        return Run(std::filesystem::path(argv[1]));
    }
    catch (const std::exception& exception) {
        std::cerr << "Failed: " << exception.what() << '\n';
        return 1;
    }
}
#else
int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "Usage: Sly1BrxRebuilder <decompressed-level.brx>\n";
        return 2;
    }

    try {
        return Run(std::filesystem::path(argv[1]));
    }
    catch (const std::exception& exception) {
        std::cerr << "Failed: " << exception.what() << '\n';
        return 1;
    }
}
#endif
