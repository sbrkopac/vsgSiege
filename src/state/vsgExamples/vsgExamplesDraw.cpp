
#include "vsgExamplesDraw.hpp"
#include "Systems.hpp"

#include <spdlog/spdlog.h>

#include <vsg/all.h>

namespace ehb
{
    void vsgExamplesDrawState::enter()
    {
        // set up search paths to SPIRV shaders and textures
        vsg::Paths searchPaths = vsg::getEnvPaths("VSG_FILE_PATH");

        // load shaders
        vsg::ref_ptr<vsg::ShaderStage> vertexShader = vsg::ShaderStage::read(VK_SHADER_STAGE_VERTEX_BIT, "main", vsg::findFile("shaders/vert_PushConstants.spv", searchPaths));
        vsg::ref_ptr<vsg::ShaderStage> fragmentShader = vsg::ShaderStage::read(VK_SHADER_STAGE_FRAGMENT_BIT, "main", vsg::findFile("shaders/frag_PushConstants.spv", searchPaths));
        if (!vertexShader || !fragmentShader)
        {
            std::cout << "Could not create shaders." << std::endl;
            return;
        }

        // read texture image
        vsg::Path textureFile("textures/lz.vsgb");
        auto textureData = vsg::read_cast<vsg::Data>(vsg::findFile(textureFile, searchPaths));
        if (!textureData)
        {
            std::cout << "Could not read texture file : " << textureFile << std::endl;
            return;
        }

        // set up graphics pipeline
        vsg::DescriptorSetLayoutBindings descriptorBindings{
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
        };

        auto descriptorSetLayout = vsg::DescriptorSetLayout::create(descriptorBindings);

        vsg::PushConstantRanges pushConstantRanges{
            {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
        };

        vsg::VertexInputState::Bindings vertexBindingsDescriptions{
            VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // vertex data
            VkVertexInputBindingDescription{1, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX}, // colour data
            VkVertexInputBindingDescription{2, sizeof(vsg::vec2), VK_VERTEX_INPUT_RATE_VERTEX}  // tex coord data
        };

        vsg::VertexInputState::Attributes vertexAttributeDescriptions{
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, // vertex data
            VkVertexInputAttributeDescription{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}, // colour data
            VkVertexInputAttributeDescription{2, 2, VK_FORMAT_R32G32_SFLOAT, 0}     // tex coord data
        };

        vsg::GraphicsPipelineStates pipelineStates{
            vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
            vsg::InputAssemblyState::create(),
            vsg::RasterizationState::create(),
            vsg::MultisampleState::create(),
            vsg::ColorBlendState::create(),
            vsg::DepthStencilState::create() };

        auto pipelineLayout = vsg::PipelineLayout::create(vsg::DescriptorSetLayouts{ descriptorSetLayout }, pushConstantRanges);
        auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{ vertexShader, fragmentShader }, pipelineStates);
        auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

        // create texture image and associated DescriptorSets and binding
        auto texture = vsg::DescriptorImage::create(vsg::Sampler::create(), textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

        auto descriptorSet = vsg::DescriptorSet::create(descriptorSetLayout, vsg::Descriptors{ texture });
        auto bindDescriptorSet = vsg::BindDescriptorSet::create(VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, descriptorSet);

        // create StateGroup as the root of the scene/command graph to hold the GraphicsProgram, and binding of Descriptors to decorate the whole graph
        systems.scene3d->add(bindGraphicsPipeline);
        systems.scene3d->add(bindDescriptorSet);

        // set up model transformation node
        auto transform = vsg::MatrixTransform::create(); // VK_SHADER_STAGE_VERTEX_BIT

        // add transform to root of the scene graph
        systems.scene3d->addChild(transform);

        // set up vertex and index arrays
        auto vertices = vsg::vec3Array::create(
            { {-0.5f, -0.5f, 0.0f},
             {0.5f, -0.5f, 0.0f},
             {0.5f, 0.5f, 0.0f},
             {-0.5f, 0.5f, 0.0f},
             {-0.5f, -0.5f, -0.5f},
             {0.5f, -0.5f, -0.5f},
             {0.5f, 0.5f, -0.5f},
             {-0.5f, 0.5f, -0.5f} }); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_INSTANCE, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

        auto colors = vsg::vec3Array::create(
            {
                {1.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, 0.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
                {1.0f, 0.0f, 0.0f},
                {0.0f, 1.0f, 0.0f},
                {0.0f, 0.0f, 1.0f},
                {1.0f, 1.0f, 1.0f},
            }); // VK_FORMAT_R32G32B32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

        auto texcoords = vsg::vec2Array::create(
            { {0.0f, 0.0f},
             {1.0f, 0.0f},
             {1.0f, 1.0f},
             {0.0f, 1.0f},
             {0.0f, 0.0f},
             {1.0f, 0.0f},
             {1.0f, 1.0f},
             {0.0f, 1.0f} }); // VK_FORMAT_R32G32_SFLOAT, VK_VERTEX_INPUT_RATE_VERTEX, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

        auto indices = vsg::ushortArray::create(
            { 0, 1, 2,
             2, 3, 0,
             4, 5, 6,
             6, 7, 4 }); // VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE

        // setup geometry
        auto drawCommands = vsg::Commands::create();
        drawCommands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{ vertices, colors, texcoords }));
        drawCommands->addChild(vsg::BindIndexBuffer::create(indices));
        drawCommands->addChild(vsg::DrawIndexed::create(12, 1, 0, 0, 0));

        // add drawCommands to transform
        transform->addChild(drawCommands);

        // Below are attempts to work-around dynamic scenegraph crashes
        // Actively being worked on by Robert
        // https://github.com/vsg-dev/vsgExamples/issues/124

#if 1
        vsg::CollectResourceRequirements collectRequirements;
        systems.scene3d->accept(collectRequirements);
        systems.scene3d->setObject("ResourceHints", collectRequirements.createResourceHints());
#endif

        auto resourceHints = vsg::ResourceHints::create();
        resourceHints->maxSlot = 2;
        systems.viewer->compile(resourceHints);
    }

    void vsgExamplesDrawState::leave() {}

    void vsgExamplesDrawState::update(double deltaTime) {}
} // namespace ehb
