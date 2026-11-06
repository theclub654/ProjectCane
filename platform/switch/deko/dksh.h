#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>

namespace CaneDeko
{
struct ShaderBinary
{
    const uint8_t* control;
    const uint8_t* code;
    uint32_t codeSize, scratchSize;
};
inline ShaderBinary DecodeDksh(const uint8_t* bytes, size_t size)
{

    uint32_t header[6];
    if (size < sizeof(header)) throw std::runtime_error("Truncated DKSH header");
    std::memcpy(header,bytes,sizeof(header));
    const auto magic=header[0],headerSize=header[1],controlSize=header[2],codeSize=header[3],programOffset=header[4],programCount=header[5];
    if (magic!=0x48534b44 || headerSize<sizeof(header) || controlSize<headerSize || uint64_t(controlSize)+codeSize>size || programCount!=1 || uint64_t(programOffset)+64>controlSize || !codeSize || codeSize%256)
        throw std::runtime_error("Invalid DKSH sections");
    uint32_t program[6];std::memcpy(program,bytes+programOffset,sizeof(program));
    if (program[5]>64*1024) throw std::runtime_error("Shader scratch requirement exceeds backend limit");
    return {bytes,bytes+controlSize,codeSize,program[5]};
}
}
