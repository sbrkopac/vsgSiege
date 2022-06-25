
#pragma once

#include <vector>

#include <vsg/maths/vec3.h>

namespace ehb
{
	// Enumerated type defines axis of division
	enum DIVAXIS
	{
		DIV_XAXIS_HALF = 0,	// Divide along the X-axis half
		DIV_XAXIS_QUARTER_BOTTOM = 1,	// Divide along the X-axis quarter bottom
		DIV_XAXIS_QUARTER_TOP = 2,	// Divide along the X-axis quarter top

		DIV_YAXIS_HALF = 3,	// Divide along the Y-axis half
		DIV_YAXIS_QUARTER_BOTTOM = 4,	// Divide along the Y-axis quarter bottom
		DIV_YAXIS_QUARTER_TOP = 5,	// Divide along the Y-axis quarter top

		DIV_ZAXIS_HALF = 6,	// Divide along the Z-axis half
		DIV_ZAXIS_QUARTER_BOTTOM = 7,	// Divide along the Z-axis quarter bottom
		DIV_ZAXIS_QUARTER_TOP = 8,	// Divide along the Z-axis quarter top

		DIV_ALIGN = 0x7FFFFFFF,	// DWORD align this enumerated type for speed
	};

	// Definition of a triangle/normal pair
	struct TriNorm
	{
		vsg::vec3		m_Vertices[3];		// List of triangle vertices
		vsg::vec3		m_Normal;			// Face normal of this triangle
	};

	// Definition of a BSP node
	struct BSPNode
	{
		vsg::vec3		m_MinBound;			// Minimum bound of node
		vsg::vec3		m_MaxBound;			// Maximum bound of node

		bool			m_IsLeaf;			// Is this node a leaf node?
		unsigned short* m_Triangles;		// List of triangles in this node (index into main list)
		unsigned short	m_NumTriangles;		// Number of triangles in this node

		BSPNode* m_LeftChild;		// Pointer to left child node
		BSPNode* m_RightChild;		// Pointer to right child node
	};

	// Definition of a ray trace pair
	struct IntersectTestNode
	{
		vsg::vec3		origin;				// Ray origin for this iteration of tracing
		BSPNode* node;				// Node to trace into
	};


	// Class definition of a BSP tree
	class BSPTree
	{
	public:

		// Typedefs
		typedef std::vector< IntersectTestNode >	IntersectStack;
		typedef std::vector< BSPNode* >			NodeStack;
		typedef std::vector< unsigned short >		TriangleIndexColl;

		// The constructor handles the generation of the tree
		BSPTree(const vsg::vec3* vertices, const unsigned int numvertices,
			const unsigned short* indices, const unsigned short numtriangles,
			const unsigned int maxprimitives = 8, const unsigned int maxdepth = 16);
		BSPTree(const TriNorm* triNorms, const unsigned short numtrinorms,
			const unsigned int maxprimitives = 8, const unsigned int maxdepth = 16,
			bool bGenerateTree = true);

		~BSPTree();

		// Trace a ray into the tree
		bool		RayIntersectTree(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
			float& ray_t, vsg::vec3& facenormal);

		bool		RayIntersectTreeTri(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
			float& ray_t, TriNorm& triangle);

		bool		YRayIntersectTree(const vsg::vec3& ray_orig, const float ray_dir,
			float& ray_t, vsg::vec3& facenormal);

		// Find the triangles enclosed by given axis-aligned volume
		void		BoxIntersectTree(const vsg::vec3& minBox, const vsg::vec3& maxBox,
			TriangleIndexColl& triangleIndices);

		// Draw the tree
		// void		DrawTree(Rapi& renderer); // TOOD: replace with vsg visitor?

		// See if an error has occured
		bool		HasError() { return m_Error; }

		// Get the root of the tree /* BE CAREFUL WITH IT! */
		BSPNode* GetRoot() { return m_BSPRoot; }

		// Get the triangles that compose this tree
		TriNorm* GetTriangles() { return m_Triangles; }
		int			GetNumTriangles() { return m_numTriangles; }

	private:

		// Recursive tree generation
		void		GenerateTreeNode(BSPNode* current_node, unsigned int current_depth,
			DIVAXIS current_div);

		// Get the minimum and maximum bounds of a set of vertices
		void		GetBounds(const unsigned short* indices, const unsigned short numtriangles,
			vsg::vec3& minbound, vsg::vec3& maxbound);

		// Decide what axis this box should be split down next
		DIVAXIS		GetAxisOfDivision(const BSPNode* node);

		// Test an axis for possible splitting
		int			TestAxisOfDivision(const BSPNode* node, const DIVAXIS div);

		// Initialize the triangle list from the raw data
		void		InitTriNorm(const vsg::vec3* vertices, const unsigned int numvertices,
			const unsigned short* indices, const unsigned short numtriangles);

		// Setup the boxes that define the children of a node
		void		SetupChildBoxes(const BSPNode* current_node, const DIVAXIS div, vsg::vec3& maxBound, vsg::vec3& minBound);

		// Determine if any part of a triangle intersects a bounding volume
		bool		TriIntersectsBox(const vsg::vec3& minBound, const vsg::vec3& maxBound, const unsigned short tri);

		// Destroy a node and all its children
		void		DestroyNode(BSPNode* node);

		// Draw a node and all its children.  Recursively only draws leaf nodes.
		// void		DrawNode(Rapi& renderer, const BSPNode* node); // TODO: replace with vsg::Visitor


		// The actual tree information
		BSPNode* m_BSPRoot;

	public: // SAM: temp right now so I can access triangles in a test state to generate a drawable mesh in vulkan

		// The geometric information
		TriNorm* m_Triangles;
		int				m_numTriangles;

	private:

		// Balance/complexity information for this tree
		int				m_MaxPrimitives;
		int				m_MaxDepth;

		// Stack cache for speed
		NodeStack		m_nodeStack;
		IntersectStack	m_intersectStack;

		// Signifies whether or not an error has occured in the BSP
		bool			m_Error;

	};
}
