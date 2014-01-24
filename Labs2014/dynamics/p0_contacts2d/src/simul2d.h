#ifndef LAB_SIMUL2D_H
#define LAB_SIMUL2D_H

#include "spatial2d.h"
#include "window2d.h"

struct Simul2d
{
	std::vector<ConvexShape2d> shapes;
};

class WindowSimul2d;
typedef void (*sceneSpdateFunc1)(Simul2d& simul);
typedef void (*sceneSpdateFunc2)(window2d::WindowData& wd, WindowSimul2d& simul);

class WindowSimul2d : public window2d::WindowClient
{
public:

	Simul2d simul;
	sceneSpdateFunc1 updateFunc1;
	sceneSpdateFunc2 updateFunc2;

	WindowSimul2d() : updateFunc1(0), updateFunc2(0) {}

	virtual void init(window2d::WindowData& wd)
	{
		wd.scale = 1.0f/10.0f;
	}

	virtual void render(window2d::WindowData& wd)
	{
		using namespace window2d;
		WindowClient::render(wd);

		for (auto & s : simul.shapes)
		{
			draw_convex( wd.dc, m3_id(), s.vp(), s.v.size(), s.r, u_ijk() );
		}

		if (updateFunc1) (*updateFunc1)(simul);
		if (updateFunc2) (*updateFunc2)(wd, *this);
	}
};

#endif