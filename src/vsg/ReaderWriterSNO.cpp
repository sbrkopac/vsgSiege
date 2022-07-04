
#include "ReaderWriterSNO.hpp"

#include "io/BinaryReader.hpp"
#include "io/LocalFileSys.hpp"
#include "world/SiegeNode.hpp"
#include "world/RenderingStaticObject.hpp"

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/io/read.h>
#include <vsg/maths/quat.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/BindDescriptorSet.h>

namespace ehb
{
    static constexpr uint32_t SNO_MAGIC = 0x444F4E53;

    struct SiegeMeshHeader
    {
        // File identification
        uint32_t	m_id;
        uint32_t	m_majorVersion;
        uint32_t	m_minorVersion;

        // Door and spot information
        uint32_t	m_numDoors;
        uint32_t	m_numSpots;

        // Mesh information
        uint32_t	m_numVertices;
        uint32_t	m_numTriangles;

        // Stage information
        uint32_t	m_numStages;

        // Spatial information
        vsg::vec3		m_minBBox;
        vsg::vec3		m_maxBBox;
        vsg::vec3		m_centroidOffset;

        // Whether or not this mesh requires wrapping
        bool			m_bTile;

        // Reserved information for possible future use
        uint32_t	m_reserved0;
        uint32_t	m_reserved1;
        uint32_t	m_reserved2;
    };

    struct storeVertex
    {
        float x, y, z;
        float nx, ny, nz;
        uint32_t color;
        UV	  uv;
    };

    ReaderWriterSNO::ReaderWriterSNO(IFileSys& fileSys) :
        fileSys(fileSys) { log = spdlog::get("log"); }

    vsg::ref_ptr<vsg::Object> ReaderWriterSNO::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
    {
        if (auto fullFilePath = vsg::findFile(filename, options); !fullFilePath.empty())
        {
            if (auto file = fileSys.createInputStream(fullFilePath.string() + ".sno"); file != nullptr) { return read(*file, options); }
        }

        return {};
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterSNO::read(std::istream& stream, vsg::ref_ptr<const vsg::Options> options) const
    {
        BinaryReader reader(stream);

        // prefer auto since the template provides the type and it's easier to change if we have to

        auto header = reader.read<SiegeMeshHeader>();

        if (header.m_majorVersion > 6 ||
            (header.m_majorVersion == 6 && header.m_minorVersion >= 2))
        {
            uint32_t checksum = reader.read<uint32_t>();
        }

        if (header.m_id != SNO_MAGIC) return {};

        // Construct our vsg::Group for the graph
        vsg::ref_ptr<SiegeNodeMesh> group = SiegeNodeMesh::create();

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
            vsg::dmat4 xform;

            xform(0, 0) = a00;
            xform(0, 1) = a01;
            xform(0, 2) = a02;
            xform(1, 0) = a10;
            xform(1, 1) = a11;
            xform(1, 2) = a12;
            xform(2, 0) = a20;
            xform(2, 1) = a21;
            xform(2, 2) = a22;
            xform(3, 0) = pos.x;
            xform(3, 1) = pos.y;
            xform(3, 2) = pos.z;

            group->doorXform.emplace_back(id, std::move(xform));
        }

        // read spot data
        for (uint32_t index = 0; index < header.m_numSpots; index++)
        {
            // rot, pos, string?
            reader.skipBytes(44);
            auto tmp = reader.readString();
        }
        
        // experimental: create software mesh for easier lookup later
        sVertex* pVertices = new sVertex[header.m_numVertices];
        group->m_pNormals = new vsg::vec3[header.m_numVertices];

        // create vertex data per entire mesh
        auto vertices = vsg::vec3Array::create(header.m_numVertices);
        auto normals = vsg::vec3Array::create(header.m_numVertices);
        auto colors = vsg::vec4Array::create(header.m_numVertices);
        auto tcoords = vsg::vec2Array::create(header.m_numVertices);

        // read in our vertex data
        for (uint32_t index = 0; index < header.m_numVertices; index++)
        {
#if 1
            auto buildVertex = reader.read<storeVertex>();
            
            sVertex& nVertex = pVertices[index];
            nVertex.x = buildVertex.x;
            nVertex.y = buildVertex.y;
            nVertex.z = buildVertex.z;
            nVertex.uv = buildVertex.uv;

            vsg::vec3& nNormal = group->m_pNormals[index];
            nNormal.x = buildVertex.nx;
            nNormal.y = buildVertex.ny;
            nNormal.z = buildVertex.nz;

            // group->m_pColors[index] = buildVertex.color;

            (*vertices)[index].x = nVertex.x; (*vertices)[index].y = nVertex.y; (*vertices)[index].z = nVertex.z;
            (*normals)[index].x = nNormal.x; (*normals)[index].y = nNormal.y; (*normals)[index].z = nNormal.z;
            (*tcoords)[index].x = nVertex.uv.u; (*tcoords)[index].y = nVertex.uv.v;

#else
            reader.readBytes(reinterpret_cast<char*>(&(*vertices)[index]), sizeof(vsg::vec3));
            reader.readBytes(reinterpret_cast<char*>(&(*normals)[index]), sizeof(vsg::vec3));
            reader.readBytes(reinterpret_cast<char*>(&(*colors)[index]), sizeof(uint32_t)); // color is swizzled - not sure if i should address that here or in the vulkan side
            reader.readBytes(reinterpret_cast<char*>(&(*tcoords)[index]), sizeof(vsg::vec2));
#endif
        }

        // Currently for each SiegeNode we create multiple "command graphs" in order to make sure we can pick
        // them apart in the graph for things like fading
        // Not sure if we should be doing this or creating a software representation and just passing 1 draw call to the GPU?
        TexStageList stageList;
        for (uint32_t index = 0; index < header.m_numStages; index++)
        {
            auto textureName = reader.readString(); // std::getline(stream, textureName, '\0');
            auto textureFileName = textureName + ".raw";

            sTexStage nStage;

            nStage.tIndex = reader.read<uint32_t>(); nStage.numVerts = reader.read<uint32_t>(); nStage.numVIndices = reader.read<uint32_t>();

            // read in our indices to the software representation
            nStage.pVIndices = new uint16_t[nStage.numVIndices];
            reader.readBytes(nStage.pVIndices, sizeof(uint16_t) * nStage.numVIndices);

            nStage.numLIndices = 0;
            nStage.pLIndices = nullptr;

            stageList.push_back(nStage);

            // assign those indices to our GPU
            auto indices = vsg::ushortArray::create(nStage.numVIndices);
            indices->assign(nStage.numVIndices, nStage.pVIndices);

            // offset them based on their starting point            
            std::transform(std::begin(*indices), std::end(*indices), std::begin(*indices), [&nStage](uint16_t v) -> uint16_t { return v + nStage.tIndex; });

            // TODO: handle layers and animated SiegeNode textures
            if (auto layout = static_cast<const vsg::PipelineLayout*>(options->getObject("SiegeNodeLayout")); layout != nullptr)
            {
                if (auto textureData = vsg::read_cast<vsg::Data>(textureName, options); textureData != nullptr)
                {
                    auto texture = vsg::DescriptorImage::create(vsg::Sampler::create(), textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

                    //! NOTE: should we be accessing the first element of the vector?
                    //! TODO: check with Robert about having to const_cast in loaders to get the pipeline layout
                    auto descriptorSet = vsg::DescriptorSet::create(layout->setLayouts[0], vsg::Descriptors{texture});
                    auto bindDescriptorSets = vsg::BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, const_cast<vsg::PipelineLayout*>(layout), 0, vsg::DescriptorSets{descriptorSet});

                    group->addChild(bindDescriptorSets);
                }
            }
            else
            {
                // This is a critical error and causes huge problems and we shouldn't ever get here
                // TODO: throw?
                log->error("No layout passed via options. Mesh will probably render incorrectly.");

                return {};
            }

            auto attributeArrays = vsg::DataList{ vertices, colors, tcoords };

            // Create a command graph for each surface in the mesh
            auto commands = vsg::Commands::create();
            commands->addChild(vsg::BindVertexBuffers::create(0, attributeArrays));
            commands->addChild(vsg::BindIndexBuffer::create(indices));
            commands->addChild(vsg::DrawIndexed::create(static_cast<uint32_t>(indices->valueCount()), 1, 0, 0, 0));
            group->addChild(commands);
        }

        group->m_pRenderObject = new RenderingStaticObject(pVertices, header.m_numVertices, header.m_numTriangles, stageList);

        return group;
    };
} // namespace ehb