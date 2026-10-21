#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

enum class StringEncoding { Narrow, Utf16Le, Auto };

struct Vec2 { float x{}, y{}; };
struct Vec3 { float x{}, y{}, z{}; };
struct Vec4 { float x{}, y{}, z{}, w{}; };

class CBinaryInputStream {
public:
    explicit CBinaryInputStream(const std::filesystem::path& path,
        StringEncoding encoding = StringEncoding::Auto);

    void ReadBytes(void* destination, std::size_t size);
    void Skip(std::size_t size);
    void Align(std::size_t alignment);
    void Seek(std::size_t offset);

    [[nodiscard]] std::size_t GetOffset() const;
    [[nodiscard]] std::size_t GetSize() const { return size_; }
    [[nodiscard]] std::size_t Remaining() const;

    std::uint8_t U8Read();
    std::uint16_t U16Read();
    std::uint32_t U32Read();
    std::int8_t S8Read();
    std::int16_t S16Read();
    std::int32_t S32Read();
    float F32Read();
    Vec2 ReadVector2();
    Vec3 ReadVector();
    Vec4 ReadVector4();
    void ReadMatrix3();
    void ReadMatrix4();

    std::string ReadStringSw();
    std::string ReadUtf16StringSw();
    std::string ReadFixedString32();
    void SetStringEncoding(StringEncoding encoding) { encoding_ = encoding; }
    [[nodiscard]] StringEncoding GetStringEncoding() const { return encoding_; }

private:
    std::string ReadNarrowString(std::uint16_t length);
    std::string ReadUtf16String(std::uint16_t length);
    bool LooksLikeUtf16(std::uint16_t length);

    mutable std::ifstream file_;
    std::size_t size_{};
    StringEncoding encoding_{};
};
