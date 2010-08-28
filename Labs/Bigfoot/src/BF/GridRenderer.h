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
	:	mGridWidth(1)
	,	mGridLength(1)
	,	mScale(1.0f)
	{
	}

	void Setup(int inWidth, int inLength, float inScale)
	{
		mGridWidth = inWidth;
		mGridLength = inLength;
		mScale = inScale;
	}

	void Render()
	{
		glEnable(GL_COLOR_MATERIAL);
		
		glm::vec3 x_color(0.0f, 1.0f, 0.0f);
		glm::vec3 y_color(1.0f, 0.0f, 0.0f);
		glm::vec3 thin_color(102.0f / 255.0f);
		glm::vec3 thick_color(82.0f / 255.0f);
				
		glColor3fv(glm::value_ptr(thin_color));

		glBegin(GL_LINES);
		for (int x=-mGridWidth; x<=mGridWidth; ++x)
		{
			if (x % 10 == 0)
			{
				if (x == 0)
					glColor3fv(glm::value_ptr(x_color));
				else
					glColor3fv(glm::value_ptr(thick_color));
			}

			glVertex3fv(glm::value_ptr(ToWorldVertex(x, -mGridLength))); 
			glVertex3fv(glm::value_ptr(ToWorldVertex(x, mGridLength))); 

			if (x % 10 == 0)
			{
				glColor3fv(glm::value_ptr(thin_color));
			}
		}
		for (int y=-mGridLength; y<=mGridLength; ++y)
		{
			if (y % 10 == 0)
			{
				if (y == 0)
					glColor3fv(glm::value_ptr(y_color));
				else
					glColor3fv(glm::value_ptr(thick_color));
			}

			glVertex3fv(glm::value_ptr(ToWorldVertex(-mGridWidth, y))); 
			glVertex3fv(glm::value_ptr(ToWorldVertex(mGridWidth, y))); 

			if (y % 10 == 0)
			{
				glColor3fv(glm::value_ptr(thin_color));
			}
		}
		glEnd();

		glDisable(GL_COLOR_MATERIAL);
	}

	glm::vec3 ToWorldVertex(int inX, int inY)
	{
		return glm::vec3(mScale * (float) inX, 0.0f, mScale * (float) inY);
	}

	int mGridWidth;
	int mGridLength;
	float mScale;
};

}

#endif