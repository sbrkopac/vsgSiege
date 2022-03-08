
#include "TankFileSys.hpp"

#include "StringTool.hpp"
#include "cfg/IConfig.hpp"

#include <vsg/io/FileSystem.h>

#include <sstream>

// TODO: move or remove - legacy from: https://github.com/openscenegraph/OpenSceneGraph/blob/34a1d8bc9bba5c415c4ff590b3ea5229fa876ba8/src/osgDB/FileNameUtils.cpp#L86
namespace ehb
{
    const char UNIX_PATH_SEPARATOR = '/';
    const char WINDOWS_PATH_SEPARATOR = '\\';
    const char* const PATH_SEPARATORS = "/\\";

#if defined(WIN32) && !defined(__CYGWIN__)
    const char delimiterNative = WINDOWS_PATH_SEPARATOR;
    const char delimiterForeign = UNIX_PATH_SEPARATOR;
    const char envPathDelimiter = ';';
#else
    const char delimiterNative = UNIX_PATH_SEPARATOR;
    const char delimiterForeign = WINDOWS_PATH_SEPARATOR;
    const char envPathDelimiter = ':';
#endif

    // replace with std::filesystem::path::generic_string()?
    std::string convertFileNameToUnixStyle(const std::string& fileName)
    {
        std::string new_fileName(fileName);

        std::string::size_type slash = 0;
        while ((slash = new_fileName.find_first_of(WINDOWS_PATH_SEPARATOR, slash)) != std::string::npos)
        {
            new_fileName[slash] = UNIX_PATH_SEPARATOR;
        }

        return new_fileName;
    }
} // namespace ehb

namespace ehb
{
    InputStream TankFileSys::createInputStream(const std::string& filename_)
    {
        const std::string path = stringtool::convertToLowerCase(filename_);
        std::string_view filename{path};

        // the first location we check is the bits path
        if (bits)
        {
            // remove leading / if this is an absolute path in the filesystem
            if (path.front() == '/' || path.front() == '\\')
                filename.remove_prefix(1);

            auto path = *bits / fs::path(filename);

            if (auto stream = std::make_unique<std::ifstream>(path, std::ios_base::binary); stream->is_open())
            {
                return stream;
            }

            // file wasn't located in the bits so lets move onto the tanks
        }

        // iterate over the tanks to try and find our file
        for (auto& entry : eachTank)
        {
            if (auto data = entry->reader.extractResourceToMemory(entry->tank, path, false); data.size() != 0)
            {
                auto stream = std::make_unique<std::stringstream>();

                stream->write(reinterpret_cast<const char*>(data.data()), data.size());

                return stream;
            }
        }

        // file doesn't exist
        return {};
    }

    FileList TankFileSys::getFiles() const
    {
        return cache;
    }

    FileList TankFileSys::getDirectoryContents(const std::string& directory_) const
    {
        FileList result;

        std::string directory = stringtool::convertToLowerCase(directory_);
        if (directory.back() != '/') directory.push_back('/');

        for (const std::string& filename : cache)
        {
            // skip filesystem binary liquid files
            if (filename.find("dir.lqd20") != std::string::npos)
                ;

            if (filename.size() > directory.size() && filename.find(directory) == 0 && filename.find('/', directory.size() + 1) == std::string::npos)
            {
                result.emplace(filename);
            }
        }

        return result;
    }

    bool TankFileSys::init(IConfig& config)
    {
        log = spdlog::get("filesystem");

        std::unordered_multimap<std::string, std::string> defaultValues;

        FileList eachTankDir, eachTankFile;

        // the first pass we do is into the bits directory, if there are files in the bits
        // they shouldn't end up in final cache
        if (const std::string& bitsPath = config.getString("bits"); !bitsPath.empty())
        {
            bits = bitsPath;

            try
            {
                for (const auto& itr : fs::recursive_directory_iterator(bitsPath))
                {
                    const auto& filename = itr.path();

                    if (fs::is_directory(filename) || fs::is_regular_file(filename))
                    {
                        // this seems like a needless convert when dealing with local files?
                        auto path = convertFileNameToUnixStyle(filename.string().substr((*bits).string().size()));

                        cache.emplace(path);
                    }
                }
            }
            catch (std::exception& e)
            {
                log->error("TankFileSys - while parsing bits we got an error: {}", e.what());
            }
        }

        if (const std::string& dsInstallPath = config.getString("ds-install-path"); !dsInstallPath.empty())
        {
            defaultValues.emplace("map_paths", vsg::concatPaths(vsg::Path(dsInstallPath), vsg::Path("Maps")));
            // defaultValues.emplace("res_paths", osgDB::concatPaths(dsInstallPath, "DSLOA"));
            defaultValues.emplace("res_paths", vsg::concatPaths(vsg::Path(dsInstallPath), vsg::Path("Resources")));
        }
        else
        {
            log->error("--ds-install path not set. Please check registry settings or pass to the exe path manually");

            return false;
        }

        if (const std::string& dataDir = config.getString("data-dir"); !dataDir.empty())
        {
            defaultValues.emplace("mod_paths", vsg::concatPaths(vsg::Path(dataDir), vsg::Path("Mods")));
        }

        for (const std::string& key : {"map_paths", "mod_paths", "res_paths"})
        {
            bool useDefaultValues = true;

            if (std::string value = config.getString(key); !value.empty())
            {
                if (value[0] == '!')
                {
                    value.erase(0, 1);
                    useDefaultValues = false;
                }

                eachTankDir.emplace(value);
            }

            if (useDefaultValues)
            {
                const auto range = defaultValues.equal_range(key);

                for (auto itr = range.first; itr != range.second; ++itr)
                {
                    eachTankDir.emplace(itr->second);
                }
            }
        }

        // attempt to load each tank file
        for (const std::string& directory : eachTankDir)
        {
            log->info("[TankFileSys] looking for tank files under {}", directory);

            // TODO: this will crash if there is an empty directory
            for (const auto& entry : fs::directory_iterator(directory))
            {
                const fs::path ext = entry.path().extension();

                if (ext == ".dsm" || ext == ".dsmap" || ext == ".dsmod" || ext == ".dsr" || ext == ".dsres")
                {
                    const std::string fullFileName = entry.path().generic_string();

                    eachTankFile.emplace(fullFileName);
                }
            }
        }

        // attempt to load each tank file
        for (const std::string& fullFileName : eachTankFile)
        {
            log->info("[TankFileSys] attempting to index {}", fullFileName);

            auto entry = std::make_unique<TankEntry>();

            entry->tank.openForReading(fullFileName);
            entry->reader.indexFile(entry->tank);

            { // cache the entire list of files...
                const auto list = entry->reader.getFileList();

                std::merge(std::begin(cache), std::end(cache), std::begin(list), std::end(list), std::inserter(cache, std::end(cache)));
            }
            { // ...and directories
                auto list = entry->reader.getDirectoryList();

                for (auto& directory : list)
                {
                    directory.pop_back();
                }

                std::merge(std::begin(cache), std::end(cache), std::begin(list), std::end(list), std::inserter(cache, std::end(cache)));
            }

            eachTank.emplace_back(std::move(entry));
        }

        // remove duplicate tanks
        std::sort(eachTank.begin(), eachTank.end(), [](const auto& lhs, const auto& rhs) {
            return lhs->tank.getFileHeader().dataCrc32 < rhs->tank.getFileHeader().dataCrc32;
        });

        const auto last = std::unique(eachTank.begin(), eachTank.end(), [](const auto& lhs, const auto& rhs) {
            return lhs->tank.getFileHeader().dataCrc32 == rhs->tank.getFileHeader().dataCrc32;
        });

        eachTank.erase(last, eachTank.end());

        // sort tanks based on priority
        std::sort(eachTank.begin(), eachTank.end(), [](const auto& lhs, const auto& rhs) {
            return lhs->tank.getFileHeader().priority > rhs->tank.getFileHeader().priority;
        });

        // don't want root entry
        cache.erase("/");

        return true;
    }
} // namespace ehb
