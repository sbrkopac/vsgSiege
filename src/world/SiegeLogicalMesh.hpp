
#pragma once

#include <cstdint>

#include <vsg/maths/vec3.h>

namespace ehb
{
	class BSPTree;
	struct BSPNode;
	struct TriNorm;
}

namespace ehb
{
	// Forward dec
	struct LMESHLEAFINFO;
	struct LCCOLLECTION;
	class SiegeLogicalNode;
	class BinaryReader;

	// Leaf search structure, used for pathfinding
	struct LMESHSEARCHLEAF
	{
		// Identifier of this leaf
		SiegeLogicalNode* pLogicalNode;
		LMESHLEAFINFO* pLeafInfo;

		// Search information
		float						costToStart;
		float						costToFinish;
		float						combinedCost;

		uint32_t						parent;
		bool						bInOpen;
	};

	// Leaf information
	struct LMESHLEAFINFO
	{
		// Identifier
		uint16_t				id;

		// Bounding information
		vsg::vec3					minBox;
		vsg::vec3					maxBox;
		vsg::vec3					center;

		// Triangle information (used to calc connections)
		uint16_t				numTriangles;
		uint16_t* pTriangles;

		// Connection information for local leaves
		uint32_t				numLocalConnections;
		LMESHLEAFINFO** localConnections;
	};

	struct SiegeMeshHeader;
	class SiegeLogicalMesh
	{
		friend class SiegeMesh;

	public:

		// Destruction
		~SiegeLogicalMesh();

		// Trace a ray into this logical mesh
		bool HitTest(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
			float& ray_t, vsg::vec3& facenormal);

		bool HitTestTri(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
			float& ray_t, TriNorm& triangle);

		// Get ID
		uint8_t	GetID() { return m_id; }

		// Get flags
		uint32_t	GetFlags() { return m_flags; }

		// Get the BSPTree for this logical mesh
		BSPTree* GetBSPTree() { return m_pBSPTree; }

		// Get bounding information
		const vsg::vec3& GetMinimumBounds() { return m_minBox; }
		const vsg::vec3& GetMaximumBounds() { return m_maxBox; }
		const vsg::vec3	GetHalfDiag() { return (m_maxBox - m_minBox) * 0.5f; }
		const vsg::vec3	GetCenter() { return m_minBox + ((m_maxBox - m_minBox) * 0.5f); }

		// Get leaf connection info
		uint32_t	GetNumLeafConnections() { return m_numLeafConnections; }
		LMESHLEAFINFO* GetLeafConnectionInfo() { return m_pLeafConnectionInfo; }

		// Get node connection info
		uint32_t	GetNumNodalConnections() { return m_numNodalConnections; }
		LCCOLLECTION* GetNodalConnectionInfo() { return m_pNodalConnectionInfo; }

	private:

		// Construction
		SiegeLogicalMesh();

		// Load from file
		bool Load(BinaryReader& reader, const SiegeMeshHeader& header);

		// Recursive function for reading BSP information from a file
		void ReadBSPNodeFromFile(BinaryReader& reader, BSPNode* pNode);


		// Unique identifier
		uint8_t				m_id;

		// Bounding box of this logical node
		vsg::vec3					m_minBox;
		vsg::vec3					m_maxBox;

		// BSP'd geometrical information
		BSPTree* m_pBSPTree;

		// Bit flags that define node attributes
		uint32_t				m_flags;

		// Leaf connections
		uint32_t				m_numLeafConnections;
		LMESHLEAFINFO* m_pLeafConnectionInfo;

		// Nodal connections
		uint32_t				m_numNodalConnections;
		LCCOLLECTION* m_pNodalConnectionInfo;
	};
}
