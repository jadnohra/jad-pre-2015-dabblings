#ifndef LAMBY_RBODY_H
#define LAMBY_RBODY_H

class Thingies;

struct RBody
{
	V3 q;
	V3 v;
};
RBody* ctor(PhysWorld* th, RBody* el, const V3& q, const V3& v)  { el->q=q; el->v=v; return el; }
void dtor_rbody(void* th, RBody* el)  { }

void stepState(RBody& b, Rl dt)
{
	b.q = add( b.q, muls(b.v, dt) );
}


struct PhysWorld
{
	typedef ThingiesArr<RBody> RBodies;
	typedef RBodies::Iter RBodyIter;
	typedef RBodies::Add RBodyAdd;
	RBodies rbodies;

	Rl dt;
	V3 g;
	
	PhysWorld() : rbodies(32, dtor_rbody) 
	{
		dt = 1.0f/30.0f;
		g = muls(u_j(), -9.8f);
	}

	RBody* addRBody(const V3& q, const V3& v)
	{
		RBodies::Add add(rbodies); RBody* el = add.add(); ctor(this, el, q, v); return el;
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
};

#endif
