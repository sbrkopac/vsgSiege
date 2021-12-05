
#include "Aspect.hpp"
#include "AspectImpl.hpp"

#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/state/PipelineLayout.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/DescriptorSet.h>
#include <vsg/io/read.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    Aspect::Aspect(std::shared_ptr<Impl> impl, vsg::ref_ptr<const vsg::Options> options) : d(std::move(impl))
    {
        for (const auto& mesh : d->subMeshes)
        {
            uint32_t f = 0; // track which face the loader is loading across the sub mesh
            for (uint32_t i = 0; i < mesh.textureCount; ++i)
            {
                // create vertex data per sub mesh
                auto vertices = vsg::vec3Array::create(mesh.cornerCount);
                auto normals = vsg::vec3Array::create(mesh.cornerCount);
                auto colors = vsg::vec4Array::create(mesh.cornerCount);
                auto tcoords = vsg::vec2Array::create(mesh.cornerCount);

                auto attributeArrays = vsg::DataList{ vertices, colors, tcoords };

                for (uint32_t cornerCounter = 0; cornerCounter < mesh.cornerCount; ++cornerCounter)
                {
                    const auto& c = mesh.corners[cornerCounter];
                    // const auto& w = mesh.wCorners[cornerCounter];

                    (*vertices)[cornerCounter] = c.position;
                    (*normals)[cornerCounter] = c.normal;
                    (*colors)[cornerCounter] = vsg::vec4(c.color[0], c.color[1], c.color[2], c.color[3]);
                    (*tcoords)[cornerCounter] = c.texCoord;

                }

                // each face is a triangle so faceSpan * 3
                auto indices = vsg::ushortArray::create(mesh.matInfo[i].faceSpan * 3);

                // loop each face and expand out the loop by 3 each time
                for (uint32_t fpt = 0; fpt < mesh.matInfo[i].faceSpan * 3; fpt += 3)
                {
                    (*indices)[fpt] = mesh.faceInfo.cornerIndex.at(f).index[0] + mesh.faceInfo.cornerStart[i];
                    (*indices)[fpt + 1] = mesh.faceInfo.cornerIndex.at(f).index[1] + mesh.faceInfo.cornerStart[i];
                    (*indices)[fpt + 2] = mesh.faceInfo.cornerIndex.at(f).index[2] + mesh.faceInfo.cornerStart[i];

                    ++f;
                }

                if (auto layout = static_cast<const vsg::PipelineLayout*>(options->getObject("SiegeNodeLayout")); layout != nullptr)
                {
                    if (auto textureData = vsg::read_cast<vsg::Data>(d->textureNames[i], options))
                    {
                        auto texture = vsg::DescriptorImage::create(vsg::Sampler::create(), textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                        auto descriptorSet = vsg::DescriptorSet::create(layout->setLayouts[0], vsg::Descriptors{ texture });
                        auto bindDescriptorSets = vsg::BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, const_cast<vsg::PipelineLayout*>(layout), 0, vsg::DescriptorSets{ descriptorSet });

                        addChild(bindDescriptorSets);
                    }
                }

                auto commands = vsg::Commands::create();
                commands->addChild(vsg::BindVertexBuffers::create(0, attributeArrays));
                commands->addChild(vsg::BindIndexBuffer::create(indices));
                commands->addChild(vsg::DrawIndexed::create(static_cast<uint32_t>(indices->valueCount()), 1, 0, 0, 0));
                addChild(commands);
            }
        }
    }
}