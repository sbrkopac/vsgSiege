
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

        static std::string siegeNode("t_grs01_houses_generic-a-log");
        //static std::string siegeNode("t_xxx_flr_04x04-v0");

        TimePoint start = Timer::now();

        if (auto sno = vsg::read_cast<SiegeNodeMesh>(siegeNode, vsg::ref_ptr<vsg::Options>(&options)); sno != nullptr)
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(pipeline);

            // transforms are required to connect two nodes together using doors
            auto t1 = vsg::MatrixTransform::create();
            auto t2 = vsg::MatrixTransform::create();

            t1->addChild(sno);
            t2->addChild(sno);

            SiegeNodeMesh::connect(t1, 2, t2, 1);

            scene3d.addChild(t1);
            scene3d.addChild(t2);

            // workaround
            compile(systems, systems.scene3d);
        }
        else
        {
            log->critical("Unabel to load SiegeNode: {}", siegeNode);
        }

        Duration duration = Timer::now() - start;
        log->info("SiegeNodeTest entire state profiled @ {} milliseconds", duration.count());
    }

    void SiegeNodeTestState::leave() {}

    void SiegeNodeTestState::update(double deltaTime) {}
} // namespace ehb
