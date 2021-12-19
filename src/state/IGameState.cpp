
// This file is temporary for the compile workaround

#include "state/IGameState.hpp"

#include "Systems.hpp"

#include <vsg/vk/ResourceRequirements.h>

#include <spdlog/fmt/ostr.h> // must be included

namespace vsg
{
    std::ostream& operator<<(std::ostream& ostr, const ResourceRequirements& reqs)
    {
        return ostr << "Resource Requirements calculated -"
                    << " maxSlot: " << reqs.maxSlot;
    }
} // namespace vsg

namespace ehb
{
    void IGameState::compile(Systems& systems, vsg::ref_ptr<vsg::Object> object)
    {
        vsg::CollectResourceRequirements collectRequirements;
        object->accept(collectRequirements);
        systems.viewer->compile(collectRequirements.createResourceHints());

        spdlog::get("log")->info("{}", collectRequirements.requirements);
    }
} // namespace ehb