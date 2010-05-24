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

void RoundedRectangle::SetPosSize(const glm::vec2& inPos, const glm::vec2& inSize, float inRadius, int inNumPoint, const float inColors[4])
{
	using namespace BE::detail;

	glm::vec2 rect_positions[4];

	RectTopLeftAndSizeToPoints(inPos, inSize, rect_positions);
	ToRoundedRectangle(rect_positions, inRadius, inNumPoint, mPositions);

	{
		{
			mCenter = glm::vec2();

			for (int i=0; i<mPositions.size(); ++i)
			{
				mCenter += mPositions[i];
			}
			mCenter = mCenter / ((float) mPositions.size());

			mCenterColor = 0.0f;
			for (int i=0; i<4; ++i)
			{
				mCenterColor += inColors[i];
			}

			mCenterColor = mCenterColor / 4.0f;
		}

		// FIX USING ANGLES!!!
		{
			mColors.resize(mPositions.size());

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


void RoundedRectangle::RenderGL(const float* inOutlineColors)
{
	glBegin(GL_TRIANGLE_FAN);

	glColor4f(mCenterColor, mCenterColor, mCenterColor, 1.0f);
	glVertex2f(horiz2d(mCenter), vert2d(mCenter));
	
	for (int i=0; i<mPositions.size(); ++i)
	{
		glColor4f(mColors[i], mColors[i], mColors[i], 1.0f);
		glVertex2f(horiz2d(mPositions[i]), vert2d(mPositions[i]));
	}

	glColor4f(mColors[0], mColors[0], mColors[0], 1.0f);
	glVertex2f(horiz2d(mPositions[0]), vert2d(mPositions[0]));

	glEnd();

	if (inOutlineColors != NULL)
	{
		glColor4f(inOutlineColors[0], inOutlineColors[1], inOutlineColors[2], 1.0f);

		//glLineWidth(1.5f); // or check z order!
		glLineWidth(1.0f);
		glBegin(GL_LINE_LOOP);
		for (int i=0; i<mPositions.size(); ++i)
		{
			glVertex2f(horiz2d(mPositions[i]), vert2d(mPositions[i]));
		}
		glEnd();
	}
}

}