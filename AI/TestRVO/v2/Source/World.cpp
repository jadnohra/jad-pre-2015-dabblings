#include "Agent.h"
#include "World.h"
#include "CollisionAvoidanceManager.h"
#include "Time.h"
#include "WorldController.h"
#include "Terrain.h"
#include "App.h"


World::World()
: mAvoidanceManager(new CollisionAvoidanceManager_RobustWait_ReactiveDeadlockResolve())
, mTerrain(new Terrain())
, mApp(NULL)	
{
}

World::~World()
{
	for (Agents::iterator it = mAgents.begin(); it != mAgents.end(); ++it)
	{
		Agent* pAgent = (*it);
		delete pAgent;
	}

	delete mAvoidanceManager;
	delete mTerrain;
}

void World::Add(Agent& agent)
{
	mAgents.push_back(&agent);

	if (mAvoidanceManager)
	{
		mAvoidanceManager->AddAgent(&agent, (int) mAgents.size());
	}

	if (mTerrain)
	{
		mTerrain->AddAgent(&agent);
	}
}

void World::Remove(Agent& agent)
{
	Agents::iterator it = std::find(mAgents.begin(), mAgents.end(), &agent);
	
	if (it != mAgents.end())
	{
		mAgents.erase(it);
		delete &agent;

		if (mAvoidanceManager)
		{
			mAvoidanceManager->RemoveAgent(&agent);
		}
	}
}


void World::Update(float time, float dt) 
{
	for (Agents::iterator it = mAgents.begin(); it != mAgents.end(); ++it)
	{
		(*it)->Update(time, dt);
	}

}

void World::Draw(float time, Agent* pFocusAgent) 
{
	for (Agents::iterator it = mAgents.begin(); it != mAgents.end(); ++it)
	{
		(*it)->Draw(time, pFocusAgent);
	}
}

Agent* World::PickAgent(const Vector2D& pos)
{
	for (Agents::iterator it = mAgents.begin(); it != mAgents.end(); ++it)
	{
		Agent* pAgent = *it;
		
		if (Distance(pAgent->GetPos(), pos) <= pAgent->GetRadius())
		{
			return pAgent;
		}
	}

	return NULL;
}


void World::MainLoop(App& app)
{
	bool is_running = true;
	bool draw_terrain = false;
	mApp = &app;

	is_running &= mRenderer.InitVideo();

	b2AABB terrain_limits;
	terrain_limits.lowerBound.Set(-100, -100);
	terrain_limits.upperBound.Set(100, 100);
	mTerrain->Init(terrain_limits, false);
	
	WorldController worldController(*this, mAvoidanceManager);

	GlobalTime globalTime;
	Timer renderTimer;
	Timer updateTimer;

	renderTimer.Start(globalTime, 1000);
	updateTimer.Start(globalTime, 30);

	
	unsigned int fpsLastTime = SDL_GetTicks();
	unsigned int frameCount = 0;

	app.OnStart(*this);

	while (is_running)
	{
		unsigned int startFrame;
		
		if (renderTimer.Update(startFrame))
		{
			//printf("%f\n", renderTimer.GetTime());

			Color clear_color = app.GetBackgroundColor(*this);
			Color terrain_el_color = app.GetTerrainElementColor(*this);
			Color active_terrain_el_color = app.GetFocusedTerrainElementColor(*this);
			Color obstacle_color = app.GetObstacleColor(*this);

			glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.0f);

			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (draw_terrain)
				mTerrain->DrawWaypoints(*this, true, terrain_el_color, terrain_el_color);

			mTerrain->DrawObstacles(*this, obstacle_color, 3.0f);

			if (worldController.mpFocusAgent)
				mTerrain->DrawTerrainInfo(*this, worldController.mpFocusAgent, active_terrain_el_color, active_terrain_el_color);

			Draw(renderTimer.GetTime() - updateTimer.GetFrameTime(), worldController.mpFocusAgent);
			worldController.Draw();
			app.Draw(*this);
			SDL_GL_SwapBuffers();
		}
	

		if (updateTimer.Update(startFrame))
		{
			unsigned int frames = updateTimer.GetDeltaFrameIndex();
			float dt = updateTimer.GetFrameTime();

			//printf ("%f\n", updateTimer.GetFrameLockedTime());
			for (unsigned int i = 0, frame = startFrame; i < frames; ++i)
			{
				float t = updateTimer.GetFrameSyncedTime(frame);
				
				mAvoidanceManager->Update(t, dt);
				Update(t, dt);
				mTerrain->Update(t, dt);
				if (!app.Update(*this, t, dt))
				{
					is_running = false;
					break;
				}
			}
		}

		worldController.Update();
		
		SDL_Delay(2);

		{
			SDL_Event input_event;
			while(SDL_PollEvent(&input_event))
			{
				worldController.HandleEvent(input_event);

				if (input_event.type == SDL_KEYDOWN) 
				{
					switch (input_event.key.keysym.sym)
					{
						case SDLK_ESCAPE: 
						{
							is_running = false;
						}
						break;

						case SDLK_p:
						{
							if (globalTime.isPaused)
							{
								globalTime.resume();
							}
							else
							{
								globalTime.pause();
							}
							
						}
						break;

						case SDLK_w:
						{
							draw_terrain = !draw_terrain;
						}
						break;

						case SDLK_o:
						{
							globalTime.stepPaused((unsigned int) (1000.0f * updateTimer.GetFrameTime()));
						}
						break;
					}

					
				}
				else if (input_event.type == SDL_QUIT) 
				{
					is_running = false;
				}
			}
		}

		++frameCount;
		if (SDL_GetTicks() - fpsLastTime >= 1000)
		{
			//printf("%d\n", frameCount);

			frameCount = 0;
			fpsLastTime = SDL_GetTicks();
		}
	}

	app.OnEnd(*this);
}