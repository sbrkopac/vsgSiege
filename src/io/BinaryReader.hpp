
#pragma once

#include <cstdint>
#include <istream>
#include <string>
#include <vector>

// TODO: refactor this to its own output header
#include <spdlog/fmt/ostr.h>

namespace ehb
{

    // Zero fills a POD type, such as a C struct or union.
    template<class T>
    void clearPodObject(T& s) noexcept
    {
        static_assert(std::is_pod<T>::value, "Type must be Plain Old Data!");
        std::memset(&s, 0, sizeof(T));
    }

    // Zero fills a statically allocated array of POD or built-in types. Array length inferred by the compiler.
    template<class T, size_t N>
    void clearArray(T (&arr)[N]) noexcept
    {
        static_assert(std::is_pod<T>::value, "Type must be Plain Old Data!");
        std::memset(arr, 0, sizeof(T) * N);
    }

    // Zero fills an array of POD or built-in types, with array length provided by the caller.
    template<class T>
    void clearArray(T* arrayPtr, const size_t arrayLength) noexcept
    {
        static_assert(std::is_pod<T>::value, "Type must be Plain Old Data!");
        assert(arrayPtr != nullptr && arrayLength != 0);
        std::memset(arrayPtr, 0, sizeof(T) * arrayLength);
    }

    using ByteArray = std::vector<uint8_t>;

    struct FourCC final
    {
        uint8_t c0 = 0, c1 = 0, c2 = 0, c3 = 0;
    };

    bool operator==(FourCC a, FourCC b) noexcept;
    bool operator!=(FourCC a, FourCC b) noexcept;

    bool operator==(FourCC a, const char fccStr[]) noexcept;
    bool operator!=(FourCC a, const char fccStr[]) noexcept;

    std::ostream& operator<<(std::ostream& s, const FourCC& fcc);

    class BinaryReader final
    {
    public:
        explicit BinaryReader(std::istream& stream);
        BinaryReader(ByteArray data);

        void readBytes(void* buffer, size_t numBytes);
        void skipBytes(const size_t numBytes);

        template<typename T>
        T read()
        {
            // TODO: endian
            T type;
            readBytes(&type, sizeof(T));
            return type;
        }

        bool readFourCC(FourCC& fourCC)
        {
            if (readPosition == data.size() || (readPosition + sizeof(FourCC) > data.size()))
                return false;

            const unsigned char* dataPtr = data.data() + readPosition;
            std::memcpy(&fourCC, dataPtr, sizeof(FourCC));
            readPosition += sizeof(FourCC);

            return true;
        }

        std::string readString()
        {
            // std::getline ???

            std::string tmp;
            for (;;)
            {
                char c = '\0';
                readBytes(&c, 1);
                if (c == '\0')
                {
                    break;
                }
                tmp.push_back(c);
            }

            return tmp;
        }

    private:
        size_t readPosition = 0;
        const ByteArray data;
    };

    inline BinaryReader::BinaryReader(ByteArray data) :
        data(data)
    {
    }

    inline BinaryReader::BinaryReader(std::istream& stream) :
        data(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>())
    {
    }

} // namespace ehb