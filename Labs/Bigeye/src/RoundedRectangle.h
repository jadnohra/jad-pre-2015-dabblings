#ifndef _BIGEYE_ROUNDEDRECTANGLE_H
#define _BIGEYE_ROUNDEDRECTANGLE_H

#include "OGLState.h"
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

		void SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoints, const glm::vec4 inColors[4]);
		void RenderGL();
		void RenderOutlineGL(const glm::vec4& inOutlineColor);

		bool IsSet() const { return !mPositions.empty(); }

	protected:

		typedef std::vector<glm::vec2> Positions;
		typedef std::vector<glm::vec4> Colors;

		glm::vec2 mCenter;
		glm::vec4 mCenterColor;
		Positions mPositions;
		Colors mColors;
	};

	class RoundedRectangleWithShadow
	{
	public:
		
		void SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoints, 
						const glm::vec4 inColors[4], float inShadowIntensity, bool inDoubleSideShadow);
		void RenderGL(const OGLStateManager& inStateManager, int inShadowState, int inNormalState);
		
		bool IsSet() const { return mMainRect.IsSet(); }

	protected:

		RoundedRectangle mMainRect;
		RoundedRectangle mShadowRect[4];
	};
}

#endif