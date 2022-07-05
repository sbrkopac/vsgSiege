
#pragma once

#include <vsg/core/Inherit.h>
#include <vsg/io/ReaderWriter.h>
#include <vsg/nodes/Group.h>
#include <vsg/state/GraphicsPipeline.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    class IFileSys;
    class ReaderWriterSiegeMesh final : public vsg::Inherit<vsg::ReaderWriter, ReaderWriterSiegeMesh>
    {
    public:
        ReaderWriterSiegeMesh(IFileSys& fileSys);

        virtual vsg::ref_ptr<vsg::Object> read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> = {}) const override;
        virtual vsg::ref_ptr<vsg::Object> read(std::istream& stream, vsg::ref_ptr<const vsg::Options> = {}) const override;

        vsg::ref_ptr<vsg::BindGraphicsPipeline> createOrShareGraphicsPipeline();

    private:
        IFileSys& fileSys;

        vsg::ref_ptr<vsg::BindGraphicsPipeline> bindGraphicsPipeline;

        std::shared_ptr<spdlog::logger> log;
    };
} // namespace ehb
