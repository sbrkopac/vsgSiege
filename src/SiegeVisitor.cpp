
#include "SiegeVisitor.hpp"

#include "world/Aspect.hpp"
#include "world/Region.hpp"
#include "world/SiegeNode.hpp"

namespace ehb
{
    bool SiegeVisitorBase::handleCustomGroups(vsg::Group& group)
    {
        if (auto region = group.cast<Region>())
        {
            apply(*region);

            return true;
        }
        else if (auto sno = group.cast<SiegeNode>())
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