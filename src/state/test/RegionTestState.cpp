
#include "RegionTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"
#include "world/Region.hpp"

#include <spdlog/spdlog.h>

#include <vsg/io/read.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/ResourceRequirements.h>

namespace ehb
{
    class SiegeNodeMeshCounter : public SiegeVisitorBase
    {
    public:

        int32_t count = 0;

        virtual void apply(vsg::Object& object) override
        {
            object.traverse(*this);
        }

        virtual void apply(SiegeNodeMesh& mesh) override
        {
            count++;
        }
    };
}

namespace ehb
{
    void RegionTestState::enter()
    {
        log->info("Entered Region Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        vsg::Options& options = *systems.options;

        static std::string region = "town_center";
        static std::string regionpath = "/world/maps/multiplayer_world/regions/" + region + ".region"; // extension for the loader

        if (auto region = vsg::read_cast<Region>(regionpath, vsg::ref_ptr<vsg::Options>(&options)))
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(pipeline);
            scene3d.addChild(region);

            SiegeNodeMeshCounter v;
            region->traverse(v);

            log->info("Visitor has {} meshes", v.count);

            // workaround
            compile(systems, systems.scene3d);
        }
    }

    void RegionTestState::leave() {}

    void RegionTestState::update(double deltaTime) {}
} // namespace ehb
