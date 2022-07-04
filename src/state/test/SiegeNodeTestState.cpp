
#include "SiegeNodeTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/MatrixTransform.h>

#include <spdlog/spdlog.h>

#include <vsg/all.h>

namespace ehb
{
    static std::string vert_PushConstants3 = R"(#version 450
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
    static std::string frag_PushConstants3 = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

void main() {
    vec3 white = vec3 (1.0, 1.0, 1.0);
    outColor = vec4(white, 1.0);
})";
}

namespace ehb
{
    class CreateNormalDebugData : public SiegeVisitorBase
    {
    public:

        vsg::ref_ptr<vsg::Group> group = vsg::Group::create();

        static vsg::ref_ptr<vsg::BindGraphicsPipeline> createGraphicsPipeline()
        {
            vsg::ref_ptr<vsg::ShaderStage> vertexShader = vsg::ShaderStage::create(VK_SHADER_STAGE_VERTEX_BIT, "main", vert_PushConstants3);
            vsg::ref_ptr<vsg::ShaderStage> fragmentShader = vsg::ShaderStage::create(VK_SHADER_STAGE_FRAGMENT_BIT, "main", frag_PushConstants3);

            vsg::PushConstantRanges pushConstantRanges{
                {VK_SHADER_STAGE_VERTEX_BIT, 0, 128} // projection view, and model matrices, actual push constant calls automatically provided by the VSG's DispatchTraversal
            };

            // set up graphics pipeline
            vsg::DescriptorSetLayoutBindings descriptorBindings{
                {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr} // { binding, descriptorTpe, descriptorCount, stageFlags, pImmutableSamplers}
            };

            vsg::DescriptorSetLayouts descriptorSetLayouts{ vsg::DescriptorSetLayout::create(descriptorBindings) };

            vsg::VertexInputState::Bindings vertexBindingsDescriptions{
                VkVertexInputBindingDescription{0, sizeof(vsg::vec3), VK_VERTEX_INPUT_RATE_VERTEX} // vertex data
            };

            vsg::VertexInputState::Attributes vertexAttributeDescriptions{
                VkVertexInputAttributeDescription{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0} // vertex data
            };

            vsg::GraphicsPipelineStates pipelineStates{ vsg::VertexInputState::create(vertexBindingsDescriptions, vertexAttributeDescriptions),
                                           vsg::InputAssemblyState::create(VK_PRIMITIVE_TOPOLOGY_LINE_LIST),
                                           vsg::RasterizationState::create(),
                                           vsg::MultisampleState::create(),
                                           vsg::ColorBlendState::create(),
                                           vsg::DepthStencilState::create() };
            auto pipelineLayout = vsg::PipelineLayout::create(descriptorSetLayouts, pushConstantRanges);
            auto graphicsPipeline = vsg::GraphicsPipeline::create(pipelineLayout, vsg::ShaderStages{ vertexShader, fragmentShader }, pipelineStates);
            auto bindGraphicsPipeline = vsg::BindGraphicsPipeline::create(graphicsPipeline);

            return bindGraphicsPipeline;
        }

        void apply(SiegeNodeMesh& mesh) override
        {
            auto log = spdlog::get("log");
            if (auto renderObject = mesh.renderObject())
            {
                log->info("apply(SiegeNodeMesh&)");

                sVertex norm[2];
                for (int32_t i = 0; i < renderObject->numVertices(); ++i)
                {
                    norm[0] = norm[1] = renderObject->vertices()[i];

                    norm[1].x += mesh.normals()[i].x;
                    norm[1].y += mesh.normals()[i].y;
                    norm[1].z += mesh.normals()[i].z;

                    auto vertices = vsg::vec3Array::create(2);
                    auto indices = vsg::ushortArray::create({ 0, 1 });

                    (*vertices)[0].x = norm[0].x;
                    (*vertices)[0].y = norm[0].y;
                    (*vertices)[0].z = norm[0].z;

                    (*vertices)[1].x = norm[1].x;
                    (*vertices)[1].y = norm[1].y;
                    (*vertices)[1].z = norm[1].z;

                    auto vid = vsg::VertexIndexDraw::create();
                    vid->assignArrays(vsg::DataList{ vertices });
                    vid->assignIndices(indices);
                    vid->indexCount = static_cast<uint32_t>(indices->size());
                    vid->indexCount = 1;

                    group->addChild(vid);
                }
            }
        }
    };
}

namespace ehb
{
    void SiegeNodeTestState::enter()
    {
        log->info("Entered Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        auto options = systems.options;

        static std::string siegeNode("t_grs01_houses_generic-a-log");
        //static std::string siegeNode("t_xxx_flr_04x04-v0");

        TimePoint start = Timer::now();

        if (auto sno = vsg::read_cast<SiegeNodeMesh>(siegeNode, options); sno != nullptr)
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options->getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            // add the siege node graphics pipeline
            scene3d.addChild(pipeline);

            // transforms are required to connect two nodes together using doors
            auto t1 = vsg::MatrixTransform::create();
            auto t2 = vsg::MatrixTransform::create();

            t1->addChild(sno);
            t2->addChild(sno);

            SiegeNodeMesh::connect(t1, 2, t2, 1);

            scene3d.addChild(t1);
            scene3d.addChild(t2);

            // add the graphics pipeline to visualize the normal data
            scene3d.addChild(CreateNormalDebugData::createGraphicsPipeline());
            auto g = vsg::visit<CreateNormalDebugData>(sno).group;
            scene3d.addChild(g);

            // workaround
            compile(systems, systems.scene3d);
        }
        else
        {
            log->critical("Unable to load SiegeNode: {}", siegeNode);
        }

        Duration duration = Timer::now() - start;
        log->info("SiegeNodeTest entire state profiled @ {} milliseconds", duration.count());
    }

    void SiegeNodeTestState::leave() {}

    void SiegeNodeTestState::update(double deltaTime) {}
} // namespace ehb
