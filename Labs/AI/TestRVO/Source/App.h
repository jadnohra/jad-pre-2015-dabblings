#ifndef APP_H
#define APP_H

#include "World.h"

class App
{
public:

	virtual int OnStart(World& world, int version) { return version; }
	virtual void OnEnd(World& world) {}

	virtual bool Update(World& world, float time, float dt) { return true; }
	virtual void Draw(World& world) {}

	virtual Color GetBackgroundColor(World& world) { return Color::kBlack; }
	virtual Color GetDefaultAgentColor(World& world) { return Color::kWhite; }
	virtual Color GetTerrainElementColor(World& world) { return Color(40, 40, 40); }
	virtual Color GetFocusedTerrainElementColor(World& world) { return Color(120, 140, 120); }
	virtual Color GetObstacleColor(World& world) { return Color(100, 100, 140); }
};

#endif