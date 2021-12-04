
#include "BinaryReader.hpp"

namespace ehb
{
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
