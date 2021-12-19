
#pragma once

#include <vsg/core/Visitor.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>

#include <unordered_map>

#include <spdlog/spdlog.h>

#include "gas/Fuel.hpp"
// #include "vsg/Aspect.hpp"
#include "world/SiegeNode.hpp"
#include <vsg/core/Visitor.h>

#include <spdlog/spdlog.h>

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

    class CalculateAndPlaceObjects : public vsg::Visitor
    {
    public:
        SiegeNodeGuidToXformMap& map;

        CalculateAndPlaceObjects(SiegeNodeGuidToXformMap& map);

        void apply(vsg::Node& node) override;

        void apply(vsg::MatrixTransform& t) override;
    };
} // namespace ehb