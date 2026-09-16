#include "bis.h"

CBinaryInputStream::CBinaryInputStream()
{
}

CBinaryInputStream::CBinaryInputStream(const std::string& filePath)
{
    OpenFile(filePath);
}

void CBinaryInputStream::OpenFile(const std::string& filePath)
{
    Close();

    file.open(filePath, std::ios::in | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open binary input file: " + filePath);
    }

    m_bisk = BISK_File;
    m_pb = nullptr;
    m_cb = 0;
    m_ib = 0;
}

void CBinaryInputStream::OpenMemory(int cb, const void* pv)
{
    Close();

    if (cb < 0)
    {
        throw std::invalid_argument("CBinaryInputStream::OpenMemory received a negative size");
    }

    if (cb > 0 && !pv)
    {
        throw std::invalid_argument("CBinaryInputStream::OpenMemory received a null buffer");
    }

    m_bisk = BISK_Mem;
    m_pb = static_cast<const std::uint8_t*>(pv);
    m_cb = static_cast<std::size_t>(cb);
    m_ib = 0;
}

void CBinaryInputStream::ReadBytes(
    void* destination,
    std::size_t cb)
{
    if (cb == 0)
        return;

    if (!destination)
    {
        throw std::invalid_argument("CBinaryInputStream::ReadBytes received a null destination");
    }

    switch (m_bisk)
    {
        case BISK_File:
        {
            if (!file.is_open())
            {
                throw std::runtime_error("CBinaryInputStream file is not open");
            }

            if (cb > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))
                throw std::length_error("CBinaryInputStream read is too large");

            const auto readSize = static_cast<std::streamsize>(cb);

            file.read(static_cast<char*>(destination), readSize);

            if (file.gcount() != readSize)
            {
                throw std::runtime_error("Unexpected end of binary input file");
            }

            break;
        }

        case BISK_Mem:
        {
            if (m_ib > m_cb || cb > m_cb - m_ib)
            {
                throw std::runtime_error(
                    "Unexpected end of binary memory stream");
            }

            std::memcpy(destination,m_pb + m_ib, cb);

            m_ib += cb;
            break;
        }

        default:
        {
            throw std::runtime_error( "CBinaryInputStream is not open");
        }
    }
}

std::size_t CBinaryInputStream::GetOffset()
{
    switch (m_bisk)
    {
    case BISK_File:
    {
        if (!file.is_open())
        {
            throw std::runtime_error(
                "CBinaryInputStream file is not open");
        }

        const std::streampos position = file.tellg();

        if (position == std::streampos(-1))
        {
            throw std::runtime_error(
                "Failed to query binary input file position");
        }

        const std::streamoff offset =
            position - std::streampos(0);

        if (offset < 0)
        {
            throw std::runtime_error(
                "Binary input file returned a negative position");
        }

        return static_cast<std::size_t>(offset);
    }

    case BISK_Mem:
        return m_ib;

    default:
    {
        throw std::runtime_error(
            "CBinaryInputStream is not open");
    }
    }
}

void CBinaryInputStream::Align(int n)
{
    if (n <= 1)
        return;

    const std::size_t alignment =
        static_cast<std::size_t>(n);

    switch (m_bisk)
    {
    case BISK_File:
    {
        const std::size_t position = GetOffset();
        const std::size_t remainder =
            position % alignment;

        if (remainder != 0)
        {
            const std::size_t padding =
                alignment - remainder;

            if (padding > static_cast<std::size_t>(
                std::numeric_limits<std::streamoff>::max()))
            {
                throw std::length_error(
                    "Binary input alignment is too large");
            }

            file.seekg(
                static_cast<std::streamoff>(padding),
                std::ios::cur);

            if (!file)
            {
                throw std::runtime_error(
                    "Failed to align binary input file");
            }
        }

        break;
    }

    case BISK_Mem:
    {
        if (m_ib > m_cb)
        {
            throw std::runtime_error(
                "Binary memory stream position exceeds its size");
        }

        const std::size_t remainder =
            m_ib % alignment;

        const std::size_t padding =
            remainder == 0
            ? 0
            : alignment - remainder;

        if (padding > m_cb - m_ib)
        {
            throw std::runtime_error(
                "Binary memory stream alignment exceeded buffer size");
        }

        m_ib += padding;
        break;
    }

    default:
    {
        throw std::runtime_error(
            "CBinaryInputStream is not open");
    }
    }
}

byte CBinaryInputStream::U8Read()
{
    byte value{};
    ReadBytes(&value, sizeof(value));
    return value;
}

uint16_t CBinaryInputStream::U16Read()
{
    uint16_t value{};
    ReadBytes(&value, sizeof(value));
    return value;
}

uint32_t CBinaryInputStream::U32Read()
{
    uint32_t value{};
    ReadBytes(&value, sizeof(value));
    return value;
}

int8_t CBinaryInputStream::S8Read()
{
    int8_t value{};
    ReadBytes(&value, sizeof(value));
    return value;
}

int16_t CBinaryInputStream::S16Read()
{
    int16_t value{};
    ReadBytes(&value, sizeof(value));
    return value;
}

int32_t CBinaryInputStream::S32Read()
{
    int32_t value{};
    ReadBytes(&value, sizeof(value));
    return value;
}

float CBinaryInputStream::F32Read()
{
    float value{};
    ReadBytes(&value, sizeof(value));
    return value;
}

glm::vec2 CBinaryInputStream::ReadVector2()
{
    // Separate statements guarantee stream read order.
    const float x = F32Read();
    const float y = F32Read();

    return glm::vec2(x, y);
}

glm::vec3 CBinaryInputStream::ReadVector()
{
    // Separate statements guarantee stream read order.
    const float x = F32Read();
    const float y = F32Read();
    const float z = F32Read();

    return glm::vec3(x, y, z);
}

glm::vec4 CBinaryInputStream::ReadVector4()
{
    // Separate statements guarantee stream read order.
    const float x = F32Read();
    const float y = F32Read();
    const float z = F32Read();
    const float w = F32Read();

    return glm::vec4(x, y, z, w);
}

glm::mat3 CBinaryInputStream::ReadMatrix()
{
    // The BRX MATRIX3 contains three consecutive VECTOR values:
    // 3 columns * 3 floats = 36 bytes.
    const glm::vec3 column0 = ReadVector();
    const glm::vec3 column1 = ReadVector();
    const glm::vec3 column2 = ReadVector();

    glm::mat3 matrix(1.0f);

    matrix[0] = column0;
    matrix[1] = column1;
    matrix[2] = column2;

    return matrix;
}

glm::mat4 CBinaryInputStream::ReadMatrix4()
{
    // The BRX MATRIX4 stores four consecutive three-float vectors:
    // 4 columns * 3 floats = 48 bytes.
    const glm::vec3 column0 = ReadVector();
    const glm::vec3 column1 = ReadVector();
    const glm::vec3 column2 = ReadVector();
    const glm::vec3 column3 = ReadVector();

    glm::mat4 matrix(1.0f);

    matrix[0] = glm::vec4(column0, 0.0f);
    matrix[1] = glm::vec4(column1, 0.0f);
    matrix[2] = glm::vec4(column2, 0.0f);
    matrix[3] = glm::vec4(column3, 1.0f);

    return matrix;
}

std::shared_ptr<char[]>
CBinaryInputStream::ReadStringAlloc()
{
    const uint16_t cch = U16Read();

    std::shared_ptr<char[]> text(
        new char[static_cast<std::size_t>(cch) + 1]());

    if (cch > 0)
    {
        ReadBytes(
            text.get(),
            static_cast<std::size_t>(cch));
    }

    text[cch] = '\0';

    return text;
}

std::string CBinaryInputStream::ReadStringSw()
{
    const uint16_t cch = U16Read();

    std::string text(static_cast<std::size_t>(cch), '\0');

    if (cch > 0)
        ReadBytes(text.data(), static_cast<std::size_t>(cch));

    return text;
}

std::string CBinaryInputStream::ReadFixedString32NoAlloc()
{
    char buffer[32]{};

    ReadBytes(
        buffer,
        sizeof(buffer));

    std::size_t length = 0;

    while (length < sizeof(buffer) &&
        buffer[length] != '\0')
    {
        ++length;
    }

    return std::string(buffer, length);
}

std::shared_ptr<char[]> CBinaryInputStream::ReadFixedString32()
{
    char buffer[32]{};

    ReadBytes(
        buffer,
        sizeof(buffer));

    if (buffer[0] == '\0')
        return nullptr;

    std::shared_ptr<char[]> text(new char[33]());

    std::memcpy(
        text.get(),
        buffer,
        sizeof(buffer));

    text[32] = '\0';

    return text;
}

void CBinaryInputStream::Close()
{
    if (file.is_open())
        file.close();

    file.clear();

    m_bisk = BISK_Nil;
    m_pb = nullptr;
    m_cb = 0;
    m_ib = 0;
}

CBinaryInputStream::~CBinaryInputStream()
{
    Close();
}