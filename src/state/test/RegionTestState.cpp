
#include "RegionTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"
#include "world/Region.hpp"

#include <spdlog/spdlog.h>

#include <vsg/io/read.h>
#include <vsg/maths/transform.h>
#include <vsg/maths/mat4.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/ResourceRequirements.h>

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
    class CreateNormalDebugDataForRegion : public SiegeVisitorBase
    {
    public:

        vsg::ref_ptr<vsg::Group> group = vsg::Group::create();

        MatrixStack matrixStack;

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

        void apply(vsg::Group& group) override
        {
            if (handleCustomGroups(group)) return;

            group.traverse(*this);
        }

        void apply(Region& region) override
        {
            region.traverse(*this);
        }

        void apply(vsg::MatrixTransform& transform) override
        {
            if (matrixStack.empty())
                matrixStack.push_back(transform.transform(vsg::dmat4{}));
            else
                matrixStack.push_back(transform.transform(matrixStack.back()));

            transform.traverse(*this);

            matrixStack.pop_back();
        }

        void apply(SiegeNodeMesh& mesh) override
        {
            auto log = spdlog::get("log");
            if (auto renderObject = mesh.mesh()->renderObject())
            {
                sVertex norm[2];
                for (int32_t i = 0; i < renderObject->numVertices(); ++i)
                {
                    norm[0] = norm[1] = renderObject->vertices()[i];

                    norm[1].x += mesh.mesh()->normals()[i].x;
                    norm[1].y += mesh.mesh()->normals()[i].y;
                    norm[1].z += mesh.mesh()->normals()[i].z;

                    auto vertices = vsg::vec3Array::create(
                        { { norm[0].x, norm[0].y, norm[0].z },
                          { norm[1].x, norm[1].y, norm[1].z }
                        });
                    auto indices = vsg::ushortArray::create({ 0, 1 });

                    for (auto& v : *vertices)
                    {
                        v = matrixStack.back() * (vsg::dvec3) v;
                    }

                    // TODO: VertexIndexDraw
                    auto commands = vsg::Commands::create();
                    commands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList{ vertices }));
                    commands->addChild(vsg::BindIndexBuffer::create(indices));
                    commands->addChild(vsg::DrawIndexed::create(static_cast<uint32_t>(indices->valueCount()), 1, 0, 0, 0));

                    group->addChild(commands);
                }
            }
        }
    };
}

namespace ehb
{
    class SiegeNodeMeshCounter : public SiegeVisitorBase
    {
    public:

        int32_t count = 0;

        virtual void apply(vsg::Object& object) override
        {
            object.traverse(*this);
        }

        virtual void apply(SiegeNodeMesh& mesh) override
        {
            count++;
        }
    };
}

namespace ehb
{
    void RegionTestState::enter()
    {
        log->info("Entered Region Test State");

        vsg::StateGroup& scene3d = *systems.scene3d;
        auto options = systems.options;

        static std::string region = "town_center";
        static std::string regionpath = "/world/maps/multiplayer_world/regions/" + region + ".region"; // extension for the loader

        if (auto region = vsg::read_cast<Region>(regionpath, options))
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options->getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(pipeline);
            scene3d.addChild(region);

            SiegeNodeMeshCounter v;
            region->traverse(v);

            scene3d.addChild(CreateNormalDebugDataForRegion::createGraphicsPipeline());
            CreateNormalDebugDataForRegion debugDataForRegion;
            region->accept(debugDataForRegion);
            scene3d.addChild(debugDataForRegion.group);

            log->info("Visitor has {} meshes", v.count);

            // workaround
            compile(systems, systems.scene3d);
        }
    }

    void RegionTestState::leave() {}

    void RegionTestState::update(double deltaTime) {}
} // namespace ehb
