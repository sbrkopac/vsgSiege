
#include "BinaryReader.hpp"

#include <cstring>

namespace ehb
{
    bool operator==(const FourCC a, const FourCC b) noexcept
    {
        return (a.c0 == b.c0) && (a.c1 == b.c1) &&
               (a.c2 == b.c2) && (a.c3 == b.c3);
    }

    bool operator!=(const FourCC a, const FourCC b) noexcept
    {
        return (a.c0 != b.c0) || (a.c1 != b.c1) ||
               (a.c2 != b.c2) || (a.c3 != b.c3);
    }

    bool operator==(const FourCC a, const char fccStr[]) noexcept
    {
        return (a.c0 == fccStr[0]) && (a.c1 == fccStr[1]) &&
               (a.c2 == fccStr[2]) && (a.c3 == fccStr[3]);
    }

    bool operator!=(const FourCC a, const char fccStr[]) noexcept
    {
        return (a.c0 != fccStr[0]) || (a.c1 != fccStr[1]) ||
               (a.c2 != fccStr[2]) || (a.c3 != fccStr[3]);
    }

    std::ostream& operator<<(std::ostream& s, const FourCC& fcc)
    {
        s << static_cast<char>(fcc.c0) << static_cast<char>(fcc.c1)
          << static_cast<char>(fcc.c2) << static_cast<char>(fcc.c3);
        return s;
    }

    void BinaryReader::readBytes(void* buffer, size_t numBytes)
    {
        if (readPosition == data.size() || (readPosition + numBytes) > data.size()) return;

        const uint8_t* ptr = data.data() + readPosition;
        std::memcpy(buffer, ptr, numBytes);
        readPosition += numBytes;
    }

    void BinaryReader::skipBytes(const size_t numBytes)
    {
        readPosition += numBytes;
    }

} // namespace ehb
