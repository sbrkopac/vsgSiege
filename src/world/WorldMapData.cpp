
#include "WorldMapData.hpp"

#include "io/IFileSys.hpp"

namespace ehb
{
    void StitchIndex::init(IFileSys& fileSys, const std::string& path)
    {
        auto log = spdlog::get("log");

        std::unordered_map<uint32_t, uint32_t> node2region;

        if (auto doc = fileSys.openGasFile(path))
        {
            log->info("parsing {}", path);

            const auto root = doc->child("stitch_index");

            // first pass
            for (const auto node : root->eachChild())
            {
                if (node->type() == "stitch_index")
                {
                    const uint32_t regionGuid = std::stoul(node->name(), nullptr, 16);

                    for (const auto entry : node->eachAttribute())
                    {
                        if (const auto itr = entry.value.find(','); itr != std::string::npos)
                        {
                            const uint32_t nodeGuid = std::stoul(entry.value.substr(0, itr), nullptr, 16);

                            node2region.emplace(nodeGuid, regionGuid);

                            // log->info("pairing {} with region {}", nodeGuid, regionGuid);
                        }
                    }
                }
            }

            // second pass
            for (auto node : root->eachChild())
            {
                if (node->type() == "stitch_index")
                {
                    const uint32_t regionGuid = std::stoul(node->name(), nullptr, 16);

                    for (const auto entry : node->eachAttribute())
                    {
                        const auto i1 = entry.value.find(',', 0);
                        const auto i2 = entry.value.find(',', i1 + 1);
                        const auto i3 = entry.value.find(',', i2 + 1);

                        // we read this as a 64 bit value but only store 32, this was to workaround some weird errors in gas files
                        const uint32_t node1 = std::stoul(entry.value.substr(0, i1), nullptr, 16);
                        const uint32_t door1 = std::stoi(entry.value.substr(i1 + 1, i2 - i1 - 1));
                        const uint32_t node2 = std::stoul(entry.value.substr(i2 + 1, i3 - i2 - 1), nullptr, 16);
                        const uint32_t door2 = std::stoi(entry.value.substr(i3 + 1, std::string::npos));

                        if (const auto itr = node2region.find(node2); itr != node2region.end())
                        {
                            const uint32_t region1 = regionGuid;
                            const uint32_t region2 = itr->second;

                            data[region1].emplace(region2, Data{ node1, node2, door1, door2 });
                        }
                        else
                        {
                            log->critical("invalid stitch map");
                        }
                    }
                }
            }
        }
        else
        {
            log->error("filesystem unable to open {}", path);
        }
    }

    const std::unordered_map<uint32_t, StitchIndex::Data>& StitchIndex::eachStitch(uint32_t regionGuid) const
    {
        static std::unordered_map<uint32_t, Data> empty;

        const auto itr = data.find(regionGuid);
        return itr != data.end() ? itr->second : empty;
    }

    void WorldMapDataCache::WorldMapData::init(IFileSys& fileSys, const std::string& path)
    {
        auto log = spdlog::get("log");

        stitchIndex.init(fileSys, path + "/index/stitch_index.gas");

        if (auto doc = fileSys.openGasFile(path + "/main.gas"))
        {
            log->info("parsing main.gas for {}", path + "/main.gas");

            name = path.substr(path.find_last_of("/") + 1, path.size());
            description = doc->valueOf("map:description");
            screen_name = doc->valueOf("map:screen_name");
        }

        // handle each region
        for (auto regionfolder : fileSys.getDirectoryContents(path + "/regions"))
        {
            uint32_t regionGuid = 0;

            if (auto doc = fileSys.openGasFile(regionfolder + "/main.gas"))
            {
                if (const auto root = doc->child("region"))
                {
                    regionGuid = root->valueAsUInt("guid");

                    auto regionName = regionfolder.substr(regionfolder.find_last_of("/") + 1, regionfolder.size());

                    // add a ".region" extension so the loaders can make sense of whats coming to them
                    nameMap.emplace(regionName, std::make_pair(regionfolder + ".region", regionGuid));

                    log->info("added {} - {} to the name map with path {}", regionName, regionGuid, regionfolder + ".region");
                }
            }

            if (auto stream = fileSys.createInputStream(regionfolder + "/terrain_nodes/nodes.gas"))
            {
                std::string line;
                while (std::getline(*stream, line))
                {
                    if (line.compare(1, 11, "[t:snode,n:") == 0)
                    {
                        const uint32_t nodeGuid = std::stoul(line.substr(12, 10), nullptr, 16);

                        nodeMap.emplace(nodeGuid, regionGuid);
                    }
                }
            }
        }
    }

    const std::pair<std::string, uint32_t>& WorldMapDataCache::WorldMapData::regionNameAndPathFromRegionGuid(uint32_t guid) const
    {
        static std::pair<std::string, uint32_t> empty;

        for (const auto& [name, region] : nameMap)
        {
            if (guid == region.second) return region;
        }

        spdlog::get("log")->error("unable to find a region name for guid {}", guid);

        return empty;
    }

    uint32_t WorldMapDataCache::WorldMapData::regionGuidFromName(const std::string& regionName) const
    {
        const auto itr = nameMap.find(regionName);
        return itr != nameMap.end() ? itr->second.second : 0;
    }

    uint32_t WorldMapDataCache::WorldMapData::regionForNode(uint32_t nodeGuid) const
    {
        const auto itr = nodeMap.find(nodeGuid);
        return itr != nodeMap.end() ? itr->second : 0;
    }

    void WorldMapDataCache::init(IFileSys& fileSys)
    {
        auto log = spdlog::get("log");

        log->info("caching world maps...");

        for (const auto& path : fileSys.getDirectoryContents("/world/maps"))
        {
            WorldMapData worldMapData;
            worldMapData.init(fileSys, path);

            data.emplace(worldMapData.name, std::move(worldMapData));
        }
    }
} // namespace ehb