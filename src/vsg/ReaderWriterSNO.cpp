
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
        vsg::ref_ptr<SiegeNode> group = SiegeNode::create();

        group->_mesh = new SiegeMesh();
        group->_mesh->load(reader, header, options);

        // Currently for each SiegeNode we create multiple "command graphs" in order to make sure we can pick
        // them apart in the graph for things like fading
        // Not sure if we should be doing this or creating a software representation and just passing 1 draw call to the GPU?
        group->addChild(group->mesh()->renderObject()->createOrShareBuildCommands());

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