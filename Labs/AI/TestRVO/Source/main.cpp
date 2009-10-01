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

class Object
{
public:

	virtual void Update(float time, float dt) {}
	virtual void Draw() {}
};

class MovingAgent
{
public:

	float pos[2];
	float radius;
	float vel[2];

	MovingAgent()
	{
		pos[0] = 100.0f;
		pos[1] = 100.0f;

		radius = 30.0f;

		vel[0] = 0.0f;
		vel[1] = 0.0f;
	}

	virtual void Update(float time, float dt) 
	{
		pos[0] += 200.0f * dt;
	}
	
	virtual void Draw() 
	{
		glColor4f(1.0, 1.0, 1.0, 0.75f);
		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);
		
		//glBegin(GL_TRIANGLE_FAN);
		//glVertex2f(pos[0], pos[1]);
		
		for (float angle = 0.0f; angle <= 2.0f * MATH_PIf; angle += (2.0f * MATH_PIf) / 32.0f)
		{
			glVertex2f(pos[0] + sinf(angle) * radius, pos[1] + cosf(angle) * radius);
		}

		glEnd();
	}
};


/*
float x1 = 10.0f;
float y1 = 10.0f;
float x2 = 40.0f;
float y2 = 40.0f;
*/

void Update(float time, float dt)
{
	/*
	float diff = 40.0f * dt;

	x1 += diff;
	y1 += diff;
	x2 += diff;
	y2 += diff;
	*/
}

/*
void Draw(float adjustTime)
{
	//printf("%f\n", x1);

	

	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_QUADS);

	float dd = 50.1f * adjustTime;
	dd = 0.0f;

	glVertex2f(x1 + dd, y1 + dd); glVertex2f(x2 + dd, y1 + dd); glVertex2f(x2 + dd, y2 + dd); glVertex2f(x1 + dd, y2 + dd);
	glEnd();

	
}
*/

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

	while (is_running)
	{
		unsigned int elapsed_millis  = SDL_GetTicks() - sdl_time_millis;
		

		if (elapsed_millis >= frame_tick_millis)
		{
			while (elapsed_millis >= frame_tick_millis)
			{
				elapsed_millis -= frame_tick_millis;
				time_millis += frame_tick_millis;
				float new_time = ((float) time_millis) / 1000.0f;

				agent.Update(new_time, frame_tick_secs);
				Update(new_time, frame_tick_secs);
				
				time = new_time;

				sdl_time_millis += frame_tick_millis;
				++frame_count;
			}

			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			agent.Draw();
			SDL_GL_SwapBuffers();
			//Draw(((float) elapsed_millis) / 1000.0f);
		}
		else
		{
			SDL_Delay(1);
		}
		

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