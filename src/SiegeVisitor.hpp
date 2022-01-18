
#pragma once

#include <vsg/core/Visitor.h>
#include <vsg/nodes/Group.h>

namespace ehb
{
    class SiegeNodeMesh;
    class Aspect;

    class SiegeVisitorBase : public vsg::Inherit<vsg::Visitor, SiegeVisitorBase>
    {

    public:

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
        virtual void apply(SiegeNodeMesh& node) {}
        virtual void apply(Aspect& aspect) {}
    };
} // namespace ehb