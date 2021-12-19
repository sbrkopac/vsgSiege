
#include "RegionTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"
#include "world/Region.hpp"

#include <spdlog/spdlog.h>

#include <vsg/io/read.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/vk/ResourceRequirements.h>

namespace ehb
{
    struct SiegePos
    {
        uint32_t guid = 0;
        vsg::vec3 pos;

        explicit SiegePos(float x, float y, float z, uint32_t guid) : guid(guid)
        {
            pos.x = x;
            pos.y = y;
            pos.z = z;
        }
    };

    // SiegePos pos = valueAsSiegePos("-1.11579,0,-3.83528,0xc4660a9d");
    SiegePos valueAsSiegePos(const std::string& value)
    {
        const auto itr1 = value.find(',');
        const auto itr2 = value.find(',', itr1 + 1);
        const auto itr3 = value.find(',', itr2 + 2);

        float x = std::stof(value.substr(0, itr1));
        float y = std::stof(value.substr(itr1 + 1, itr2 - itr1 - 1));
        float z = std::stof(value.substr(itr2 + 1, itr3 - itr2 - 1));
        unsigned int node = std::stoul(value.substr(itr3 + 1), nullptr, 16);

        return SiegePos(x, y, z, node);
    }

    vsg::dquat toQuat(std::array<float, 4> array)
    {
        return { array[0], array[1], array[2], array[3] };
    }
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
        static std::string regionpath = "/world/maps/multiplayer_world/regions/" + region;
        static std::string regionpathext = "/world/maps/multiplayer_world/regions/" + region + ".region"; // extension for the loader

        bool loadObjects = true;

        if (auto region = vsg::read_cast<Region>(regionpathext, options)) 
        {
            vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options->getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));

            scene3d.addChild(pipeline);
            scene3d.addChild(region);

            auto regionData = Region::create();
            regionData->setNodeData(region);

            if (loadObjects)
            {
                systems.contentDb.init(systems.fileSys);

                if (systems.contentDb.isReady)
                {
                    // easy grouping of objects for now - this will require a different pipeline in the future
                    auto objectGroup = vsg::Group::create();

                    const std::string objectspath = regionpath + "/objects/regular/";

                    for (const auto& filename : { "non_interactive.gas", "actor.gas" })
                    {
                        if (auto doc = systems.fileSys.openGasFile(objectspath + filename))
                        {
                            for (const auto& node : doc->eachChild())
                            {
                                const auto tmpl = node->type();
                                const auto scid = node->name();

                                // log->info("tmp: {} - scid: {}", tmpl, scid);

                                if (auto placement = node->child("placement"))
                                {
                                    auto position = valueAsSiegePos(placement->valueOf("position"));
                                    vsg::quat orientation = placement->valueAsQuat("orientation");
                                    
                                    auto go = systems.contentDb.getGameObjectTmpl(tmpl);
                                    auto model = go->valueOf("aspect:model");

                                    if (vsg::ref_ptr<vsg::Group> asp = vsg::read_cast<vsg::Group>(model, options); asp != nullptr)
                                    {
                                        auto xform = vsg::MatrixTransform::create();
                                        xform->addChild(asp);

                                        if (auto localNodeTransform = regionData->placedNodeXformMap[position.guid]; localNodeTransform != nullptr)
                                        {
                                            xform->matrix = localNodeTransform->matrix * vsg::dmat4(vsg::translate(position.pos));
                                            xform->matrix = xform->matrix * vsg::dmat4(vsg::rotate(orientation));
                                        }
                                        else { log->error("Couldn't find a siege node with guid {} for template {}", position.guid, tmpl); }

                                        objectGroup->addChild(xform);

                                        log->info("added: {} to graph", model);
                                    }
                                    else
                                    {
                                        log->error("unable to read model: {}", model);
                                    }
                                }
                            }
                        }
                    }

                    systems.scene3d->addChild(objectGroup);
                }
            }

            SiegeNodeMeshCounter v;
            region->traverse(v);

            log->info("Visitor has {} meshes", v.count);

            // workaround
            compile(systems, systems.scene3d);
        }
    }

    void RegionTestState::leave() {}

    void RegionTestState::update(double deltaTime) {}
} // namespace ehb
