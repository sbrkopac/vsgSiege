
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

#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>

namespace ehb
{
    static std::string vert_PushConstants2 = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelview;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = (pc.projection * pc.modelview) * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
)";

    static std::string frag_PushConstants2 = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
})";
}

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

        group->m_pColors = new uint32_t[header.m_numVertices];
        
        // experimental: create software mesh for easier lookup later
        sVertex* pVertices = new sVertex[header.m_numVertices];
        group->m_pNormals = new vsg::vec3[header.m_numVertices];

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

            vsg::vec3& nNormal = group->m_pNormals[index];
            nNormal.x = buildVertex.nx;
            nNormal.y = buildVertex.ny;
            nNormal.z = buildVertex.nz;

            group->m_pColors[index] = buildVertex.color;

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

        group->m_pRenderObject = new RenderingStaticObject(options, pVertices, header.m_numVertices, header.m_numTriangles, stageList);

        // Currently for each SiegeNode we create multiple "command graphs" in order to make sure we can pick
        // them apart in the graph for things like fading
        // Not sure if we should be doing this or creating a software representation and just passing 1 draw call to the GPU?
        group->addChild(group->m_pRenderObject->createOrShareBuildCommands());

        return group;
    };

    vsg::ref_ptr<vsg::BindGraphicsPipeline> ReaderWriterSNO::createOrShareGraphicsPipeline()
    {
        if (!bindGraphicsPipeline)
        {
            vsg::ref_ptr<vsg::ShaderStage> vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", vert_PushConstants2);
            vsg::ref_ptr<vsg::ShaderStage> fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", frag_PushConstants2);

            // set up graphics pipeline
            vsg::DescriptorSetLayoutBindings descriptorBindings{
                {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
            };

            vsg::DescriptorSetLayouts descriptorSetLayouts{ vsg::DescriptorSetLayout::create(descriptorBindings) };

            vsg::PushConstantRanges pushConstantRanges{
                {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
            };

            // TODO: normals
            vsg::VertexInputState::Bindings vertexBindingsDescriptions{
                VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
                VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // colour data
                VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}  // tex coord data
            };

            // TODO: normals
            vsg::VertexInputState::Attributes vertexAttributeDescriptions{
                VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
                VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}, // colour data
                VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},    // tex coord data
            };

            vsg::GraphicsPipelineStates pipelineStates{ vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
                                                       vsg::InputAssemblyState::create(),
                                                       vsg::RasterizationState::create(),
                                                       vsg::MultisampleState::create(),
                                                       vsg::ColorBlendState::create(),
                                                       vsg::DepthStencilState::create() };

            auto pipelineLayout = vsg::PipelineLayout::create(descriptorSetLayouts, pushConstantRanges);
            auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{ vertexShader, fragmentShader }, pipelineStates);
            bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);
        }

        return bindGraphicsPipeline;
    }
} // namespace ehb