
#pragma once

#include <cstdint>
#include <cstring>
#include <vector>
#include <list>
#include <set>

#include <vsg/commands/Commands.h>
#include <vsg/io/Options.h>

namespace ehb
{
    struct UV
    {
        float u, v;
    };

    // untransformed vertex with color and texture
    struct sVertex
    {
        float x, y, z;
        uint32_t color;
        UV uv;

        bool operator == (const sVertex& s)
        {
            return (!std::memcpy((void*)&s, this, sizeof(sVertex)));
        }
    };

    struct sTexStage
    {
        std::string name;               // sam: added for vulkan so we can load the actual texture when we need it
        uint32_t tIndex;				// Which texture is active in this stage
        uint32_t startIndex;			// Where to start the indices in the VBuffer
        uint32_t numVerts;				// Number of vertices
        uint16_t* pVIndices;				// Pointer to the indices for this stage
        uint32_t numVIndices;			// Number of indices in this stage
        uint32_t* pLIndices;				// Pointer to the lighting indices
        uint32_t numLIndices;			// Number of lighting indices
    };

    // static object texture info
    struct StaticObjectTex
    {
        uint32_t textureId; // sam: not currently used as this was used in fixed pipeline to keep track of the texture index
        bool alpha;
        bool noalphasort;
    };

    struct sBuildTexStage
    {
        uint32_t tIndex;				// Which texture is active in this stage
        std::vector< sVertex > verts;		// List of vertices for this stage
        std::vector< uint16_t > vIndex;			// Vertex indices for this stage
        std::vector< uint32_t > lIndex;		// Lighting indices back into the main vertex array
        uint32_t numVerts;				// How many verts in list (size() is CPU intensive)
    };
}

namespace ehb
{
    using TexStageList = std::vector<sTexStage>;
    using TexList = std::vector<StaticObjectTex>;

    class RenderingStaticObject
    {

    public:

        RenderingStaticObject(sVertex* vertices, int32_t numVertices, uint16_t* indices, int32_t numIndices, uint32_t* textureTriCount, TexList& textureList);
        RenderingStaticObject(vsg::ref_ptr<const vsg::Options> options, sVertex* pVertices, int32_t numVertices, int32_t numTriangles, TexStageList& stageList);

        ~RenderingStaticObject();

        const int32_t numVertices() { return m_numVertices; }
        const int32_t numTriangles() { return m_numTriangles; }

        const TexList& textureList() { return m_texlist; }

        TexStageList& stageList() { return m_TexStageList; }

        sVertex* vertices() { return m_pVertices; }

        // VSG specific
        vsg::ref_ptr<vsg::Group> buildDrawCommands();

    private:

        void organizeInformation(sVertex* verts, uint16_t* indices, uint32_t* textureTriCount);

    private:

        vsg::ref_ptr<const vsg::Options> options;

        int32_t m_numVertices;
        int32_t m_numTriangles;

        TexStageList m_TexStageList;
        TexList m_texlist;
        sVertex* m_pVertices;
    };

    inline RenderingStaticObject::RenderingStaticObject(sVertex* vertices, int32_t numVertices, uint16_t* indices, int32_t numIndices, uint32_t* textureTriCount, TexList& textureList) :
        m_numVertices(numVertices), m_numTriangles(numIndices / 3), m_texlist(textureList), m_pVertices(nullptr)
    {
        organizeInformation(vertices, indices, textureTriCount);
    }

    inline RenderingStaticObject::RenderingStaticObject(vsg::ref_ptr<const vsg::Options> options, sVertex* pVertices, int32_t numVertices, int32_t numTriangles, TexStageList& stageList) :
        options(options), m_numVertices(numVertices), m_numTriangles(numTriangles), m_pVertices(pVertices), m_TexStageList(stageList)
    {
        // Build up a default texture list
        StaticObjectTex tex;
        tex.textureId = 0;
        tex.alpha = false;
        m_texlist.resize(m_TexStageList.size(), tex);
    }

    inline RenderingStaticObject::~RenderingStaticObject()
    {
        for (auto itr = m_TexStageList.begin(); itr != m_TexStageList.end(); ++itr)
        {
            if ((*itr).pLIndices)
            {
                delete[](*itr).pLIndices;
            }

            delete (*itr).pVIndices;
        }

        m_TexStageList.clear();

        delete[] m_pVertices;
    }

    inline void RenderingStaticObject::organizeInformation(sVertex* verts, uint16_t* indices, uint32_t* textureTriCount)
    {
        std::list< sBuildTexStage > texStageList;

        // Build the lists
        uint32_t texCount = 0;
        uint32_t indexOffset = 0;
        uint32_t totalVertCount = 0;
        for (TexList::iterator i = m_texlist.begin(); i != m_texlist.end(); ++i, ++texCount)
        {
            // Build the stage
            sBuildTexStage	newStage;

            // Fill in data we can get quickly
            newStage.tIndex = texCount;

            std::set< uint16_t > UniqueVerts;
            uint32_t n;
            for (n = 0; n < (textureTriCount[texCount] * 3); ++n)
            {
                UniqueVerts.insert(indices[indexOffset + n]);
                newStage.vIndex.push_back(indices[indexOffset + n]);
            }
            indexOffset += n;

            uint16_t nIndex = 0;
            for (std::set< uint16_t >::iterator v = UniqueVerts.begin(); v != UniqueVerts.end(); ++v, ++nIndex)
            {
                newStage.verts.push_back(verts[(*v)]);
                newStage.lIndex.push_back((*v));

                for (std::vector< uint16_t >::iterator x = newStage.vIndex.begin(); x != newStage.vIndex.end(); ++x)
                {
                    if ((*x) == (*v))
                    {
                        (*x) = nIndex;
                    }
                }
            }

            newStage.numVerts = newStage.verts.size();
            totalVertCount += newStage.numVerts;

            texStageList.push_back(newStage);
        }

        // Build vertices
        m_pVertices = new sVertex[totalVertCount];
        memset(m_pVertices, 0, sizeof(sVertex) * totalVertCount);

        // Get a pointer to our vertex listing that we can fill up
        sVertex* plVertices = m_pVertices;

        uint32_t currentIndex = 0;
        std::list< sBuildTexStage >::iterator o;
        for (o = texStageList.begin(); o != texStageList.end(); ++o)
        {
            // Create a new sTexStage for this stage
            sTexStage newTexStage;

            // Set known info
            newTexStage.tIndex = (*o).tIndex;
            newTexStage.startIndex = currentIndex;
            newTexStage.numVerts = (*o).numVerts;

            uint32_t index = 0;

            // Fill in the indices for the lighting info
            newTexStage.numLIndices = (*o).lIndex.size();
            newTexStage.pLIndices = new uint32_t[newTexStage.numLIndices];
            for (std::vector< uint32_t >::iterator d = (*o).lIndex.begin(); d != (*o).lIndex.end(); ++d, ++index)
            {
                newTexStage.pLIndices[index] = (*d);
            }

            // Fill in the vertex indices
            newTexStage.numVIndices = (*o).vIndex.size();
            newTexStage.pVIndices = new uint16_t[newTexStage.numVIndices];
            index = 0;
            for (std::vector< uint16_t >::iterator w = (*o).vIndex.begin(); w != (*o).vIndex.end(); ++w, ++index)
            {
                newTexStage.pVIndices[index] = (*w);
            }

            // Fill in the vertices
            for (std::vector< sVertex >::iterator v = (*o).verts.begin(); v != (*o).verts.end(); ++v, ++plVertices, ++currentIndex)
            {
                (*plVertices) = (*v);
            }

            // Put the new stage on our main list
            m_TexStageList.push_back(newTexStage);
        }
    }
}