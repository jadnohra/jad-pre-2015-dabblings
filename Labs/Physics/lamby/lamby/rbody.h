#ifndef LAMBY_RBODY_H
#define LAMBY_RBODY_H

#include "gaussy.h"

class Thingies;
class PhysWorld;

struct RShape
{
	V2 v[8];
	int count;
	Rl r;
};

struct RBody
{
	V3 q;
	V3 v;
	int shape;
};
RBody* ctor(PhysWorld* th, RBody* el, const V3& q, const V3& v, int shape)  { el->q=q; el->v=v; el->shape=shape; return el; }
void dtor_rbody(void* th, RBody* el)  { }

void stepState(RBody& b, Rl dt)
{
	b.q = add( b.q, muls(b.v, dt) );
}


class PhysWorld
{
public:

	typedef ThingiesArr<RBody> RBodies;
	typedef RBodies::Iter RBodyIter;
	typedef RBodies::Add RBodyAdd;
	RBodies rbodies;

	RShape rshapes[128];

	Rl dt;
	V3 g;
	
	PhysWorld() : rbodies(32, dtor_rbody) 
	{
		dt = 1.0f/30.0f;
		g = muls(u_j(), -9.8f);

		int si=0;
		rshapes[si].v[0] = v2_z();
		rshapes[si].count = 1;
		rshapes[si].r = 0.5f;
		si++;

		rshapes[si].v[0] = V2(-0.5f, 0.0f);
		rshapes[si].v[1] = V2(0.5f, 0.0f);
		rshapes[si].v[2] = V2(0.0f, 0.5f);
		rshapes[si].count = 3;
		rshapes[si].r = 0.0f;
		si++;

		rshapes[si].v[0] = V2(-0.5f, 0.0f);
		rshapes[si].v[1] = V2(0.5f, 0.0f);
		rshapes[si].v[2] = V2(0.0f, 0.5f);
		rshapes[si].count = 3;
		rshapes[si].r = 0.05f;
		si++;

		rshapes[si].v[0] = V2(-0.5f, -0.5f);
		rshapes[si].v[1] = V2(-0.5f, 0.5f);
		rshapes[si].v[2] = V2(0.5f, 0.5f);
		rshapes[si].v[3] = V2(0.5f, -0.5f);
		rshapes[si].count = 4;
		rshapes[si].r = 0.0f;
		si++;

		rshapes[si].v[0] = V2(-0.5f, -0.5f);
		rshapes[si].v[1] = V2(-0.5f, 0.5f);
		rshapes[si].v[2] = V2(0.5f, 0.5f);
		rshapes[si].v[3] = V2(0.5f, -0.5f);
		rshapes[si].count = 4;
		rshapes[si].r = 0.05f;
		si++;
	}

	RBody* addRBody(const V3& q, const V3& v, int shape = 0)
	{
		RBodies::Add add(rbodies); RBody* el = add.add(); ctor(this, el, q, v, shape); return el;
	}

	RBody* iter(RBodies::Iter& it)
	{
		return it.next();
	}

	void applyGravity()
	{
		RBodies::Iter i(rbodies);
		RBody* b;

		while( b = iter(i) )
			b->v = add( b->v, muls(g, dt) );
	}

	void step(Rl dt)
	{
		RBodies::Iter i(rbodies);
		RBody* b;

		while( b = iter(i) )
			stepState(*b, dt);
	}
};

#endif
