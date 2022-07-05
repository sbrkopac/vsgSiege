
#pragma once

#include <memory>
#include <vector>

#include <vsg/maths/vec3.h>
#include <vsg/maths/mat3.h>

namespace ehb
{
    //! SiegeMeshDoor's live in local space of the loaded SiegeMesh and that never changes
    //! SiegeNodeDoors use these to help orient themselves properly
    class SiegeMeshDoor
    {
    public:

        SiegeMeshDoor(SiegeMeshDoor const&) = delete;
        SiegeMeshDoor& operator = (SiegeMeshDoor const&) = delete;

    public:

        SiegeMeshDoor() = default;

        SiegeMeshDoor(const uint32_t& id, const vsg::vec3 & center, const vsg::mat3& orientation) :
            _id(id), _center(center), _orientation(orientation)
        {
            _transform(0, 0) = orientation(0, 0);
            _transform(0, 1) = orientation(0, 1);
            _transform(0, 2) = orientation(0, 2);
            _transform(1, 0) = orientation(1, 0);
            _transform(1, 1) = orientation(1, 1);
            _transform(1, 2) = orientation(1, 2);
            _transform(2, 0) = orientation(2, 0);
            _transform(2, 1) = orientation(2, 1);
            _transform(2, 2) = orientation(2, 2);

            _transform(3, 0) = _center.x;
            _transform(3, 1) = _center.y;
            _transform(3, 2) = _center.z;
        }

        uint32_t id() { return _id; }

        const vsg::vec3& center() { return _center; }
        const vsg::mat3& orientation() { return _orientation; }

        const vsg::dmat4& transform() { return _transform; }

    protected:

        uint32_t _id;

        vsg::vec3 _center;
        vsg::mat3 _orientation;

        // combination of _center and _orientation
        vsg::dmat4 _transform;
    };
}