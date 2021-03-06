
#pragma once

#include <string>
#include <map>

#include <vsg/core/Object.h>
#include <vsg/core/Inherit.h>

namespace ehb
{
	// hacked up solution to pass to vsg::Options
	class MeshDatabase : public vsg::Inherit<vsg::Object, MeshDatabase>
	{
	public:
		using MeshFileMap = std::map<uint32_t, std::string>;

		// Construction
		~MeshDatabase() = default;

		// Insert a new entry into the mapping
		void InsertMeshMapping(uint32_t meshGUID, const std::string& fileName);

		// Remove an entry from the mapping
		void RemoveMeshMapping(uint32_t meshGUID);

		// Find the name of a mesh file
		const char* FindFileName(uint32_t meshGUID) const;

		// Get the mesh file mapping
		MeshFileMap& FileNameMap() { return m_MeshFileMap; }

	private:

		// Mesh file index
		MeshFileMap				m_MeshFileMap;
	};

	// Insert a new entry into the mapping
	inline void MeshDatabase::InsertMeshMapping(uint32_t meshGUID, const std::string& fileName)
	{
		m_MeshFileMap.insert(std::pair<uint32_t, std::string>(meshGUID, fileName));
	}

	// Remove an entry from the mapping
	inline void MeshDatabase::RemoveMeshMapping(uint32_t meshGUID)
	{
		m_MeshFileMap.erase(meshGUID);
	}

	// Find the name of a mesh file
	inline const char* MeshDatabase::FindFileName(uint32_t meshGUID) const
	{
		// Find the mesh in the mapping
		if (auto i = m_MeshFileMap.find(meshGUID); i != m_MeshFileMap.end())
		{
			return (*i).second.c_str();
		}

		return nullptr;
	}
}