
#pragma once

#include <cstdint>
#include <vector>
#include <map>

#include "SiegePos.hpp"

namespace ehb
{
	//struct RegionId_;
	//typedef const RegionId_* RegionId;
	typedef uint32_t RegionId;
}

namespace ehb
{
	class IFileSys;
    class WorldMap
    {
    public:

		using database_guid = uint32_t;

		struct StartingPos
		{
			SiegePos       spos;
			// CameraPosition cpos;
		};

		using SPosMap = std::map<uint32_t, StartingPos>;

		struct WorldStartLevel
		{
			std::string sWorld;
			float	 requiredLevel;
		};

		using WorldStartLevels = std::vector<WorldStartLevel>;

		struct StartingGroup
		{
			int		  id;
			SPosMap   positions;
			bool      bDefault;
			std::string  sGroup;
			std::wstring sDescription;		// pretranslated
			std::wstring sScreenName;		// pretranslated

			WorldStartLevels startLevels;

			StartingGroup()
			{
				id = 0;
				bDefault = false;
			}
		};

		using SGroupColl = std::vector <StartingGroup>;

		struct WorldLocation
		{
			std::string  sName;
			std::wstring sScreenName;		// pretranslated
		};

		using WorldLocationMap = std::map <uint32_t, WorldLocation>;

		WorldMap(IFileSys& fileSys) : fileSys(fileSys)
		{
			m_ClampingRegionGUID = 0;
			m_IndexesLoaded = false;
		}
		~WorldMap() = default;

		bool Init (const char* name, const char* world = nullptr, bool force = false, bool forLoadingSavedGame = false);
		bool Shutdown (bool fullShutdown = true);
		bool CheckIndexesLoaded();

		const std::string& GetMapName() const { return (m_MapName); }

		std::string      MakeMapDirAddress() const;
		std::string      MakeMapDirFuel() const;
		std::string      MakeMapDefAddress() const;
		std::string      MakeMapDefFuel() const;

		const std::string GetRegionName(RegionId id) const;
		std::string		MakeRegionDirAddress(RegionId id) const;

		std::string		MakeNodeAddress(database_guid guid) const;

		void RestrictStreamingToRegion(RegionId id) { m_ClampingRegionGUID = id; }

	private:

		bool loadDatabases();

		bool loadNodeInfoDb(RegionId regionId);
		
		IFileSys& fileSys;

		std::string m_MapName;

		using RegionIdToNameDb = std::map <RegionId, std::string>;

		// holds info about a node in the map
		struct NodeInfo
		{
			uint32_t m_ScidIndex;								// starting index into m_ScidColl for
			uint16_t  m_ScidCount;								// number of scids in this node
			uint16_t  m_RegionIndex;							// index into m_RegionIdToNameDb for this node

			NodeInfo ()
			{
				m_ScidIndex = 0xFFFFFFFF;
				m_ScidCount = 0;
				m_RegionIndex = 0xFFFF;
			}
		};
		static_assert(sizeof(NodeInfo) == 8);			// guarantee i'm getting the size i want

		using NodeGuidToNodeInfoDb = std::map <database_guid, NodeInfo>;

		RegionIdToNameDb       m_RegionIdToNameDb;			// map of region id's to their internal region names
		NodeGuidToNodeInfoDb   m_NodeGuidToNodeInfoDb;		// map of node guid's to node infoz

		bool m_IndexesLoaded;
		RegionId m_ClampingRegionGUID; // clamp streaming to this region (set to invalid region id when normal gameplay)
    };
}