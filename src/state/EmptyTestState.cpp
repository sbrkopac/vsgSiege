
#include "EmptyTestState.hpp"
#include "Systems.hpp"

#include <spdlog/spdlog.h>

#include <vsg/all.h>

namespace ehb
{
    std::string vert = R"(
#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec3 vColor;

layout (location = 0) out vec3 outColor;

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelview;
} pc;

void main()
{
	gl_Position = (pc.projection * pc.modelview) *  vec4(vPosition, 1.0f);
	outColor = vColor;
})";

    std::string frag = R"(
//glsl version 4.5
#version 450

//output write
layout (location = 0) out vec4 outFragColor;

void main()
{
	//return red
	outFragColor = vec4(1.f,0.f,0.f,1.0f);
})";
} // namespace ehb

namespace ehb
{
    struct Vertex
    {
        vsg::vec3 position, normal, color;
    };

    void EmptyTestState::enter()
    {

        log->info("Entered EmptyTestState");

        auto vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", vert);
        auto fragShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", frag);

        if (!vertexShader || !fragShader)
        {
            log->error("Unable to create shaders");

            return;
        }

        // TODO: normals
        vsg::VertexInputState::Bindings vertexBindingsDescriptions{
            VkVertexInputBindingDescription{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}, // data described in 1 struct
            VkVertexInputBindingDescription{1, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}, // data described in 1 struct
            VkVertexInputBindingDescription{2, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};

        vsg::VertexInputState::Attributes vertexAttributeDescriptions{
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)}, // vertex data
            VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)},   // normal
            VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)},    // colour
        };

        vsg::PushConstantRanges pushConstantRanges{
            {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
        };

        // vertex positions
        vsg::ref_ptr<vsg::vec3Array> vertices = vsg::vec3Array::create(
            {{1.f, 1.f, 0.0f},
             {-1.f, 1.f, 0.0f},
             {0.f, -1.f, 0.0f}});

        //vertex colors, all green
        vsg::ref_ptr<vsg::vec3Array> colors = vsg::vec3Array::create(
            {
                {0.f, 1.f, 0.0f}, //pure green
                {0.f, 1.f, 0.0f}, //pure green
                {0.f, 1.f, 0.0f}  //pure green
            });

        auto vertexInputState = vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions);

        // input assembly is VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST by default
        // primitiveRestartEnable = vk_false is default
        auto inputAssemblyState = vsg::InputAssemblyState::create();

        auto rasterizationState = vsg::RasterizationState::create();
        rasterizationState->cullMode = VK_CULL_MODE_NONE;
        rasterizationState->frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationState->depthBiasEnable = VK_FALSE;
        rasterizationState->depthBiasConstantFactor = 0.0f;

        auto multisamplingState = vsg::MultisampleState::create();
        multisamplingState->minSampleShading = 1.0f;

        auto colorBlendState = vsg::ColorBlendState::create();

        vsg::GraphicsPipelineStates pipelineStates = {

            vertexInputState,
            inputAssemblyState,
            rasterizationState,
            multisamplingState,
            colorBlendState,
            vsg::DepthStencilState::create()};

        auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{}, pushConstantRanges);

        auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragShader}, pipelineStates);
        auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

        auto bindVertexBuffer = vsg::BindVertexBuffers::create();
        bindVertexBuffer->assignArrays({vertices, vertices, colors});

        auto group = vsg::StateGroup::create();
        group->addChild(bindGraphicsPipeline);
        group->addChild(bindVertexBuffer);
        systems.scene3d->addChild(group);
    }

    void EmptyTestState::leave() {}

    void EmptyTestState::update(double deltaTime) {}
} // namespace ehb
