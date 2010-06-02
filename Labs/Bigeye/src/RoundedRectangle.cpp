#include "RoundedRectangle.h"


namespace BE { namespace detail
{

	template<typename TVec>
	void RectTopLeftAndSizeToPoints(const TVec& inTopLeft, const TVec& inSize, TVec inPoints[4])
	{
		inPoints[RoundedRectangle::ETopLeft] = inTopLeft;
		inPoints[RoundedRectangle::ETopRight] = inTopLeft;
		inPoints[RoundedRectangle::ETopRight].x += inSize.x;
		inPoints[RoundedRectangle::EBottomRight] = inPoints[RoundedRectangle::ETopRight];
		inPoints[RoundedRectangle::EBottomRight].y += inSize.y;
		inPoints[RoundedRectangle::EBottomLeft] = inPoints[RoundedRectangle::EBottomRight];
		inPoints[RoundedRectangle::EBottomLeft].x = inTopLeft.x;
	}


	template<typename TVec>
	void ToRoundedRectangle(const TVec inRectPoints[4], float inRadius, int inTesselationPointCount)
	{
		RenderRoundedRectangle(inRectPoints[0], inRectPoints[1], inRectPoints[2], inRectPoints[3]);
	}


	template<typename TVec>
	void ToRoundedRectangle_GenCircleArc(const TVec& inCenter, const TVec& inFrom, const TVec& inTo, 
										float inRadius, int inPointCount, 
										std::vector<TVec>& outPoints, int inFirstPointIndex)
	{
		const float factor = 1.0f / (inPointCount+1);
		float t = factor;

		for (int i=0 ; i<inPointCount; ++i)
		{
			TVec point = inFrom * (1.0f - t) + inTo * (t);
			glm::vec2 dir = todim<TVec, glm::vec2>::dir(point - inCenter);
			glm::vec2 new_pt = glm::normalize(dir) * inRadius;
			outPoints[inFirstPointIndex+i] = inCenter + todim<glm::vec2, TVec>::point(new_pt);

			t += factor;
		}
	}


	template<typename TVec>
	void ToRoundedRectangle(const TVec inRectPoints[4], 
							float inRadius, int inTesselationPointCount, std::vector<TVec>& outPoints)
	{
		const int side_point_count = 2 + inTesselationPointCount;
		outPoints.resize(side_point_count*4);

		{
			int top_side_point_index = 0 * side_point_count;

			outPoints[top_side_point_index] = inRectPoints[RoundedRectangle::ETopLeft];
			outPoints[top_side_point_index+1] = inRectPoints[RoundedRectangle::ETopRight];

			horiz2d(outPoints[top_side_point_index]) += right2d(inRadius);
			horiz2d(outPoints[top_side_point_index+1]) += left2d(inRadius);

			TVec center_circle = outPoints[top_side_point_index+1];
			vert2d(center_circle) += down2d(inRadius);

			TVec end_circle = center_circle;
			horiz2d(end_circle) += right2d(inRadius);

			ToRoundedRectangle_GenCircleArc(center_circle, outPoints[top_side_point_index+1], end_circle, 
											inRadius, inTesselationPointCount, outPoints, top_side_point_index+2);
		}

		{
			int right_side_point_index = 1 * side_point_count;

			outPoints[right_side_point_index] = inRectPoints[RoundedRectangle::ETopRight];
			outPoints[right_side_point_index+1] = inRectPoints[RoundedRectangle::EBottomRight];

			vert2d(outPoints[right_side_point_index]) += down2d(inRadius);
			vert2d(outPoints[right_side_point_index+1]) += up2d(inRadius);

			TVec center_circle = outPoints[right_side_point_index+1];
			horiz2d(center_circle) += left2d(inRadius);

			TVec end_circle = center_circle;
			vert2d(end_circle) += down2d(inRadius);

			ToRoundedRectangle_GenCircleArc(center_circle, outPoints[right_side_point_index+1], end_circle, 
											inRadius, inTesselationPointCount, outPoints, right_side_point_index+2);
		}

		{
			int bottom_side_point_index = 2 * side_point_count;

			outPoints[bottom_side_point_index] = inRectPoints[RoundedRectangle::EBottomRight];
			outPoints[bottom_side_point_index+1] = inRectPoints[RoundedRectangle::EBottomLeft];

			horiz2d(outPoints[bottom_side_point_index]) += left2d(inRadius);
			horiz2d(outPoints[bottom_side_point_index+1]) += right2d(inRadius);

			TVec center_circle = outPoints[bottom_side_point_index+1];
			vert2d(center_circle) += up2d(inRadius);

			TVec end_circle = center_circle;
			horiz2d(end_circle) += left2d(inRadius);

			ToRoundedRectangle_GenCircleArc(center_circle, outPoints[bottom_side_point_index+1], end_circle, 
											inRadius, inTesselationPointCount, outPoints, bottom_side_point_index+2);
		}

		{
			int left_side_point_index = 3 * side_point_count;

			outPoints[left_side_point_index] = inRectPoints[RoundedRectangle::EBottomLeft];
			outPoints[left_side_point_index+1] = inRectPoints[RoundedRectangle::ETopLeft];

			vert2d(outPoints[left_side_point_index]) += up2d(inRadius);
			vert2d(outPoints[left_side_point_index+1]) += down2d(inRadius);

			TVec center_circle = outPoints[left_side_point_index+1];
			horiz2d(center_circle) += right2d(inRadius);

			TVec end_circle = center_circle;
			vert2d(end_circle) += up2d(inRadius);

			ToRoundedRectangle_GenCircleArc(center_circle, outPoints[left_side_point_index+1], end_circle,
											inRadius, inTesselationPointCount, outPoints, left_side_point_index+2);
		}
	}

	template<typename TVec, typename TCallback>
	void OGLRenderRoundedRect(const std::vector<TVec>& inPoints, TCallback* inCallback)
	{
		glBegin(GL_TRIANGLE_FAN);

		int point_count_per_side = inPoints.size() / 4;
		//TVec center = 0.25f * (inPoints[0*point_count_per_side] + inPoints[1*point_count_per_side] + inPoints[2*point_count_per_side]+ inPoints[3*point_count_per_side]);

		int point_index = 0;

		for (int i=0; i<4; ++i)
		{
			if (inCallback)
				inCallback(i);

			for (int j=0; j<point_count_per_side; ++j)
			{
				glVertex2f(horiz2d(inPoints[point_index]), vert2d(inPoints[point_index]));
				++point_index;
			}
		}

		glEnd();
	}

	inline void OGLRenderRoundedRect_NoCallback(int inIndex) {}

	template<typename TVec>
	inline void OGLRenderRoundedRect(const std::vector<TVec>& inPoints)
	{
		OGLRenderRoundedRect<TVec>(inPoints, &OGLRenderRoundedRect_NoCallback);
	}

} }


namespace BE
{


void RoundedRectangleSkeleton::SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoints)
{
	using namespace BE::detail;

	glm::vec2 rect_positions[4];

	RectTopLeftAndSizeToPoints(inPos, inSize, rect_positions);
	ToRoundedRectangle(rect_positions, inRadius, inNumPoints, mPositions);

	{
		mCenter = glm::vec2();

		for (size_t i=0; i<mPositions.size(); ++i)
		{
			mCenter += mPositions[i];
		}
		mCenter = mCenter / ((float) mPositions.size());
	}
}


void RoundedRectangle::SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoints, const glm::vec4 inColors[4])
{
	using namespace BE::detail;

	mSkeleton.SetPosSize(inPos, inSize, inRadius, inNumPoints);

	{
		{
			mCenterColor = glm::vec4();
			for (int i=0; i<4; ++i)
			{
				mCenterColor += inColors[i];
			}

			mCenterColor = mCenterColor / 4.0f;
		}

		// FIX USING ANGLES!!!
		{
			glm::vec2 rect_positions[4];
			RectTopLeftAndSizeToPoints(inPos, inSize, rect_positions);

			mColors.resize(mSkeleton.GetPositions().size());

			int point_count_per_side = mSkeleton.GetPositions().size() / 4;
			int point_index = 0;
			int ref_index = 0;

			const int point_count = (int) mSkeleton.GetPositions().size();

			while (point_index < point_count)
			{
				const glm::vec2& from_point = rect_positions[(ref_index) % 4];
				const glm::vec2& tp_point = rect_positions[(ref_index+1) % 4];

				glm::vec4 from_color = inColors[(ref_index) % 4];
				glm::vec4 to_color = inColors[(ref_index+1) % 4];

				glm::vec2 from_dir = glm::normalize(from_point-mSkeleton.GetCenter());
				float factor_from = glm::dot(from_dir, glm::normalize(mSkeleton.GetPositions()[point_index]-mSkeleton.GetCenter()));

				while (factor_from >= 0.0f)
				{
					mColors[point_index++] = from_color * factor_from + to_color * (1.0f-factor_from);

					if (point_index < point_count)
						factor_from = glm::dot(from_dir, glm::normalize(mSkeleton.GetPositions()[point_index]-mSkeleton.GetCenter()));
					else
						break;
				}

				++ref_index;
			}
		}


		// FIX USING ANGLES!!!
		/*
		{
			mColors.resize(mPositions.size());
			float center_angles[4];

			for (int i=0; i<4; ++i)
			{
				const glm::vec2& from_corner = rect_positions[i];
				const glm::vec2& to_corner = rect_positions[(i+1) % 4];

				center_angles[4] = gtx::orientedAngle(glm::normalize(from_corner-mCenter), glm::normalize(to_corner-mCenter));
			}

			int ref_index = 0;
			int ref_to_index = (ref_index+1) % 4;

			glm::vec2 ref_corner = rect_positions[(ref_index) % 4];
			glm::vec2 ref_dir =  glm::normalize(ref_corner-mCenter);

			glm::vec2 ref_to_corner = rect_positions[ref_to_index];
			glm::vec2 ref_to_dir =  glm::normalize(ref_to_corner-mCenter);

			float ref_color = inColors[(ref_index) % 4];
			float ref_to_color = inColors[ref_to_index];

			for (int i=0; i<mPositions.size(); ++i)
			{
				glm::vec2 pos_dir = glm::normalize(mPositions[i]-mCenter);

				float from_angle = gtx::orientedAngle(glm::normalize(ref_corner-mCenter), pos_dir);
				float to_angle = gtx::orientedAngle(glm::normalize(ref_corner-mCenter), pos_dir);

				while (from_angle < center_angles[ref_index]
						|| to_angle > center_angles[ref_to_index])
				{
					++ref_index;

					ref_corner = rect_positions[(ref_index) % 4];
					ref_dir =  glm::normalize(ref_corner-mCenter);

					ref_to_corner = rect_positions[ref_to_index];
					ref_to_dir =  glm::normalize(ref_to_corner-mCenter);

					ref_color = inColors[(ref_index) % 4];
					ref_to_color = inColors[ref_to_index];
				}


			}

			int point_count_per_side = mPositions.size() / 4;
			int point_index = 0;
			int ref_index = 0;

			while (point_index < mPositions.size())
			{
				const glm::vec2& from_point = rect_positions[(ref_index) % 4];
				const glm::vec2& tp_point = rect_positions[(ref_index+1) % 4];

				float from_color = inColors[(ref_index) % 4];
				float to_color = inColors[(ref_index+1) % 4];

				glm::vec2 from_dir = glm::normalize(from_point-mCenter);
				float factor_from = glm::dot(from_dir, glm::normalize(mPositions[point_index]-mCenter));

				while (factor_from >= 0.0f)
				{
					mColors[point_index++] = from_color * factor_from + to_color * (1.0f-factor_from);

					if (point_index < mPositions.size())
						factor_from = glm::dot(from_dir, glm::normalize(mPositions[point_index]-mCenter));
					else
						break;
				}

				++ref_index;
			}
		}
		*/
	}
}


void RoundedRectangle::RenderGL()
{
	glBegin(GL_TRIANGLE_FAN);

	glColor4f(mCenterColor.r, mCenterColor.g, mCenterColor.b, mCenterColor.a);
	glVertex2f(horiz2d(mSkeleton.GetCenter()), vert2d(mSkeleton.GetCenter()));
	
	for (size_t i=0; i<mSkeleton.GetPositions().size(); ++i)
	{
		glColor4f(mColors[i].r, mColors[i].g, mColors[i].b, mColors[i].a);
		glVertex2f(horiz2d(mSkeleton.GetPositions()[i]), vert2d(mSkeleton.GetPositions()[i]));
	}

	glColor4f(mColors[0].r, mColors[0].g, mColors[0].b, mColors[0].a);
	glVertex2f(horiz2d(mSkeleton.GetPositions()[0]), vert2d(mSkeleton.GetPositions()[0]));

	glEnd();
}


void RoundedRectangle::RenderOutlineGL(const glm::vec4& inOutlineColor)
{
	glColor4f(inOutlineColor.r, inOutlineColor.g, inOutlineColor.b, inOutlineColor.a);

	glBegin(GL_LINE_LOOP);
	for (size_t i=0; i<mSkeleton.GetPositions().size(); ++i)
	{
		glVertex2f(horiz2d(mSkeleton.GetPositions()[i]), vert2d(mSkeleton.GetPositions()[i]));
	}
	glEnd();
}


void RoundedRectangleWithShadow::SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoints,
											const glm::vec4 inColors[4], float inShadowIntensity, bool inDoubleSideShadow)
{
	mMainRect.SetPosSize(inPos, inSize, inRadius, inNumPoints, inColors);

	glm::vec4 shadow_colors[4];
	glm::vec3 shadow_color(1.0f, 1.0f, 1.0f);

	{
		glm::vec2 shadow_pos = inPos; 
		glm::vec2 shadow_size = inSize;
		horiz2d(shadow_pos) += left2d(0.0f);
		vert2d(shadow_pos) += down2d(1.0f);
		
		if (inDoubleSideShadow)
			horiz2d(shadow_size) += right2d(0.0f);

		float shadow_color_factor = (1.0f - inShadowIntensity);

		for (int i=0; i<4; ++i)
			shadow_colors[i] = glm::vec4(shadow_color.r * shadow_color_factor, shadow_color.g * shadow_color_factor, shadow_color.b * shadow_color_factor, 1.0f);

		mShadowRect[0].SetPosSize(shadow_pos, shadow_size, inRadius, inNumPoints, shadow_colors);
	}

	{
		glm::vec2 shadow_pos = inPos; 
		glm::vec2 shadow_size = inSize;
		horiz2d(shadow_pos) += left2d(1.0f);
		vert2d(shadow_pos) += down2d(2.0f);

		if (inDoubleSideShadow)
			horiz2d(shadow_size) += right2d(0.0f);

		float shadow_color_factor = (1.0f - (inShadowIntensity*0.9f));

		for (int i=0; i<4; ++i)
			shadow_colors[i] = glm::vec4(shadow_color.r * shadow_color_factor, shadow_color.g * shadow_color_factor, shadow_color.b * shadow_color_factor, 1.0f);

		mShadowRect[1].SetPosSize(shadow_pos, shadow_size, inRadius, inNumPoints, shadow_colors);
	}

	{
		glm::vec2 shadow_pos = inPos; 
		glm::vec2 shadow_size = inSize;
		horiz2d(shadow_pos) += left2d(2.0f);
		vert2d(shadow_pos) += down2d(3.0f);

		if (inDoubleSideShadow)
			horiz2d(shadow_size) += right2d(3.0f);

		float shadow_color_factor = (1.0f - (inShadowIntensity*0.8f));

		for (int i=0; i<4; ++i)
			shadow_colors[i] = glm::vec4(shadow_color.r * shadow_color_factor, shadow_color.g * shadow_color_factor, shadow_color.b * shadow_color_factor, 1.0f);

		mShadowRect[2].SetPosSize(shadow_pos, shadow_size, inRadius, inNumPoints, shadow_colors);
	}

	{
		glm::vec2 shadow_pos = inPos; 
		glm::vec2 shadow_size = inSize;
		horiz2d(shadow_pos) += left2d(3.0f);
		vert2d(shadow_pos) += down2d(4.0f);

		if (inDoubleSideShadow)
			horiz2d(shadow_size) += right2d(5.0f);

		float shadow_color_factor = (1.0f - (inShadowIntensity*0.7f));

		for (int i=0; i<4; ++i)
			shadow_colors[i] = glm::vec4(shadow_color.r * shadow_color_factor, shadow_color.g * shadow_color_factor, shadow_color.b * shadow_color_factor, 1.0f);

		mShadowRect[3].SetPosSize(shadow_pos, shadow_size, inRadius, inNumPoints, shadow_colors);
	}
}


void RoundedRectangleWithShadow::RenderGL(const OGLStateManager& inStateManager, int inShadowState, int inNormalState)
{
	inStateManager.Enable(inShadowState);

	for (int i=0; i<4; ++i)
		mShadowRect[i].RenderGL();

	inStateManager.Enable(inNormalState);

	mMainRect.RenderGL();
}


void ShapeOutline::SetToShape(const Positions& inPositions, float inRadius, float inLinearity)
{
	using namespace BE::detail;

	if (inPositions.size() < 1)
	{
		mPoints.clear();
		return;
	}

	mPoints.reserve(inPositions.size() * 3);
	int set_index = 0;

	Positions temp_positions;
	

	for (size_t p=((int)inPositions.size()-1),i=0, j=1; i<inPositions.size(); ++i)
	{
		const glm::vec2& from = inPositions[p];
		const glm::vec2& at = inPositions[i];
		const glm::vec2& to = inPositions[j];

		//if (!glm::areSimilar(at, from, 0.0f/*, std::numeric_limits<float>::epsilon()*/)
		//	&& !glm::areSimilar(at, to, 0.0f/*, std::numeric_limits<float>::epsilon()*/)
		//	)
		{
			glm::vec2 from_dir = glm::normalize(at-from);
			glm::vec2 from_normal(from_dir.y, -from_dir.x);

			glm::vec2 to_dir = glm::normalize(to-at);
			glm::vec2 to_normal(to_dir.y, -to_dir.x);
			
			glm::vec2 normal = 0.5f * (from_normal + to_normal);
			
			mPoints.push_back(Point(EInner, at));
			
			glm::vec2 tip_from = at + from_normal * inRadius;
			glm::vec2 tip_to = at + to_normal * inRadius;
			
			float tip_dist = glm::length(tip_to - tip_from);
			float needed_round_point_count = tip_dist / inLinearity;
			
			if (needed_round_point_count > 2.0f)
			{
				int point_count = (int) (needed_round_point_count + 0.5f);
				temp_positions.resize(point_count);
				ToRoundedRectangle_GenCircleArc(at, tip_from, tip_to, 
												inRadius, point_count, 
												temp_positions, 0);

				mPoints.push_back(Point(EOuterRoundingStart, tip_from));
				for (size_t i=0; i<temp_positions.size(); ++i)
				{
					mPoints.push_back(Point(EOuterRounding, temp_positions[i]));
				}
				mPoints.push_back(Point(EOuterRoundingEnd, tip_to));
			}
			else
			{
				glm::vec2 tip_avg = at + glm::normalize(0.5f * (from_normal + to_normal)) * inRadius;
				mPoints.push_back(Point(EOuterNormal,tip_avg));
			}

			++set_index;
		}

		{
			++j;
			if (j == inPositions.size())
				j = 0;
		}

		{
			++p;
			if (p == inPositions.size())
				p = 0;
		}
	}
}


void ShapeOutline::RenderGL(const glm::vec4& inColor)
{
	if (mPoints.empty())
		return;

	glColor4f(inColor.r, inColor.g, inColor.b, inColor.a);

	glBegin(GL_TRIANGLE_STRIP);

	glm::vec2 rounding_center;

	for (size_t i=0; i<mPoints.size(); ++i)
	{
		if (mPoints[i].type == EInner
			|| mPoints[i].type == EOuterNormal)
		{
			glVertex2f(horiz2d(mPoints[i].pos), vert2d(mPoints[i].pos));
		}
		else if (mPoints[i].type == EOuterRoundingStart)
		{
			rounding_center = mPoints[i-1].pos;

			glVertex2f(horiz2d(mPoints[i].pos), vert2d(mPoints[i].pos));
			glVertex2f(horiz2d(rounding_center), vert2d(rounding_center));
		}
		else if (mPoints[i].type == EOuterRounding)
		{
			glVertex2f(horiz2d(mPoints[i].pos), vert2d(mPoints[i].pos));
			glVertex2f(horiz2d(rounding_center), vert2d(rounding_center));
		}
		else if (mPoints[i].type == EOuterRoundingEnd)
		{
			glVertex2f(horiz2d(mPoints[i].pos), vert2d(mPoints[i].pos));
		}
	}

	//glVertex2f(horiz2d(mPoints[0].pos), vert2d(mPoints[0].pos));
	//glVertex2f(horiz2d(mPoints[0].pos), vert2d(mPoints[0].pos));

	glEnd();
}

}