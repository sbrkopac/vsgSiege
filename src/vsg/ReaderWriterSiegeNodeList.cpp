
#include "ReaderWriterSiegeNodeList.hpp"
#include "io/IFileSys.hpp"
#include "io/StringTool.hpp"
#include "vsg/ReaderWriterSNO.hpp"
#include "world/SiegeNode.hpp"

#include <vsg/io/read.h>

namespace ehb
{
    ReaderWriterSiegeNodeList::ReaderWriterSiegeNodeList(IFileSys& fileSys) :
        fileSys(fileSys)
    {
        log = spdlog::get("log");

        static const std::string directory = "/world/global/siege_nodes";

        fileSys.eachGasFile(
            directory,
            [this](const std::string& filename, auto doc) {
                for (auto root : doc->eachChild())
                {
                    for (auto node : root->eachChild())
                    {
                        const auto itr = keyMap.emplace(node->valueOf("guid"), stringtool::convertToLowerCase(node->valueOf("filename")));

                        if (itr.second != true)
                        {
                            log->error("duplicate mesh mapping found: tried to insert {} for guid {}, but found filename {} there already", node->valueOf("filename"), node->valueOf("guid"), itr.first->second);
                        }
                    }
                }
            });

        log->info("{} loaded nodes {} into its mappings", __func__, keyMap.size());
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterSiegeNodeList::read(const vsg::Path& filename, vsg::ref_ptr<const vsg::Options> options) const
    {
        const std::string simpleFilename = vsg::simpleFilename(filename);

        // check to make sure this is a nodes.gas file
        if (simpleFilename != "nodes") return {};

        log->info("about to build region with path : {} and simpleFilename {}", filename, simpleFilename);

        InputStream stream = fileSys.createInputStream(filename);

        return read(*stream, options);
    }

    vsg::ref_ptr<vsg::Object> ReaderWriterSiegeNodeList::read(std::istream& stream, vsg::ref_ptr<const vsg::Options> options) const
    {
        struct DoorEntry
        {
            uint32_t id = 0;
            uint32_t farDoor = 0;
            uint32_t farGuid = 0;
        };

        std::unordered_multimap<uint32_t, DoorEntry> doorMap;
        std::unordered_map<uint32_t, vsg::MatrixTransform*> nodeMap;
        std::set<uint32_t> completeSet;

        if (Fuel doc; doc.load(stream))
        {
            auto group = vsg::Group::create();

            for (const auto node : doc.eachChildOf("siege_node_list"))
            {
                const uint32_t nodeGuid = node->valueAsUInt("guid");
                // const uint64_t meshGuid = node->valueAsUInt("mesh_guid");
                const std::string meshGuid = stringtool::convertToLowerCase(node->valueOf("mesh_guid"));

                const std::string& texSetAbbr = node->valueOf("texsetabbr");

                // log->info("nodeGuid: {}, meshGuid: {}, texSetAbbr: '{}'", nodeGuid, meshGuid, texSetAbbr);

                for (const auto child : node->eachChild())
                {
                    DoorEntry e;

                    e.id = child->valueAsInt("id");
                    e.farDoor = child->valueAsInt("fardoor");
                    // NOTE: explicitly not using valueAsUInt because of 64bit value
                    e.farGuid = std::stoul(child->valueOf("farguid"), nullptr, 16);

                    doorMap.emplace(nodeGuid, std::move(e));
                }

                const std::string meshFileName = resolveFileName(meshGuid);

                if (meshFileName != meshGuid)
                {
                    if (auto mesh = vsg::read_cast<vsg::Group>(meshFileName, options); mesh != nullptr)
                    {
                        auto xform = vsg::MatrixTransform::create();

                        xform->setValue("bounds_camera", node->valueAsBool("bounds_camera"));
                        xform->setValue("camera_fade", node->valueAsBool("camera_fade"));
                        xform->setValue<uint32_t>("guid", node->valueAsUInt("guid"));
                        xform->setValue<uint32_t>("nodelevel", node->valueAsUInt("nodelevel"));
                        xform->setValue<uint32_t>("nodeobject", node->valueAsUInt("nodeobject"));
                        xform->setValue<uint32_t>("nodesection", node->valueAsUInt("nodesection"));
                        xform->setValue("occludes_camera", node->valueAsBool("occludes_camera"));
                        xform->setValue("occludes_light", node->valueAsBool("occludes_light"));

                        group->addChild(xform);
                        xform->addChild(mesh);

                        nodeMap.emplace(nodeGuid, xform);
                    }
                }
                else
                {
                    log->error("mesh guid {} is not listed in {}", meshGuid, "/world/global/siege_nodes");
                }
            }

            // now position it all
            const uint32_t targetGuid = doc.valueAsUInt("siege_node_list:targetnode");

            std::function<void(const uint32_t)> func;

            func = [&func, &doorMap, &nodeMap, &completeSet](const uint32_t guid) {
                if (completeSet.insert(guid).second)
                {
                    auto targetNode = nodeMap[guid];

                    const auto range = doorMap.equal_range(guid);

                    for (auto entry = range.first; entry != range.second; ++entry)
                    {
                        auto connectNode = nodeMap[entry->second.farGuid];

                        SiegeNodeMesh::connect(targetNode, entry->second.id, connectNode, entry->second.farDoor);

                        if (completeSet.count(entry->second.farGuid) == 0) { func(entry->second.farGuid); }
                    }
                }
            };

            func(targetGuid);

            log->debug("region loaded with {} nodes, targetGuid: 0x{:x}", group->children.size(), targetGuid);

            return group;
        }

        return {};
    };
} // namespace ehb
