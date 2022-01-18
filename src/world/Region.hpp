
#pragma once

#include <vsg/core/Visitor.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>

#include <unordered_map>

#include <spdlog/spdlog.h>

#include "gas/Fuel.hpp"
// #include "vsg/Aspect.hpp"
#include "world/Region.hpp"
#include "world/SiegeNode.hpp"

namespace ehb
{
    using SiegeNodeGuidToXformMap = std::unordered_map<uint32_t, vsg::ref_ptr<vsg::MatrixTransform>>;

    // hold all region data
    class Region : public vsg::Inherit<vsg::Group, Region>
    {
    public:
        Region() = default;
        ~Region() = default;

        void setObjects(vsg::ref_ptr<vsg::Group> objects);

        uint32_t guid = 0;

        SiegeNodeGuidToXformMap placedNodeXformMap; // holds the final matrix transform
                                           // against the node guid
    };

    struct GenerateGlobalSiegeNodeGuidToNodeXformMap : public SiegeVisitorBase
    {
        using vsg::Visitor::apply;

        SiegeNodeGuidToXformMap& map;

        GenerateGlobalSiegeNodeGuidToNodeXformMap(SiegeNodeGuidToXformMap& map) :
            map(map) {}

        void apply(vsg::Node& node) { node.traverse(*this); }

        void apply(Region& region) override
        {
            region.traverse(*this);
        }

        void apply(vsg::MatrixTransform& t)
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
    };

    struct CalculateAndPlaceObjects : public vsg::Visitor
    {
        using vsg::Visitor::apply;

        SiegeNodeGuidToXformMap& map;

        CalculateAndPlaceObjects(SiegeNodeGuidToXformMap& map) :
            map(map) {}

        void apply(vsg::Node& node) { node.traverse(*this); }

        void apply(vsg::MatrixTransform& t)
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
    };
} // namespace ehb