
#pragma once

#include <cstdint>
#include <unordered_map>

#include <vsg/maths/vec3.h>

#include "DatabaseGuid.hpp"
#include "SiegeBSP.hpp"

namespace siege
{
    enum eLogicalNodeFlags;
    struct TriNorm;
} // namespace siege

namespace ehb
{
    class SiegeNode;
    class SiegeLogicalMesh;
    class SiegeLogicalNode;
    struct LMESHLEAFINFO;

    using LMESHSEARCHLEAFMAP = std::unordered_map<LMESHLEAFINFO*, uint32_t>;

    // Logical node attributes
    enum eLogicalNodeFlags
    {
        LF_NONE = 0,
        //		LF_LEGACY_HUMAN_OR_COMPUTER = 1<<0,  $$$ leave bit 0 unassigned

        // Player types
        LF_HUMAN_PLAYER = 1 << 1,
        LF_COMPUTER_PLAYER = 1 << 2,
        LF_PLAYER_MASK = LF_HUMAN_PLAYER + LF_COMPUTER_PLAYER,

        // Material types
        LF_DIRT = 1 << 3,
        LF_SHALLOW_WATER = 1 << 4,
        LF_DEEP_WATER = 1 << 5,
        LF_ICE = 1 << 6,
        LF_LAVA = 1 << 7,
        LF_SIZE1_MOVER = 1 << 10,
        LF_SIZE2_MOVER = 1 << 11,
        LF_SIZE3_MOVER = 1 << 12,
        LF_SIZE4_MOVER = 1 << 13,
        LF_MIST = 1 << 14,
        LF_MATERIAL_MASK = LF_DIRT + LF_SHALLOW_WATER + LF_DEEP_WATER + LF_ICE + LF_LAVA +
                           LF_SIZE1_MOVER + LF_SIZE2_MOVER + LF_SIZE3_MOVER + LF_SIZE4_MOVER + LF_MIST,
        // Monster types
        LF_HOVER = 1 << 8,
        LF_BOSS = 1 << 9,
        LF_TYPE_MASK = LF_HOVER + LF_BOSS,

        LF_ALL = 0xFFFFFFFF,

        // Special tag
        LF_CLEAR = 1 << 28,

        // Ray cast flags
        LF_IS_WALL = 1 << 29,
        LF_IS_FLOOR = 1 << 30,
        LF_IS_WATER = 1 << 31,
        LF_IS_ANY = 0xFFFFFFFF,

        LF_uint32_t_ALIGN = 0x7FFFFFFF,
    };

    // Node search structure, used for pathfinding
    struct LNODESEARCH
    {
        // Owner logical node
        SiegeLogicalNode* m_pLogicalNode;

        // Cost
        float m_CostToStart;
        float m_CostToFinish;
        float m_CombinedCost;

        // Parent
        uint32_t m_Parent;

        // State info
        bool m_bInOpen;
    };

    struct NODALLEAFCONNECT
    {
        // Local leaf identifier
        uint16_t local_id;

        // Far leaf identifier
        uint16_t far_id;
    };

    struct LCCOLLECTION
    {
        // Unique identifier of node (corresponds to logical mesh id)
        unsigned char m_farid;

        // List of leaf connections
        unsigned int m_numNodalLeafConnections;
        NODALLEAFCONNECT* m_pNodalLeafConnections;
    };

    struct LNODECONNECT
    {
        // Siege node that owns this object
        DatabaseGuid m_farSiegeNode;

        // Collection of information describing this connection
        LCCOLLECTION* m_pCollection;
    };

    class SiegeLogicalNode
    {
        friend class SiegePathfinder;

    public:
        // Construction and destruction
        SiegeLogicalNode();
        ~SiegeLogicalNode();

        // Load logical information from LNO file
        bool Load(SiegeNode* pSiegeNode, const char*& pData);

        // Trace a ray into this logical node
        // The ray needs to be in the node space coordinates of it's owner siege node.
        bool HitTest(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
                     float& ray_t, vsg::vec3& facenormal);

        bool HitTestTri(const vsg::vec3& ray_orig, const vsg::vec3& ray_dir,
                        float& ray_t, TriNorm& triangle);

        // Get identifier information
        unsigned char GetID() const { return m_id; }
        void SetID(unsigned char id) { m_id = id; }

        // Get the SiegeNode that owns me
        SiegeNode* GetSiegeNode() const { return m_pSiegeNode; }
        void SetSiegeNode(SiegeNode* pSiegeNode) { m_pSiegeNode = pSiegeNode; }

        // Get logical mesh information
        SiegeLogicalMesh* GetLogicalMesh() const { return m_pLogicalMesh; }
        void SetLogicalMesh(SiegeLogicalMesh* plMesh);

        // Get node connection information
        uint16_t GetNumNodeConnections() const { return m_numNodeConnections; }
        LNODECONNECT* GetNodeConnectionInfo() const { return m_pNodeConnectionInfo; }

        // Set node connection information
        void SetNumNodeConnections(uint16_t num) { m_numNodeConnections = num; }
        void SetNodeConnectionInfo(LNODECONNECT* pNC) { m_pNodeConnectionInfo = pNC; }

        // Get the flags that describe the properties of this logical node
        unsigned int GetFlags() const { return m_flags; }
        void SetFlags(unsigned int flags) { m_flags = flags; }

        // Test the given flags against a set of rules used to determine permissions
        bool AreFlagsAllowable(unsigned int flags);

        // Pathfinding status indicator
        bool GetActivePath() const { return m_bActivePath; }
        void SetActivePath(bool bActive) { m_bActivePath = bActive; }

        // Get the current path index
        uint32_t GetPathSearchIndex() const { return m_pathSearchIndex; }
        void SetPathSearchIndex(uint32_t searchIndex) { m_pathSearchIndex = searchIndex; }

        // Blocked leaf information
        void MarkLeafAsBlocked(uint16_t id, bool bBlocked);
        bool IsLeafBlocked(uint16_t id);

        // Is this node connected to me?
        bool IsConnectedTo(const SiegeLogicalNode* pNode);

    private:
        using BlockedLeafMap = std::unordered_map<uint16_t, uint32_t>;

        // Pointer to my corresponding SiegeLogicalMesh
        SiegeLogicalMesh* m_pLogicalMesh;

        // Identifier of this logical node (corresponds to logical mesh id)
        unsigned char m_id;

        // SiegeNode that owns me
        SiegeNode* m_pSiegeNode;

        // Bit flags that define node attributes
        unsigned int m_flags;

        // Logical node connections
        uint16_t m_numNodeConnections;
        LNODECONNECT* m_pNodeConnectionInfo;

        // Flag for use with the pathfinder, indicates status
        bool m_bActivePath;

        // Index for pathing info
        uint32_t m_pathSearchIndex;

        // Set of leaves that are currently blocked by something
        BlockedLeafMap m_BlockedLeafMap;

        // Search leaf information
        LMESHSEARCHLEAFMAP m_searchLeafMap;
    };
} // namespace ehb
