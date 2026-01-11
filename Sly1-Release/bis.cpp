#include "bis.h"

CBinaryInputStream::CBinaryInputStream(std::string filePath)
{
    file.open(filePath, std::ios::binary);
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

glm::vec2 CBinaryInputStream::ReadVector2()
{
    glm::vec2 temp{};
    file.read(reinterpret_cast<char*> (&temp), sizeof(glm::vec2));
    return temp;
}

glm::vec3 CBinaryInputStream::ReadVector()
{   
    glm::vec3 temp{};
    file.read(reinterpret_cast<char*> (&temp), sizeof(glm::vec3));
    return temp;
}

glm::vec4 CBinaryInputStream::ReadVector4()
{
    glm::vec4 temp{};
    file.read(reinterpret_cast<char*> (&temp), sizeof(glm::vec4));
    return temp;
}

glm::mat3 CBinaryInputStream::ReadMatrix()
{
    glm::mat3 temp{};
    file.read(reinterpret_cast<char*> (&temp), sizeof(glm::mat3));
    return temp;
}

glm::mat4 CBinaryInputStream::ReadMatrix4()
{
    glm::mat4 temp{};
    file.read(reinterpret_cast<char*> (&temp[0]), sizeof(glm::vec3));
    file.read(reinterpret_cast<char*> (&temp[1]), sizeof(glm::vec3));
    file.read(reinterpret_cast<char*> (&temp[2]), sizeof(glm::vec3));
    file.read(reinterpret_cast<char*> (&temp[3]), sizeof(glm::vec3));
    temp[3][3] = 1.0f;
    return temp;
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