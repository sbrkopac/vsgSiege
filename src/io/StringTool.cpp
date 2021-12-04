
#include "StringTool.hpp"

#include <algorithm>

namespace ehb
{
    namespace stringtool
    {
        void trimLeading(std::string& str) noexcept
        {
            std::size_t startpos = str.find_first_not_of(" \t");
            if (std::string::npos != startpos) { str = str.substr(startpos); }
        }

        void trimTrailing(std::string& str) noexcept
        {
            std::size_t endpos = str.find_last_not_of(" \t\r\n");
            if (endpos != std::string::npos) { str = str.substr(0, endpos + 1); }
        }

        // in preliminary tests with NamingKeyMap calling trimLeading and trimTrailing had overhead
        void trim(std::string& str) noexcept
        {
            if (!str.size()) return;
            std::string::size_type first = str.find_first_not_of(" \t");
            std::string::size_type last = str.find_last_not_of("  \t\r\n");
            str = str.substr(first, last - first + 1);
        }

        bool startsWith(std::string_view str, std::string_view match) noexcept { return str.length() >= match.length() && 0 == str.compare(0, match.length(), match); }

        bool endsWith(std::string_view str, std::string_view match) noexcept { return str.length() >= match.length() && 0 == str.compare(str.length() - match.length(), match.length(), match); }

        // potential slowdown on the explicit conversion from string_view to string
        std::string stringBetween(std::string_view str, const char& startChar, const char& endChar) noexcept
        {
            auto start = str.find_first_of(startChar);
            if (start == std::string::npos) return {};

            auto end = str.find_first_of(endChar, start);
            if (end == std::string::npos) return {};

            if ((end - start) - 1 == 0) return {};

            return std::string{str.substr(start + 1, (end - start) - 1)};
        }

        // potential slowdown on the explicit conversion from string_view to string
        std::vector<std::string> split(std::string_view str, const char& separator) noexcept
        {
            std::vector<std::string> elements;

            std::string::size_type prev_pos = 0, pos = 0;

            while ((pos = str.find(separator, pos)) != std::string::npos)
            {
                auto substring = str.substr(prev_pos, pos - prev_pos);
                elements.push_back(std::string{substring});
                prev_pos = ++pos;
            }

            elements.push_back(std::string{str.substr(prev_pos, pos - prev_pos)});

            return elements;
        }

        std::string convertToLowerCase(const std::string& str) noexcept
        {
            std::string lowcase_str(str);
            std::transform(lowcase_str.begin(), lowcase_str.end(), lowcase_str.begin(), ::tolower);
            return lowcase_str;
        }
    } // namespace stringtool
} // namespace ehb