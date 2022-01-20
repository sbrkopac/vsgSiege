
#include "WritableConfig.hpp"
#include <cassert>
#include <spdlog/spdlog.h>

#ifdef WIN32
#    include <shlobj.h>
#    include <windows.h>

#    include <filesystem>
namespace fs = std::filesystem;
#else
#    include <basedir.h>

#    include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace ehb
{
    // TODO: is there a better way to do this?
    bool createDirectory(const fs::path& value)
    {
        try
        {
            fs::create_directory(value);
        }

        catch (std::exception& exception)
        {
            spdlog::get("log")->error("Error creating path for [{}] - [{}]", value.string(), exception.what());

            return false;
        }

        return true;
    }

    // TODO: make an IConfigSrc class
    // TODO: convert this into an implementation of the IConfigSrc class
    void userConfig(WritableConfig& config)
    {

#ifdef WIN32
        CHAR path[MAX_PATH];
#else
        // grab the freedesktop.org specific folders to use for cache, config, and user data
        xdgHandle xdg;
        xdgInitHandle(&xdg);
#endif

#ifdef WIN32
        if (SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
#else
        if (const char* path = xdgCacheHome(&xdg))
#endif
        {
            const fs::path value = fs::path(path) / "OpenSiege";

            if (createDirectory(value)) { config.setString("cache-dir", value.string()); }
        }

#ifdef WIN32
        if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
#else
        if (const char* path = xdgConfigHome(&xdg))
#endif
        {
            const fs::path value = fs::path(path) / "OpenSiege";

            if (createDirectory(value)) { config.setString("config-dir", value.string()); }
        }

#ifdef WIN32
        if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, path) == S_OK)
#else
        if (const char* path = xdgDataHome(&xdg))
#endif
        {
            const fs::path value = fs::path(path) / "OpenSiege";

            if (createDirectory(value)) { config.setString("data-dir", value.string()); }
        }

#ifndef WIN32
        xdgWipeHandle(&xdg);
#endif

        // platform independent folders and setup

        assert(!config.getString("data-dir", "").empty());
        assert(!config.getString("config-dir", "").empty());
        assert(!config.getString("cache-dir", "").empty());

        const fs::path data = config.getString("data-dir", "");

        const fs::path logs = data / "Logs";
        const fs::path shots = data / "Screen Shots";

        if (createDirectory(logs)) config.setString("logs_path", logs.string());
        if (createDirectory(shots)) config.setString("shots_path", shots.string());
    }
} // namespace ehb
