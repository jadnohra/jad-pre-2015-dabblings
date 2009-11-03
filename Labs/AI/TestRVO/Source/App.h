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

	//http://colorschemedesigner.com/
	virtual Color GetBackgroundColor(World& world) { return Color::kBlack; }
	virtual Color GetDefaultAgentColor(World& world) { return Color::kWhite; }
	virtual Color GetTerrainElementColor(World& world) { return Color(0.0f, 0.0f, 0.5f); }
	virtual Color GetFocusedTerrainElementColor(World& world) { return Color(0.5f, 0.0f, 0.0f); }
	virtual Color GetObstacleColor(World& world) { return GetDefaultAgentColor(world); }
};

#endif