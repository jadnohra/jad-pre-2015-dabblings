#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>

#include "SDL.h"
#include "SDL_opengl.h" 

#define MATH_PIf 3.14159265f
int SCREEN_WIDTH = 1024;
int SCREEN_HEIGHT = 768;
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

	Vector2D operator*(const Vector2D& mul) const
	{
		return Vector2D(x * mul.x, y * mul.y);
	}

	Vector2D operator+(const Vector2D& mul) const
	{
		return Vector2D(x + mul.x, y + mul.y);
	}

	Vector2D operator-(const Vector2D& mul) const
	{
		return Vector2D(x - mul.x, y - mul.y);
	}

	Vector2D& operator*=(const Vector2D& mul) 
	{
		return (*this = Vector2D(x * mul.x, y * mul.y));
	}

	Vector2D& operator+=(const Vector2D& mul) 
	{
		return (*this = Vector2D(x + mul.x, y + mul.y));
	}

	Vector2D& operator-=(const Vector2D& mul) 
	{
		return (*this = Vector2D(x - mul.x, y - mul.y));
	}

	bool operator==(const Vector2D& comp) const
	{
		return x == comp.x && y == comp.y;
	}

	void Zero()
	{
		*this = kZero;
	}

	float Length()
	{
		return sqrtf(x*x + y*y);
	}

	Vector2D& Normalize()
	{
		float length = Length();

		if (length > 0.0f)
			(*this = *this * (1.0f / Length()));
		else
			Zero();

		return *this;
	}

	static const Vector2D kZero;
};
const Vector2D Vector2D::kZero(0.0f, 0.0f);

float Dot(const Vector2D& p1, const Vector2D& p2)
{
	return p1.x * p2.x + p1.y * p2.y;
}

float Distance(const Vector2D& p1, const Vector2D& p2)
{
	Vector2D d = p2 - p1;

	return sqrtf(Dot(d, d));
}

Vector2D WorldToScreen(const Vector2D& v)
{
	return (v * kWorldScale) + Vector2D(0.5f * (float) SCREEN_WIDTH, 0.5f * (float) SCREEN_HEIGHT);
}

float WorldToScreen(float v)
{
	return v * kWorldScale;
}

Vector2D ScreenToWorld(const Vector2D& v)
{
	return (v - Vector2D(0.5f * (float) SCREEN_WIDTH, 0.5f * (float) SCREEN_HEIGHT)) * (1.0f / kWorldScale);
}

Vector2D ScreenToWorldDir(const Vector2D& v)
{
	return (v) * (1.0f / kWorldScale);
}

float ScreenToWorld(float v)
{
	return v / kWorldScale;
}


Vector2D rotate(const Vector2D& v, float rads)
{
	return Vector2D(cosf(rads) * v.x - sinf(rads) * v.y, cosf(rads) * v.y + sinf(rads) * v.x);
}

void DrawCircle(const Vector2D& v, float radius, const Color& color, float alpha = -1.0f)
{
	if (alpha < 0.0f)
		alpha = color.a;

	glColor4f(color.r, color.g, color.b, kGlobalAlphaMul * alpha);
	glLineWidth(3.0f);
	glBegin(GL_LINE_LOOP);

	//glBegin(GL_TRIANGLE_FAN);
	//glVertex2f(pos[0], pos[1]);

	for (float angle = 0.0f; angle <= 2.0f * MATH_PIf; angle += (2.0f * MATH_PIf) / 32.0f)
	{
		glVertex2f(v.x + sinf(angle) * radius, v.y + cosf(angle) * radius);
	}

	glEnd();
}


void DrawLine(const Vector2D& p1, const Vector2D& p2, const Color& color, float alpha = -1.0f)
{
	if (alpha < 0.0f)
		alpha = color.a;

	glColor4f(color.r, color.g, color.b, kGlobalAlphaMul * alpha);
	glLineWidth(1.0f);
	glBegin(GL_LINE_LOOP);

	glVertex2f(p1.x, p1.y);
	glVertex2f(p2.x, p2.y);

	glEnd();
}


void DrawArrow(const Vector2D& p1, const Vector2D& p2, const Color& color, float alpha = -1.0f)
{
	DrawLine(p1, p2, color, alpha);
	
	float lineLength = (p2 - p1).Length();
	float headSize = 0.03f * lineLength;

	Vector2D dir1 = (p2 - p1).Normalize();
	Vector2D dir2 = rotate(dir1, 0.5f * MATH_PIf);

	Vector2D offset1 = dir1 * -(2.0f * headSize) + dir2 * headSize;
	Vector2D offset2 = dir1 * -(2.0f * headSize) + dir2 * -headSize;

	DrawLine(p2, p2 + offset1, color, alpha);
	DrawLine(p2, p2 + offset2, color, alpha);
}


class GlobalTime
{
public:

	unsigned int pauseTime;
	bool isPaused;
	unsigned int millisAtPauseStart;
	unsigned int currPauseStartTime;

	GlobalTime()
	:	pauseTime(SDL_GetTicks())
	,	isPaused(false)
	{

	}

	unsigned int GetMillis()
	{
		if (isPaused)
			return millisAtPauseStart;

		return SDL_GetTicks() - pauseTime;
	}

	void pause()
	{
		if (!isPaused)
		{
			millisAtPauseStart = GetMillis();
			isPaused = true;
			currPauseStartTime = SDL_GetTicks();
		}
	}

	void resume()
	{
		if (isPaused)
		{
			isPaused = false;
			pauseTime += SDL_GetTicks() - currPauseStartTime;
		}
	}

	void stepPaused(unsigned int millis)
	{
		currPauseStartTime += millis;
		millisAtPauseStart += millis;
	}
};

class Timer
{
public:

	GlobalTime* pGlobalTime;
	unsigned int sdl_time;
	unsigned int frame_time;
	unsigned int frame_index;
	unsigned int rest_time;
	unsigned int delta_frame_index;
	float delta_time;
	float last_returned_time;

	void Start(GlobalTime& globalTime, unsigned int framesPerSec)
	{
		pGlobalTime = &globalTime;
		frame_time = 1000 / framesPerSec;
		sdl_time = pGlobalTime->GetMillis();
		frame_index = 0;
		rest_time = 0;
		delta_frame_index = 0;
		delta_time = 0.0f;
		last_returned_time = 0.0f;
	}

	bool Update()
	{
		unsigned int new_sdl_time = pGlobalTime->GetMillis();
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
	virtual const Vector2D& GetVel() { return vel; }
	virtual float GetRadius() { return radius; }


	virtual void SetPos(const Vector2D& position) { pos = position; }
	virtual void SetVel(const Vector2D& velocity) { vel = velocity; }

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
			Vector2D worldPos = WorldToScreen(pos);

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

		DrawCircle(WorldToScreen(lerpPos), WorldToScreen(radius), color);

		if (!(vel == Vector2D::kZero))
		{
			DrawArrow(WorldToScreen(lerpPos), WorldToScreen(lerpPos + (vel * 0.4f)), color);
		}
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

	Agent* PickAgent(const Vector2D& pos)
	{
		for (Agents::iterator it = agents.begin(); it != agents.end(); ++it)
		{
			Agent* pAgent = *it;
			
			if (Distance(pAgent->GetPos(), pos) <= pAgent->GetRadius())
			{
				return pAgent;
			}
		}

		return NULL;
	}
};

class SceneController
{
public:

	AgentManager& m_AgentManager;
	Agent* m_pAgent;
	bool m_IsPressed;
	Uint32 m_StartPressTime;
	Vector2D m_MoveAvgVel;
	Vector2D m_AgentCenterOffset;

	SceneController(AgentManager& agentManager)
	:	m_AgentManager(agentManager)
	,	m_pAgent(NULL)
	,	m_IsPressed(false)
	{
	}

	void Update()
	{
		int x;
		int y;
		int relX;
		int relY;

		if (m_pAgent)
		{
			if (SDL_GetMouseState(&x, &y)&SDL_BUTTON(1))
			{
				Vector2D worldPos = ScreenToWorld(Vector2D((float) x, (float) y));
				m_pAgent->SetPos(worldPos + m_AgentCenterOffset);

				SDL_GetRelativeMouseState(&relX, &relY);
				Vector2D worldPosDiff = ScreenToWorldDir(Vector2D((float) relX, (float) relY));

				if (m_IsPressed)
				{
					m_MoveAvgVel += worldPosDiff;
					//printf("%f,%f\n", m_MoveAvgVel.x, m_MoveAvgVel.y);
				}
				else
				{
					m_IsPressed = true;
					m_MoveAvgVel = Vector2D::kZero;
					m_StartPressTime = SDL_GetTicks();
				}
			}
			else
			{
				float pressTime = ((float) (SDL_GetTicks() - m_StartPressTime)) / 1000.0f;

				Vector2D vel = m_MoveAvgVel * (1.0f / pressTime);
				m_pAgent->SetVel(vel);
				m_pAgent = NULL;
				m_IsPressed = false;
			}
		}
	}

	void HandleEvent(const SDL_Event& evt)
	{
		if (evt.type == SDL_MOUSEMOTION)
		{
			int x = evt.motion.x;
			int y = evt.motion.y;
		}

		if (evt.type == SDL_MOUSEBUTTONDOWN)
		{
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				int x = evt.motion.x;
				int y = evt.motion.y;

				Vector2D worldPos = ScreenToWorld(Vector2D((float) x, (float) y));
				Agent* pAgent = m_AgentManager.PickAgent(worldPos);

				if (pAgent != NULL)
				{
					m_pAgent = pAgent;

					m_AgentCenterOffset = m_pAgent->GetPos() - worldPos;
					m_pAgent->SetVel(Vector2D::kZero);
				}
			}
		}
	}
};


int main(int argc, char *argv[])
{
	bool is_running = true;

	is_running &= InitVideo();

	GlobalTime globalTime;
	Timer renderTimer;
	Timer updateTimer;

	renderTimer.Start(globalTime, 1000);
	updateTimer.Start(globalTime, 30);

	AgentManager agents;
	agents.Add(*(new Agent(Vector2D::kZero, Vector2D::kZero/*Vector2D(4.0f, -10.0f)*/, 2.0f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(3.0f, 6.0f), Vector2D(16.5f, 1.5f), 2.5f, Color::kRed)));
	agents.Add(*(new Agent(Vector2D(-3.0f, 6.0f), Vector2D(2.5f, -15.5f), 1.5f, Color::kBlue)));

	SceneController sceneController(agents);

	unsigned int fpsLastTime = SDL_GetTicks();
	unsigned int frameCount = 0;

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

		sceneController.Update();
		

		SDL_Delay(2);


		{
			SDL_Event input_event;
			while(SDL_PollEvent(&input_event))
			{
				sceneController.HandleEvent(input_event);

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
