
#include "ReaderWriterSNO.hpp"

#include "io/BinaryReader.hpp"
#include "io/LocalFileSys.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/io/read.h>
#include <vsg/maths/quat.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>

namespace ehb
{
    static constexpr uint32_t SNO_MAGIC = 0x444F4E53;

    ReaderWriterSNO::ReaderWriterSNO(IFileSys& fileSys) :
        fileSys(fileSys) { log = spdlog::get("log"); }

    vsg::ref_ptr<vsg::Object> ReaderWriterSNO::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
    {
        if (auto fullFilePath = vsg::findFile(filename, options); !fullFilePath.empty())
        {
            if (auto file = fileSys.createInputStream(fullFilePath + ".sno"); file != nullptr) { return read(*file, options); }
        }

        return {};
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterSNO::read(std::istream& stream, vsg::ref_ptr<const vsg::Options> options) const
    {
        BinaryReader reader(stream);

        // prefer auto since the template provides the type and it's easier to change if we have to

        auto magic = reader.read<uint32_t>();
        auto version = reader.read<uint32_t>();
        auto unk1 = reader.read<uint32_t>();

        if (magic != SNO_MAGIC) return {};

        auto doorCount = reader.read<uint32_t>();
        auto spotCount = reader.read<uint32_t>();
        auto cornerCount = reader.read<uint32_t>();
        auto faceCount = reader.read<uint32_t>();
        auto textureCount = reader.read<uint32_t>();

        // SiegeMax calculates a bounding box for us but we will let VSG do some box calculations
        // This is good for comparatives if we get a "weird" result from VSG
        auto min = reader.read<vsg::vec3>();
        auto max = reader.read<vsg::vec3>();

        // No idea what is under here
        auto unk2 = reader.read<float>(), unk3 = reader.read<float>(), unk4 = reader.read<float>();
        auto unk5 = reader.read<uint32_t>(), unk6 = reader.read<uint32_t>(), unk7 = reader.read<uint32_t>(), unk8 = reader.read<uint32_t>();
        auto checksum = reader.read<float>();

        // Construct our vsg::Group for the graph
        vsg::ref_ptr<SiegeNodeMesh> group = SiegeNodeMesh::create();

        // read door data
        for (uint32_t index = 0; index < doorCount; index++)
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
        for (uint32_t index = 0; index < spotCount; index++)
        {
            // rot, pos, string?
            reader.skipBytes(44);
            auto tmp = reader.readString();
        }

        // create vertex data per entire mesh
        auto vertices = vsg::vec3Array::create(cornerCount);
        auto normals = vsg::vec3Array::create(cornerCount);
        auto colors = vsg::vec4Array::create(cornerCount);
        auto tcoords = vsg::vec2Array::create(cornerCount);

        // read in our vertex data
        for (uint32_t index = 0; index < cornerCount; index++)
        {
            reader.readBytes(reinterpret_cast<char*>(&(*vertices)[index]), sizeof(vsg::vec3));
            reader.readBytes(reinterpret_cast<char*>(&(*normals)[index]), sizeof(vsg::vec3));
            reader.readBytes(reinterpret_cast<char*>(&(*colors)[index]), sizeof(uint32_t)); // color is swizzled - not sure if i should address that here or in the vulkan side
            reader.readBytes(reinterpret_cast<char*>(&(*tcoords)[index]), sizeof(vsg::vec2));
        }

        // Currently for each SiegeNode we create multiple "command graphs" in order to make sure we can pick
        // them apart in the graph for things like fading
        // Not sure if we should be doing this or creating a software representation and just passing 1 draw call to the GPU?
        for (uint32_t index = 0; index < textureCount; index++)
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

        return group;
    };
} // namespace ehb