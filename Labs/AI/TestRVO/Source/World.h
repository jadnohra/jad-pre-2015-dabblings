#ifndef WORLD_H
#define WORLD_H

#include <vector>
#include "Math.h"
#include "Renderer.h"

class ICollisionAvoidanceManager;
class Agent;
class App;
class Terrain;

class World
{
public:

	typedef std::vector<Agent*> Agents;
	Agents mAgents;
	Renderer mRenderer;
	ICollisionAvoidanceManager* mAvoidanceManager;
	Terrain* mTerrain;

	World();
	~World();

	void Add(Agent& agent);
	void Remove(Agent& agent);

	void Update(float time, float dt);
	void Draw(float time, Agent* pFocusAgent);

	Agent* PickAgent(const Vector2D& pos);
	Renderer& GetRenderer() { return mRenderer; }

	Vector2D WorldToScreen(const Vector2D& v)
	{
		return mRenderer.WorldToScreen(v);
	}

	float WorldToScreen(float v)
	{
		return mRenderer.WorldToScreen(v);
	}

	Vector2D ScreenToWorld(const Vector2D& v)
	{
		return mRenderer.ScreenToWorld(v);
	}

	Vector2D ScreenToWorldDir(const Vector2D& v)
	{
		return mRenderer.ScreenToWorldDir(v);
	}

	float ScreenToWorld(float v)
	{
		return mRenderer.ScreenToWorld(v);
	}
	
	void MainLoop(App& app);
};


#endif