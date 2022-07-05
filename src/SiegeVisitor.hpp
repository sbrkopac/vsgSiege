
#pragma once

#include <vsg/core/Visitor.h>
#include <vsg/nodes/Group.h>

namespace ehb
{
    class Region;
    class SiegeNode;
    class Aspect;

    class SiegeVisitorBase : public vsg::Inherit<vsg::Visitor, SiegeVisitorBase>
    {

    public:

        using MatrixStack = std::vector<vsg::dmat4>;

        // cast checks from vsg to siege types
        // we do this here as it is cheaper than casting on every apply
        bool handleCustomGroups(vsg::Group& group);

        // handle native type and start our cast testing
        void apply(vsg::Group& group) override
        {
            handleCustomGroups(group);

            group.traverse(*this);
        }

        // custom types
        virtual void apply(Region& region) {}
        virtual void apply(SiegeNode& node) {}
        virtual void apply(Aspect& aspect) {}
    };
} // namespace ehb