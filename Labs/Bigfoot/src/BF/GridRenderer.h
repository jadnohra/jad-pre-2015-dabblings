#ifndef _INCLUDED_BIGFOOT_GRID_RENDERER_H
#define _INCLUDED_BIGFOOT_GRID_RENDERER_H

#include "Rendering.h"
#include "BFMath.h"

namespace BF
{

class GridRenderer
{
public:

	GridRenderer()
	:	mDivisionCount(1.0f)
	{
	}

	void Setup(const AAB& inBox, glm::vec2& inDivisionCount)
	{
		mBox = inBox;
		mDivisionCount = inDivisionCount;
	}

	static float RoundByInc(float inNum, float inInc)
	{
		return (float) ((int) (inNum / inInc)) * inInc;
	}

	void Render()
	{
		glEnable(GL_COLOR_MATERIAL);
		
		glm::vec3 x_color(0.0f, 1.0f, 0.0f);
		glm::vec3 y_color(1.0f, 0.0f, 0.0f);
		glm::vec3 thin_color(102.0f / 255.0f);
		glm::vec3 thick_color(82.0f / 255.0f);
				
		glColor3fv(glm::value_ptr(thin_color));

		glm::vec2 unit_inc;
		unit_inc.x = mBox.GetExtents().x / mDivisionCount.x;
		unit_inc.y = mBox.GetExtents().z / mDivisionCount.y;

		AAB unit_nice_box;
		unit_nice_box.mMin.x = RoundByInc(mBox.mMin.x, unit_inc.x) - unit_inc.x;
		unit_nice_box.mMin.y = RoundByInc(mBox.mMin.z, unit_inc.y) - unit_inc.y;
		unit_nice_box.mMax.x = RoundByInc(mBox.mMax.x, unit_inc.x) + unit_inc.x;
		unit_nice_box.mMax.y = RoundByInc(mBox.mMax.z, unit_inc.y) + unit_inc.y;

		glBegin(GL_LINES);
		for (float x=unit_nice_box.mMin.x; x<=unit_nice_box.mMax.x; x += unit_inc.x)
		{
			if ((int) x % 10 == 0)
			{
				if ((int) x == 0)
					glColor3fv(glm::value_ptr(x_color));
				else
					glColor3fv(glm::value_ptr(thick_color));
			}

			glVertex3fv(glm::value_ptr(ToWorldVertex(x, unit_nice_box.mMin.y))); 
			glVertex3fv(glm::value_ptr(ToWorldVertex(x, unit_nice_box.mMax.y))); 

			if ((int) x % 10 == 0)
			{
				glColor3fv(glm::value_ptr(thin_color));
			}
		}
		for (float y=unit_nice_box.mMin.y; y<=unit_nice_box.mMax.y; y += unit_inc.y)
		{
			if ((int) y % 10 == 0)
			{
				if ((int) y == 0)
					glColor3fv(glm::value_ptr(y_color));
				else
					glColor3fv(glm::value_ptr(thick_color));
			}

			glVertex3fv(glm::value_ptr(ToWorldVertex(unit_nice_box.mMin.x, y))); 
			glVertex3fv(glm::value_ptr(ToWorldVertex(unit_nice_box.mMax.x, y))); 

			if ((int) y % 10 == 0)
			{
				glColor3fv(glm::value_ptr(thin_color));
			}
		}
		glEnd();

		glDisable(GL_COLOR_MATERIAL);
	}

	glm::vec3 ToWorldVertex(float inX, float inY)
	{
		return glm::vec3(inX, 0.0f, inY);
	}

	AAB mBox;
	glm::vec2 mDivisionCount;
};

}

#endif