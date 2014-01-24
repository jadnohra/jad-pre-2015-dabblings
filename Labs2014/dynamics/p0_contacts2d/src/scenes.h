#ifndef LAB_SCENES_H
#define LAB_SCENES_H

#include "simul2d.h"
#include "spatial2d.h"

void shapeCircle(ConvexShape2d& s, Sc r)
{
	s.v.clear();
	s.v.push_back(v2_z());
	s.r = r;
}

void shapeConvex(ConvexShape2d& s, V2p v0, Sc r)
{
	s.v.clear();
	s.v.push_back(v0);
	s.r = r;
}

void shapeConvex(ConvexShape2d& s, V2p v0, V2p v1, Sc r)
{
	s.v.clear();
	s.v.push_back(v0); s.v.push_back(v1);
	s.r = r;
}

void shapeConvex(ConvexShape2d& s, V2p v0, V2p v1, V2p v2, Sc r)
{
	s.v.clear();
	s.v.push_back(v0); s.v.push_back(v1); s.v.push_back(v2);
	s.r = r;
}

void shapeConvex(ConvexShape2d& s, V2p v0, V2p v1, V2p v2, V2p v3, Sc r)
{
	s.v.clear();
	s.v.push_back(v0); s.v.push_back(v1); s.v.push_back(v2); s.v.push_back(v3);
	s.r = r;
}

void shapeConvex(ConvexShape2d& s, const V2* v, int cnt, Sc r)
{
	s.v.clear();
	for (int i=0; i<cnt; ++i) s.v.push_back(v[i]);
	s.r = r;
}



void createScene0(Simul2d& s)
{
	ConvexShape2d shape;

	shapeCircle(shape, 1.5f); s.shapes.push_back(shape);
	shapeConvex(shape, V2(-0.5f, 0.0f), V2(0.0f, 1.0f), V2(0.5f, 0.0f), 0.0f); s.shapes.push_back(shape);
	shapeConvex(shape, V2(-0.5f, -0.5f), V2(-0.5f, 0.5f), V2(0.5f, 0.5f), V2(0.5f, -0.5f), 0.0f); s.shapes.push_back(shape);
	shapeConvex(shape, V2(-0.5f, -0.5f), V2(-0.5f, 0.5f), V2(0.5f, 0.5f), V2(0.5f, -0.5f), 0.2f); s.shapes.push_back(shape);
}

void updateScene1(window2d::WindowData& wd, WindowSimul2d& simul)
{
	using namespace sat;
	
	ConvexShape2d testShape; testShape.v.push_back(wd.cursor);

	if (wd.cursor_pressed)
	{
		int x=0;x;
	}

	for (auto & shape : simul.simul.shapes)
	{
		HyperplaneSep2d sep;
		V3 col = v3_z();

		if (dist(m3_id(), shape, m3_id(), testShape, sep))
		{
			if (sep.dist <= 0.0f) 
				col = u_j();
			else
				col = v3_z();
		}
		else
		{
			col = u_i();
		}

		if (col != v3_z())
			draw_convex( wd.dc, m3_id(), shape.vp(), shape.vl(), shape.r, col );
	}
}

void createScene1(Simul2d& s)
{
	ConvexShape2d shape;

	shapeCircle(shape, 1.5f); s.shapes.push_back(shape);
	shapeConvex(shape, V2(-0.5f, 0.0f), V2(0.0f, 1.0f), V2(0.5f, 0.0f), 0.0f); s.shapes.push_back(shape);
	shapeConvex(shape, V2(-0.5f, -0.5f), V2(-0.5f, 0.5f), V2(0.5f, 0.5f), V2(0.5f, -0.5f), 0.0f); s.shapes.push_back(shape);
}

#endif