#ifndef LAB_SPATIAL2D_H
#define LAB_SPATIAL2D_H

#include "math_ops.h"
#include <vector>

struct ConvexShape2d
{
	std::vector<V2> v;
	Sc r;

	const V2* vp() const { return &(*v.begin()); }
};


#endif