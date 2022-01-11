
#pragma once

#include <vsg/core/Visitor.h>

namespace ehb
{
    class SiegeNodeMesh;
    class Aspect;

    class SiegeVisitorBase : public vsg::Visitor
    {

    public:

        virtual void apply(SiegeNodeMesh& node) {}
        virtual void apply(Aspect& aspect) {}
    };
}