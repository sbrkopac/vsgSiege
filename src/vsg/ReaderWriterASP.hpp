
#pragma once

#include <vsg/core/Inherit.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/Group.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    class IFileSys;
    class ReaderWriterASP final : public vsg::Inherit<vsg::ReaderWriter, ReaderWriterASP>
    {
    public:
        ReaderWriterASP(IFileSys& fileSys);

        virtual vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> = {}) const override;
        virtual vsg::ref_ptr<vsg::Object> read(std::istream& stream, vsg::ref_ptr<const vsg::Options> = {}) const override;

    private:
        IFileSys& fileSys;

        std::shared_ptr<spdlog::logger> log;
    };
} // namespace ehb
