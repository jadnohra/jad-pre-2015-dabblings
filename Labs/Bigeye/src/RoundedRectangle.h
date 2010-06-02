#ifndef _BIGEYE_ROUNDEDRECTANGLE_H
#define _BIGEYE_ROUNDEDRECTANGLE_H

#include "OGLState.h"
#include "BEMath.h"
#include <vector>

namespace BE
{

	class RoundedRectangleSkeleton
	{
	public:

		enum ECorner
		{
			ETopLeft,
			ETopRight,
			EBottomRight,
			EBottomLeft,
		};

		typedef std::vector<glm::vec2> Positions;

		void SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoints);
		
		const Positions& GetPositions() const { return mPositions; }
		const glm::vec2& GetCenter() const { return mCenter; }

		bool IsSet() const { return !mPositions.empty(); }

	protected:

		glm::vec2 mCenter;
		Positions mPositions;
	};

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

		bool IsSet() const { return mSkeleton.IsSet(); }

		const RoundedRectangleSkeleton::Positions& GetPositions() const { return mSkeleton.GetPositions(); }
		const glm::vec2& GetCenter() const { return mSkeleton.GetCenter(); }

	protected:

		typedef std::vector<glm::vec4> Colors;

		RoundedRectangleSkeleton mSkeleton;
		glm::vec4 mCenterColor;
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


	class ShapeOutline
	{
	public:

		typedef std::vector<glm::vec2> Positions;

		void SetToShape(const Positions& inPositions, float inRadius, float inLinearity);
		void RenderGL(const glm::vec4& inColor);

	public:
		
		enum EPointType
		{
			EInvalid, EInner, EOuterNormal, EOuterRoundingStart, EOuterRounding, EOuterRoundingEnd
		};

		struct Point
		{
			EPointType type;
			glm::vec2 pos;

			Point() {}

			Point(EPointType inType, const glm::vec2& inPos)
			:	type(inType), pos(inPos)
			{
			}
		};

		typedef std::vector<Point> Points;

		Points mPoints;
	};
}

#endif