
#include "Region.hpp"

namespace ehb
{

    GenerateGlobalSiegeNodeGuidToNodeXformMap::GenerateGlobalSiegeNodeGuidToNodeXformMap(SiegeNodeGuidToXformMap& map) :
        map(map) {}

    void GenerateGlobalSiegeNodeGuidToNodeXformMap::apply(vsg::Node& node) { node.traverse(*this); }

    void GenerateGlobalSiegeNodeGuidToNodeXformMap::apply(Region& region)
    {
        region.traverse(*this);
    }

    void GenerateGlobalSiegeNodeGuidToNodeXformMap::apply(vsg::MatrixTransform& t)
    {
        uint32_t guid = 0;
        t.getValue("guid", guid);

        if (guid == 0)
        {
            spdlog::get("log")->critical("There is a SiegeNode transform in the graph without a guid");
            return;
        }

        map.emplace(guid, &t);
    }

    CalculateAndPlaceObjects::CalculateAndPlaceObjects(SiegeNodeGuidToXformMap& map) :
        map(map) {}

    void CalculateAndPlaceObjects::apply(vsg::Node& node) { node.traverse(*this); }

    void CalculateAndPlaceObjects::apply(vsg::MatrixTransform& t)
    {
#if 0
        // ReaderWriterSiegeNodeList
        // there should only be one child under the transform
        if (t.children.size() != 0)
        {
            if (auto aspect = t.children[0].cast<Aspect>())
            {
                // local rotation of the object
                SiegePos pos;
                t.getValue("position", pos);
                SiegeRot rot;
                t.getValue("orientation", rot);

                // global rotation of the node this is applied to
                auto gt = map[pos.guid];

                t.matrix = gt->matrix * vsg::dmat4(vsg::translate(pos.pos));
                t.matrix = t.matrix * vsg::dmat4(vsg::rotate(rot.rot));
            }
        }
#endif
        t.traverse(*this);
    }

    void Region::setObjects(vsg::ref_ptr<vsg::Group> objects)
    {
        CalculateAndPlaceObjects visitor(placedNodeXformMap);
        objects->accept(visitor);

        addChild(objects);
    }

} // namespace ehb