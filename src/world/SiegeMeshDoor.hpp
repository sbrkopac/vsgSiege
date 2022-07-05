
#pragma once

#include <memory>
#include <vector>

#include <vsg/maths/vec3.h>
#include <vsg/maths/mat3.h>

namespace ehb
{
    class SiegeMeshDoor
    {
    public:

        SiegeMeshDoor(SiegeMeshDoor const&) = delete;
        SiegeMeshDoor& operator = (SiegeMeshDoor const&) = delete;

    public:

        SiegeMeshDoor() = default;

        SiegeMeshDoor(const uint32_t& id, const vsg::vec3 & center, const vsg::mat3& orientation, const std::vector<int32_t>& vertices) :
            id(id), center(center), orientation(orientation), verts(vertices)
        {

        }

        uint32_t id() { return id; }

        const vsg::vec3& center() { return center; }
        const vsg::mat3& orientation() { return orientation; }

        std::vector<int32_t>& vertexIndices() { return verts; }

    protected:

        uint32_t id;

        vsg::vec3 center;
        vsg::mat3 orientation;

        // Verts (indices to owner mesh)
        std::vector<int32_t> verts;
    };
}