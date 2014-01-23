#ifndef LAB_SIMUL2D_H
#define LAB_SIMUL2D_H

#include "spatial2d.h"
#include "window2d.h"

struct Simul2d
{
	std::vector<ConvexShape2d> shapes;
};


class WindowSimul2d : public window2d::WindowClient
{
public:

	Simul2d simul;

	virtual void init(window2d::WindowData& wd)
	{
		wd.scale = 1.0f/10.0f;
	}

	virtual void render(window2d::WindowData& wd)
	{
		using namespace window2d;
		 WindowClient::render(wd);

		for (size_t i=0; i<simul.shapes.size(); ++i)
		{
			const ConvexShape2d& s = simul.shapes[i];
			draw_convex( wd.dc, m3_id(), s.vp(), s.v.size(), s.r, u_ij() );
		}
	}
};

#endif