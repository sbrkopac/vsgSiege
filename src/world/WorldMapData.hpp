
#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

#include <spdlog/spdlog.h>

namespace ehb
{
    class IFileSys;
    struct StitchIndex
    {
        struct Data
        {
            uint32_t node1, node2;
            uint32_t door1, door2;
        };

        void init(IFileSys& fileSys, const std::string& path);

        const std::unordered_map<uint32_t, Data>& eachStitch(uint32_t regionGuid) const;

        // TODO: consider using a multi-map
        // store stitch data for each region
        // key: the owning region id, pair: the stitched incoming region with the proper entries
        std::unordered_map<uint32_t, std::unordered_map<uint32_t, Data>> data;
    };

    struct WorldMapDataCache
    {
        struct WorldMapData
        {
            void init(IFileSys& fileSys, const std::string& path);

            const std::pair<std::string, uint32_t>& regionNameAndPathFromRegionGuid(uint32_t guid) const;

            uint32_t regionGuidFromName(const std::string& regionName) const;

            uint32_t regionForNode(uint32_t nodeGuid) const;

            std::string name, description, screen_name;

            //! holds all the data of how we stitch this map together
            //! this is really important in vanilla DS as if you don't have a stitch you can't path to the next region
            StitchIndex stitchIndex;

            //! key: human readable name of the region, value: a pair that contains the main.gas path and the region guid
            std::unordered_map<std::string, std::pair<std::string, uint32_t>> nameMap;

            //! key: a node guid, value: the region the guid belongs to
            std::unordered_map<uint32_t, uint32_t> nodeMap;
        };

        void init(IFileSys& fileSys);

        std::unordered_map<std::string, WorldMapData> data;
    };
} // namespace ehb