
#pragma once

#include <vsg/maths/mat3.h>
#include <vsg/maths/vec3.h>

namespace siege
{
	class SiegeMeshDoor
	{
	public:

		// non-copyable
		SiegeMeshDoor(SiegeMeshDoor const&) = delete;
		SiegeMeshDoor& operator=(SiegeMeshDoor const&) = delete;
		
	protected:

		uint32_t m_Id;

		vsg::vec3 m_Center;
		vsg::mat3 m_Orientation;

	public:

		SiegeMeshDoor(const uint32_t& id, const vsg::vec3& center, const vsg::mat3& orient) 
			: m_Id(id)
			, m_Center(center)
			, m_Orientation(orient)
		{};

		SiegeMeshDoor() = default;

		uint32_t id();

		const vsg::vec3& center();
		const vsg::mat3& orientation();
	};

	inline uint32_t SiegeMeshDoor::id()
	{
		return m_Id;
	}

	inline const vsg::vec3& SiegeMeshDoor::center()
	{ 
		return m_Center;
	}

	inline const vsg::mat3& SiegeMeshDoor::orientation()
	{
		return m_Orientation;
	}
}
