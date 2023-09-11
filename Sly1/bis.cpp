#include "bis.h"

CBinaryInputStream::CBinaryInputStream(std::string fileName)
{
    file.open(fileName, std::ios::binary);
}

void CBinaryInputStream::Read(int numBytes, void *pv)
{

}

void CBinaryInputStream::Align(int n)
{
    std::streamoff pos = file.tellg();

    if (pos % n != 0)
        pos += (uint32_t)(n - (pos % n));

    file.seekg(pos, SEEK_SET);
}

byte CBinaryInputStream::U8Read()
{
    byte temp;
    file.read(reinterpret_cast<char*> (&temp), sizeof(byte));
    return temp;
}

uint16_t CBinaryInputStream::U16Read()
{
    int16_t temp;
    file.read(reinterpret_cast<char*> (&temp), sizeof(uint16_t));
    return temp;
}

uint32_t CBinaryInputStream::U32Read()
{
    uint32_t temp;
    file.read(reinterpret_cast<char*> (&temp), sizeof(uint32_t));
    return temp;
}

int8_t CBinaryInputStream::S8Read()
{
    int8_t temp;
    file.read(reinterpret_cast<char*> (&temp), sizeof(int8_t));
    return temp;
}

int16_t  CBinaryInputStream::S16Read()
{
    int16_t temp;
    file.read(reinterpret_cast<char*> (&temp), sizeof(int16_t));
    return temp;
}

int32_t CBinaryInputStream::S32Read()
{
    int32_t temp;
    file.read(reinterpret_cast<char*> (&temp), sizeof(int32_t));
    return temp;
}

float CBinaryInputStream::F32Read()
{
    float temp;
    file.read(reinterpret_cast<char*> (&temp), sizeof(float));
    return temp;
}

glm::vec3 CBinaryInputStream::ReadVector()
{
    return glm::vec3(F32Read(), F32Read(), F32Read());
}

glm::vec2 CBinaryInputStream::ReadVector2()
{
    return glm::vec2(F32Read(), F32Read());
}

glm::vec4 CBinaryInputStream::ReadVector4()
{
    return glm::vec4 (F32Read(), F32Read(), F32Read(), F32Read());
}

glm::mat3 CBinaryInputStream::ReadMatrix()
{
    return glm::mat3 (ReadVector(), ReadVector(), ReadVector());
}

glm::mat4 CBinaryInputStream::ReadMatrix4()
{
    return glm::mat4(ReadVector4(), ReadVector4(), ReadVector4(), ReadVector4());
}

uint16_t CBinaryInputStream::ReadGeom()
{
    F32Read();

    uint16_t vertexCount = U16Read();

    for (int i = 0; i < vertexCount; i++)
        ReadVector();

    uint16_t indexCount = U16Read();

    U16Read();

    for (int i = 0; i < indexCount; i++)
    {
        U16Read();
        U16Read();
        U16Read();
        F32Read();
        ReadVector();
        byte unk_0 = U8Read();

        for (int i = 0; i < unk_0; i++)
        {
            U16Read();
            U16Read();
            U16Read();
        }
    }

    return vertexCount;
}

void CBinaryInputStream::ReadBspc()
{
    U16Read();
    uint16_t unk0 = U16Read();

    for (int i = 0; i < unk0; i++)
    {
        U16Read();
        U16Read();
        U16Read();
    }
}

void CBinaryInputStream::ReadVbsp()
{
    uint16_t unkCount = U16Read();

    for (int i = 0; i < unkCount; i++)
    {
        ReadVector();
        F32Read();
        U32Read();
        U32Read();
    }
}

void CBinaryInputStream::ReadStringSw()
{
    short stringLength = U16Read();
    file.seekg(stringLength, SEEK_CUR);
}

void CBinaryInputStream::Close()
{
    file.clear();
    file.close();
}

CBinaryInputStream::~CBinaryInputStream()
{
    Close();
}