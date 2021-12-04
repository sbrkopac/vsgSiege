
#pragma once

#include "Fuel.hpp"
#include <algorithm>
#include <functional>
#include <istream>
#include <memory>
#include <set>
#include <string>

#include <vsg/io/FileSystem.h>

#ifdef WIN32
#    include <filesystem>
namespace fs = std::filesystem;
#else
#    include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace ehb
{
    typedef std::set<std::string> FileList;
    typedef std::unique_ptr<std::istream> InputStream;

    class IConfig;
    class IFileSys
    {
    public:
        virtual ~IFileSys() = default;

        virtual void init(IConfig& config) = 0;

        virtual InputStream createInputStream(const std::string& filename) = 0;

        virtual FileList getFiles() const = 0;
        virtual FileList getDirectoryContents(const std::string& directory) const = 0;

        void eachGasFile(const std::string& directory, std::function<void(const std::string&, std::unique_ptr<Fuel>)> func);
        std::unique_ptr<Fuel> openGasFile(const std::string& file);
    };

    inline std::string getSimpleFileName(const std::string& fileName)
    {
        fs::path path(fileName);
        return path.filename().string();
    }

    inline void IFileSys::eachGasFile(const std::string& directory, std::function<void(const std::string&, std::unique_ptr<Fuel>)> func)
    {
        for (const auto& filename : getFiles())
        {
            if (vsg::lowerCaseFileExtension(filename) == "gas")
            {
                if (filename.find(directory) == 0)
                {
                    if (auto stream = createInputStream(filename))
                    {
                        if (auto doc = std::make_unique<Fuel>(); doc->load(*stream)) { func(filename, std::move(doc)); }
                        else
                        {
                            // log->error("{}: could not parse", filename);
                        }
                    }
                    else
                    {
                        // log->error("{}: could not create input stream", filename);
                    }
                }
            }
        }
    }

    inline std::unique_ptr<Fuel> IFileSys::openGasFile(const std::string& file)
    {
        if (InputStream stream = createInputStream(file))
        {
            if (std::unique_ptr<Fuel> gas = std::make_unique<Fuel>(); gas->load(*stream))
            {
                return gas;
            }
        }

        return nullptr;
    }
} // namespace ehb
