
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
            _id(id), _center(center), _orientation(orientation), _verts(vertices)
        {

        }

        uint32_t id() { return _id; }

        const vsg::vec3& center() { return _center; }
        const vsg::mat3& orientation() { return _orientation; }

        std::vector<int32_t>& vertexIndices() { return _verts; }

    protected:

        uint32_t _id;

        vsg::vec3 _center;
        vsg::mat3 _orientation;

        // Verts (indices to owner mesh)
        std::vector<int32_t> _verts;
    };
}