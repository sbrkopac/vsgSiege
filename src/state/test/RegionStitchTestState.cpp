
#include "RegionStitchTestState.hpp"

#include "Systems.hpp"
#include "vsg/ReaderWriterSNO.hpp"
#include "world/SiegeNode.hpp"
#include "world/WorldMapData.hpp"

#include <vsg/io/read.h>
#include <vsg/maths/transform.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/threading/OperationThreads.h>

#include <spdlog/spdlog.h>

namespace ehb
{
    void RegionStitchTestState::enter()
    {
        log->info("Entered Region Test State");

        LocalFileSys& fileSys = systems.fileSys;
        vsg::StateGroup& scene3d = *systems.scene3d;
        vsg::Options& options = *systems.options;

        WorldMapDataCache worldMapDataCache;
        worldMapDataCache.init(fileSys);

        vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline"));
        if (pipeline == nullptr)
        {
            log->critical("SiegeNodeGraphicsPipeline has not been setup!");

            return;
        }

        scene3d.addChild(pipeline);

        const auto& world = worldMapDataCache.data["multiplayer_world"];
        static std::string startingRegion = "town_center";
        auto targetRegionGuid = world.regionGuidFromName(startingRegion);
        vsg::ref_ptr<vsg::MatrixTransform> targetRegionTransform;

        log->info("world will be assembled around the starting region {}:0x{:x}", startingRegion, targetRegionGuid);

        // the below code is chunked out for easier modification
        // we could probably do everything below in a single pass but that gets little complicated

        // populated by the stitch data for the incoming region - we use a set to ensure uniqueness when looping
        std::set<uint32_t> eachRegionGuidToLoad;
        eachRegionGuidToLoad.emplace(targetRegionGuid);

        // loop stitches and find out what region owns those stitches and get ready to load them
        for (const auto& stitch : world.stitchIndex.eachStitch(targetRegionGuid))
        {
            log->info("stitch entry {}", stitch.first);

            eachRegionGuidToLoad.emplace(stitch.first);
        }

        log->info("{}:0x{:x} has {} regions stitched to it", startingRegion, targetRegionGuid, eachRegionGuidToLoad.size());

        // global map of all our nodes in the graph
        std::unordered_map<uint32_t, vsg::ref_ptr<vsg::MatrixTransform>> eachNode;

        // loop and load our regions
        // its important to use the set defined above as if we don't we will get duplicated regions for loading
        vsg::Paths allRegionPaths;
        for (const uint32_t regionGuid : eachRegionGuidToLoad)
        {
            auto r = world.regionNameAndPathFromRegionGuid(regionGuid).first;
            allRegionPaths.emplace_back(r);
        }

        // hold our loaded regions prior to adding them to the graph
        std::unordered_map<uint32_t, vsg::ref_ptr<vsg::MatrixTransform>> loadedRegions;

        vsg::ref_ptr<vsg::Options> threadedOptions(&options);
        threadedOptions->operationThreads = vsg::OperationThreads::create(2);

        auto regions = vsg::read(allRegionPaths, threadedOptions);
        for (auto& [path, region] : regions)
        {
            uint32_t guid;
            region->getValue("guid", guid);
            const auto regionData = region->cast<vsg::MatrixTransform>()->children[0]->cast<vsg::Group>();
            if (guid != 0)
            {
                for (const auto& node : regionData->children)
                {
                    if (uint32_t guid; node->getValue("guid", guid)) { eachNode.emplace(guid, node.cast<vsg::MatrixTransform>()); }
                    else
                    {
                        log->critical("there is a node in a loaded region with a guid");
                    }
                }

                loadedRegions.emplace(guid, region->cast<vsg::MatrixTransform>());
            }
            else
            {
                log->critical("unable to retrieve guid for 0x{:x} - did the loader set it properly?", guid);
            }
        }

        log->info("{} additional regions are ready to be placed and rendered, we are also storing {} nodes in the global map", loadedRegions.size(), eachNode.size());

        targetRegionTransform = loadedRegions[targetRegionGuid];
        scene3d.addChild(targetRegionTransform);

        // loop stitches and attempt to stitch all the regions together
        for (const auto& entry : world.stitchIndex.eachStitch(targetRegionGuid))
        {
            const uint32_t region2Guid = entry.first;
            const auto& stitch = entry.second;

            vsg::ref_ptr<vsg::MatrixTransform> region2Xform = loadedRegions[region2Guid];
            vsg::ref_ptr<vsg::MatrixTransform> node1Xform = eachNode[stitch.node1];
            vsg::ref_ptr<vsg::MatrixTransform> node2Xform = eachNode[stitch.node2];

            if (region2Xform == nullptr || node1Xform == nullptr || node2Xform == nullptr) { log->critical("target region or stitched nodes are null - you're hosed"); }

            // Hold onto ur butts.
            SiegeNodeMesh::connect(targetRegionTransform, node1Xform, stitch.door1, region2Xform, node2Xform, stitch.door2);

            scene3d.addChild(region2Xform);
        }

        // workaround
        compile(systems, systems.scene3d);
    }

    void RegionStitchTestState::leave() {}

    void RegionStitchTestState::update(double deltaTime) {}
} // namespace ehb
