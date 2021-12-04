
#include "Region.hpp"

namespace ehb
{
    void Region::setNodeData(vsg::ref_ptr<vsg::Group> nodes)
    {
        GenerateGlobalGuidToNodeXformMap visitor(placedNodeXformMap);
        nodes->accept(visitor);

        addChild(nodes);
    }

    void Region::setObjects(vsg::ref_ptr<vsg::Group> objects)
    {
        CalculateAndPlaceObjects visitor(placedNodeXformMap);
        objects->accept(visitor);

        addChild(objects);
    }
} // namespace ehb