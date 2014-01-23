#ifndef LAB_SCENES_H
#define LAB_SCENES_H

#include "simul2d.h"

void shapeCircle(ConvexShape2d& s, Sc r)
{
	s.v.clear();
	s.v.push_back(v2_z());
	s.r = r;
}

void createScene0(Simul2d& s)
{
	ConvexShape2d shape;

	shapeCircle(shape, 1.0f); s.shapes.push_back(shape);
	shapeCircle(shape, 2.0f); s.shapes.push_back(shape);
}

#endif