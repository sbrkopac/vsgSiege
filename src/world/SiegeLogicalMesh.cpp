
#include "SiegeLogicalMesh.hpp"

#include "SiegeMesh.hpp" // required for SiegeMeshHeader
#include "SiegeLogicalNode.hpp"
#include "io/BinaryReader.hpp"

#include <cassert>

namespace ehb
{
	// Construction
	SiegeLogicalMesh::SiegeLogicalMesh()
		: m_id(0)
		, m_pBSPTree(NULL)
		, m_flags(0)
		, m_numLeafConnections(0)
		, m_pLeafConnectionInfo(NULL)
	{
	}

	// Destruction
	SiegeLogicalMesh::~SiegeLogicalMesh()
	{
		delete		m_pBSPTree;

		for (uint32_t i = 0; i < m_numLeafConnections; ++i)
		{
			delete[] m_pLeafConnectionInfo[i].pTriangles;
			delete[] m_pLeafConnectionInfo[i].localConnections;
		}
		delete[]	m_pLeafConnectionInfo;

		for (uint32_t i = 0; i < m_numNodalConnections; ++i)
		{
			delete[] m_pNodalConnectionInfo[i].m_pNodalLeafConnections;
		}
		delete[]	m_pNodalConnectionInfo;
	}

	// Trace a ray into this logical node
	bool SiegeLogicalMesh::HitTest(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
		float& ray_t, vsg::vec3& facenormal)
	{
		if (m_pBSPTree)
		{
			if (ray_dir.x == 0 && ray_dir.z == 0)
			{
				return m_pBSPTree->YRayIntersectTree(ray_orig, ray_dir.y, ray_t, facenormal);
			}
			else
			{
				return m_pBSPTree->RayIntersectTree(ray_orig, ray_dir, ray_t, facenormal);
			}
		}

		return false;
	}

	bool SiegeLogicalMesh::HitTestTri(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
		float& ray_t, TriNorm& triangle)
	{
		if (m_pBSPTree)
		{
			return m_pBSPTree->RayIntersectTreeTri(ray_orig, ray_dir, ray_t, triangle);
		}

		return false;
	}

	// Load from a file
	bool SiegeLogicalMesh::Load(BinaryReader& reader, const SiegeMeshHeader& header)
	{
		// Get the identifier
		m_id = reader.read<uint8_t>();

		// Get the bounding information
		m_minBox = reader.read<vsg::vec3>();
		m_maxBox = reader.read<vsg::vec3>();

		// Get the flags that describe this logical node
		m_flags = reader.read<uint32_t>();

		// Get the leaf connection info
		uint32_t num_lconnections = reader.read<uint32_t>();

		// Create the leaf connection array
		m_numLeafConnections = num_lconnections;
		m_pLeafConnectionInfo = new LMESHLEAFINFO[num_lconnections];

		for (uint32_t lc = 0; lc < num_lconnections; ++lc)
		{
			// Get the leaf id
			uint16_t newid = reader.read<uint16_t>();

			// Get the correct pointer
			assert(newid < num_lconnections);
			LMESHLEAFINFO* pLeaf = &m_pLeafConnectionInfo[newid];
			pLeaf->id = newid;

			// Get the leaf bounding information
			reader.readBytes(&pLeaf->minBox, sizeof(vsg::vec3));
			reader.readBytes(&pLeaf->maxBox, sizeof(vsg::vec3));

			if (header.m_majorVersion > 6 ||
				(header.m_majorVersion == 6 && header.m_minorVersion >= 4))
			{
				// Read center out directly
				reader.readBytes(&pLeaf->center, sizeof(vsg::vec3));
			}
			else
			{
				// Generate the center
				pLeaf->center = pLeaf->minBox + ((pLeaf->maxBox - pLeaf->minBox) * 0.5f);
			}

			// Get the triangle index
			if (header.m_majorVersion > 6 ||
				(header.m_majorVersion == 6 && header.m_minorVersion >= 2))
			{
				pLeaf->numTriangles = reader.read<uint16_t>();
				assert(pLeaf->numTriangles);

				pLeaf->pTriangles = new uint16_t[pLeaf->numTriangles];
				reader.readBytes(pLeaf->pTriangles, sizeof(uint16_t) * pLeaf->numTriangles);
			}
			else
			{
				pLeaf->numTriangles = 1;
				pLeaf->pTriangles = new uint16_t;

				*(pLeaf->pTriangles) = reader.read<uint16_t>();
			}

			// Get the local connections
			uint32_t num_localconnections = reader.read<uint32_t>();

			// Create the local connection array
			pLeaf->numLocalConnections = num_localconnections;
			if (num_localconnections)
			{
				pLeaf->localConnections = new LMESHLEAFINFO * [num_localconnections];

				for (uint32_t localc = 0; localc < num_localconnections; ++localc)
				{
					auto value = reader.read<uint16_t>();
					pLeaf->localConnections[localc] = &m_pLeafConnectionInfo[value];
				}
			}
			else
			{
				pLeaf->localConnections = NULL;
			}
		}

		m_numNodalConnections = reader.read<uint32_t>();

		m_pNodalConnectionInfo = new LCCOLLECTION[m_numNodalConnections];

		for (uint32_t nc = 0; nc < m_numNodalConnections; ++nc)
		{
			LCCOLLECTION* pCollection = &m_pNodalConnectionInfo[nc];

			// Unique identifier of node (corresponds to logical mesh id)
			pCollection->m_farid = reader.read<uint8_t>();

			// Connections
			pCollection->m_numNodalLeafConnections = reader.read<uint32_t>();

			pCollection->m_pNodalLeafConnections = new NODALLEAFCONNECT[pCollection->m_numNodalLeafConnections];
			for (uint32_t nlc = 0; nlc < pCollection->m_numNodalLeafConnections; ++nlc)
			{
				auto data = reader.read<NODALLEAFCONNECT>();
				pCollection->m_pNodalLeafConnections[nlc] = data;
			}
		}

		// Read in the number of triangles
		uint32_t num_triangles = reader.read<uint32_t>();

		// Create a new empty BSPTree with just our triangle information
		
		TriNorm* data = new TriNorm[num_triangles];
		reader.readBytes(data, num_triangles * sizeof(TriNorm));

		m_pBSPTree = new BSPTree(data, (uint16_t)num_triangles, 0, 0, false);

		delete[] data;

		// Read in the tree
		ReadBSPNodeFromFile(reader, m_pBSPTree->GetRoot());

		return true;
	}

	// Recursive function for reading BSP information from a file
	void SiegeLogicalMesh::ReadBSPNodeFromFile(BinaryReader& reader, BSPNode* pNode)
	{
		// Write this node's bounding information to a file
		reader.readBytes(&pNode->m_MinBound, sizeof(vsg::vec3));
		reader.readBytes(&pNode->m_MaxBound, sizeof(vsg::vec3));

		// Write whether this is a leaf or not
		pNode->m_IsLeaf = reader.read<bool>();

		// Write triangle index information
		pNode->m_NumTriangles = reader.read<uint16_t>();

		// Create the triangle array
		pNode->m_Triangles = new uint16_t[pNode->m_NumTriangles];
		reader.readBytes(pNode->m_Triangles, (pNode->m_NumTriangles * sizeof(uint16_t)));

		// Write information describing the child structure of this node
		uint8_t children = reader.read<uint8_t>();

		//Write left child
		if (children)
		{
			pNode->m_LeftChild = new BSPNode;
			memset(pNode->m_LeftChild, 0, sizeof(BSPNode));
			pNode->m_RightChild = new BSPNode;
			memset(pNode->m_RightChild, 0, sizeof(BSPNode));

			ReadBSPNodeFromFile(reader, pNode->m_LeftChild);
			ReadBSPNodeFromFile(reader, pNode->m_RightChild);
		}
	}
}