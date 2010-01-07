#include "Renderer.h"
#include "Time.h"
#include "Physics.h"
#include "TestVehicle.h"

Renderer renderer;

void Draw(float t)
{

}


void LoadOrLearnTurnRadii(Physics& physics, TestVehicle& vehicle, float dt)
{
	if (!vehicle.mSafeSteerForSpeedRP.Deserialize("SafeSteerForSpeedRP.bin")
		|| !vehicle.mSafeSteerTurnRadiusForSpeedRP.Deserialize("SafeSteerTurnRadiusForSpeedRP.bin"))
	{

		VehicleController_StableTurnRadiusLearn controller;
		controller.SetVehicle(&vehicle);
		controller.Init(&vehicle, 4.0f, 30.0f, 20, 1.0f, 0.1f, true); 
		
		float t = 0.0f;
		physics.Update(dt);
		while (!controller.IsFinished())
		{
			controller.Update(t, dt);
			physics.Update(dt);
			t+=dt;
		}

		vehicle.mSafeSteerForSpeedRP.Serialize("SafeSteerForSpeedRP.bin");
		vehicle.mSafeSteerTurnRadiusForSpeedRP.Serialize("SafeSteerTurnRadiusForSpeedRP.bin");
	}
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

	//Renderer renderer;
	renderer.InitVideo();

	Physics physics;
	physics.Init();

	TestVehicle vehicle;
	vehicle.Create(physics.physicsWorld);

	VehicleController* pController = NULL;
	VehicleController* pSteerController = NULL;

	float render_fps = 60.0f;
	float update_fps = 60.0f;

	LoadOrLearnTurnRadii(physics, vehicle, 1.0f / update_fps);

	/*
	VehicleController_Keyb controller;
	controller.SetVehicle(&vehicle);
	pController = &controller;
	*/
	
	/*
	VehicleController_TurnRadiusTest controller;
	controller.SetVehicle(&vehicle);
	controller.Init(&vehicle, 15.0f, 1.0f);
	pController = &controller;
	*/

	/*
	VehicleController_TurnRadiusLearn controller;
	controller.SetVehicle(&vehicle);
	controller.Init(&vehicle, 5.0f, 15.0f, 5, 1.0f);
	pController = &controller;
	*/
	
	/*
	VehicleController_StableSteerLearn controller;
	controller.SetVehicle(&vehicle);
	controller.Init(&vehicle, 5.0f, 0.1f, 0.05f);
	pController = &controller;
	*/

	/*
	VehicleController_StableTurnRadiusLearn controller;
	controller.SetVehicle(&vehicle);
	controller.Init(&vehicle, 15.0f, 30.0f, 15, 1.0f, 0.1f, false); 
	pController = &controller;
	*/
	
	/*
	VehicleController_HardBrakingDistanceLearn controller;
	controller.SetVehicle(&vehicle);
	controller.Init(&vehicle, 15.0f, 25.0f, 5);
	pController = &controller;
	*/

	/*
	VehicleController_NaiveSteer steer_controller;
	steer_controller.SetVehicle(&vehicle);
	steer_controller.SetMaxSpeed(90.0f/3.6f);
	steer_controller.SetFollowMouse(true);
	pSteerController = &steer_controller;
	vehicle.EnableHistory(true);
	*/

	VehicleController_LearnedNoSlideSteer steer_controller;
	steer_controller.SetVehicle(&vehicle);
	steer_controller.SetMaxSpeed(90.0f/3.6f);
	steer_controller.SetFollowMouse(true);
	pSteerController = &steer_controller;
	vehicle.EnableHistory(true);
	
	/*
	VehicleController_BasicSafetyTest controller;
	controller.SetVehicle(&vehicle);
	controller.Init(8.0f, 40.0f, 40.0f, 0.5f, 30.0f);
	pController = &controller;
	*/
	
	Vector2D fwd(0.0f, 1.0f);
	vehicle.Teleport(Vector2D(), &fwd);
	

	renderTimer.Start(globalTime, (int) render_fps);
	updateTimer.Start(globalTime, (int) update_fps);

	/*
	Poly2D test_poly;
	test_poly.AddPoint(Vector2D(0.0f, 0.0f));
	test_poly.AddPoint(Vector2D(0.0f, 4.0f));
	test_poly.AddPoint(Vector2D(4.0f, 8.0f));
	test_poly.AddPoint(Vector2D(8.0f, 2.0f));
	test_poly.AddPoint(Vector2D(2.0f, -6.0f));
	*/

	bool do_manual_test_path = true;
	bool do_load_test_path = true;
	PolyPath2D test_path;
	
	if (do_load_test_path)
	{
		test_path.Deserialize("path.bin");
	}

	PolyPath2D::ManualAddContext test_path_build_context;
	if (test_path.polys.size() == 0)
	{
		test_path.StartBuild(&test_path_build_context);
		if (do_manual_test_path == false)
		{
			test_path.AddStraight(Vector2D(0.0f, 0.0f), Vector2D(40.0f, 0.0f), Vector2D(0.0f, 14.0f), 3, true);
			test_path.AddArc(1, Vector2D(60.0f, 30.0f), 12.0f, 3.14f, 8);
			test_path.AddStraight(Vector2D(40.0f, 60.0f), Vector2D(-40.0f, 0.0f), Vector2D(0.0f, 14.0f), 3, true);
			test_path.EndBuild();
		}
	}
	else
	{
		do_manual_test_path = false;
	}
	
	
	while (is_running)
	{
		unsigned int startFrame;
		bool did_draw = false;

		if (renderTimer.Update(startFrame))
		{
			//printf("%f\n", renderTimer.GetTime());

			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderer.BeginRender();
			renderer.DrawPolyPath2DWorldToScreen(test_path, Color::kBlue);
			
			Draw(renderTimer.GetTime() - updateTimer.GetFrameTime());
			vehicle.Draw(renderer, renderTimer.GetTime() - updateTimer.GetFrameTime(), renderTimer.GetFrameTime());

			if (pController)
				pController->Draw(renderer, renderTimer.GetTime() - updateTimer.GetFrameTime());

			if (pSteerController)
				pSteerController->Draw(renderer, renderTimer.GetTime() - updateTimer.GetFrameTime());

			did_draw = true;
		}


		if (updateTimer.Update(startFrame))
		{
			unsigned int frames = updateTimer.GetDeltaFrameIndex();
			float dt = updateTimer.GetFrameTime();

			//printf ("%f\n", updateTimer.GetFrameLockedTime());
			for (unsigned int i = 0, frame = startFrame; i < frames; ++i)
			{
				float t = updateTimer.GetFrameSyncedTime(frame);

				if (pController)
				{
					pController->Update(t, dt);
				}

				if (pSteerController)
					pSteerController->Update(t, dt);

				//vehicle.steer(dt);
				physics.Update(dt);
				//Update(t, dt);
			}


			/*
			{
				int x; int y;
				SDL_GetMouseState(&x, &y);
				Vector2D worldPos = renderer.ScreenToWorld(Vector2D((float) x, (float) y));
			}
			*/

			float scroll_pixels_per_sec = 300.0f;

			Uint8* keyboardmap = SDL_GetKeyState(NULL);
			if (keyboardmap[SDLK_w])
			{
				renderer.mWorldTranslation.y -= renderer.ScreenToWorld(scroll_pixels_per_sec * (float) frames * updateTimer.GetFrameTime());
			}
			if (keyboardmap[SDLK_s])
			{
				renderer.mWorldTranslation.y += renderer.ScreenToWorld(scroll_pixels_per_sec * (float) frames * updateTimer.GetFrameTime());
			}
			if (keyboardmap[SDLK_a])
			{
				renderer.mWorldTranslation.x -= renderer.ScreenToWorld(scroll_pixels_per_sec * (float) frames * updateTimer.GetFrameTime());
			}
			if (keyboardmap[SDLK_d])
			{
				renderer.mWorldTranslation.x += renderer.ScreenToWorld(scroll_pixels_per_sec * (float) frames * updateTimer.GetFrameTime());
			}
		}

		if (did_draw)
		{

			renderer.EndRender();

			SDL_GL_SwapBuffers();
			SDL_Delay(2);
		}

		/*
		if (updateTimer.GetTime() >= 3.0f)
		{
			Vector2D fwd(0.5f, 0.5f);
			vehicle.Teleport(Vector2D(), &fwd);
		}
		*/

		{
			SDL_Event input_event;
			while(SDL_PollEvent(&input_event))
			{
				if (pController)
					pController->HandleEvent(input_event);
				
				if (input_event.type == SDL_KEYDOWN) 
				{
					switch (input_event.key.keysym.sym)
					{
					case SDLK_ESCAPE: 
						{
							is_running = false;
						}
						break;

					case SDLK_RETURN:
						{
							if (do_manual_test_path)
							{
								do_manual_test_path = false;
								test_path.EndBuild();
								test_path.Serialize("path.bin");
							}
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
				else if (input_event.type == SDL_MOUSEBUTTONDOWN)
				{
					if (do_manual_test_path)
					{
						int x; int y;
						SDL_GetMouseState(&x, &y);
						Vector2D worldPos = renderer.ScreenToWorld(Vector2D((float) x, (float) y));

						test_path.AddManualBuild(test_path_build_context, worldPos);
					}
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
