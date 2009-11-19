#include "Renderer.h"
#include "Time.h"
#include "Physics.h"
#include "TestVehicle.h"

void Draw(float t)
{

}

void Update(float t, float dt)
{

}


int main(int argc, char *argv[])
{
	bool is_running = true;
	bool draw_terrain = true;

	GlobalTime globalTime;
	Timer renderTimer;
	Timer updateTimer;

	unsigned int fpsLastTime = SDL_GetTicks();
	unsigned int frameCount = 0;

	Renderer renderer;
	renderer.InitVideo();

	Physics physics;
	physics.Init();

	TestVehicle vehicle;
	vehicle.Create(physics.physicsWorld);

	renderTimer.Start(globalTime, 60);
	updateTimer.Start(globalTime, 60);


	while (is_running)
	{
		unsigned int startFrame;

		if (renderTimer.Update(startFrame))
		{
			//printf("%f\n", renderTimer.GetTime());

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderer.BeginRender();
			
			Draw(renderTimer.GetTime() - updateTimer.GetFrameTime());
			vehicle.Draw(renderer, renderTimer.GetTime() - updateTimer.GetFrameTime(), renderTimer.GetFrameTime());
		}


		if (updateTimer.Update(startFrame))
		{
			unsigned int frames = updateTimer.GetDeltaFrameIndex();
			float dt = updateTimer.GetFrameTime();

			//printf ("%f\n", updateTimer.GetFrameLockedTime());
			for (unsigned int i = 0, frame = startFrame; i < frames; ++i)
			{
				float t = updateTimer.GetFrameSyncedTime(frame);

				vehicle.steer(dt);
				physics.Update(dt);
				Update(t, dt);
			}
		}

		renderer.EndRender();

		SDL_GL_SwapBuffers();
		SDL_Delay(2);

		{
			SDL_Event input_event;
			while(SDL_PollEvent(&input_event))
			{
				vehicle.HandleEvent(input_event);
				
				if (input_event.type == SDL_KEYDOWN) 
				{
					switch (input_event.key.keysym.sym)
					{
					case SDLK_ESCAPE: 
						{
							is_running = false;
						}
						break;

					case SDLK_PAGEUP:
						{
						}
						break;

					case SDLK_PAGEDOWN:
						{
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

	return 0;
}
