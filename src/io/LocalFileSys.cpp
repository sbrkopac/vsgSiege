
#include "LocalFileSys.hpp"
#include "cfg/IConfig.hpp"
#include "io/StringTool.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>

namespace ehb
{
    void LocalFileSys::init(IConfig& config)
    {
        log = spdlog::get("log");

        bitsDir = config.getString("bits");
    }

    InputStream LocalFileSys::createInputStream(const std::string& filename_)
    {
        if (bitsDir.empty()) { log->error("bits directory is empty... your application might bomb"); }

        std::string filename = stringtool::convertToLowerCase(filename_);

        if (filename.front() == '/' || filename.front() == '\\') { filename.erase(0, 1); }

        auto path = bitsDir / filename;

        if (auto stream = std::make_unique<std::ifstream>(bitsDir / filename, std::ios_base::binary); stream->is_open()) { return stream; }

        return {};
    }

    FileList LocalFileSys::getFiles() const
    {
        FileList result;

        try
        {
            for (auto& itr : fs::recursive_directory_iterator(bitsDir))
            {
                const auto& filename = itr.path();

                if (fs::is_directory(filename) || fs::is_regular_file(filename))
                {
                    auto unixStylePath = filename.generic_string().substr(bitsDir.string().size());
                    result.emplace(stringtool::convertToLowerCase(unixStylePath));
                }
            }
        }
        catch (std::exception& e)
        {
            log->warn("LocalFileSys::getFiles(): {}", e.what());
        }

        return result;
    }

    FileList LocalFileSys::getDirectoryContents(const std::string& directory_) const
    {
        std::string directory = stringtool::convertToLowerCase(directory_);
        if (directory.front() == '/' || directory.front() == '\\') { directory.erase(0, 1); }

        FileList result;

        try
        {
            for (auto& itr : fs::directory_iterator(bitsDir / directory))
            {
                const auto& filename = itr.path();

                if (fs::is_directory(filename) || fs::is_regular_file(filename))
                {
                    auto unixStylePath = filename.generic_string().substr(bitsDir.string().size());
                    result.emplace(stringtool::convertToLowerCase(unixStylePath));
                }
            }
        }
        catch (std::exception& e)
        {
            log->warn("LocalFileSys::getDirectoryContents({}): {}", directory, e.what());
        }

        return result;
    }
} // namespace ehb
