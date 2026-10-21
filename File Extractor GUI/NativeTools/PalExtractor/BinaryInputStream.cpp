#include "BinaryInputStream.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace {
std::string Utf8FromCodePoint(std::uint32_t cp) {
    std::string out;
    if (cp <= 0x7f) out.push_back(static_cast<char>(cp));
    else if (cp <= 0x7ff) {
        out.push_back(static_cast<char>(0xc0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else if (cp <= 0xffff) {
        out.push_back(static_cast<char>(0xe0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    } else {
        out.push_back(static_cast<char>(0xf0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
    return out;
}
}

CBinaryInputStream::CBinaryInputStream(const std::filesystem::path& path,
    StringEncoding encoding) : encoding_(encoding) {
    file_.open(path, std::ios::binary);
    if (!file_) throw std::runtime_error("Could not open: " + path.string());
    file_.seekg(0, std::ios::end);
    const auto end = file_.tellg();
    if (end < 0) throw std::runtime_error("Could not determine file size");
    size_ = static_cast<std::size_t>(end);
    file_.seekg(0, std::ios::beg);
}

void CBinaryInputStream::ReadBytes(void* destination, std::size_t size) {
    if (size > Remaining()) throw std::runtime_error("Unexpected EOF at 0x" + std::to_string(GetOffset()));
    file_.read(static_cast<char*>(destination), static_cast<std::streamsize>(size));
    if (!file_) throw std::runtime_error("Read failed");
}

void CBinaryInputStream::Skip(std::size_t size) { Seek(GetOffset() + size); }
void CBinaryInputStream::Align(std::size_t alignment) {
    if (alignment < 2) return;
    const auto p = GetOffset();
    Seek((p + alignment - 1) / alignment * alignment);
}
void CBinaryInputStream::Seek(std::size_t offset) {
    if (offset > size_) throw std::runtime_error("Seek past EOF");
    file_.clear();
    file_.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if (!file_) throw std::runtime_error("Seek failed");
}
std::size_t CBinaryInputStream::GetOffset() const {
    const auto p = file_.tellg();
    if (p < 0) throw std::runtime_error("tellg failed");
    return static_cast<std::size_t>(p);
}
std::size_t CBinaryInputStream::Remaining() const { return size_ - GetOffset(); }

#define READ_SCALAR(name, type) type CBinaryInputStream::name() { type v{}; ReadBytes(&v, sizeof(v)); return v; }
READ_SCALAR(U8Read, std::uint8_t)
READ_SCALAR(U16Read, std::uint16_t)
READ_SCALAR(U32Read, std::uint32_t)
READ_SCALAR(S8Read, std::int8_t)
READ_SCALAR(S16Read, std::int16_t)
READ_SCALAR(S32Read, std::int32_t)
READ_SCALAR(F32Read, float)
#undef READ_SCALAR

Vec2 CBinaryInputStream::ReadVector2() { return {F32Read(), F32Read()}; }
Vec3 CBinaryInputStream::ReadVector() { return {F32Read(), F32Read(), F32Read()}; }
Vec4 CBinaryInputStream::ReadVector4() { return {F32Read(), F32Read(), F32Read(), F32Read()}; }
void CBinaryInputStream::ReadMatrix3() { for (int i = 0; i < 3; ++i) ReadVector(); }
void CBinaryInputStream::ReadMatrix4() { for (int i = 0; i < 4; ++i) ReadVector(); }

std::string CBinaryInputStream::ReadNarrowString(std::uint16_t length) {
    std::string result(length, '\0');
    if (length) ReadBytes(result.data(), length);
    return result;
}

std::string CBinaryInputStream::ReadUtf16String(std::uint16_t length) {
    std::vector<std::uint16_t> text(length);
    if (length) ReadBytes(text.data(), text.size() * sizeof(std::uint16_t));
    std::string result;
    for (std::size_t i = 0; i < text.size(); ++i) {
        std::uint32_t cp = text[i];
        if (cp >= 0xd800 && cp <= 0xdbff && i + 1 < text.size()) {
            const std::uint32_t low = text[i + 1];
            if (low >= 0xdc00 && low <= 0xdfff) {
                cp = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
                ++i;
            }
        }
        result += Utf8FromCodePoint(cp);
    }
    return result;
}

bool CBinaryInputStream::LooksLikeUtf16(std::uint16_t length) {
    if (!length || static_cast<std::size_t>(length) * 2 > Remaining()) return false;
    const auto saved = GetOffset();
    const auto sample = std::min<std::size_t>(length, 16);
    std::size_t zeroHigh = 0;
    for (std::size_t i = 0; i < sample; ++i) {
        const auto c = U16Read();
        if ((c & 0xff00u) == 0 && (c & 0xffu) >= 0x20) ++zeroHigh;
    }
    Seek(saved);
    return sample >= 2 && zeroHigh * 4 >= sample * 3;
}

std::string CBinaryInputStream::ReadStringSw() {
    const auto length = U16Read();
    if (encoding_ == StringEncoding::Utf16Le ||
        (encoding_ == StringEncoding::Auto && LooksLikeUtf16(length)))
        return ReadUtf16String(length);
    return ReadNarrowString(length);
}

std::string CBinaryInputStream::ReadUtf16StringSw() {
    return ReadUtf16String(U16Read());
}

std::string CBinaryInputStream::ReadFixedString32() {
    char text[32]{};
    ReadBytes(text, sizeof(text));
    const auto end = std::find(std::begin(text), std::end(text), '\0');
    return std::string(text, end);
}
