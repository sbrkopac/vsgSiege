
#pragma once

#include <memory>
#include <vector>

#include "SiegeMesh.hpp"
#include "SiegeVisitor.hpp"
#include <vsg/core/Inherit.h>
#include <vsg/maths/mat4.h>
#include <vsg/nodes/Group.h>
#include <vsg/nodes/MatrixTransform.h>

namespace ehb
{
    class SiegeMesh;
}

namespace ehb
{
    class SiegeNode final : public vsg::Inherit<vsg::Group, SiegeNode>
    {
        friend class ReaderWriterSNO;

    public:
        using SiegeMeshDoorList = std::list<std::unique_ptr<SiegeMeshDoor>>;

        explicit SiegeNode() = default;

        static void connect(const vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        static void connect(const vsg::MatrixTransform* targetRegion, vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectRegion, const vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        SiegeMesh* mesh() { return _mesh; }

    protected:
        virtual ~SiegeNode() = default;

    private:
        vsg::ref_ptr<SiegeMesh> _mesh;
    };

} // namespace ehb