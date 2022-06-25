
#pragma once

#include <vector>
#include <list>

#include "SiegeMeshDoor.hpp"
#include "SiegeVisitor.hpp"
#include "bsp.hpp"
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

        using SiegeMeshDoorList = std::list<std::unique_ptr<SiegeMeshDoor>>;

        explicit SiegeNodeMesh() = default;

        static void connect(const vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        static void connect(const vsg::MatrixTransform* targetRegion, vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectRegion, const vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        BSPTree* tree();

    protected:
        virtual ~SiegeNodeMesh() = default;

    private:

        SiegeMeshDoorList doorList;

        std::unique_ptr<BSPTree> bspTree;

        std::vector<std::pair<uint32_t, vsg::dmat4>> doorXform;
    };

    inline BSPTree* SiegeNodeMesh::tree()
    {
        return bspTree.get();
    }

} // namespace ehb