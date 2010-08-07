#ifndef _INCLUDED_BIGFOOT_BFMATH_H
#define _INCLUDED_BIGFOOT_BFMATH_H

#include <algorithm>
#include "glm/glm.hpp"
//#include "glm/gtx/vector_query.hpp"

namespace BF
{
	template<typename T>
	inline T gMathPi() { return (T) 3.14159265358979323846; }

	template<typename T>
	inline float gDegToRad(T inValue) { return inValue * (gMathPi<T>() / (T)180); }

	template<typename T>
	inline float gRadToDeg(T inValue) { return inValue * ((T)180 / gMathPi<T>()); }

	 template <typename T> 
	 inline glm::detail::tmat4x4<T> lookAt2(
			const glm::detail::tvec3<T>& eye, 
			const glm::detail::tvec3<T>& center, 
			const glm::detail::tvec3<T>& up)
	{
		glm::detail::tvec3<T> f = glm::normalize(center - eye);
		glm::detail::tvec3<T> u = glm::normalize(up);
		glm::detail::tvec3<T> s = glm::normalize(cross(f, u));
		u = glm::cross(s, f);

		glm::detail::tmat4x4<T> Result(1);
		Result[0][0] = s.x;
		Result[0][1] = s.y;
		Result[0][2] = s.z;
		Result[1][0] = u.x;
		Result[1][1] = u.y;
		Result[1][2] = u.z;
		Result[2][0] =-f.x;
		Result[2][1] =-f.y;
		Result[2][2] =-f.z;
		Result[3][0] =eye.x;
		Result[3][1] =eye.y;
		Result[3][2] =eye.z;

		return Result;
	}
	

	class AAB
	{
	public:

		glm::vec3 mMin;
		glm::vec3 mMax;

		AAB()
		{
			mMin.x = mMin.y = mMin.z = std::numeric_limits<glm::vec3::value_type>::max();
			mMax.x = mMax.y = mMax.z = std::numeric_limits<glm::vec3::value_type>::max();
		}

		void Include(const glm::vec3& inPoint)
		{
			if (inPoint.x < mMin.x)
				mMin.x = inPoint.x;

			if (inPoint.y < mMin.y)
				mMin.y = inPoint.y;

			if (inPoint.z < mMin.z)
				mMin.z = inPoint.z;


			if (inPoint.x > mMax.x)
				mMax.x = inPoint.x;

			if (inPoint.y > mMax.y)
				mMax.y = inPoint.y;

			if (inPoint.z > mMax.z)
				mMax.z = inPoint.z;
		}

		glm::vec3 GetCenter() const
		{
			return (mMin + mMax) * 0.5f;
		}

		glm::vec3 GetExtents() const
		{
			return (mMax - mMin);
		}

	};


	class Sphere
	{
	public:

		glm::vec3 mPosition;
		float mRadius;

		Sphere() : mRadius(0.0f)
		{
		}

		void InitFrom(const AAB& inBox)
		{
			mPosition = inBox.GetCenter();
			glm::vec3 extents = inBox.GetExtents();
			mRadius = std::max(std::max(extents.x, extents.y), extents.z);
		}
	};
}

#endif