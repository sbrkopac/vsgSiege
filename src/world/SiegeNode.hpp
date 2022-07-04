
#pragma once

#include <vector>

#include "SiegeVisitor.hpp"
#include "world/RenderingStaticObject.hpp"
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
        explicit SiegeNodeMesh();

        static void connect(const vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        static void connect(const vsg::MatrixTransform* targetRegion, vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectRegion, const vsg::MatrixTransform* connectNode, uint32_t connectDoor);

    protected:
        virtual ~SiegeNodeMesh();

    private:

        vsg::vec3* m_pNormals;
        uint32_t* m_pColors;

        RenderingStaticObject* m_pRenderObject;

        std::vector<std::pair<uint32_t, vsg::dmat4>> doorXform;
    };

    inline SiegeNodeMesh::SiegeNodeMesh() : 
        m_pRenderObject(nullptr), m_pNormals (nullptr), m_pColors(nullptr)
    {

    }

    inline SiegeNodeMesh::~SiegeNodeMesh()
    {
        delete[] m_pNormals;
        delete m_pRenderObject;
        delete[] m_pColors;
    }

} // namespace ehb