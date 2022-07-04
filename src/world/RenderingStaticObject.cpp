
#include "RenderingStaticObject.hpp"

#include <vsg/io/read.h>
#include <vsg/nodes/Group.h>
#include <vsg/commands/BindIndexBuffer.h>
#include <vsg/commands/BindVertexBuffers.h>
#include <vsg/commands/Commands.h>
#include <vsg/commands/DrawIndexed.h>
#include <vsg/state/DescriptorImage.h>
#include <vsg/state/BindDescriptorSet.h>
#include <vsg/utils/SharedObjects.h>

namespace ehb
{
    vsg::ref_ptr<vsg::Group> RenderingStaticObject::buildDrawCommands()
    {
        auto sharedObjects = options->sharedObjects;

        auto group = vsg::Group::create();

        // create our GPU arrays
        // TODO: can we just setup sVertex in the pipeline rather than the individual components?
        auto vertices = vsg::vec3Array::create(m_numVertices);
        auto colors = vsg::uintArray::create(m_numVertices);
        auto tcoords = vsg::vec2Array::create(m_numVertices);

        for (auto i = 0; i < m_numVertices; ++i)
        {
            auto softwareVertex = m_pVertices[i];

            (*vertices)[i].x = softwareVertex.x;
            (*vertices)[i].y = softwareVertex.y;
            (*vertices)[i].z = softwareVertex.z;

            (*tcoords)[i].x = softwareVertex.uv.u;
            (*tcoords)[i].y = softwareVertex.uv.v;
        }

        sharedObjects->share(vertices);
        sharedObjects->share(colors);
        sharedObjects->share(tcoords);

        for (auto i = m_TexStageList.begin(); i != m_TexStageList.end(); ++i)
        {
            if (!m_texlist[(*i).tIndex].alpha)
            {
                auto layout = options->getObject("SiegeNodeLayout")->cast<const vsg::PipelineLayout>();

                auto textureData = vsg::read_cast<vsg::Data>((*i).name, options);
                sharedObjects->share(textureData);

                auto texture = vsg::DescriptorImage::create(vsg::Sampler::create(), textureData, 0, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
                sharedObjects->share(texture);

                auto descriptorSet = vsg::DescriptorSet::create(layout->setLayouts[0], vsg::Descriptors{ texture });
                sharedObjects->share(descriptorSet);

                auto bindDescriptorSets = vsg::BindDescriptorSets::create(VK_PIPELINE_BIND_POINT_GRAPHICS, const_cast<vsg::PipelineLayout*>(layout), 0, vsg::DescriptorSets{ descriptorSet });
                sharedObjects->share(bindDescriptorSets);

                // add texture to graph
                group->addChild(bindDescriptorSets);

                auto indices = vsg::ushortArray::create((*i).numVIndices);
                indices->assign((*i).numVIndices, (*i).pVIndices);

                // offset our indices since we are using a global vertex pool
                std::transform(std::begin(*indices), std::end(*indices), std::begin(*indices), [&i](uint16_t v) -> uint16_t { return v + (*i).startIndex; });

                sharedObjects->share(indices);

                auto commands = vsg::Commands::create();
                commands->addChild(vsg::BindVertexBuffers::create(0, vsg::DataList { vertices, colors, tcoords }));
                commands->addChild(vsg::BindIndexBuffer::create(indices));
                commands->addChild(vsg::DrawIndexed::create(static_cast<uint32_t>(indices->valueCount()), 1, 0, 0, 0));

                sharedObjects->share(commands);

                group->addChild(commands);
            }
        }

        return group;
    }
}