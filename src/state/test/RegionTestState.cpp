
#include "RegionTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"

#include <spdlog/spdlog.h>

#include <vsg/io/read.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>

namespace ehb
{
    void RegionTestState::enter()
    {
        log->info("Entered Region Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        vsg::Options& options = *systems.options;

        // decorate the graph with the pipeline
        // TODO: this should really be done somewhere else i think?
        if (vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline")); pipeline) { scene3d.addChild(pipeline); }
        else
        {
            log->critical("failed to find a graphics pipeline");
        }

        static std::string region = "town_center";
        static std::string regionpath = "world/maps/multiplayer_world/regions/" + region + ".region"; // extension for the loader

        if (auto region = vsg::read_cast<vsg::MatrixTransform>(regionpath, vsg::ref_ptr<vsg::Options>(&options))) { scene3d.addChild(region); }
    }

    void RegionTestState::leave() {}

    void RegionTestState::update(double deltaTime) {}
} // namespace ehb
