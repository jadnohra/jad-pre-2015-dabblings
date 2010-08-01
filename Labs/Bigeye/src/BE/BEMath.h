#ifndef _INCLUDED_BIGEYE_BEMATH_H
#define _INCLUDED_BIGEYE_BEMATH_H

#include "glm/glm.hpp"
#include "glm/gtx/vector_query.hpp"

namespace BE
{

	typedef glm::mat4 SceneTransform;
	static const SceneTransform kIdentitySceneTransform;

	static const glm::vec2 kvec2d_up(0.0f, -1.0f);
	static const glm::vec2 kvec2d_down(0.0f, 1.0f);
	static const glm::vec2 kvec2d_left(-1.0f, 0.0f);
	static const glm::vec2 kvec2d_right(1.0f, 0.0f);

	inline float up2d(float inValue) { return -inValue; }
	inline float down2d(float inValue) { return inValue; }
	inline float left2d(float inValue) { return -inValue; }
	inline float right2d(float inValue) { return inValue; }

	inline float& vert2d(glm::vec2& inVec) { return inVec.y; }
	inline float& horiz2d(glm::vec2& inVec) { return inVec.x; }
	inline const float& vert2d(const glm::vec2& inVec) { return inVec.y; }
	inline const float& horiz2d(const glm::vec2& inVec) { return inVec.x; }


	inline glm::vec2 to2d_point(const glm::vec3& inVec) { return glm::vec2(inVec.x, inVec.y); }
	inline glm::vec2 to2d_dir(const glm::vec3& inVec) { return glm::vec2(inVec.x, inVec.y); }

	inline glm::vec3 to3d_point(const glm::vec2& inVec) { return glm::vec3(inVec.x, inVec.y, 0.0f); }
	inline glm::vec3 to3d_dir(const glm::vec2& inVec) { return glm::vec3(inVec.x, inVec.y, 0.0f); }


	template<typename T_FROM_DIM, typename T_TO_DIM>
	struct todim
	{
	public:

		static inline T_TO_DIM dir(const T_FROM_DIM& inVec) { return inVec; }
		static inline T_TO_DIM point(const T_FROM_DIM& inVec) { return inVec; }
	};

	template<>
	struct todim<glm::vec3, glm::vec2>
	{
	public:

		static inline glm::vec2 dir(const glm::vec3& inVec) { return to2d_dir(inVec); }
		static inline glm::vec2 point(const glm::vec3& inVec) { return to2d_point(inVec); }
	};


	template<>
	struct todim<glm::vec2, glm::vec3>
	{
	public:

		static inline glm::vec3 dir(const glm::vec2& inVec) { return to3d_dir(inVec); }
		static inline glm::vec3 point(const glm::vec2& inVec) { return to3d_point(inVec); }
	};
}

#endif