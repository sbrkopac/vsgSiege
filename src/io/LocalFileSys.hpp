
#pragma once

#include "IFileSys.hpp"
#include "vsg/io/FileSystem.h"
#include <memory>

#include <spdlog/spdlog.h>

// VSG currently doesn't use the standard filesystem library
#ifdef WIN32
#    include <filesystem>
namespace fs = std::filesystem;
#else
#    include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace ehb
{
    class IConfig;
    class LocalFileSys final : public IFileSys
    {
    public:
        LocalFileSys() = default;

        virtual ~LocalFileSys() = default;

        virtual void init(IConfig& config) override;

        virtual InputStream createInputStream(const std::string& filename) override;

        virtual FileList getFiles() const override;
        virtual FileList getDirectoryContents(const std::string& directory) const override;

    private:
        fs::path bitsDir;

        std::shared_ptr<spdlog::logger> log;
    };
} // namespace ehb
