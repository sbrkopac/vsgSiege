
#include "ASPMeshTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/MatrixTransform.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    void ASPMeshTestState::enter()
    {
        log->info("{}", name());

        vsg::StateGroup& scene3d = *systems.scene3d;
        vsg::Options& options = *systems.options;

        // decorate the graph with the pipeline
        // TODO: this should really be done somewhere else i think?
        if (vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline")); pipeline) { scene3d.addChild(pipeline); }
        else
        {
            log->critical("failed to find a graphics pipeline");
        }

        static std::string model("m_c_gah_fg_pos_a1");

        if (vsg::ref_ptr<vsg::Group> asp = vsg::read_cast<vsg::Group>(model, vsg::ref_ptr<vsg::Options>(&options)); asp != nullptr)
        {
            auto t1 = vsg::MatrixTransform::create();
            t1->addChild(asp);

            scene3d.addChild(t1);

            // workaround
            compile(systems, systems.scene3d);
        }
    }

    void ASPMeshTestState::leave() {}

    void ASPMeshTestState::update(double deltaTime) {}
} // namespace ehb
