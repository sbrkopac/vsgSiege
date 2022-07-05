
#pragma once

#include <cassert>
#include <memory>
#include <vector>

#include <vsg/maths/box.h>
#include <vsg/maths/mat3.h>
#include <vsg/maths/vec3.h>

#include "RenderingStaticObject.hpp"
#include "SiegeMeshDoor.hpp"

namespace ehb
{
    class BinaryReader;

    struct SiegeMeshHeader
    {
        // File identification
        uint32_t m_id;
        uint32_t m_majorVersion;
        uint32_t m_minorVersion;

        // Door and spot information
        uint32_t m_numDoors;
        uint32_t m_numSpots;

        // Mesh information
        uint32_t m_numVertices;
        uint32_t m_numTriangles;

        // Stage information
        uint32_t m_numStages;

        // Spatial information
        vsg::vec3 m_minBBox;
        vsg::vec3 m_maxBBox;
        vsg::vec3 m_centroidOffset;

        // Whether or not this mesh requires wrapping
        bool m_bTile;

        // Reserved information for possible future use
        uint32_t m_reserved0;
        uint32_t m_reserved1;
        uint32_t m_reserved2;
    };
} // namespace ehb

namespace ehb
{
    //! TODO: I'm making this a vsg::Object so we can call share on it but maybe a more complete cache solution is appropriate
    class SiegeMesh : public vsg::Object
    {
        friend class ReaderWriterSNO;

    public:
        using SiegeMeshDoorList = std::list<std::unique_ptr<SiegeMeshDoor>>;
        using TexStrings = std::vector<std::string>;

        enum
        {
            FLAG_DRAW_FLOORS = 1 << 1,
            FLAG_DRAW_NORMALS = 1 << 2,
            FLAG_DRAW_SPOTS = 1 << 3,
            FLAG_DRAW_ORIGIN = 1 << 4,
            FLAG_DRAW_WATER = 1 << 5,
        };

        struct storeVertex
        {
            float x, y, z;
            float nx, ny, nz;
            uint32_t color;
            UV uv;
        };

    public:
        SiegeMesh();

        SiegeMesh(SiegeMesh const&) = delete;
        SiegeMesh& operator=(SiegeMesh const&) = delete;

        bool load(BinaryReader& reader, SiegeMeshHeader const& header, vsg::ref_ptr<const vsg::Options> options);

        RenderingStaticObject* renderObject() { return m_pRenderObject.get(); }
        vsg::vec3* normals() { return m_pNormals; }
        uint32_t* colors() { return m_pColors; }

        const SiegeMeshDoorList& doors() const { return doorList; };
        SiegeMeshDoor* doorByIndex(uint32_t index) const;
        SiegeMeshDoor* doorById(uint32_t id) const;

    protected:
        virtual ~SiegeMesh();

    private:
        SiegeMeshDoorList doorList;

        uint32_t numDoors;
        uint32_t numSpots;

        vsg::vec3* m_pNormals;

        TexStrings textures;

        std::unique_ptr<RenderingStaticObject> m_pRenderObject;

        uint32_t numStages;

        vsg::box boundingBox;
        vsg::vec3 centroidOffset;

        bool tiled;

        uint32_t* m_pColors;
    };

    inline SiegeMesh::SiegeMesh() :
        numDoors(0), numSpots(0), m_pNormals(nullptr), numStages(0), tiled(false), m_pColors(nullptr)
    {
    }

    inline SiegeMesh::~SiegeMesh()
    {
        delete[] m_pNormals;
        delete[] m_pColors;
    }

    inline SiegeMeshDoor* SiegeMesh::doorByIndex(uint32_t index) const
    {
        uint32_t currIndex = 0;
        for (auto d = doorList.begin(); d != doorList.end(); ++d, ++currIndex)
        {
            assert((*d));

            if (index == currIndex)
            {
                return d->get();
            }
        }

        return nullptr;
    }

    inline SiegeMeshDoor* SiegeMesh::doorById(uint32_t id) const
    {
        for (auto d = doorList.begin(); d != doorList.end(); ++d)
        {
            assert((*d));

            if (id == (*d)->id())
            {
                return d->get();
            }
        }

        return nullptr;
    }
} // namespace ehb