
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

        static std::string model("m_c_gah_fg_pos_a1");

        if (vsg::ref_ptr<vsg::Group> asp = vsg::read_cast<vsg::Group>(model, vsg::ref_ptr<vsg::Options>(&options)); asp != nullptr)
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> bindGraphicsPipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(bindGraphicsPipeline);
            scene3d.addChild(asp);

            // workaround
            compile(systems, systems.scene3d);
        }
    }

    void ASPMeshTestState::leave() {}

    void ASPMeshTestState::update(double deltaTime) {}
} // namespace ehb
