#include <stdio.h>
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h" 

#define MATH_PIf 3.14159265f

 
bool InitVideo(int width = 640, int height = 480, unsigned int flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_DOUBLEBUF) 
{
	// Load SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
		return false;
	}
	atexit(SDL_Quit); // Clean it up nicely :)

	SDL_Surface* screen = SDL_SetVideoMode(width, height, 32, flags);
	if (screen == NULL) {
		fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	
	
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1); 
	// This does not seem to work, only the driver settings are usually used...
	int check2;
	int check3 = SDL_GL_GetAttribute( SDL_GL_SWAP_CONTROL, &check2 );
	
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 4 );
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	
	glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable( GL_TEXTURE_2D );

	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

	glViewport( 0, 0, width, height );

	glClear( GL_COLOR_BUFFER_BIT );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// For 2D pixel precise mode
	glTranslatef (0.375f, 0.375f, 0.0f);

	return true;
}

class Timer
{
public:

	unsigned int sdl_time;
	unsigned int frame_time;
	unsigned int frame_index;
	unsigned int rest_time;
	unsigned int delta_frame_index;
	float delta_time;
	float last_returned_time;

	void Start(unsigned int framesPerSec)
	{
		frame_time = 1000 / framesPerSec;
		sdl_time = SDL_GetTicks();
		frame_index = 0;
		rest_time = 0;
		delta_frame_index = 0;
		delta_time = 0.0f;
		last_returned_time = 0.0f;
	}

	bool Update()
	{
		unsigned int new_sdl_time = SDL_GetTicks();
		unsigned int elapsed = new_sdl_time - sdl_time;

		if (elapsed >= frame_time)
		{
			delta_frame_index = elapsed / frame_time;
			frame_index += delta_frame_index;
			rest_time = elapsed - (delta_frame_index * frame_time);

			sdl_time += delta_frame_index * frame_time;

			return true;
		}

		return false;
	}

	float GetFrameLockedTime()
	{
		return (float) (frame_index * frame_time) / 1000.0f;
	}

	float GetTime()
	{
		float curr_time = (GetFrameLockedTime() + ((float) rest_time / 1000.0f));

		delta_time = last_returned_time - curr_time;
		last_returned_time = curr_time;
		return curr_time;
	}

	float GetDeltaTime()
	{
		return delta_time;
	}

	unsigned int GetDeltaFrameIndex()
	{
		return delta_frame_index;
	}

	float GetFrameTime()
	{
		return (float) (frame_time) / 1000.0f;
	}
};

class Object
{
public:

	virtual void Update(float time, float dt) {}
	virtual void Draw() {}
};

class MovingAgent
{
public:

	float t;
	float pos[2];
	float radius;
	float vel[2];

	float last_pos[2];
	float last_t;

	MovingAgent()
	{
		pos[0] = 100.0f;
		pos[1] = 100.0f;

		radius = 30.0f;

		vel[0] = 350.0f;
		vel[1] = 300.0f;
	}

	virtual void Update(float time, float dt) 
	{
		last_t = t;
		last_pos[0] = pos[0];
		last_pos[1] = pos[1];

		t = time;
		pos[0] += vel[0] * dt;
		pos[1] += vel[1] * dt;
		
		if (pos[0] > 640.0f)
			vel[0] = -vel[0];
		else if (pos[0] < 0.0f)
			vel[0] = -vel[0];

		if (pos[1] > 480.0f)
			vel[1] = -vel[1];
		else if (pos[1] < 0.0f)
			vel[1] = -vel[1];
	}
	
	virtual void Draw(float time) 
	{
		float interp_pos[2];

		float lerp = (time - last_t) / (t - last_t);
		interp_pos[0] = last_pos[0] + lerp * (pos[0] - last_pos[0]);
		interp_pos[1] = last_pos[1] + lerp * (pos[1] - last_pos[1]);

		glColor4f(1.0, 1.0, 1.0, 0.75f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		
		//glBegin(GL_TRIANGLE_FAN);
		//glVertex2f(pos[0], pos[1]);
		
		for (float angle = 0.0f; angle <= 2.0f * MATH_PIf; angle += (2.0f * MATH_PIf) / 32.0f)
		{
			glVertex2f(interp_pos[0] + sinf(angle) * radius, interp_pos[1] + cosf(angle) * radius);
		}

		glEnd();
	}
};



int main(int argc, char *argv[])
{
	bool is_running = true;

	is_running &= InitVideo();

	unsigned int sdl_time_millis = SDL_GetTicks();
	unsigned int time_millis = 0;
	unsigned int draw_time_millis = 0;
	float time = 0.0f;
	float draw_time = 0.0f;
	float frame_rate = 60.0f;
	float draw_rate = 30.0f;
	float frame_tick_secs = 1.0f / frame_rate;
	float draw_tick_secs = 1.0f / draw_rate;
	unsigned int frame_tick_millis = (unsigned int) (frame_tick_secs * 1000.0f);
	unsigned int draw_tick_millis = (unsigned int) (draw_tick_secs * 1000.0f);
	unsigned int frame_count = 0;

	MovingAgent agent;

	sdl_time_millis = SDL_GetTicks();

	Timer renderTimer;
	Timer updateTimer;

	renderTimer.Start(60);
	updateTimer.Start(30);


	while (is_running)
	{
		if (renderTimer.Update())
		{
			//printf("%f\n", renderTimer.GetTime());

			glClearColor(0.0, 0.0, 0.0, 1.0f);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			agent.Draw(renderTimer.GetTime());
			SDL_GL_SwapBuffers();
		}

		if (updateTimer.Update())
		{
			unsigned int frames = updateTimer.GetDeltaFrameIndex();

			for (unsigned int i = 0; i < frames; ++i)
			{
				agent.Update(updateTimer.GetFrameLockedTime(), updateTimer.GetFrameTime());
			}
		}

		SDL_Delay(1);

		/*
		unsigned int ticks = SDL_GetTicks();
		unsigned int elapsed_millis  = ticks - sdl_time_millis;
		
		if (elapsed_millis >= frame_tick_millis)
		{
			glClearColor(0.0, 0.0, 0.0, 1.0f);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			agent.Draw();
			SDL_GL_SwapBuffers();

			//printf("%d\n", elapsed_millis);

			while (elapsed_millis >= frame_tick_millis)
			{
				elapsed_millis -= frame_tick_millis;
				time_millis += frame_tick_millis;
				float new_time = ((float) time_millis) / 1000.0f;

				agent.Update(new_time, frame_tick_secs);
				
				time = new_time;

				sdl_time_millis += frame_tick_millis;
				++frame_count;
			}

			{
				unsigned int newTicks = SDL_GetTicks();
				if (ticks != newTicks)
				{
					int x = 0;
				}
			}

			if (elapsed_millis > 1)
				printf("%d\n", elapsed_millis);

		

			//{
			//	unsigned int newTicks = SDL_GetTicks();
			//	if (ticks != newTicks)
			//	{
			//		glClearColor(1.0, 0.0, 0.0, 1.0f);
			//		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//		agent.Draw();
			//		SDL_GL_SwapBuffers();
//
			//		printf("%d\n", newTicks - ticks);
			//	}
			//}
			
			//if (elapsed_millis > 0)
				//printf("%d\n", elapsed_millis);
		
			SDL_Delay(1);
		}
		else
		{
			SDL_Delay(1);
		}
		
*/
		

		{
			SDL_Event input_event;
			while(SDL_PollEvent(&input_event))
			{
				if (input_event.type == SDL_KEYDOWN && input_event.key.keysym.sym == SDLK_ESCAPE) 
				{
					is_running = false;
				}
				else if (input_event.type == SDL_QUIT) 
				{
					is_running = false;
				}
			}
		}
	}

	return 0;
}
