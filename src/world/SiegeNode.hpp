
#pragma once

#include <vector>
#include <memory>

#include "SiegeVisitor.hpp"
#include "world/RenderingStaticObject.hpp"
#include "SiegeMeshDoor.hpp"
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

        explicit SiegeNodeMesh();

        static void connect(const vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        static void connect(const vsg::MatrixTransform* targetRegion, vsg::MatrixTransform* targetNode, uint32_t targetDoor, vsg::MatrixTransform* connectRegion, const vsg::MatrixTransform* connectNode, uint32_t connectDoor);

        RenderingStaticObject* renderObject() { return m_pRenderObject.get(); }
        vsg::vec3* normals() { return m_pNormals; }
        uint32_t* colors() { return m_pColors; }

        const SiegeMeshDoorList& doors() const { return doorList; };
        SiegeMeshDoor* doorByIndex(uint32_t index) const;
        SiegeMeshDoor* doorById(uint32_t id) const;

    protected:
        virtual ~SiegeNodeMesh();

    private:

        vsg::vec3* m_pNormals;
        uint32_t* m_pColors;

        std::unique_ptr<RenderingStaticObject> m_pRenderObject;

        SiegeMeshDoorList doorList;
    };

    inline SiegeNodeMesh::SiegeNodeMesh() : 
        m_pRenderObject(nullptr), m_pNormals (nullptr), m_pColors(nullptr)
    {
    }

    inline SiegeNodeMesh::~SiegeNodeMesh()
    {
        delete[] m_pNormals;
        delete[] m_pColors;
    }

} // namespace ehb