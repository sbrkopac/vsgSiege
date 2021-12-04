
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ehb
{
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
    } // namespace stringtool
} // namespace ehb