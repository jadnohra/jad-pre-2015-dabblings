#ifndef APP_H
#define APP_H

#include "World.h"

class App
{
public:

	virtual void OnStart(World& world) {}
	virtual void OnEnd(World& world) {}

	virtual bool Update(World& world, float time, float dt) { return true; }
	virtual void Draw(World& world) {}
};

#endif