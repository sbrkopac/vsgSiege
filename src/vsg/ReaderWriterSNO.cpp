
#include "ReaderWriterSNO.hpp"

#include "io/BinaryReader.hpp"
#include "io/LocalFileSys.hpp"
#include "world/SiegeNode.hpp"
#include "world/bsp.hpp"

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

    struct SNOHeader
    {
        uint32_t id;
        uint32_t majorVersion;
        uint32_t minorVersion;

        uint32_t doorCount;
        uint32_t spotCount;

        uint32_t cornerCount;
        uint32_t triangleCount;

        uint32_t textureCount;

        vsg::vec3 min;
        vsg::vec3 max;
        vsg::vec3 centroidOffset;

        bool tiled;

        uint32_t unk0, unk1, unk2;
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

        auto header = reader.read<SNOHeader>();

        if (header.id != SNO_MAGIC)
        {
            log->critical("SNO file has unknown id");
            return {};
        }

        if (header.majorVersion > 6 || (header.majorVersion == 6 && header.minorVersion >= 2))
        {
            auto checksum = reader.read<float>(); // TODO: should we do anything with the checksum?
        }

        // Construct our vsg::Group for the graph
        vsg::ref_ptr<SiegeNodeMesh> group = SiegeNodeMesh::create();

        // read door data
        for (uint32_t index = 0; index < header.doorCount; index++)
        {
            auto id = reader.read<uint32_t>();

            auto center = reader.read<vsg::vec3>();
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
            xform(3, 0) = center.x;
            xform(3, 1) = center.y;
            xform(3, 2) = center.z;

            group->doorXform.emplace_back(id, std::move(xform));

            // TODO: use the below
            // the above combines and conflates the data that belongs to the door
            // eventually the above will be removed and we will use the below door list to make our connections
            vsg::mat3 orient;
            orient(0, 0) = a00;
            orient(0, 1) = a01;
            orient(0, 2) = a02;
            orient(1, 0) = a10;
            orient(1, 1) = a11;
            orient(1, 2) = a12;
            orient(2, 0) = a20;
            orient(2, 1) = a21;
            orient(2, 2) = a22;

            group->doorList.emplace_back(std::make_unique<SiegeMeshDoor>(id, center, orient));
        }

        // read spot data
        for (uint32_t index = 0; index < header.spotCount; index++)
        {
            // rot, pos, string?
            reader.skipBytes(44);
            auto tmp = reader.readString();
        }

        // create vertex data per entire mesh
        auto vertices = vsg::vec3Array::create(header.cornerCount);
        auto normals = vsg::vec3Array::create(header.cornerCount);
        auto colors = vsg::vec4Array::create(header.cornerCount);
        auto tcoords = vsg::vec2Array::create(header.cornerCount);

        // read in our vertex data
        for (uint32_t index = 0; index < header.cornerCount; index++)
        {
            reader.readBytes(reinterpret_cast<char*>(&(*vertices)[index]), sizeof(vsg::vec3));
            reader.readBytes(reinterpret_cast<char*>(&(*normals)[index]), sizeof(vsg::vec3));
            reader.readBytes(reinterpret_cast<char*>(&(*colors)[index]), sizeof(uint32_t)); // color is swizzled - not sure if i should address that here or in the vulkan side
            reader.readBytes(reinterpret_cast<char*>(&(*tcoords)[index]), sizeof(vsg::vec2));
        }

        // Currently for each SiegeNode we create multiple "command graphs" in order to make sure we can pick
        // them apart in the graph for things like fading
        // Not sure if we should be doing this or creating a software representation and just passing 1 draw call to the GPU?
        for (uint32_t index = 0; index < header.textureCount; index++)
        {
            auto textureName = reader.readString(); // std::getline(stream, textureName, '\0');
            auto textureFileName = textureName + ".raw";

            auto start = reader.read<uint32_t>(), span = reader.read<uint32_t>(), count = reader.read<uint32_t>();

            // std::cout << "handling texture: " << textureName << std::endl;

            auto attributeArrays = vsg::DataList{vertices, colors, tcoords};

            // batch read in our indices
            auto indices = vsg::ushortArray::create(count);
            reader.readBytes(static_cast<char*>(indices->dataPointer()), sizeof(uint16_t) * count);

            // offset them based on their starting point
            std::transform(std::begin(*indices), std::end(*indices), std::begin(*indices), [&start](uint16_t v) -> uint16_t { return v + start; });

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

            // Create a command graph for each surface in the mesh
            auto commands = vsg::Commands::create();
            commands->addChild(vsg::BindVertexBuffers::create(0, attributeArrays));
            commands->addChild(vsg::BindIndexBuffer::create(indices));
            commands->addChild(vsg::DrawIndexed::create(static_cast<uint32_t>(indices->valueCount()), 1, 0, 0, 0));
            group->addChild(commands);
        }

        auto logicalMeshCount = reader.read<uint32_t>();
        log->info("logicalMeshCount = {}", logicalMeshCount);

        for (uint32_t i = 0; i < logicalMeshCount; ++i)
        {
            auto id = reader.read<uint8_t>();
            auto min = reader.read<vsg::vec3>();
            auto max = reader.read<vsg::vec3>();

            auto flags = reader.read<uint32_t>();

            auto connectionCount = reader.read<uint32_t>();

            for (uint32_t j = 0; j < connectionCount; ++j)
            {
                auto leafid = reader.read<uint16_t>();

                auto leafmin = reader.read<vsg::vec3>();
                auto leafmax = reader.read<vsg::vec3>();

                if (header.majorVersion > 6 || (header.majorVersion == 6 && header.minorVersion >= 4))
                {
                    auto center = reader.read<vsg::vec3>();
                }
                else
                {
                    auto center = leafmin + ((leafmax - leafmin) * 0.5f);
                }

                if (header.majorVersion > 6 || (header.majorVersion == 6 && header.minorVersion >= 2))
                {
                    auto triangleCount = reader.read<uint16_t>();
                    reader.skipBytes(sizeof(uint16_t) * triangleCount);
                }
                else
                {
                    auto triangleCount = 1;
                    reader.skipBytes(sizeof(uint16_t));
                }

                auto localConnectionCount = reader.read<uint32_t>();
                if (localConnectionCount != std::numeric_limits<uint32_t>::max())
                {
                    for (uint32_t k = 0; k < localConnectionCount; ++k)
                    {
                        reader.skipBytes(sizeof(uint16_t));
                    }
                }
                else
                {
                    log->info("NULL");
                }
            }

            auto nodalConnectionCount = reader.read<uint32_t>();

            for (uint32_t nc = 0; nc < nodalConnectionCount; ++nc)
            {
                auto farid = reader.read<uint8_t>();
                auto nodalLeafConnectionCount = reader.read<uint32_t>();

                for (uint32_t nlc = 0; nlc < nodalLeafConnectionCount; ++nlc)
                {
                    auto localid = reader.read<uint16_t>();
                    auto localfarid = reader.read<uint16_t>();
                }
            }

            auto triangleCount = reader.read<uint32_t>();
            TriNorm* data = new TriNorm[triangleCount];
            reader.readBytes(data, sizeof(TriNorm)* triangleCount);

            group->bspTree = std::make_unique<BSPTree>(data, triangleCount, 0, 0, false);

            readBSPNodeFromFile(reader, group->bspTree->GetRoot());
        }

        return group;
    };

    void ReaderWriterSNO::readBSPNodeFromFile(BinaryReader& reader, BSPNode* node) const
    {
        reader.readBytes(&node->m_MinBound, sizeof(vsg::vec3));
        reader.readBytes(&node->m_MaxBound, sizeof(vsg::vec3));
        reader.readBytes(&node->m_IsLeaf, sizeof(bool));
        reader.readBytes(&node->m_NumTriangles, sizeof(uint16_t));

        node->m_Triangles = new uint16_t[node->m_NumTriangles];
        reader.readBytes(node->m_Triangles, sizeof(uint16_t) * node->m_NumTriangles);

        auto children = reader.read<uint8_t>();
        if (children)
        {
            node->m_LeftChild = new BSPNode;
            std::memset(node->m_LeftChild, 0, sizeof(BSPNode));
            node->m_RightChild = new BSPNode;
            std::memset(node->m_RightChild, 0, sizeof(BSPNode));

            readBSPNodeFromFile(reader, node->m_LeftChild);
            readBSPNodeFromFile(reader, node->m_RightChild);
        }
    }
} // namespace ehb