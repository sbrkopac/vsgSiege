
#include "SiegeMesh.hpp"

#include "io/BinaryReader.hpp"

namespace ehb
{
    bool SiegeMesh::load(BinaryReader& reader, SiegeMeshHeader const& header, vsg::ref_ptr<const vsg::Options> options)
    {
        // read door data
        for (uint32_t index = 0; index < header.m_numDoors; index++)
        {
            auto id = reader.read<uint32_t>();

            auto pos = reader.read<vsg::vec3>();
            float a00 = reader.read<float>(), a01 = reader.read<float>(), a02 = reader.read<float>(),
                a10 = reader.read<float>(), a11 = reader.read<float>(), a12 = reader.read<float>(),
                a20 = reader.read<float>(), a21 = reader.read<float>(), a22 = reader.read<float>();

            auto count = reader.read<uint32_t>();

            reader.skipBytes(count * 4);

            /*
             * this is pretty straight forward but just documenting that vsg and
             * dungeon siege node transformation matrices ARE compatible so no funky
             * modifications are required here
             */
            vsg::mat3 orient{ a00, a01, a02,
                              a10, a11, a12,
                              a20, a21, a22 };

            doorList.emplace_back(std::make_unique<SiegeMeshDoor>(id, pos, orient));
        }

        // read spot data
        for (uint32_t index = 0; index < header.m_numSpots; index++)
        {
            // rot, pos, string?
            reader.skipBytes(44);
            auto tmp = reader.readString();
        }

        m_pColors = new uint32_t[header.m_numVertices];

        // experimental: create software mesh for easier lookup later
        sVertex* pVertices = new sVertex[header.m_numVertices];
        m_pNormals = new vsg::vec3[header.m_numVertices];

        // create vertex data per entire mesh
        auto vertices = vsg::vec3Array::create(header.m_numVertices);
        auto normals = vsg::vec3Array::create(header.m_numVertices);
        auto colors = vsg::uintArray::create(header.m_numVertices);
        auto tcoords = vsg::vec2Array::create(header.m_numVertices);

        // read in our vertex data
        for (uint32_t index = 0; index < header.m_numVertices; index++)
        {
            auto buildVertex = reader.read<storeVertex>();

            sVertex& nVertex = pVertices[index];
            nVertex.x = buildVertex.x;
            nVertex.y = buildVertex.y;
            nVertex.z = buildVertex.z;
            nVertex.uv = buildVertex.uv;

            vsg::vec3& nNormal = m_pNormals[index];
            nNormal.x = buildVertex.nx;
            nNormal.y = buildVertex.ny;
            nNormal.z = buildVertex.nz;

            m_pColors[index] = buildVertex.color;

            (*vertices)[index].x = nVertex.x; (*vertices)[index].y = nVertex.y; (*vertices)[index].z = nVertex.z;
            (*normals)[index].x = nNormal.x; (*normals)[index].y = nNormal.y; (*normals)[index].z = nNormal.z;
            (*tcoords)[index].x = nVertex.uv.u; (*tcoords)[index].y = nVertex.uv.v;
        }

        TexStageList stageList;
        for (uint32_t index = 0; index < header.m_numStages; index++)
        {
            sTexStage nStage;

            nStage.name = reader.readString(); // std::getline(stream, textureName, '\0');            

            nStage.tIndex = index;
            nStage.startIndex = reader.read<uint32_t>();
            nStage.numVerts = reader.read<uint32_t>();
            nStage.numVIndices = reader.read<uint32_t>();

            nStage.pVIndices = new uint16_t[nStage.numVIndices];
            reader.readBytes(nStage.pVIndices, sizeof(uint16_t) * nStage.numVIndices);

            nStage.numLIndices = 0;
            nStage.pLIndices = nullptr;

            stageList.push_back(nStage);
        }

        m_pRenderObject = std::make_unique<RenderingStaticObject>(options, pVertices, header.m_numVertices, header.m_numTriangles, stageList);
    }
}