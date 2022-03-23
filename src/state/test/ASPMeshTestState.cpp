
#include "ASPMeshTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"
#include "world/Aspect.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/MatrixTransform.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    class AspectMeshCounter : public SiegeVisitorBase
    {
    public:

        int32_t count = 0;

        virtual void apply(Aspect& mesh) override
        {
            count++;
        }
    };
}

namespace ehb
{
    void ASPMeshTestState::enter()
    {
        log->info("{}", name());

        vsg::StateGroup& scene3d = *systems.scene3d;
        auto options = systems.options;

        static std::string model("m_c_gah_fg_pos_a1");

        if (auto asp = vsg::read_cast<Aspect>(model, options); asp != nullptr)
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> bindGraphicsPipeline(options->getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(bindGraphicsPipeline);
            scene3d.addChild(asp);

            AspectMeshCounter v;
            scene3d.traverse(v);

            log->info("Visitor has {} meshes", v.count);

            // workaround
            compile(systems, systems.scene3d);
        }
    }

    void ASPMeshTestState::leave() {}

    void ASPMeshTestState::update(double deltaTime) {}
} // namespace ehb
