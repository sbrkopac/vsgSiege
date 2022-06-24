
#pragma once

#include <math.h> // std::fabs
#include <algorithm> // std::min
#include <vsg/maths/vec3.h>

namespace ehb
{
	// NOTE: THE BELOW ARE PROBABLY NOT VSG CANDIDATES BUT GOOD INTERNAL OpenSiege CANDIDATES
	constexpr float FLOAT_TOLERANCE = 0.00001f; // std::numeric_limits<float>::epsilon(); // ?
	float constexpr RealZero = 0.0f;
	float constexpr RealIdentity = 1.0f;
	constexpr float FLOAT_INFINITE = (float)0x7FFFFFFF;	// works as int or float, use as an arbitrary test value
	constexpr float FLOAT_INDEFINITE = (float)0xEFC00000;	// use a Signalling SNaN, rather than a quiet QNaN
	float constexpr RealMaximum = std::numeric_limits<float>::max();

	inline bool  IsPositive(float f, float epsilon) { return (f > epsilon); }
	inline bool  IsPositive(float f) { return (f > FLOAT_TOLERANCE); }

	inline bool  IsNegative(float f, float epsilon) { return (f < -epsilon); }
	inline bool  IsNegative(float f) { return (f < -FLOAT_TOLERANCE); }

	inline bool  IsZero(float f, float tolerance) { return (std::fabsf(f) < tolerance); }
	inline bool  IsZero(float f) { return (std::fabsf(f) < FLOAT_TOLERANCE); }

	typedef float real; // remove this garbage, i think this was typedef'd so it could be swapped to a double?

	inline void CalcFaceNormal(vsg::vec3& normal, const vsg::vec3& v1, const vsg::vec3& v2, const vsg::vec3& v3)
	{
		auto a = v3 - v2;
		auto b = v1 - v2;

		auto cross = vsg::cross(a, b);
		vsg::normalize(cross);

		normal.x = cross.x;
		normal.y = cross.y;
		normal.z = cross.z;

#if 0 // SAM: I guess somebody had the need - the need for speed
		__asm
		{
			mov		eax, dword ptr[v1]
			fld		dword ptr[eax + 4]		//v1.y
			mov		ecx, dword ptr[v3]
			fld		dword ptr[ecx + 4]		//v3.y
			fsub	st(0), st(1)
			mov		edx, dword ptr[v2]
			fld		dword ptr[edx + 4]		//v2.y
			fsubrp	st(2), st(0)
			mov		esi, dword ptr[normal]

			fld		dword ptr[eax + 8]		// v1.z
			fld		dword ptr[ecx + 8]		// v3.z
			fsub	st(0), st(1)
			fld		dword ptr[edx + 8]		// v2.z
			fsubrp	st(2), st(0)

			fld		st(0)
			fmul	st(0), st(4)
			fld		st(2)
			fmul	st(0), st(4)
			fsubp	st(1), st(0)
			fstp	dword ptr[esi]
			//////////////////////////////////////

			fld		dword ptr[eax]			// v1.x
			fld		dword ptr[ecx]			// v3.x
			fsub	st(0), st(1)
			fmul	st(3), st(0)
			fxch	st(2)

			fld		dword ptr[edx]			// v2.x
			fsubrp	st(2), st(0)
			fmul	st(0), st(1)
			fsubp	st(3), st(0)
			fxch	st(2)
			fstp	dword ptr[esi + 4]
			/////////////////////////////////////

			fmulp	st(3), st(0)
			fmulp	st(1), st(0)

			fsubrp	st(1), st(0)
			fstp	dword ptr[esi + 8]
		}
#endif
	}

	// END OF WEIRDNESS

	inline bool  IsZero(const vsg::vec3& v, float Tolerance = FLOAT_TOLERANCE)
	{
		return (IsZero(v.x, Tolerance) && IsZero(v.y, Tolerance) && IsZero(v.z, Tolerance));
	}

	//! box
    inline bool PointInBox(const vsg::vec3& min, const vsg::vec3& max, const vsg::vec3& point)
    {
		if (point.x < min.x) {
			return false;
		}
		if (point.x > max.x) {
			return false;
		}
		if (point.y < min.y) {
			return false;
		}
		if (point.y > max.y) {
			return false;
		}
		if (point.z < min.z) {
			return false;
		}
		if (point.z > max.z) {
			return false;
		}
		return true;
    }

	//! box
	inline bool RayIntersectsBox(const vsg::vec3& minBound, const vsg::vec3& maxBound, const vsg::vec3& ray_orig, const vsg::vec3& ray_dir, vsg::vec3& coord)
	{
		vsg::vec3	quadrant(2.0f, 2.0f, 2.0f);
		vsg::vec3	candidateplane;
		bool		inside = true;

		// First we need to generate candidate planes
		if (ray_orig.x < minBound.x) {
			quadrant.x = 1.0f;	// LEFT
			candidateplane.x = minBound.x;
			inside = false;
		}
		else if (ray_orig.x > maxBound.x) {
			quadrant.x = 0.0f;	// RIGHT
			candidateplane.x = maxBound.x;
			inside = false;
		}

		if (ray_orig.y < minBound.y) {
			quadrant.y = 1.0f;	// LEFT
			candidateplane.y = minBound.y;
			inside = false;
		}
		else if (ray_orig.y > maxBound.y) {
			quadrant.y = 0.0f;	// RIGHT
			candidateplane.y = maxBound.y;
			inside = false;
		}

		if (ray_orig.z < minBound.z) {
			quadrant.z = 1.0f;	// LEFT
			candidateplane.z = minBound.z;
			inside = false;
		}
		else if (ray_orig.z > maxBound.z) {
			quadrant.z = 0.0f;	// RIGHT
			candidateplane.z = maxBound.z;
			inside = false;
		}

		// Return if we are inside the box
		if (inside)
		{
			coord = ray_orig;
			return true;
		}

		// Calculate relative distance as T
		float maxT[3];
		if (quadrant.x != 2.0f && ray_dir.x != 0.0f) {
			maxT[0] = (candidateplane.x - ray_orig.x) / ray_dir.x;
		}
		else {
			maxT[0] = -1.0f;
		}

		if (quadrant.y != 2.0f && ray_dir.y != 0.0f) {
			maxT[1] = (candidateplane.y - ray_orig.y) / ray_dir.y;
		}
		else {
			maxT[1] = -1.0f;
		}

		if (quadrant.z != 2.0f && ray_dir.z != 0.0f) {
			maxT[2] = (candidateplane.z - ray_orig.z) / ray_dir.z;
		}
		else {
			maxT[2] = -1.0f;
		}

		// Get the largest T for intersection
		int whichplane = 0;
		if (maxT[0] < maxT[1]) {
			whichplane = 1;
		}
		if (maxT[whichplane] < maxT[2]) {
			whichplane = 2;
		}

		// Return if no possible intersection
		if (maxT[whichplane] < 0.0f) {
			return false;
		}

		// Check for final intersection
		if (whichplane != 0) {
			// Do X plane intersection
			coord.x = ray_orig.x + maxT[whichplane] * ray_dir.x;
			if ((coord.x < minBound.x) || (coord.x > maxBound.x))
			{
				return false;
			}
		}
		else
		{
			coord.x = candidateplane.x;
		}

		if (whichplane != 1) {
			// Do Y plane intersection
			coord.y = ray_orig.y + maxT[whichplane] * ray_dir.y;
			if ((coord.y < minBound.y) || (coord.y > maxBound.y))
			{
				return false;
			}
		}
		else
		{
			coord.y = candidateplane.y;
		}

		if (whichplane != 2) {
			// Do Z plane intersection
			coord.z = ray_orig.z + maxT[whichplane] * ray_dir.z;
			if ((coord.z < minBound.z) || (coord.z > maxBound.z))
			{
				return false;
			}
		}
		else
		{
			coord.z = candidateplane.z;
		}

		return true;
	}

	//! triangle
	/* ************************************************************************
   Function: RayIntersectsTriangle
   Description: Intersects a ray with a triangle
   ************************************************************************ */


	inline float SAME_SIGN(const float a, const float b)
	{
		return ((!IsPositive(a) && !IsPositive(b)) || (!IsNegative(a) && !IsNegative(b)));
	}

	inline bool RayIntersectsTriangle(
			vsg::vec3 const& RayOrigin, vsg::vec3 const& RayDirection,
			vsg::vec3 const& Vertex0, vsg::vec3 const& Vertex1,
			vsg::vec3 const& Vertex2, real& TCoordinate, real& UCoordinate,
			real& VCoordinate)
	{
		// Relaxed the tolerances in this routine
		// and added support for coplanar rays & triangles --biddle

		// Calculate edge boundaries
		vsg::vec3 const& Edge1 = Vertex1 - Vertex0;
		vsg::vec3 const& Edge2 = Vertex2 - Vertex0;

		// Find the determinant of the matrix formed by the direction
		// and the two edges (by means of the triple product)
		// then the ray is in the same plane as the triangle
		vsg::vec3 const P = vsg::cross(RayDirection, Edge2);
		float const Determinant = vsg::dot(P, Edge1);

		// Since the determinant measures the area suggested by the matrix,
		// if it is near zero then the ray in the plane of the triangle


		//if((Determinant > RealTolerance) || (Determinant < -RealTolerance))
		if (!IsZero(Determinant))
		{
			float const InverseDeterminant = 1.0f / Determinant;
			vsg::vec3 const T = RayOrigin - Vertex0;
			UCoordinate = vsg::dot(T, P) * InverseDeterminant;
			//if((UCoordinate > RealZero) && (UCoordinate < RealIdentity))
			if (!IsNegative(UCoordinate, 0.001f) && !IsPositive(UCoordinate - RealIdentity, 0.001f))
			{
				vsg::vec3 const Q = cross(T, Edge1);
				VCoordinate = vsg::dot(RayDirection, Q) * InverseDeterminant;
				//if((VCoordinate > RealZero) && ((UCoordinate + VCoordinate) < RealIdentity))
				if (!IsNegative(VCoordinate, 0.001f) && !IsPositive(UCoordinate + VCoordinate - RealIdentity, 0.001f))
				{
					TCoordinate = vsg::dot(Q, Edge2) * InverseDeterminant;
					return true;
				}
				else
				{
					TCoordinate = 0.0f;	// Need these to set BPoints
				}
			}
			else
			{
				TCoordinate = 0.0f;		// Need these to set BPoints
			}

		}
		else
		{

			// Find normal of plane containing RayDirection and parallel to triangle

			const vsg::vec3 TriNorm = cross(Edge1, Edge2);

			if (IsZero(TriNorm)) {
				// This is a line, not a triangle!
				return false;
			}

			const vsg::vec3 V0 = Vertex0 - RayOrigin;

			float planar_check = vsg::dot(TriNorm, V0);

			if (!IsZero(planar_check, 0.001f)) {
				// The ray is PARALLEL  to the triangle, but does not intersect it
				return false;
			}

			const vsg::vec3 RayNorm = vsg::normalize(vsg::cross(TriNorm, RayDirection));

			const vsg::vec3 V1 = Vertex1 - RayOrigin;
			const vsg::vec3 V2 = Vertex2 - RayOrigin;

			const float dist0 = vsg::dot(RayNorm, V0);
			const float dist1 = vsg::dot(RayNorm, V1);
			const float dist2 = vsg::dot(RayNorm, V2);

			float t0, t1, t2;

			vsg::vec3 i0, i1, i2;

			if (!SAME_SIGN(dist0, dist1))
			{
				i0 = (V0 * dist1 - V1 * dist0) / (dist1 - dist0);
				t0 = vsg::dot(RayDirection, i0) / vsg::length2(RayDirection);
			}
			else
			{
				t0 = FLOAT_INFINITE;
			}

			if (!SAME_SIGN(dist1, dist2))
			{
				i1 = (V1 * dist2 - V2 * dist1) / (dist2 - dist1);
				t1 = vsg::dot(RayDirection, i1) / vsg::length2(RayDirection);
			}
			else
			{
				t1 = FLOAT_INFINITE;
			}

			if (!SAME_SIGN(dist2, dist0))
			{
				i2 = (V2 * dist0 - V0 * dist2) / (dist0 - dist2);
				t2 = vsg::dot(RayDirection, i2) / vsg::length2(RayDirection);
			}
			else
			{
				t2 = FLOAT_INFINITE;
			}

			TCoordinate = std::min(t0, std::min(t1, t2));

			return TCoordinate != FLOAT_INFINITE;

			/*
					// U and V coordinates are not set!!!

					const vsg::vec3 T = Vertex0-RayOrigin;

					if (!IsZero(TriNorm.DotProduct(T)))
					{
						const vsg::vec3 RayNorm = cross(RayDirection,TriNorm);
						const float dist0 = RayNorm.DotProduct(T);
						const float dist1 = RayNorm.DotProduct(Vertex1-RayOrigin);
						const float dist2 = RayNorm.DotProduct(Vertex2-RayOrigin);

						if (!SAME_SIGN(dist0,dist1))
						{
							const vsg::vec3 p0 = ((Vertex0*dist0) - (Vertex1*dist1)) / (dist0-dist1);
							if (!SAME_SIGN(dist1,dist2))
							{
								vsg::vec3 p1 = ((Vertex1*dist1) - (Vertex2*dist2)) / (dist1-dist2);
								const float len0 = (p0-RayOrigin).Length2();
								const float len1 = (p1-RayOrigin).Length2();
								if (len0 < len1)
								{
									TCoordinate = SQRTF(len0/RayDirection.Length2());
								}
								else
								{
									TCoordinate = SQRTF(len1/RayDirection.Length2());
								}
							}
							else
							{
								TCoordinate = SQRTF((p0-RayOrigin).Length2()/RayDirection.Length2());
							}
							return true;
						}
						else
						{
							if (!SAME_SIGN(dist1,dist2))
							{
								const vsg::vec3 p1 = ((Vertex1*dist1) - (Vertex2*dist2)) / (dist1-dist2);
								TCoordinate = SQRTF((p1-RayOrigin).Length2()/RayDirection.Length2());
								return true;
							}
						}

					}
			*/
		}

		return false;

	}

	// Determine if a single dimension ray hits a box
	inline bool YRayIntersectsBox(const vsg::vec3& minBound, const vsg::vec3& maxBound,
		const vsg::vec3& ray_orig, const float ray_dir, vsg::vec3& coord)
	{
		// Check our XZ dimension
		if (ray_orig.x < minBound.x) { return false; }
		if (ray_orig.x > maxBound.x) { return false; }
		if (ray_orig.z < minBound.z) { return false; }
		if (ray_orig.z > maxBound.z) { return false; }

		if (ray_dir > 0.0f)
		{
			if (ray_orig.y > maxBound.y)
			{
				return false;
			}
			else
			{
				coord = ray_orig;
				if (ray_orig.y < minBound.y)
				{
					coord.y = minBound.y;
				}
				return true;
			}
		}
		else
		{
			if (ray_orig.y < minBound.y)
			{
				return false;
			}
			else
			{
				coord = ray_orig;
				if (ray_orig.y > maxBound.y)
				{
					coord.y = maxBound.y;
				}
				return true;
			}
		}
	}

	// Method to determine if one axis aligned box touches or intersects another
	inline bool BoxIntersectsBox(const vsg::vec3& minBox1, const vsg::vec3& maxBox1,
		const vsg::vec3& minBox2, const vsg::vec3& maxBox2)
	{
		if ((maxBox2.x < minBox1.x || maxBox2.z < minBox1.z) ||
			(minBox2.x > maxBox1.x || minBox2.z > maxBox1.z) ||
			(maxBox2.y < minBox1.y || minBox2.y > maxBox1.y))
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}