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

void draw_feature(window2d::DrawContext& dc, M3p mat, const ConvexShape2d& shape, const sat::FeatureRef2D& fr, V3p col)
{
	if (fr.dim == 1)
	{
		draw_line( dc, mat, shape.v[fr.index[0]], shape.v[fr.index[1]], col );
	}
	else
	{
		draw_circle( dc, mat, shape.v[fr.index[0]], 0.05f, col );
	}
}

void draw_feature(window2d::DrawContext& dc, int shapeIndex, const sat::ContactProjection& proj, V3p col)
{
	if (proj.vertexCount[shapeIndex] == 2)
	{
		draw_line( dc, m3_id(), proj.vertices[shapeIndex][0], proj.vertices[shapeIndex][1], col );
	}
	else
	{
		draw_circle( dc, m3_id(), proj.vertices[shapeIndex][0], 0.05f, col );
	}
}

void updateScene1(window2d::WindowData& wd, WindowSimul2d& simul)
{
	using namespace sat;
	
	ConvexShape2d testShape; testShape.v.push_back(v2_z());
	M3 testMatrix = rigid(wd.cursor, 0.0f);
	if (0) // Segment
	{
		Sc scale = 0.4f;
		shapeConvex(testShape, scale*V2(-0.5f, 0.0f), scale*V2(0.5f, 0.0f), 0.0f);
		testMatrix = rigid(wd.cursor, 0.0f * Sc(simul.clock.time));
		draw_convex( wd.dc, testMatrix, testShape.vp(), testShape.vl(), testShape.r, u_k() );
	}
	if (0) // Triangle
	{
		Sc scale = 0.4f;
		shapeConvex(testShape, scale*V2(-0.5f, 0.0f), scale*V2(0.0f, 1.0f), scale*V2(0.5f, 0.0f), 0.0f);
		testMatrix = rigid(wd.cursor, 0.0f * Sc(simul.clock.time));
		draw_convex( wd.dc, testMatrix, testShape.vp(), testShape.vl(), testShape.r, u_k() );
	}
	if (1) // Square
	{
		Sc scale = 0.25f;
		shapeConvex(testShape, scale*V2(-0.5f, -0.5f), scale*V2(-0.5f, 0.5f), scale*V2(0.5f, 0.5f), scale*V2(0.5f, -0.5f), 0.0f);
		testMatrix = rigid(wd.cursor, 0.0f * Sc(simul.clock.time));
		draw_convex( wd.dc, testMatrix, testShape.vp(), testShape.vl(), testShape.r, u_k() );
	}
	
	
	if (wd.cursor_pressed)
	{
		int x=0;x;
	}

	for (auto & shape : simul.simul.shapes)
	{
		M3 shapeMatrix = m3_id();
		HyperplaneSep2d sep;
		V3 col = v3_z();

		if (dist(shapeMatrix, shape, testMatrix, testShape, sep))
		{
			if (sep.dist <= 0.0f) 
			{
				dist(shapeMatrix, shape, testMatrix, testShape, sep);
				draw_convex( wd.dc, shapeMatrix, shape.vp(), shape.vl(), shape.r, u_j() );

				if (0)
				{
					draw_feature( wd.dc, shapeMatrix, shape, sep.nearestFeature[0], u_ij() );
					draw_feature( wd.dc, testMatrix, testShape, sep.nearestFeature[1], u_ij() );
				}
				
				ContactProjection proj;
				contact_projection(shapeMatrix, shape, testMatrix, testShape, sep, proj);
				draw_feature( wd.dc, 0, proj, u_ik() );
			}
			else
			{
				int x=0;x;
			}
		}
		else
		{
			draw_convex( wd.dc, shapeMatrix, shape.vp(), shape.vl(), shape.r, u_i() );
		}
	}
}

void createScene1(Simul2d& s)
{
	ConvexShape2d shape;

	//shapeCircle(shape, 1.5f); s.shapes.push_back(shape);
	//shapeConvex(shape, V2(-0.5f, 0.0f), V2(0.0f, 1.0f), V2(0.5f, 0.0f), 0.0f); s.shapes.push_back(shape);
	shapeConvex(shape, V2(-0.5f, -0.5f), V2(-0.5f, 0.5f), V2(0.5f, 0.5f), V2(0.5f, -0.5f), 0.0f); s.shapes.push_back(shape);
}

#endif