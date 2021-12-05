
#include "NamingKeyMap.hpp"

#include <algorithm>
#include <sstream>
#include <list>

#include <spdlog/spdlog.h>

#include "IFileSys.hpp"
#include "StringTool.hpp"

namespace ehb
{
    static void parseTree(std::unordered_map<std::string, std::string>& namingKeyMap, std::istream& stream)
    {
        for (std::string line; std::getline(stream, line);)
        {
            stringtool::trim(line);

            if (line.empty()) { continue; }

            if (line.front() != '#')
            {
                std::stringstream lineStream(line);

                std::string type, key, value, description, extra;

                // split an entry into its components
                std::getline(lineStream, type, '=');
                std::getline(lineStream, key, ',');
                std::getline(lineStream, value, ',');
                std::getline(lineStream, description, ',');
                std::getline(lineStream, extra);

                // make sure we have no rogue characters hanging around (looking at you \t)
                stringtool::trim(type);
                stringtool::trim(key);
                stringtool::trim(value);
                stringtool::trim(extra);

                // CANIMCLASS is unhandled, not sure if we will need it in the future
                if (type == "TREE")
                {
                    // some keys values will be in windows style paths such as "World\AI\Jobs"
                    // my guess is in native DS this is actually turned into fuel handles and replaced with a ':'
                    std::replace(value.begin(), value.end(), '\\', '/');

                    // case sensitivity important for linux
                    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
                    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
                    std::transform(extra.begin(), extra.end(), extra.begin(), ::tolower);

                    std::string fullFileName;

                    if (auto index = key.find_last_of('_'); index != std::string::npos)
                    {
                        if (auto itr = namingKeyMap.find(key.substr(0, index)); itr != namingKeyMap.end())
                        {
                            fullFileName += itr->second;
                            fullFileName += value;
                            fullFileName += '/';

                            namingKeyMap.emplace(key, fullFileName);

                            // take care of fighting stances
                            if (!extra.empty())
                            {
                                std::stringstream extraStream(extra);
                                std::string stance;

                                while (std::getline(extraStream, stance, ','))
                                {
                                    stringtool::trim(stance);

                                    namingKeyMap.emplace(key + '_' + stance, fullFileName + stance + '/');
                                }
                            }

                            continue;
                        }
                    }

                    fullFileName += value;

                    if (!fullFileName.empty()) { fullFileName += '/'; }

                    namingKeyMap.emplace(key, fullFileName);
                }
            }
        }

        spdlog::get("log")->info("...succeeded, parsed {} standard entries", namingKeyMap.size());
    }

    void NamingKeyMap::init(IFileSys& fileSys)
    {
        std::set<std::string> std, ext;

        for (const auto& filename : fileSys.getDirectoryContents("/art"))
        {
            if (vsg::lowerCaseFileExtension(filename) == "nnk")
            {
                const std::string lowerCaseFileName = stringtool::convertToLowerCase(getSimpleFileName(filename));

                if (lowerCaseFileName.compare(0, 9, "namingkey") == 0) { std.emplace(filename); }
                else
                {
                    ext.emplace(filename);
                }
            }
        };

        std::list<std::string> eachFileName;

        for (auto itr = std.rbegin(); itr != std.rend(); ++itr)
        {
            eachFileName.push_back(*itr);

            break;
        }

        for (auto itr = ext.begin(); itr != ext.end(); ++itr)
        {
            eachFileName.push_back(*itr);
        }

        for (const std::string& filename : eachFileName)
        {
            std::stringstream stream;

            if (auto stream = fileSys.createInputStream(filename))
            {
                spdlog::get("log")->info("loading NAMINGKEY file: {}", filename);

                // process it...
                parseTree(keyMap, *stream);
            }
        }
    }

    std::string ehb::NamingKeyMap::findDataFile(const std::string& filename) const
    {
        std::string actualFileName = filename;

        if (filename.find_first_of('/') == std::string::npos)
        {
            std::string resolvedFileName;

            for (std::string::size_type index = filename.rfind('_'); index != std::string::npos && index != 0; index = filename.rfind('_', index - 1))
            {
                auto itr = keyMap.find(filename.substr(0, index));

                if (itr != keyMap.end())
                {
                    resolvedFileName = itr->second + filename;
                    break;
                }
            }

            if (!resolvedFileName.empty())
            {
                const std::string prefix = filename.substr(0, 2);

                if (prefix == "a_" || prefix == "b_" || prefix == "m_" || prefix == "t_")
                {
                    actualFileName.clear();

                    actualFileName += "/art/";
                    actualFileName += resolvedFileName;
                }
            }
        }

        return actualFileName;
    }
} // namespace ehb
