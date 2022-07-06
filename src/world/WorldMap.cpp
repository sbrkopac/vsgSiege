
#include "WorldMap.hpp"

#include "io/IFileSys.hpp"

#include <spdlog/spdlog.h>

namespace ehb
{
    bool WorldMap::Init(const char* name, const char* world, bool force, bool forLoadingSavedGame)
    {
        if (world == nullptr)
        {
            world = "";
        }

        std::string mapAddress("world/maps/");
        mapAddress += name;

        // TODO: load gas properties

        m_MapName = name;

        return true;
    }

    bool WorldMap::CheckIndexesLoaded()
    {
        if (m_IndexesLoaded)
        {
            return true;
        }
        m_IndexesLoaded = true;

        bool success = true;
        if (success)
        {
            success = loadDatabases();
        }

        return success;
    }

    std::string WorldMap::MakeMapDirAddress() const
    {
        return ("/world/maps/" + GetMapName());
    }

    std::string WorldMap::MakeNodeAddress(database_guid guid) const
    {
        const_cast <WorldMap*>(this)->CheckIndexesLoaded();

        std::string address;

        if (auto found = m_NodeGuidToNodeInfoDb.find(guid); found != m_NodeGuidToNodeInfoDb.end())
        {
            auto regionId = m_RegionIdToNameDb.find(found->second.m_RegionIndex); auto regionName = regionId->second;
            address = MakeMapDirAddress();
            address += "/regions/";
            address += regionName;
            address += "/terrain_nodes/siege_node_list/";
        }

        return address;
    }

    const std::string WorldMap::GetRegionName(RegionId id) const
    {
        const_cast <WorldMap*>(this)->CheckIndexesLoaded();

        RegionIdToNameDb::const_iterator found = m_RegionIdToNameDb.find(id);
        return ((found != m_RegionIdToNameDb.end()) ? found->second : "");
    }

    std::string WorldMap::MakeRegionDirAddress(RegionId id) const
    {
        const std::string& regionName = GetRegionName(id);
        if (!regionName.empty())
        {
            return (MakeMapDirAddress() + "/regions/" + regionName);
        }
        else
        {
            return {};
        }
    }

    bool WorldMap::loadDatabases()
    {
        { // region guid to region name loading
            auto regionFolder = MakeMapDirAddress() + "/regions";
            for (auto region : fileSys.getDirectoryContents(regionFolder))
            {
                if (auto doc = fileSys.openGasFile(region + "/main.gas"))
                {
                    if (auto&& root = doc->child("region"))
                    {
                        auto regionGuid = root->valueAsUInt("guid");
                        auto regionName = region.substr(region.find_last_of("/") + 1, region.size());

                        auto rc = m_RegionIdToNameDb.insert(std::make_pair(regionGuid, regionName));

                        if (!rc.second)
                        {
                            spdlog::get("log")->error("Redefined region id {:08x}", regionGuid);
                        }
                    }
                }
            }
        }

        {
            for (auto i = m_RegionIdToNameDb.begin(); i != m_RegionIdToNameDb.end(); ++i)
            {
                if (m_ClampingRegionGUID == i->first)
                {
                    loadNodeInfoDb(i->first);
                }
            }
        }

        return true;
    }

    bool WorldMap::loadNodeInfoDb(RegionId regionId)
    {
        auto nodeIndex = MakeRegionDirAddress(regionId) + "/index/streamer_node_index.gas";

        if (auto doc = fileSys.openGasFile(nodeIndex))
        {
            auto streamer_index = doc->child("streamer_node_index");
            for (auto&& index : streamer_index->eachAttribute())
            {
                auto nodeGuid = std::strtoul(index.value.c_str(), nullptr, 0);
                
                auto pair = std::make_pair(database_guid(nodeGuid), NodeInfo());
                auto rc = m_NodeGuidToNodeInfoDb.emplace(pair);
                if (!rc.second)
                {
                    spdlog::get("log")->error("nodeGuid {:08x} already exists for this region", nodeGuid);
                }
            }
        }

        return true;
    }
}