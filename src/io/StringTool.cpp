
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

        std::string wideStringToStdString(const WideString& wStr)
        {
            static_assert(sizeof(WideChar) == 2, "This will only work if we are dealing with 2-byte wchars!");

            //
            // Convert 2-byte long Windows wchar_t string to a C string.
            //
            // Currently not doing a proper conversion, just grabbing
            // the lower byte of each WideChar.
            //
            // Also uses a fixed size buffer, so string length is
            // limited to MaxTempStringLen!
            //

            if (wStr.empty())
            {
                return std::string();
            }

            int i;
            char temBuf[2048];

            for (i = 0; i < (2048 - 1); ++i)
            {
                const char c = static_cast<char>(wStr[i] & 0x00FF);
                temBuf[i] = c;
                if (c == 0)
                {
                    break;
                }
            }

            if (i == (2048 - 1))
            {
                return {};
            }

            temBuf[i] = '\0';
            return temBuf;
        }

        std::string toString(const WideString& wStr)
        {
            return wStr.empty() ? "<EMPTY>" : ("\"" + wideStringToStdString(wStr) + "\"");
        }

        std::string removeTrailingFloatZeros(const std::string& floatStr)
        {
            // Only process if the number is decimal (has a dot somewhere):
            if (floatStr.find_last_of('.') == std::string::npos)
            {
                return floatStr;
            }

            std::string trimmed(floatStr);

            // Remove trailing zeros:
            while (!trimmed.empty() && (trimmed.back() == '0'))
            {
                trimmed.pop_back();
            }

            // If the dot was left alone at the end, remove it too:
            if (!trimmed.empty() && (trimmed.back() == '.'))
            {
                trimmed.pop_back();
            }

            return trimmed;
        }

        std::string formatMemoryUnit(uint64_t memorySizeInBytes, bool abbreviated)
        {
            const char* memUnitStr;
            double adjustedSize;
            char numStrBuf[128];

            if (memorySizeInBytes < 1024)
            {
                memUnitStr = (abbreviated ? "B" : "Bytes");
                adjustedSize = static_cast<double>(memorySizeInBytes);
            }
            else if (memorySizeInBytes < (1024 * 1024))
            {
                memUnitStr = (abbreviated ? "KB" : "Kilobytes");
                adjustedSize = (memorySizeInBytes / 1024.0);
            }
            else if (memorySizeInBytes < (1024 * 1024 * 1024))
            {
                memUnitStr = (abbreviated ? "MB" : "Megabytes");
                adjustedSize = (memorySizeInBytes / 1024.0 / 1024.0);
            }
            else
            {
                memUnitStr = (abbreviated ? "GB" : "Gigabytes");
                adjustedSize = (memorySizeInBytes / 1024.0 / 1024.0 / 1024.0);
            }

            // We only care about the first 2 decimal digits.
            std::snprintf(numStrBuf, sizeof(numStrBuf), "%.2f", adjustedSize);

            // Remove trailing zeros if no significant decimal digits:
            return removeTrailingFloatZeros(numStrBuf) + std::string(" ") + memUnitStr;
        }
    } // namespace stringtool
} // namespace ehb