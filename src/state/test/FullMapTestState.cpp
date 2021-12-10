
#include "FullMapTestState.hpp"

#include "Systems.hpp"
#include "world/SiegeNode.hpp"
#include "world/WorldMapData.hpp"

#include <filesystem>
#include <spdlog/spdlog.h>

#include <vsg/io/read.h>
#include <vsg/nodes/MatrixTransform.h>
#include <vsg/nodes/StateGroup.h>
#include <vsg/threading/OperationThreads.h>

namespace ehb
{
    void FullMapTestState::enter()
    {
        log->info("Entered Region Test State");

        LocalFileSys& fileSys = systems.fileSys;
        vsg::StateGroup& scene3d = *systems.scene3d;
        vsg::Options& options = *systems.options;

        WorldMapDataCache worldMapDataCache;
        worldMapDataCache.init(fileSys);

        // decorate the graph with the pipeline
        // TODO: this should really be done somewhere else i think?
        if (vsg::ref_ptr<vsg::BindGraphicsPipeline> pipeline(options.getObject<vsg::BindGraphicsPipeline>("SiegeNodeGraphicsPipeline")); pipeline) { scene3d.addChild(pipeline); }
        else
        {
            log->critical("failed to find a graphics pipeline");
        }

        const auto& world = worldMapDataCache.data["multiplayer_world"];
        static std::string startingRegion = "town_center";
        auto targetRegionGuid = world.regionGuidFromName(startingRegion);

        log->info("world will be assembled around the starting region {}:0x{:x}", startingRegion, targetRegionGuid);

        // the below code is chunked out for easier modification
        // we could probably do everything below in a single pass but that gets little complicated

        // populated by the stitch data for the incoming region - we use a set to ensure uniqueness when looping
        std::set<uint32_t> eachRegionGuidToLoad;

        // loop all available regions and load them into memory
        for (const auto& [region, stitch] : world.stitchIndex.data)
        {
            for (const auto& stitch : world.stitchIndex.eachStitch(region))
            {
                eachRegionGuidToLoad.emplace(stitch.first);
            }
        }

        // global map of all our nodes in the graph
        std::unordered_map<uint32_t, vsg::ref_ptr<vsg::MatrixTransform>> eachNode;

        // hold our loaded regions prior to adding them to the graph
        std::unordered_map<uint32_t, vsg::ref_ptr<vsg::MatrixTransform>> loadedRegions;

        vsg::ref_ptr<vsg::Options> threadedOptions(&options);
        threadedOptions->operationThreads = vsg::OperationThreads::create(2);

        // loop and load our regions
        // its important to use the set defined above as if we don't we will get duplicated regions for loading
        vsg::Paths allRegionPaths;
        for (const uint32_t regionGuid : eachRegionGuidToLoad)
        {
            auto r = world.regionNameAndPathFromRegionGuid(regionGuid).first;
            allRegionPaths.emplace_back(r);
        }

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

        log->info("{} regions are ready to be placed and rendered, we are also storing {} nodes in the global map", loadedRegions.size(), eachNode.size());

        // a set to the already processed regions - maybe we can do a std::pair in a set with a loaded flag?
        std::set<uint32_t> placedRegions;
        placedRegions.emplace(targetRegionGuid);           // our starting region doesn't need processing
        scene3d.addChild(loadedRegions[targetRegionGuid]); // make sure its in the graph

        std::function<void(uint32_t guid)> recurse = [&, this](uint32_t guid) {
            for (const auto& entry : world.stitchIndex.eachStitch(guid))
            {
                const uint32_t region2Guid = entry.first;
                const auto& stitch = entry.second;

                // quick check to ensure this region hasn't already been placed
                if (placedRegions.count(region2Guid) == 0)
                {
                    vsg::ref_ptr<vsg::MatrixTransform> targetRegion = loadedRegions[guid];
                    vsg::ref_ptr<vsg::MatrixTransform> region2Xform = loadedRegions[region2Guid];
                    vsg::ref_ptr<vsg::MatrixTransform> node1Xform = eachNode[stitch.node1];
                    vsg::ref_ptr<vsg::MatrixTransform> node2Xform = eachNode[stitch.node2];

                    if (targetRegion == nullptr || region2Xform == nullptr || node1Xform == nullptr || node2Xform == nullptr)
                    {
                        log->critical("target region: 0x{:x}, region2xform: 0x{:x} or stitched nodes are null - you're hosed", guid, region2Guid);
                    }

                    if (world.regionForNode(stitch.node1) != guid) { log->critical("mismatch between stitch and guid"); }

                    log->info("stitching region 0x{:x} to region 0x{:x}", region2Guid, guid);

                    // Hold onto ur butts.
                    SiegeNodeMesh::connect(targetRegion, node1Xform, stitch.door1, region2Xform, node2Xform, stitch.door2);

                    scene3d.addChild(region2Xform);

                    placedRegions.emplace(region2Guid);

                    recurse(region2Guid);
                }
                else
                {
                    // log->warn("already placed region, skipping");
                }
            }
        };

        recurse(targetRegionGuid);

        // workaround
        compile(systems, systems.scene3d);
    }

    void FullMapTestState::leave() {}

    void FullMapTestState::update(double deltaTime) {}
} // namespace ehb
