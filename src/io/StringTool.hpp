
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ehb
{
    // Dungeon Siege used WCHAR or wchar_t for some text strings,
    // which is 16bits wide on Windows. They are saved to file
    // as a pair of bytes so we need to use a properly sized type,
    // since the assumption that wchar_t is 16bits everywhere is
    // false. It is 32bits wide on Mac and Linux.
    using WideChar = char16_t;

    // Redefine std::string for the WideChar type.
    using WideString = std::basic_string<WideChar>;

    namespace stringtool
    {
        // trim leading spaces/tabs
        void trimLeading(std::string& str) noexcept;

        // trim trailing spaces/tabs
        void trimTrailing(std::string& str) noexcept;

        // trim line by trimming leading and trailing characters
        void trim(std::string& str) noexcept;

        // return true if string starts with match string
        bool startsWith(std::string_view str, std::string_view match) noexcept;

        // return true if string end with match string
        bool endsWith(std::string_view str, std::string_view match) noexcept;

        // returns the string between the start and end character
        std::string stringBetween(std::string_view str, const char& startChar, const char& endChar) noexcept;

        // split a string into a vector delineated by separator
        std::vector<std::string> split(std::string_view str, const char& separator) noexcept;

        // every time you assume characters are ascii - god kills a kitten
        std::string convertToLowerCase(const std::string& str) noexcept;

        std::string wideStringToStdString(const WideString& wStr);

        // converts a wide string (16 bytes)
        std::string toString(const WideString& wstr);

        std::string removeTrailingFloatZeros(const std::string& floatStr);

        std::string formatMemoryUnit(uint64_t memorySizeInBytes, bool abbreviated = false);
    } // namespace stringtool
} // namespace ehb