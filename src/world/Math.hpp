
#pragma once

#include <cmath>
#include <vsg/maths/vec3.h>

// contains some addition comparatives such as epsilon
namespace ehb
{
    namespace math
    {
        bool isEqual(float l, float r, float tolerance)
        {
            return std::fabsf(l - r) < tolerance;
        }

        bool isEqual(const vsg::vec3& l, const vsg::vec3& r, float epsilon)
        {
            return (isEqual(l.x, r.x, epsilon) && isEqual(l.y, r.y, epsilon) && isEqual(l.z, r.z, epsilon));
        }
    } // namespace math
} // namespace ehb