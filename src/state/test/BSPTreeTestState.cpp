
#include "BSPTreeTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"
#include "world/Region.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/MatrixTransform.h>

#include <spdlog/spdlog.h>
#include <vsg/all.h>

namespace ehb
{
    class BSPTreeRenderer : public SiegeVisitorBase
    {
    public:

        int count = 0;

        void apply(vsg::Group& group) override
        {
            // this will dispatch to our custom types
            if (handleCustomGroups(group)) return;

            group.traverse(*this);
        }

        // we need to handle that region and tell this visitor to traverse it or we'll never get to our nodes
        void apply(Region& region) override
        {
            region.traverse(*this);
        }

        void apply(SiegeNodeMesh& mesh) override
        {
            count++;
        }
    };
}

namespace ehb
{
    std::string vert_PushConstants2 = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PushConstants {
    mat4 projection;
    mat4 modelview;
} pc;

layout(location = 0) in vec3 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = (pc.projection * pc.modelview) * vec4(inPosition, 1.0);
}
)";

    std::string frag_PushConstants2 = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

void main() {
    vec3 green = vec3 (0.078, 0.309, 0.141);
    outColor = vec4(green, 1.0);
})";
} // namespace ehb

namespace ehb
{
    vsg::ref_ptr<vsg::Group> bspTriangles; // there is probably a better way to do this
    void generateBSPDrawRender(BSPTree* tree, const BSPNode* node) // the first param is because BSPTree holds a pointer to all the triangles
    {
        assert(node != nullptr);

        static auto log = spdlog::get("log");

        if (node->m_IsLeaf)
        {
            if (node->m_Triangles)
            {
                for (int32_t i = 0; i < node->m_NumTriangles; ++i)
                {
                    TriNorm& tri = tree->m_Triangles[node->m_Triangles[i]];

                    auto vertices = vsg::vec3Array::create(3);
                    auto indices = vsg::ushortArray::create({ 0, 1, 2 });

                    std::memcpy(&(*vertices)[0], &tri.m_Vertices[0], sizeof(vsg::vec3));
                    std::memcpy(&(*vertices)[1], &tri.m_Vertices[1], sizeof(vsg::vec3));
                    std::memcpy(&(*vertices)[2], &tri.m_Vertices[2], sizeof(vsg::vec3));

                    auto commands = vsg::Commands::create();
                    commands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{ vertices }));
                    commands->addChild(vsg::BindIndexBuffer::create(indices));
                    commands->addChild(vsg::DrawIndexed::create(static_cast<uint32_t>(indices->valueCount()), 1, 0, 0, 0));

                    bspTriangles->addChild(commands);
                }
            }
        }
        else
        {
            generateBSPDrawRender(tree, node->m_LeftChild);
            generateBSPDrawRender(tree, node->m_RightChild);
        }
    }
}

namespace ehb
{
    void BSPTreeTestState::enter()
    {
        log->info("Entered Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        auto options = systems.options;

        static std::string siegeNode("t_grs01_houses_generic-a-log");

        bspTriangles = new vsg::Group;

        TimePoint start = Timer::now();

        // set up search paths and load shaders
        vsg::ref_ptr<vsg::ShaderStage> vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", vert_PushConstants2);
        vsg::ref_ptr<vsg::ShaderStage> fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", frag_PushConstants2);
        if (!vertexShader || !fragmentShader) { log->error("Could not create shaders."); }

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
            VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX} // vertex data
        };

        // TODO: normals
        vsg::VertexInputState::Attributes vertexAttributeDescriptions{
            VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0} // vertex data
        };

        vsg::GraphicsPipelineStates pipelineStates{ vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
                                                   vsg::InputAssemblyState::create(),
                                                   vsg::RasterizationState::create(),
                                                   vsg::MultisampleState::create(),
                                                   vsg::ColorBlendState::create(),
                                                   vsg::DepthStencilState::create() };

        auto pipelineLayout = vsg::PipelineLayout::create(descriptorSetLayouts, pushConstantRanges);
        auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{ vertexShader, fragmentShader }, pipelineStates);
        auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

#if 0
        if (auto sno = vsg::read_cast<SiegeNodeMesh>(siegeNode, options); sno != nullptr)
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options->getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(pipeline);

            // transforms are required to connect two nodes together using doors
            auto t1 = vsg::MatrixTransform::create();

            t1->addChild(sno);

            scene3d.addChild(t1);

            scene3d.addChild(bindGraphicsPipeline);

            generateBSPDrawRender(sno->tree(), sno->tree()->GetRoot());

            scene3d.addChild(bspTriangles);

            auto count = vsg::visit<BSPTreeRenderer>(&scene3d).count;
            log->info("COUNT = {}", count);

            // workaround
            compile(systems, systems.scene3d);
        }
        else
        {
            log->critical("Unable to load SiegeNode: {}", siegeNode);
        }
#else
        static std::string region = "town_center";
        static std::string regionpath = "/world/maps/multiplayer_world/regions/" + region + ".region"; // extension for the loader

        if (auto region = vsg::read_cast<Region>(regionpath, options))
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options->getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(pipeline);
            scene3d.addChild(region);

            auto count = vsg::visit<BSPTreeRenderer>(region).count;
            log->info("Visitor has {} meshes", count);

            // workaround
            compile(systems, systems.scene3d);
        }

        Duration duration = Timer::now() - start;
        log->info("SiegeNodeTest entire state profiled @ {} milliseconds", duration.count());
#endif
    }

    void BSPTreeTestState::leave() {}

    void BSPTreeTestState::update(double deltaTime) {}
} // namespace ehb