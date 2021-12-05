
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
        log->info("Entered Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        vsg::Options& options = *systems.options;

        static std::string model("m_c_gah_fg_pos_a1");

        TimePoint start = Timer::now();

        if (vsg::ref_ptr<vsg::Group> asp = vsg::read_cast<vsg::Group>(model, vsg::ref_ptr<vsg::Options>(&options)); asp != nullptr)
        {
            auto t1 = vsg::MatrixTransform::create();
            t1->addChild(asp);

            scene3d.addChild(t1);
        }
    }

    void ASPMeshTestState::leave() {}

    void ASPMeshTestState::update(double deltaTime) {}
} // namespace ehb
