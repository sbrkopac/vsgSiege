
#pragma once

#include "DatabaseGuid.hpp"
#include "Math.hpp"

#include <vsg/maths/quat.h>
#include <vsg/maths/vec3.h>

namespace ehb
{
    struct SiegePos
    {
        // Default constructor
        SiegePos() {}

        // Data constructor
        inline SiegePos(const vsg::vec3& ipos, const DatabaseGuid& inode) :
            pos(ipos), node(inode) {}

        static const SiegePos INVALID;

        // Equality
        bool operator==(const SiegePos& p) const { return ((node == p.node) && (pos == p.pos)); }
        bool operator!=(const SiegePos& p) const { return ((node != p.node) || (pos != p.pos)); }

        bool isEqual(const SiegePos& p, float epsilon) const { return ((node == p.node) && math::isEqual(pos, p.pos, epsilon)); }
        bool isExactlyEqual(const SiegePos& p) const { return (!::memcmp(this, (void*)&p, sizeof(SiegePos))); }

        // Positional information
        vsg::vec3 pos;
        DatabaseGuid node;
    };

    struct SiegeRot
    {

        // Default constructor
        SiegeRot() :
            node(UNDEFINED_GUID) {}

        static const SiegeRot INVALID;

        // Data constructor
        inline SiegeRot(const vsg::quat& rot, const DatabaseGuid& node) :
            rot(rot), node(node) {}

        bool operator==(const SiegeRot& r) const
        {
            return ((rot == r.rot) && (node == r.node));
        }

        vsg::quat rot;
        DatabaseGuid node;
    };

    inline SiegePos& _ToSiegePosRef(int32_t siegePosRef) { return (*(SiegePos*)(siegePosRef)); }
} // namespace ehb
