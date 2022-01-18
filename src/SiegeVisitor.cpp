
#include "SiegeVisitor.hpp"

#include "world/SiegeNode.hpp"
#include "world/Aspect.hpp"

namespace ehb
{
    bool SiegeVisitorBase::handleCustomGroups(vsg::Group& group)
    {
        if (auto sno = group.cast<SiegeNodeMesh>())
        {
            apply(*sno);

            return true;
        }
        else if (auto asp = group.cast<Aspect>())
        {
            apply(*asp);

            return true;
        }

        return false;
    }
} // namespace ehb