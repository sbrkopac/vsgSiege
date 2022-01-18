
#include "Region.hpp"

namespace ehb
{
    void Region::setObjects(vsg::ref_ptr<vsg::Group> objects)
    {
        CalculateAndPlaceObjects visitor(placedNodeXformMap);
        objects->accept(visitor);

        addChild(objects);
    }
} // namespace ehb