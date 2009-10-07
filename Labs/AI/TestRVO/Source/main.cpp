#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "SDL.h"
#include "SDL_opengl.h" 

#define MATH_PIf 3.14159265f
int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;
static const float kGlobalAlphaMul = 0.75f;
static const float kWorldScale = 20.0f;
static const bool kLimitBounce = true;
static const float kTimeScale = 1.0f;

bool InitVideo(unsigned int flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_DOUBLEBUF) 
{
	int width = SCREEN_WIDTH; 
	int height = SCREEN_HEIGHT;

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
	
	
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0); 
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

struct Color
{
	union
	{
		struct 
		{
			float v[4];
		};

		struct 
		{
			float r;
			float g;
			float b;
			float a;
		};
	};

	Color(float r_ = 1.0f, float g_ = 1.0f, float b_ = 1.0f, float a_ = 1.0f)
	:	r(r_), g(g_), b(b_), a(a_)
	{
	}

	static const Color kBlack;
	static const Color kWhite;
	static const Color kRed;
	static const Color kBlue;
	static const Color kGreen;
};

const Color Color::kBlack(0.0f, 0.0f, 0.0f);
const Color Color::kWhite(1.0f, 1.0f, 1.0f);
const Color Color::kRed(1.0f, 0.0f, 0.0f);
const Color Color::kBlue(0.0f, 0.0f, 1.0f);
const Color Color::kGreen(0.0f, 1.0f, 0.0f);



struct Vector2D
{
	union
	{
		struct 
		{
			float v[2];
		};

		struct 
		{
			float x;
			float y;
		};
	};


	Vector2D(float v0 = 0.0f, float v1 = 0.0f)
		:	x(v0), y(v1)
	{
	}

	float& operator[](int index) { return v[index]; }

	Vector2D operator*(float mul) const
	{
		return Vector2D(x * mul, y * mul);
	}

	Vector2D operator+(float mul) const
	{
		return Vector2D(x + mul, y + mul);
	}

	Vector2D operator-(float mul) const
	{
		return Vector2D(x - mul, y - mul);
	}

	Vector2D operator*(Vector2D mul) const
	{
		return Vector2D(x * mul.x, y * mul.y);
	}

	Vector2D operator+(Vector2D mul) const
	{
		return Vector2D(x + mul.x, y + mul.y);
	}

	Vector2D operator-(Vector2D mul) const
	{
		return Vector2D(x - mul.x, y - mul.y);
	}

	Vector2D& operator*=(Vector2D mul) 
	{
		return (*this = Vector2D(x * mul.x, y * mul.y));
	}

	Vector2D& operator+=(Vector2D mul) 
	{
		return (*this = Vector2D(x + mul.x, y + mul.y));
	}

	Vector2D& operator-=(Vector2D mul) 
	{
		return (*this = Vector2D(x - mul.x, y - mul.y));
	}

	void Zero()
	{
		*this = kZero;
	}

	static const Vector2D kZero;
};
const Vector2D Vector2D::kZero(0.0f, 0.0f);

Vector2D ToWorld(const Vector2D& v)
{
	return (v * kWorldScale) + Vector2D(0.5f * (float) SCREEN_WIDTH, 0.5f * (float) SCREEN_HEIGHT);
}

float ToWorld(float v)
{
	return v * kWorldScale;
}

void DrawCircle(const Vector2D& v, float radius, const Color& color, float alpha)
{
	glColor4f(color.r, color.g, color.b, kGlobalAlphaMul * alpha);
	glLineWidth(2.0f);
	glBegin(GL_LINE_LOOP);

	//glBegin(GL_TRIANGLE_FAN);
	//glVertex2f(pos[0], pos[1]);

	for (float angle = 0.0f; angle <= 2.0f * MATH_PIf; angle += (2.0f * MATH_PIf) / 32.0f)
	{
		glVertex2f(v.x + sinf(angle) * radius, v.y + cosf(angle) * radius);
	}

	glEnd();
}

void DrawCircle(const Vector2D& v, float radius, const Color& color)
{
	DrawCircle(v, radius, color, color.a);
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




class Agent
{
public:

	Vector2D pos;
	Vector2D vel;
	float radius;

	float updateTime;
	Vector2D prevUpdatePos;
	float prevUpdateTime;
	Color color;

	Agent()
	{
		init(Vector2D::kZero, Vector2D::kZero, 1.0f, Color::kWhite);
	}

	Agent(const Vector2D& pos_, const Vector2D& vel_, float radius_, const Color color_)
	{
		init(pos_, vel_, radius_, color_);
	}

	void init(const Vector2D& pos_, const Vector2D& vel_, float radius_, const Color color_)
	{
		updateTime = 0.0f;
		prevUpdateTime = 0.0f;
		
		radius = radius_;
		vel = vel_; 
		pos = pos_;
		color = color_;
	}

	virtual const Vector2D& GetPos() { return pos; }
	virtual float GetRadius() { return radius; }

	virtual void Update(float time, float dt) 
	{
		prevUpdateTime = updateTime;
		prevUpdatePos = pos;

		if (prevUpdateTime == 0.0f)
			prevUpdateTime = time;

		updateTime = time;
		pos += vel * dt;

		if (kLimitBounce)
		{
			Vector2D worldPos = ToWorld(pos);

			if (worldPos[0] > (float) SCREEN_WIDTH)
				vel[0] = -vel[0];
			else if (worldPos[0] < 0.0f)
				vel[0] = -vel[0];

			if (worldPos[1] > (float) SCREEN_HEIGHT)
				vel[1] = -vel[1];
			else if (worldPos[1] < 0.0f)
				vel[1] = -vel[1];
		}
	}


	virtual void Draw(float time) 
	{
		float lerp = (time - prevUpdateTime) / (updateTime - prevUpdateTime);

		if (prevUpdateTime == updateTime || lerp < 0.0f)
			return;

		Vector2D lerpPos = prevUpdatePos + ((pos - prevUpdatePos) * lerp);

		DrawCircle(ToWorld(lerpPos), ToWorld(radius), color);
	}
};

class AgentManager
{
public:

	typedef std::vector<Agent*> Agents;
	Agents agents;

	void Add(Agent& agent)
	{
		agents.push_back(&agent);
	}
	
	void Remove(Agent& agent)
	{
		Agents::iterator it = std::find(agents.begin(), agents.end(), &agent);
		
		if (it != agents.end())
		{
			agents.erase(it);
			delete &agent;
		}
	}

	~AgentManager()
	{
		for (Agents::iterator it = agents.begin(); it != agents.end(); ++it)
		{
			Agent* pAgent = (*it);
			delete pAgent;
		}
	}

	void Update(float time, float dt) 
	{
		for (Agents::iterator it = agents.begin(); it != agents.end(); ++it)
		{
			(*it)->Update(time, dt);
		}

	}

	void Draw(float time) 
	{
		for (Agents::iterator it = agents.begin(); it != agents.end(); ++it)
		{
			(*it)->Draw(time);
		}
	}
};


int main(int argc, char *argv[])
{
	bool is_running = true;

	is_running &= InitVideo();

	Timer renderTimer;
	Timer updateTimer;

	renderTimer.Start(60);
	updateTimer.Start(30);

	AgentManager agents;
	agents.Add(*(new Agent(Vector2D::kZero, Vector2D(4.0f, -10.0f), 2.0f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(3.0f, 6.0f), Vector2D(16.5f, 1.5f), 2.5f, Color::kRed)));
	agents.Add(*(new Agent(Vector2D(-3.0f, 6.0f), Vector2D(2.5f, -15.5f), 1.5f, Color::kBlue)));

	while (is_running)
	{
		if (renderTimer.Update())
		{
			//printf("%f\n", renderTimer.GetTime());

			glClearColor(0.0, 0.0, 0.0, 1.0f);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			agents.Draw(renderTimer.GetTime() - updateTimer.GetFrameTime());
			SDL_GL_SwapBuffers();
		}

		if (updateTimer.Update())
		{
			unsigned int frames = updateTimer.GetDeltaFrameIndex();

			//printf ("%f\n", updateTimer.GetFrameLockedTime());
			for (unsigned int i = 0; i < frames; ++i)
			{
				agents.Update(updateTimer.GetFrameLockedTime(), updateTimer.GetFrameTime());
			}
		}

		

		SDL_Delay(1);

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
