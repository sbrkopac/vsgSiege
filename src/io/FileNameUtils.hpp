
#pragma once

#include "IFileSys.hpp"
#include "vsg/io/FileSystem.h"
#include <memory>

namespace ehb
{
    class LocalFileSys : public IFileSys
    {
    public:
        LocalFileSys();

        virtual ~LocalFileSys() = default;

        virtual InputStream createInputStream(const std::string& filename) override;

        virtual FileList getFiles() const override;
        virtual FileList getDirectoryContents(const std::string& directory) const override;

        void setDir(const vsg::Path& directory);

    private:
        vsg::Path bitsDir;
    };

    inline LocalFileSys::LocalFileSys() {}

    inline void LocalFileSys::setDir(const vsg::Path& directory) { bitsDir = directory; }
} // namespace ehb
