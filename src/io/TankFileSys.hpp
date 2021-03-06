
#pragma once

#include <filesystem>
#include <optional>

#include "IFileSys.hpp"
#include "tank/TankFile.hpp"

#include <spdlog/spdlog.h>

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
    class TankFileSys : public IFileSys
    {
    public:
        virtual ~TankFileSys() = default;

        virtual bool init(IConfig& config) override;

        virtual InputStream createInputStream(const std::string& filename) override;

        virtual FileList getFiles() const override;
        virtual FileList getDirectoryContents(const std::string& directory) const override;

    private:
        struct TankEntry
        {
            TankFile tank;
            TankFile::Reader reader;
        };

    private:
        //! store tank files, this vector removed duplicates and orders by priority
        std::vector<std::unique_ptr<TankEntry>> eachTank;

        //! contains a full list of files from the tanks that are loaded
        FileList cache;

        //! the optional bits path
        std::optional<fs::path> bits;

        std::shared_ptr<spdlog::logger> log;
    };
} // namespace ehb
