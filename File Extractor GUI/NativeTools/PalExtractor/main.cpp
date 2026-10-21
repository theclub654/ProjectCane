#include "PalIsoExtractor.h"

#include <exception>
#include <filesystem>
#include <iostream>
#include <string_view>

#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
    try {
        if (argc != 4 || std::wstring_view(argv[1]) != L"extract") {
            std::wcerr << L"Usage: Sly1PalExtractor extract <pal-or-japanese.iso> <output-folder>\n";
            return 2;
        }

        PalIsoExtractor{}.ExtractDetectedRegion(
            std::filesystem::path(argv[2]),
            std::filesystem::path(argv[3]));
        std::cout << "Extraction completed successfully.\n";
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "Failed: " << exception.what() << '\n';
        return 1;
    }
}
#else
int main(int argc, char** argv) {
    try {
        if (argc != 4 || std::string_view(argv[1]) != "extract") {
            std::cerr << "Usage: Sly1PalExtractor extract <pal-or-japanese.iso> <output-folder>\n";
            return 2;
        }

        PalIsoExtractor{}.ExtractDetectedRegion(
            std::filesystem::path(argv[2]),
            std::filesystem::path(argv[3]));
        std::cout << "Extraction completed successfully.\n";
        return 0;
    } catch (const std::exception& exception) {
        std::cerr << "Failed: " << exception.what() << '\n';
        return 1;
    }
}
#endif
