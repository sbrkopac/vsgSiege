
#include "SiegeNodeTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/MatrixTransform.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    void SiegeNodeTestState::enter()
    {
        log->info("Entered Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        vsg::Options& options = *systems.options;

        // decorate the graph with the pipeline
        // TODO: this should really be done somewhere else i think?
        if (vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline")); pipeline) { scene3d.addChild(pipeline); }
        else
        {
            log->critical("failed to find a graphics pipeline");
        }

        static std::string siegeNode("t_grs01_houses_generic-a-log");
        //static std::string siegeNode("t_xxx_flr_04x04-v0");

        TimePoint start = Timer::now();

        if (vsg::ref_ptr<vsg::Group> sno = vsg::read_cast<vsg::Group>(siegeNode, vsg::ref_ptr<vsg::Options>(&options)); sno != nullptr)
        {
            auto t1 = vsg::MatrixTransform::create();
            t1->addChild(sno);

            auto t2 = vsg::MatrixTransform::create();
            t2->addChild(sno);

            scene3d.addChild(t1);
            scene3d.addChild(t2);

            SiegeNodeMesh::connect(t1, 2, t2, 1);
        }

        Duration duration = Timer::now() - start;
        log->info("SiegeNodeTest entire state profiled @ {} milliseconds", duration.count());

        // workaround
        compile(systems, systems.scene3d);
    }

    void SiegeNodeTestState::leave() {}

    void SiegeNodeTestState::update(double deltaTime) {}
} // namespace ehb
