#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>

#include <glm/glm.hpp>

using byte = std::uint8_t;

enum BISK
{
    BISK_Nil = -1,
    BISK_File = 0,
    BISK_Mem = 1
};

class CBinaryInputStream
{
public:
    CBinaryInputStream();
    explicit CBinaryInputStream(const std::string& filePath);

    CBinaryInputStream(const CBinaryInputStream&) = delete;
    CBinaryInputStream& operator=(const CBinaryInputStream&) = delete;

    void OpenFile(const std::string& filePath);
    void OpenMemory(int cb, const void* pv);

    void ReadBytes(void* destination, std::size_t cb);

    void Align(int n);

    [[nodiscard]] std::size_t GetOffset();

    byte U8Read();
    uint16_t U16Read();
    uint32_t U32Read();

    int8_t S8Read();
    int16_t S16Read();
    int32_t S32Read();

    float F32Read();

    glm::vec2 ReadVector2();
    glm::vec3 ReadVector();
    glm::vec4 ReadVector4();

    glm::mat3 ReadMatrix();
    glm::mat4 ReadMatrix4();

    std::shared_ptr<char[]> ReadStringAlloc();
    std::string ReadStringSw();

    std::string ReadFixedString32NoAlloc();
    std::shared_ptr<char[]> ReadFixedString32();

    void Close();

    ~CBinaryInputStream();

    std::ifstream file;

    BISK m_bisk = BISK_Nil;

    // OpenMemory uses a non-owning view of this buffer.
    const std::uint8_t* m_pb = nullptr;
    std::size_t m_cb = 0;
    std::size_t m_ib = 0;
};