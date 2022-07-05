
#include "SiegeBSP.hpp"

#include <algorithm>
#include <cassert>
#include <cmath> // std::abs

namespace ehb
{
    // Construct a BSPTree object and generate the tree itself from the lists
    // of data.
    BSPTree::BSPTree(const vsg::vec3* vertices, const uint32_t numvertices,
                     const uint16_t* indices, const uint16_t numtriangles,
                     const uint32_t maxprimitives, const uint32_t maxdepth) :
        m_Error(false), m_MaxPrimitives(maxprimitives), m_MaxDepth(maxdepth), m_numTriangles(0)
    {
        // Setup the TriNorm list
        InitTriNorm(vertices, numvertices, indices, numtriangles);

        // Initialize the tree by creating the root node
        m_BSPRoot = new BSPNode;
        m_BSPRoot->m_IsLeaf = false;
        m_BSPRoot->m_LeftChild = NULL;
        m_BSPRoot->m_RightChild = NULL;

        // Initialize the triangles for the entire tree
        m_BSPRoot->m_Triangles = new uint16_t[numtriangles];
        for (uint16_t i = 0; i < numtriangles; ++i)
        {
            m_BSPRoot->m_Triangles[i] = i;
        }
        m_BSPRoot->m_NumTriangles = numtriangles;

        // Initialize the bounding volume of the entire tree
        GetBounds(m_BSPRoot->m_Triangles, m_BSPRoot->m_NumTriangles,
                  m_BSPRoot->m_MinBound, m_BSPRoot->m_MaxBound);

        // Generate the tree
        GenerateTreeNode(m_BSPRoot, 0, GetAxisOfDivision(m_BSPRoot));
    }

    BSPTree::BSPTree(const TriNorm* triNorms, const uint16_t numtrinorms,
                     const uint32_t maxprimitives, const uint32_t maxdepth, bool bGenerateTree) :
        m_Error(false), m_MaxPrimitives(maxprimitives), m_MaxDepth(maxdepth)
    {
        // Create the new TriNorm list
        m_Triangles = new TriNorm[numtrinorms];
        m_numTriangles = numtrinorms;

        // Copy the lists
        memcpy(m_Triangles, triNorms, sizeof(TriNorm) * numtrinorms);

        // Initialize the tree by creating the root node
        m_BSPRoot = new BSPNode;
        memset(m_BSPRoot, 0, sizeof(BSPNode));

        m_BSPRoot->m_IsLeaf = false;
        m_BSPRoot->m_LeftChild = NULL;
        m_BSPRoot->m_RightChild = NULL;

        if (bGenerateTree)
        {
            // Initialize the triangles for the entire tree
            m_BSPRoot->m_Triangles = new uint16_t[numtrinorms];
            for (uint16_t i = 0; i < numtrinorms; ++i)
            {
                m_BSPRoot->m_Triangles[i] = i;
            }
            m_BSPRoot->m_NumTriangles = numtrinorms;

            // Initialize the bounding volume of the entire tree
            GetBounds(m_BSPRoot->m_Triangles, m_BSPRoot->m_NumTriangles,
                      m_BSPRoot->m_MinBound, m_BSPRoot->m_MaxBound);

            // Generate the tree
            GenerateTreeNode(m_BSPRoot, 0, GetAxisOfDivision(m_BSPRoot));
        }
    }

    // Destroy the tree
    BSPTree::~BSPTree()
    {
        // Destroy the TriNorms
        if (m_Triangles)
        {
            delete[] m_Triangles;
        }

        // Destroy the tree
        if (m_BSPRoot)
        {
            DestroyNode(m_BSPRoot);
            delete m_BSPRoot;
        }
    }

    // Trace a ray into the tree
    bool BSPTree::RayIntersectTree(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
                                   float& ray_t, vsg::vec3& facenormal)
    {
        assert(m_intersectStack.empty());

        return false;
    }

    bool BSPTree::RayIntersectTreeTri(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
                                      float& ray_t, TriNorm& triangle)
    {
        assert(m_intersectStack.empty());

        return false;
    }

    bool BSPTree::YRayIntersectTree(const vsg::vec3& ray_orig, const float ray_dir,
                                    float& ray_t, vsg::vec3& facenormal)
    {
        assert(m_intersectStack.empty());

        return false;
    }

    // Find the triangles enclosed by given axis-aligned volume
    void BSPTree::BoxIntersectTree(const vsg::vec3& minBox, const vsg::vec3& maxBox,
                                   TriangleIndexColl& triangleIndices)
    {
        assert(m_nodeStack.empty());
    }

    // Recursively generate the tree
    void BSPTree::GenerateTreeNode(BSPNode* current_node, uint32_t current_depth,
                                   DIVAXIS current_div)
    {
        assert(current_node);
    }

    // Setup the boxes that define the children of a node
    void BSPTree::SetupChildBoxes(const BSPNode* current_node, const DIVAXIS div, vsg::vec3& maxBound, vsg::vec3& minBound)
    {
        float cutting_plane = 0.0f;

        switch (div)
        {
        case DIV_XAXIS_HALF:
            cutting_plane = current_node->m_MinBound.x + ((current_node->m_MaxBound.x - current_node->m_MinBound.x) * 0.5f);
            maxBound.x = cutting_plane;
            minBound.x = cutting_plane;
            break;

        case DIV_XAXIS_QUARTER_BOTTOM:
            cutting_plane = current_node->m_MinBound.x + ((current_node->m_MaxBound.x - current_node->m_MinBound.x) * 0.25f);
            maxBound.x = cutting_plane;
            minBound.x = cutting_plane;
            break;

        case DIV_XAXIS_QUARTER_TOP:
            cutting_plane = current_node->m_MinBound.x + ((current_node->m_MaxBound.x - current_node->m_MinBound.x) * 0.75f);
            maxBound.x = cutting_plane;
            minBound.x = cutting_plane;
            break;

        case DIV_YAXIS_HALF:
            cutting_plane = current_node->m_MinBound.y + ((current_node->m_MaxBound.y - current_node->m_MinBound.y) * 0.5f);
            maxBound.y = cutting_plane;
            minBound.y = cutting_plane;
            break;

        case DIV_YAXIS_QUARTER_BOTTOM:
            cutting_plane = current_node->m_MinBound.y + ((current_node->m_MaxBound.y - current_node->m_MinBound.y) * 0.25f);
            maxBound.y = cutting_plane;
            minBound.y = cutting_plane;
            break;

        case DIV_YAXIS_QUARTER_TOP:
            cutting_plane = current_node->m_MinBound.y + ((current_node->m_MaxBound.y - current_node->m_MinBound.y) * 0.75f);
            maxBound.y = cutting_plane;
            minBound.y = cutting_plane;
            break;

        case DIV_ZAXIS_HALF:
            cutting_plane = current_node->m_MinBound.z + ((current_node->m_MaxBound.z - current_node->m_MinBound.z) * 0.5f);
            maxBound.z = cutting_plane;
            minBound.z = cutting_plane;
            break;

        case DIV_ZAXIS_QUARTER_BOTTOM:
            cutting_plane = current_node->m_MinBound.z + ((current_node->m_MaxBound.z - current_node->m_MinBound.z) * 0.25f);
            maxBound.z = cutting_plane;
            minBound.z = cutting_plane;
            break;

        case DIV_ZAXIS_QUARTER_TOP:
            cutting_plane = current_node->m_MinBound.z + ((current_node->m_MaxBound.z - current_node->m_MinBound.z) * 0.75f);
            maxBound.z = cutting_plane;
            minBound.z = cutting_plane;
            break;
        }
    }

    // Determine if any part of a triangle intersects a bounding volume
    bool BSPTree::TriIntersectsBox(const vsg::vec3& minBound, const vsg::vec3& maxBound, const uint16_t tri)
    {
        return false;
    }

    // Get the bounds of a set of vertices
    void BSPTree::GetBounds(const uint16_t* indices, const uint16_t numtriangles,
                            vsg::vec3& minbound, vsg::vec3& maxbound)
    {
        assert(indices && numtriangles);

        // Initialize the bounds
        minbound = maxbound = m_Triangles[(*indices)].m_Vertices[0];

        // Calculate the bounds from the verts
        for (uint16_t i = 0; i < numtriangles; ++i, ++indices)
        {
            TriNorm& tri = m_Triangles[(*indices)];

            for (uint16_t o = 0; o < 3; ++o)
            {
                vsg::vec3& vert = tri.m_Vertices[o];

                minbound.x = std::min(minbound.x, vert.x);
                minbound.y = std::min(minbound.y, vert.y);
                minbound.z = std::min(minbound.z, vert.z);

                maxbound.x = std::min(maxbound.x, vert.x);
                maxbound.y = std::min(maxbound.y, vert.y);
                maxbound.z = std::min(maxbound.z, vert.z);
            }
        }
    }

    // Decide what axis this box should be split down next
    DIVAXIS BSPTree::GetAxisOfDivision(const BSPNode* node)
    {
        // Prime split axis is the one that gives us the most even seperation of geometry
        // In order to facilitate this, we test all three axes, then choose the one that
        // is closest to our desired result.
        DIVAXIS next_div = DIV_XAXIS_HALF;
        int div_diff = TestAxisOfDivision(node, DIV_XAXIS_HALF);

        int test_diff = TestAxisOfDivision(node, DIV_XAXIS_QUARTER_BOTTOM);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_XAXIS_QUARTER_BOTTOM;
        }

        test_diff = TestAxisOfDivision(node, DIV_XAXIS_QUARTER_TOP);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_XAXIS_QUARTER_TOP;
        }

        test_diff = TestAxisOfDivision(node, DIV_YAXIS_HALF);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_YAXIS_HALF;
        }

        test_diff = TestAxisOfDivision(node, DIV_YAXIS_QUARTER_BOTTOM);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_YAXIS_QUARTER_BOTTOM;
        }

        test_diff = TestAxisOfDivision(node, DIV_YAXIS_QUARTER_TOP);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_YAXIS_QUARTER_TOP;
        }

        test_diff = TestAxisOfDivision(node, DIV_ZAXIS_HALF);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_ZAXIS_HALF;
        }

        test_diff = TestAxisOfDivision(node, DIV_ZAXIS_QUARTER_BOTTOM);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_ZAXIS_QUARTER_BOTTOM;
        }

        test_diff = TestAxisOfDivision(node, DIV_ZAXIS_QUARTER_TOP);
        if (test_diff < div_diff)
        {
            div_diff = test_diff;
            next_div = DIV_ZAXIS_QUARTER_TOP;
        }

        return next_div;
    }

    // Test an axis for possible split
    int BSPTree::TestAxisOfDivision(const BSPNode* node, const DIVAXIS div)
    {
        // Bounds
        vsg::vec3 rminBound = node->m_MinBound;
        vsg::vec3 lmaxBound = node->m_MaxBound;

        // Do axis subdivision
        SetupChildBoxes(node, div, lmaxBound, rminBound);

        // Iterate through triangles and classify them
        std::vector<uint32_t> lefttris;
        std::vector<uint32_t> righttris;
        for (int i = 0; i < node->m_NumTriangles; ++i)
        {
            if (TriIntersectsBox(node->m_MinBound,
                                 lmaxBound,
                                 node->m_Triangles[i]))
            {
                // This triangle is in the left child
                lefttris.push_back(node->m_Triangles[i]);
            }
            else if (TriIntersectsBox(rminBound,
                                      node->m_MaxBound,
                                      node->m_Triangles[i]))
            {
                // This triangle is in the right child
                righttris.push_back(node->m_Triangles[i]);
            }
        }

        int32_t size = static_cast<uint32_t>(lefttris.size()) - static_cast<uint32_t>(righttris.size());
        return (std::abs(size));
    }

    // Init the triangle listing
    void BSPTree::InitTriNorm(const vsg::vec3* vertices, const uint32_t numvertices,
                              const uint16_t* indices, const uint16_t numtriangles)
    {
        assert(vertices && numvertices);
        assert(indices && numtriangles);
    }

    // Destroy a node and all of its children
    void BSPTree::DestroyNode(BSPNode* node)
    {
        assert(node);

        if (node)
        {
            if (node->m_LeftChild)
            {
                DestroyNode(node->m_LeftChild);
                delete node->m_LeftChild;
            }
            if (node->m_RightChild)
            {
                DestroyNode(node->m_RightChild);
                delete node->m_RightChild;
            }

            if (node->m_Triangles)
            {
                delete[] node->m_Triangles;
            }
        }
    }
} // namespace ehb
