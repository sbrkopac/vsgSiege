
#pragma once

#include <vsg/core/Visitor.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/Light.h>

#include <unordered_map>

#include <spdlog/spdlog.h>

#include "gas/Fuel.hpp"
// #include "vsg/Aspect.hpp"
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

        //! loaded from main.gas
        uint32_t guid = 0;

        //! holds the final matrix transform against the node guid
        SiegeNodeGuidToXformMap placedNodeXformMap;
    };

    class GenerateGlobalSiegeNodeGuidToNodeXformMap : public SiegeVisitorBase
    {
    public:
        SiegeNodeGuidToXformMap& map;

        GenerateGlobalSiegeNodeGuidToNodeXformMap(SiegeNodeGuidToXformMap& map);

        void apply(vsg::Node& node) override;

        void apply(Region& region) override;

        void apply(vsg::MatrixTransform& t) override;
    };

    // this requires us to store a SiegePos against each object but allows us to recalculate things on the fly
    class CalculateAndPlaceLights : public SiegeVisitorBase
    {
    public:

        const Region& region;

        CalculateAndPlaceLights(const Region& region) : region(region) {};

        void apply(vsg::PointLight& light) override
        {
            spdlog::get("log")->info("visiting vsg::PointLight& light");

            uint32_t node = 0; light.getValue<uint32_t>("node", node);
            vsg::vec3 position; light.getValue<vsg::vec3>("position", position);

            if (node == 0)
            {
                spdlog::get("log")->error("We have a valid light but it's not assigned to a SiegeNode");
                return;
            }

            // look up our destination siege node
            const auto destinationSiegeNodeXform = region.placedNodeXformMap.at(node);

            // final placement
            vsg::dmat4 mat = destinationSiegeNodeXform->matrix * vsg::dmat4(vsg::translate(position));

            light.position = vsg::dvec3(mat[3][0], mat[3][1], mat[3][2]);
        }
    };

    class CalculateAndPlaceObjects : public vsg::Visitor
    {
    public:
        SiegeNodeGuidToXformMap& map;

        CalculateAndPlaceObjects(SiegeNodeGuidToXformMap& map);

        void apply(vsg::Node& node) override;

        void apply(vsg::MatrixTransform& t) override;
    };
} // namespace ehb