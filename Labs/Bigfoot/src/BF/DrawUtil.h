#ifndef _INCLUDED_BIGFOOT_DRAWUTIL_H
#define _INCLUDED_BIGFOOT_DRAWUTIL_H

#include "BFMath.h"	
#include "BE/OGL.h"

namespace BF
{

	void DrawCircle(const float x, const float y, const float z,
					const float r)
	{
		static const int NUM_SEG = 40;
		static float dir[40*2];
		static bool init = false;
		if (!init)
		{
			init = true;
			for (int i = 0; i < NUM_SEG; ++i)
			{
				const float a = (float)i/(float)NUM_SEG*BE::M_PI*2;
				dir[i*2] = cosf(a);
				dir[i*2+1] = sinf(a);
			}
		}
		
		for (int i = 0, j = NUM_SEG-1; i < NUM_SEG; j = i++)
		{
			glVertex3f(x+dir[j*2+0]*r, y, z+dir[j*2+1]*r);
			glVertex3f(x+dir[i*2+0]*r, y, z+dir[i*2+1]*r);
		}
	}
}

#endif