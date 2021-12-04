
#pragma once

#include "io/NamingKeyMap.hpp"
#include <vsg/io/ReaderWriter.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    class IFileSys;
    class ReaderWriterRegion final : public vsg::Inherit<vsg::ReaderWriter, ReaderWriterRegion>
    {
    public:
        ReaderWriterRegion(IFileSys& fileSys);

        virtual vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> = {}) const override;
        virtual vsg::ref_ptr<vsg::Object> read(std::istream& stream, vsg::ref_ptr<const vsg::Options> = {}) const override;

    private:
        IFileSys& fileSys;

        std::shared_ptr<spdlog::logger> log;
    };
} // namespace ehb
