
#include "InitState.hpp"
#include "Systems.hpp"
#include "state/GameStateMgr.hpp"

#include "vsg/ReaderWriterRAW.hpp"
#include "vsg/ReaderWriterRegion.hpp"
#include "vsg/ReaderWriterSNO.hpp"
#include "vsg/ReaderWriterASP.hpp"
#include "vsg/ReaderWriterSiegeNodeList.hpp"

#include <vsg/io/ObjectCache.h>
#include <vsg/state/ColorBlendState.h>
#include <vsg/state/DepthStencilState.h>
#include <vsg/state/InputAssemblyState.h>
#include <vsg/state/MultisampleState.h>
#include <vsg/state/RasterizationState.h>
#include <vsg/state/ShaderStage.h>
#include <vsg/state/VertexInputState.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    std::string vert_PushConstants = R"(#version 450
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

    std::string frag_PushConstants = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, fragTexCoord);
})";
} // namespace ehb

namespace ehb
{
    // hook into our virtual filesystem
    vsg::Path findFileCallback(const vsg::Path& filename, const vsg::Options* options)
    {
        static auto namingKeyMap = options->getObject<NamingKeyMap>("NamingKeyMap");

        return namingKeyMap->findDataFile(filename);
    }

    void InitState::enter()
    {
        log->info("Entered Init State");

        IFileSys& fileSys = systems.fileSys;
        IConfig& config = systems.config;
        NamingKeyMap& namingKeyMap = systems.namingKeyMap;
        auto options = systems.options;

        // sanity check to stop errors from being thrown when using vsgExamples
        if (!config.getString("bits", "").empty())
        {
            fileSys.init(config);
            namingKeyMap.init(fileSys);

            options->setObject("NamingKeyMap", &namingKeyMap);
            namingKeyMap.ref(); // this is a vsg::Object so it can be stored in options but I don't want to make it an actual ref pointer as there should only be 1 instance

            options->readerWriters = { ReaderWriterRAW::create(fileSys), ReaderWriterSNO::create(fileSys), ReaderWriterASP::create(fileSys), ReaderWriterSiegeNodeList::create(fileSys), ReaderWriterRegion::create(fileSys) };

            options->objectCache = vsg::ObjectCache::create();

            // options.paths is required internally for VSG as a check if it should actually use the findFileCallback
            // TOOD: discuss with Robert is this is intended behavior
            options->paths = { "/" };
            options->findFileCallback = &findFileCallback;
        }

        // set up search paths and load shaders
        vsg::ref_ptr<vsg::ShaderStage> vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", vert_PushConstants);
        vsg::ref_ptr<vsg::ShaderStage> fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", frag_PushConstants);
        if (!vertexShader || !fragmentShader) { log->error("Could not create shaders."); }

        // set up graphics pipeline
        vsg::DescriptorSetLayoutBindings descriptorBindings{
            {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
        };

        vsg::DescriptorSetLayouts descriptorSetLayouts{vsg::DescriptorSetLayout::create(descriptorBindings)};

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

        vsg::GraphicsPipelineStates pipelineStates{vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
                                                   vsg::InputAssemblyState::create(),
                                                   vsg::RasterizationState::create(),
                                                   vsg::MultisampleState::create(),
                                                   vsg::ColorBlendState::create(),
                                                   vsg::DepthStencilState::create()};

        auto pipelineLayout = vsg::PipelineLayout::create(descriptorSetLayouts, pushConstantRanges);
        auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{vertexShader, fragmentShader}, pipelineStates);
        auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

        // accessed by the ReaderWriters to setup pipelines and layouts
        options->setObject("SiegeNodeGraphicsPipeline", bindGraphicsPipeline);
        options->setObject("SiegeNodeLayout", bindGraphicsPipeline->pipeline->layout);

        if (const std::string& state = config.getString("state"); !state.empty())
        {
            systems.gameStateMgr.request(state);
        }
        else
        {
#if SIEGE_VSG_EXAMPLES_ENABLED
            // Dungeon Siege content is unavailable so default to something the user can run
            if (config.getString("bits", "").empty())
            {
                systems.gameStateMgr.request("vsgExamplesDraw");

                return;
            }
#endif

#if SIEGE_TEST_STATES_ENABLED
            // default to region test state if nothing passed to command line
            systems.gameStateMgr.request("RegionTestState");
#endif
        }
    }

    void InitState::leave() {}

    void InitState::update(double deltaTime) {}
} // namespace ehb
