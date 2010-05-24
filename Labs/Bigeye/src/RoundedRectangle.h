#ifndef _BIGEYE_ROUNDEDRECTANGLE_H
#define _BIGEYE_ROUNDEDRECTANGLE_H

#include "OGL.h"
#include "BEMath.h"
#include <vector>

namespace BE
{

	class RoundedRectangle
	{
	public:

		enum ECorner
		{
			ETopLeft,
			ETopRight,
			EBottomRight,
			EBottomLeft,
		};

		void SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoint, const float inColors[4]);
		void RenderGL(const float* inOutlineColors = NULL);

		bool IsSet() { return !mPositions.empty(); }

	protected:

		typedef std::vector<glm::vec2> Positions;
		typedef std::vector<float> Floats;

		glm::vec2 mCenter;
		float mCenterColor;
		Positions mPositions;
		Floats mColors;
	};
}

#endif