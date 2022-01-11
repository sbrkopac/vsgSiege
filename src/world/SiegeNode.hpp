
#pragma once

#include <vector>

#include "SiegeVisitor.hpp"
#include <vsg/core/Inherit.h>
#include <vsg/core/Visitor.h>
#include <vsg/maths/mat4.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>

namespace ehb
{
    class SiegeNodeMesh final : public vsg::Inherit<vsg::Group, SiegeNodeMesh>
    {
        friend class ReaderWriterSNO;

    public:
        explicit SiegeNodeMesh() = default;

        static void connect(const vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        static void connect(const vsg::MatrixTransform* targetRegion, vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectRegion, const vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        void accept(vsg::Visitor& visitor) override;

    protected:
        virtual ~SiegeNodeMesh() = default;

    private:
        std::vector<std::pair<uint32_t, vsg::dmat4>> doorXform;
    };

    inline void SiegeNodeMesh::accept(vsg::Visitor& visitor)
    {
        if (SiegeVisitorBase* svb = dynamic_cast<SiegeVisitorBase*>(&visitor); svb != nullptr)
        {
            svb->apply(*this);
        }
        else
        {
            visitor.apply(*this);
        }
    }
} // namespace ehb