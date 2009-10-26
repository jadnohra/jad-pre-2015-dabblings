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
static const float kVelDrawScale = 0.4f;

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

	bool operator!=(const Vector2D& comp) const
	{
		return !(*this == comp);
	}

	void Zero()
	{
		*this = kZero;
	}

	float Length() const
	{
		return sqrtf(x*x + y*y);
	}

	void Normalize()
	{
		*this = Normalized();
	}

	Vector2D Normalized() const
	{
		float length = Length();

		if (length > 0.0f)
			return (*this * (1.0f / length));
		else
			return kZero;
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

float Randf(float min, float max)
{
	return ( 0.1f + (float)rand()/0x7fff )* (max-min) + min;
}

int IntersectLineCircle(const Vector2D& linePos, const Vector2D& lineDir, 
						const Vector2D& circlePos, float circleRadius,
						float& t, float & u)
{
	Vector2D f = linePos - circlePos;

	float a = Dot(lineDir, lineDir);
	float b = 2.0f*Dot(f, lineDir);
	float c = Dot(f, f) - circleRadius*circleRadius;

	float discriminant = b*b-4*a*c;
	if( discriminant < 0 )
	{
	  // no intersection
		return 0;
	}
	else
	{
	  // ray didn't totally miss sphere,
	  // so there is a solution to
	  // the equation.


	  discriminant = sqrtf( discriminant );
	  t = (-b - discriminant)/(2.0f*a);
	  u = (-b + discriminant)/(2.0f*a);
	  

	  return discriminant == 0.0f ? 1 : 2;
	}
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

	Vector2D dir1 = (p2 - p1).Normalized();
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

	bool isAgitated;
	float agitation;

	Agent()
	{
		isAgitated = false;
		agitation = 0.0f;

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

	virtual void Agitate()			 { isAgitated = true; }
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

		if (isAgitated)
		{
			agitation += dt;
		}
		else
		{
			agitation = 0.0f;
		}
		

		isAgitated = false;
	}


	virtual void Draw(float time) 
	{
		float lerp = (time - prevUpdateTime) / (updateTime - prevUpdateTime);

		if (prevUpdateTime == updateTime || lerp < 0.0f)
			return;

		Vector2D lerpPos = prevUpdatePos + ((pos - prevUpdatePos) * lerp);
		float lerpRadius = radius;
		Color lerpColor = color;

		if (agitation != 0.0f)
		{
			Vector2D agitationVector;
			
			while (agitationVector == Vector2D::kZero)
			{
				agitationVector = Vector2D(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f));
			}

			agitationVector.Normalize();
			float factor = std::min(0.5f, (agitation * agitation * 0.5f));
			agitationVector *= GetRadius() * factor;

			lerpPos += agitationVector;

			float agitationRadius = GetRadius() * Randf(-0.1f, 0.1f);

			lerpRadius += agitationRadius;

			lerpColor.g = std::max(0.0f, color.g - factor);
			lerpColor.b = std::max(0.0f, color.b - factor);
			lerpColor.r = std::min(1.0f, color.r + factor);
		}

		DrawCircle(WorldToScreen(lerpPos), WorldToScreen(lerpRadius), lerpColor);

		if (!(vel == Vector2D::kZero))
		{
			DrawArrow(WorldToScreen(lerpPos), WorldToScreen(lerpPos + (vel * kVelDrawScale)), lerpColor);
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


class ICollisionAvoidanceManager
{
public:

	virtual ~ICollisionAvoidanceManager() {}

	virtual void AutoAddAgents(AgentManager& agentMan) = 0;
	virtual void AddAgent(Agent* pAgent, int priority) = 0;
	virtual void ResetAgentState(Agent* pAgent) = 0;
	virtual void Update(float time, float dt) = 0;
};


class CollisionAvoidanceManager_SignlePriorityWait : public ICollisionAvoidanceManager
{
public:

	struct AgentInfo
	{
		Agent* pAgent;
		int priority;
		bool shouldWait;
		bool isWaiting;
		float startWaitTime;
		Vector2D vel;

		AgentInfo(Agent* pAgent_ = NULL, int priority_ = -1)
		:	pAgent(pAgent_)
		,	priority(priority_)
		{
			Reset();
		}

		void Reset()
		{
			shouldWait = false;
			isWaiting = false;
			startWaitTime = -1.0f;
		}
	};

	typedef std::vector<AgentInfo> AgentInfos;

	AgentInfos m_AgentInfos;
	bool m_AgentInfosIsDirty;

	CollisionAvoidanceManager_SignlePriorityWait()
	: m_AgentInfosIsDirty(false)
	{
	}

	virtual void AutoAddAgents(AgentManager& agentMan)
	{
		m_AgentInfos.resize(agentMan.agents.size());

		for (int i = 0; i < agentMan.agents.size(); ++i)
		{
			m_AgentInfos[i].pAgent = agentMan.agents[i];
			m_AgentInfos[i].priority = i;
			m_AgentInfos[i].Reset();
		}
	}

	virtual void AddAgent(Agent* pAgent, int priority)
	{
		m_AgentInfos.push_back(AgentInfo(pAgent, priority));
		m_AgentInfosIsDirty = true;
	}

	virtual void ResetAgentState(Agent* pAgent)
	{
		// TODO
	}

	static bool HasHigherPriority(const AgentInfo& info, const AgentInfo& compInfo)
	{
		return compInfo.priority > info.priority;
	}

	void UpdateAgentInfos()
	{
		std::sort(m_AgentInfos.begin(), m_AgentInfos.end(), HasHigherPriority);
	}

	virtual void Update(float time, float dt)
	{
		UpdateAgentInfos();

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			m_AgentInfos[i].shouldWait = false;

			if (!m_AgentInfos[i].isWaiting)
			{
				for (int j = i+1; j < m_AgentInfos.size(); ++j)
				{
					PerformCollisionAvoidance(m_AgentInfos[i], m_AgentInfos[j], time);

					if (m_AgentInfos[i].shouldWait)
						break;
				}
			}
		}

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = m_AgentInfos[i];

			if (agentInfo.shouldWait || 
				(agentInfo.isWaiting && (time - agentInfo.startWaitTime) < 0.5f))
			{
				if (agentInfo.isWaiting == false)
				{
					agentInfo.vel = agentInfo.pAgent->GetVel();
					agentInfo.pAgent->SetVel(Vector2D::kZero);
					agentInfo.isWaiting = true;
				}
			}
			else
			{
				if (agentInfo.isWaiting)
				{
					agentInfo.isWaiting = false;
					agentInfo.pAgent->SetVel(agentInfo.vel);
				}
			}
		}
	}

	void PerformCollisionAvoidance(AgentInfo& lowAgent, AgentInfo& highAgent, float time)
	{
		if (lowAgent.shouldWait || lowAgent.pAgent->GetVel() == Vector2D::kZero)
			return;

		//if (highAgent.pAgent->GetVel() == Vector2D::kZero)
		//	return;

		Vector2D relVel = lowAgent.pAgent->GetVel() - highAgent.pAgent->GetVel();
		float relSpeed = relVel.Length();

		if (relSpeed > 0.0f)
		{
			Vector2D relVelDir = relVel.Normalized();
			float t, u;

			if (IntersectLineCircle(lowAgent.pAgent->GetPos(), relVelDir, highAgent.pAgent->GetPos(), lowAgent.pAgent->GetRadius() + highAgent.pAgent->GetRadius(), t, u) > 0) 
			{
				if (t < 0.0f)
					t = u;

				float intersectionT = std::min(t, u);

				float timeUntilCollision = intersectionT / relSpeed;

				if (timeUntilCollision >= 0.0f && timeUntilCollision <= 0.5f)
				{
					if (lowAgent.shouldWait == false)
					{
						lowAgent.shouldWait = true;
						lowAgent.startWaitTime = time;
					}
				}
			}
		}
	}

};


class CollisionAvoidanceManager_SingleWaitOrAvoid : public ICollisionAvoidanceManager
{
public:

	struct AgentInfo
	{
		Agent* pAgent;
		int priority;
		bool shouldWait;
		bool isWaiting;
		float startWaitTime;
		Vector2D vel;

		AgentInfo(Agent* pAgent_ = NULL, int priority_ = -1)
		:	pAgent(pAgent_)
		,	priority(priority_)
		{
			Reset();
		}

		void Reset()
		{
			shouldWait = false;
			isWaiting = false;
			startWaitTime = -1.0f;
		}
	};

	enum AvoidStrategy
	{
		NONE, LOW_WAIT, HIGH_WAIT, LOW_AVOID, HIGH_AVOID, BOTH_WAIT
	};

	typedef std::vector<AgentInfo> AgentInfos;

	bool m_EnableActiveAvoidance;
	AgentInfos m_AgentInfos;
	bool m_AgentInfosIsDirty;

	CollisionAvoidanceManager_SingleWaitOrAvoid(bool enableActiveAvoidance)
	: m_AgentInfosIsDirty(false)
	, m_EnableActiveAvoidance(enableActiveAvoidance)
	{
	}

	virtual void AutoAddAgents(AgentManager& agentMan)
	{
		m_AgentInfos.resize(agentMan.agents.size());

		for (int i = 0; i < agentMan.agents.size(); ++i)
		{
			m_AgentInfos[i].pAgent = agentMan.agents[i];
			m_AgentInfos[i].priority = i;
			m_AgentInfos[i].Reset();
		}
	}

	virtual void AddAgent(Agent* pAgent, int priority)
	{
		m_AgentInfos.push_back(AgentInfo(pAgent, priority));
		m_AgentInfosIsDirty = true;
	}

	virtual void ResetAgentState(Agent* pAgent)
	{
		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			if (m_AgentInfos[i].pAgent == pAgent)
			{
				m_AgentInfos[i].shouldWait = false;
				m_AgentInfos[i].isWaiting = false;
			}
		}
	}

	static bool HasHigherPriority(const AgentInfo& info, const AgentInfo& compInfo)
	{
		return compInfo.priority > info.priority;
	}

	void UpdateAgentInfos()
	{
		std::sort(m_AgentInfos.begin(), m_AgentInfos.end(), HasHigherPriority);
	}

	virtual void Update(float time, float dt)
	{
		UpdateAgentInfos();

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			m_AgentInfos[i].shouldWait = false;
		}

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			for (int j = i+1; j < m_AgentInfos.size(); ++j)
			{
				PerformCollisionAvoidance(m_AgentInfos[i], m_AgentInfos[j], time);

				//if (m_AgentInfos[i].shouldWait)
				//	break;
			}
		}

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = m_AgentInfos[i];

			if (agentInfo.shouldWait || 
				(agentInfo.isWaiting && (time - agentInfo.startWaitTime) < 0.5f))
			{
				if (agentInfo.isWaiting == false)
				{
					agentInfo.vel = agentInfo.pAgent->GetVel();
					agentInfo.pAgent->SetVel(Vector2D::kZero);
					agentInfo.isWaiting = true;
				}
			}
			else
			{
				if (agentInfo.isWaiting)
				{
					agentInfo.isWaiting = false;
					agentInfo.pAgent->SetVel(agentInfo.vel);
				}
			}
		}
	}

	void PerformCollisionAvoidance(AgentInfo& lowAgent, AgentInfo& highAgent, float time)
	{
		Vector2D avoidVel;
		AvoidStrategy strategy = DetectCollisionAvoidance(lowAgent, highAgent, time, avoidVel);

		switch (strategy)
		{
			case LOW_WAIT:
			{
				Wait(lowAgent, time);
			}
			break;

			case HIGH_WAIT:
			{
				Wait(highAgent, time);
			}
			break;

			case LOW_AVOID:
			{
				Avoid(lowAgent, highAgent, avoidVel, time);
			}
			break;

			case HIGH_AVOID:
			{
				Avoid(highAgent, lowAgent, avoidVel, time);
			}
			break;

			case BOTH_WAIT:
			{
				Wait(lowAgent, time);
				Wait(highAgent, time);
			}
			break;
		}
	}

	void Wait(AgentInfo& agent, float time)
	{
		if (agent.shouldWait == false)
		{
			agent.shouldWait = true;
			agent.startWaitTime = time;
		}
	}

	void Avoid(AgentInfo& agent, AgentInfo& avoided, const Vector2D& avoidVel, float time)
	{
		agent.shouldWait = false;
		agent.startWaitTime = -1.0f;

		agent.pAgent->SetVel(avoidVel);
	}

	Vector2D& GetAvoidVel(AgentInfo& agent, AgentInfo& avoided, Vector2D& avoidVel)
	{
		float speed;
		speed = Randf(-10.0f, 10.0f);

		Vector2D dir;

		while (dir == Vector2D::kZero)
		{
			dir = Vector2D(Randf(0.0f, 1.0f), Randf(0.0f, 1.0f));
		}

		avoidVel = dir.Normalized() * speed;
		return avoidVel;
	}

	const Vector2D& GetOriginalVel(AgentInfo& agent)
	{
		if (agent.isWaiting)
		{
			return agent.vel;
		}

		return agent.pAgent->GetVel();
	}

	const Vector2D& GetNewVel(AgentInfo& agent)
	{
		if (agent.shouldWait)
		{
			return Vector2D::kZero;
		}

		return agent.pAgent->GetVel();
	}

	AvoidStrategy DetectCollisionAvoidance(AgentInfo& lowAgent, AgentInfo& highAgent, float time, Vector2D& avoidVel)
	{
		const float lookAheadTime = 0.25f;

		if (GetOriginalVel(lowAgent) == Vector2D::kZero
			&& GetOriginalVel(highAgent) == Vector2D::kZero)
		{
			return NONE;
		}

		AvoidStrategy strategy = NONE;

		float timeUntilCollision;

		if (m_EnableActiveAvoidance)
		{
			if (GetCollisionTime(lowAgent, GetOriginalVel(lowAgent), 
				highAgent, GetNewVel(highAgent), timeUntilCollision)
				&& (timeUntilCollision <= lookAheadTime))
			{
				
				if (GetOriginalVel(lowAgent) != Vector2D::kZero
					&& GetOriginalVel(highAgent) != Vector2D::kZero
					)
				{
					if (!GetCollisionTime(lowAgent, Vector2D::kZero, 
						highAgent, GetNewVel(highAgent), timeUntilCollision)
						|| timeUntilCollision > lookAheadTime)
					{
						strategy = LOW_WAIT;
					}
					else if (!GetCollisionTime(lowAgent, GetAvoidVel(lowAgent, highAgent, avoidVel), 
						highAgent, GetOriginalVel(highAgent), timeUntilCollision)
						|| timeUntilCollision > lookAheadTime)
					{
						strategy = LOW_AVOID;
					}
					else if (!GetCollisionTime(lowAgent, GetNewVel(lowAgent), 
						highAgent, Vector2D::kZero, timeUntilCollision)
						|| timeUntilCollision > lookAheadTime)
					{
						strategy = HIGH_WAIT;
					}
					else
					{
						strategy = NONE;

						for (int i = 0; i < 10; ++i)
						{
							if (!GetCollisionTime(lowAgent, GetNewVel(lowAgent), 
								highAgent, GetAvoidVel(highAgent, lowAgent, avoidVel), timeUntilCollision)
								|| timeUntilCollision > lookAheadTime)
							{
								strategy = HIGH_AVOID;
								break;
							}
						}
					}
				} 
				else if (GetOriginalVel(lowAgent) != Vector2D::kZero)
				{
					strategy = NONE;

					for (int i = 0; i < 10; ++i)
					{
						if (!GetCollisionTime(lowAgent, GetAvoidVel(lowAgent, highAgent, avoidVel), 
							highAgent, GetNewVel(highAgent), timeUntilCollision)
							|| timeUntilCollision > lookAheadTime)
						{
							strategy = LOW_AVOID;
							break;
						}
					}
				}
				else  if (GetOriginalVel(highAgent) != Vector2D::kZero)
				{
					strategy = NONE;

					for (int i = 0; i < 10; ++i)
					{
						if (!GetCollisionTime(lowAgent, GetNewVel(lowAgent), 
							highAgent, GetAvoidVel(highAgent, lowAgent, avoidVel), timeUntilCollision)
							|| timeUntilCollision > lookAheadTime)
						{
							strategy = HIGH_AVOID;
							break;
						}
					}
				}
			}
		}
		else
		{
			if (
				(GetCollisionTime(lowAgent, GetOriginalVel(lowAgent), 
				highAgent, GetNewVel(highAgent), timeUntilCollision)
				&& (timeUntilCollision <= lookAheadTime))

				|| (GetCollisionTime(lowAgent, GetNewVel(lowAgent), 
				highAgent, GetOriginalVel(highAgent), timeUntilCollision)
				&& (timeUntilCollision <= lookAheadTime))
				
				)
			{
				if ((GetNewVel(lowAgent) != Vector2D::kZero)
					&&
					(
					!GetCollisionTime(lowAgent, Vector2D::kZero, 
					highAgent, GetNewVel(highAgent), timeUntilCollision)
					|| timeUntilCollision > lookAheadTime))
				{
					strategy = LOW_WAIT;
				}
				else if (
					(GetNewVel(highAgent) != Vector2D::kZero)
					&&
					(
					!GetCollisionTime(lowAgent, GetNewVel(lowAgent), 
					highAgent, Vector2D::kZero, timeUntilCollision)
					|| timeUntilCollision > lookAheadTime))
				{
					strategy = HIGH_WAIT;
				}
				else
				{
					strategy = BOTH_WAIT;
				}
			}
		}

		

		return strategy;
	}

	bool GetCollisionTime(AgentInfo& lowAgent, const Vector2D& lowAgentVel, 
						  AgentInfo& highAgent, const Vector2D& highAgentVel, float& timeUntilCollision)
	{
		AvoidStrategy strategy = NONE;

		Vector2D relVel = lowAgentVel - highAgentVel;
		float relSpeed = relVel.Length();

		if (relSpeed > 0.0f)
		{
			Vector2D relVelDir = relVel.Normalized();
			float t, u;

			if (IntersectLineCircle(lowAgent.pAgent->GetPos(), relVelDir, highAgent.pAgent->GetPos(), lowAgent.pAgent->GetRadius() + highAgent.pAgent->GetRadius(), t, u) > 0) 
			{
				if (t < 0.0f)
					t = u;

				float intersectionT = std::min(t, u);

				timeUntilCollision = intersectionT / relSpeed;

				if (timeUntilCollision >= 0.0f)
				{
					return true;
				}
			}
		}

		return false;
	}
};


class CollisionAvoidanceManager_RobustWait : public ICollisionAvoidanceManager
{
public:

	struct AgentInfo
	{
		Agent* pAgent;
		int priority;
		bool shouldWait;
		bool isWaiting;
		float startWaitTime;
		Vector2D vel;

		AgentInfo(Agent* pAgent_ = NULL, int priority_ = -1)
			:	pAgent(pAgent_)
			,	priority(priority_)
		{
			Reset();
		}

		void Reset()
		{
			shouldWait = false;
			isWaiting = false;
			startWaitTime = -1.0f;
		}
	};

	enum AvoidStrategy
	{
		NONE, LOW_WAIT, HIGH_WAIT, BOTH_WAIT
	};

	typedef std::vector<AgentInfo> AgentInfos;

	AgentInfos m_AgentInfos;
	bool m_AgentInfosIsDirty;

	CollisionAvoidanceManager_RobustWait()
		: m_AgentInfosIsDirty(false)
	{
	}

	virtual void AutoAddAgents(AgentManager& agentMan)
	{
		m_AgentInfos.resize(agentMan.agents.size());

		for (int i = 0; i < agentMan.agents.size(); ++i)
		{
			m_AgentInfos[i].pAgent = agentMan.agents[i];
			m_AgentInfos[i].priority = i;
			m_AgentInfos[i].Reset();
		}
	}

	virtual void AddAgent(Agent* pAgent, int priority)
	{
		m_AgentInfos.push_back(AgentInfo(pAgent, priority));
		m_AgentInfosIsDirty = true;
	}

	virtual void ResetAgentState(Agent* pAgent)
	{
		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			if (m_AgentInfos[i].pAgent == pAgent)
			{
				m_AgentInfos[i].shouldWait = false;
				m_AgentInfos[i].isWaiting = false;
			}
		}
	}

	static bool HasHigherPriority(const AgentInfo& info, const AgentInfo& compInfo)
	{
		return compInfo.priority > info.priority;
	}

	void UpdateAgentInfos()
	{
		std::sort(m_AgentInfos.begin(), m_AgentInfos.end(), HasHigherPriority);
	}

	virtual void Update(float time, float dt)
	{
		UpdateAgentInfos();

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			m_AgentInfos[i].shouldWait = false;
		}

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			for (int j = 0; j < m_AgentInfos.size(); ++j)
			{
				if (i != j)
				{
					PerformCollisionAvoidance(m_AgentInfos[i], m_AgentInfos[j], time);
				}

				if (m_AgentInfos[i].shouldWait && i == 0)
				{
					int x = 0;
				}

				if (m_AgentInfos[i].shouldWait)
					break;
			}
		}

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = m_AgentInfos[i];

			if (agentInfo.shouldWait || 
				(agentInfo.isWaiting && (time - agentInfo.startWaitTime) < 0.5f))
			{
				if (agentInfo.isWaiting == false)
				{
					agentInfo.vel = agentInfo.pAgent->GetVel();
					agentInfo.pAgent->SetVel(Vector2D::kZero);
					agentInfo.isWaiting = true;
				}
			}
			else
			{
				if (agentInfo.isWaiting)
				{
					agentInfo.isWaiting = false;
					agentInfo.pAgent->SetVel(agentInfo.vel);
				}
			}
		}
	}

	void PerformCollisionAvoidance(AgentInfo& lowAgent, AgentInfo& highAgent, float time)
	{
		AvoidStrategy strategy = DetectCollisionAvoidance(lowAgent, highAgent, time);

		switch (strategy)
		{
		case LOW_WAIT:
			{
				Wait(lowAgent, time);
			}
			break;

		case HIGH_WAIT:
			{
				Wait(highAgent, time);
			}
			break;

		case BOTH_WAIT:
			{
				Wait(lowAgent, time);
				Wait(highAgent, time);
			}
			break;
		}
	}

	void Wait(AgentInfo& agent, float time)
	{
		if (agent.shouldWait == false)
		{
			agent.shouldWait = true;
			agent.startWaitTime = time;
		}
	}

	const Vector2D& GetOriginalVel(AgentInfo& agent)
	{
		if (agent.isWaiting)
		{
			return agent.vel;
		}

		return agent.pAgent->GetVel();
	}

	const Vector2D& GetNewVel(AgentInfo& agent)
	{
		if (agent.shouldWait)
		{
			return Vector2D::kZero;
		}

		return agent.pAgent->GetVel();
	}

	bool WillCollide(AgentInfo& testAgent, AgentInfo& obstacleAgent, float lookAheadTime)
	{
		return WillCollide(testAgent, GetOriginalVel(testAgent), obstacleAgent, lookAheadTime);
	}

	bool WillCollide(AgentInfo& testAgent, const Vector2D& testAgentVel, AgentInfo& obstacleAgent, float lookAheadTime)
	{
		float timeUntilCollision;

		return (GetCollisionTime(testAgent, testAgentVel, 
			obstacleAgent, GetNewVel(obstacleAgent), timeUntilCollision)
			&& (timeUntilCollision <= lookAheadTime));
	}

	AvoidStrategy DetectCollisionAvoidance(AgentInfo& lowAgent, AgentInfo& highAgent, float time)
	{
		const float lookAheadTime = 0.25f;

		AvoidStrategy strategy = NONE;

		// the 1.5 multiplier is needed to make sure we dont wait if the only effect is 
		// delaying a collision, in that case the other agent will end up waiting for us anyway
		// to test this make 2 perpendicular slow moving agents going into a collision
		// at one point if the 1.5 multiplier is not there they will both wait for a split second
		// then one of them will continue moving. There must be a more elegant solution to this
		if (WillCollide(lowAgent, highAgent, lookAheadTime)
			&& !WillCollide(lowAgent, Vector2D::kZero, highAgent, (lookAheadTime * 1.5f)))
		{
			strategy = LOW_WAIT;
		}

		return strategy;
	}

	bool GetCollisionTime(AgentInfo& lowAgent, const Vector2D& lowAgentVel, 
		AgentInfo& highAgent, const Vector2D& highAgentVel, float& timeUntilCollision)
	{
		AvoidStrategy strategy = NONE;

		Vector2D relVel = lowAgentVel - highAgentVel;
		float relSpeed = relVel.Length();

		if (relSpeed > 0.0f)
		{
			Vector2D relVelDir = relVel.Normalized();
			float t, u;

			if (IntersectLineCircle(lowAgent.pAgent->GetPos(), relVelDir, highAgent.pAgent->GetPos(), lowAgent.pAgent->GetRadius() + highAgent.pAgent->GetRadius(), t, u) > 0) 
			{
				if (t < 0.0f)
					t = u;

				float intersectionT = std::min(t, u);

				timeUntilCollision = intersectionT / relSpeed;

				if (timeUntilCollision >= 0.0f)
				{
					return true;
				}
			}
		}

		return false;
	}
};


class CollisionAvoidanceManager_RobustWait_ReactiveDeadlockResolve : public ICollisionAvoidanceManager
{
public:

	struct AgentInfo
	{
		Agent* pAgent;
		int priority;
		bool shouldWait;
		bool isWaiting;
		float startWaitTime;
		Vector2D vel;
		int obstacleInfoIndex;
		float startObstacleNotMovingTime;
		
		/*
		AgentInfo* pWaitRoot;
		AgentInfo* pWaitTarget;
		int waiterCount;
		int tarjanIndex;
		int tarjanLowLink;
		*/

		AgentInfo(Agent* pAgent_ = NULL, int priority_ = -1)
			:	pAgent(pAgent_)
			,	priority(priority_)
		{
			Reset();
		}

		void Reset()
		{
			obstacleInfoIndex = -1;
			shouldWait = false;
			isWaiting = false;
			startWaitTime = -1.0f;
		}
	};

	enum AvoidStrategy
	{
		NONE, LOW_WAIT, HIGH_WAIT, BOTH_WAIT
	};

	typedef std::vector<AgentInfo> AgentInfos;
	typedef std::vector<AgentInfo*> AgentInfoPtrs;

	AgentInfos m_AgentInfos;
	AgentInfoPtrs m_GraphRoots;
	bool m_AgentInfosIsDirty;


	/*
	class Tarjan {

		int index = 0;
		typedef std::vector<AgentInfo*> Stack;
		typedef std::vector<Nodes> ConnectedLists;

		Stack stack;

		public ArrayList<ArrayList<Node>> tarjan(AgentInfo* pRoot)
		{
			stack.clear();

			pRoot->tarjanIndex = index;
			pRoot->tarjanLowLink = index;
			index++;

			stack.push_back(pRoot);

			AgentInfo* pNext = pRoot->pWaitTarget;

			if (pNext)
			{

			}
			
			for(Edge e : list.getAdjacent(v)){
				Node n = e.to;
				if(n.index == -1){
					tarjan(n, list);
					v.lowlink = Math.min(v.lowlink, n.lowlink);
				}else if(stack.contains(n)){
					v.lowlink = Math.min(v.lowlink, n.index);
				}
			}
			if(v.lowlink == v.index){
				Node n;
				ArrayList<Node> component = new ArrayList<Node>();
				do{
					n = stack.remove(0);
					component.add(n);
				}while(n != v);
				SCC.add(component);
			}
			return SCC;
		}
	}
	*/


	CollisionAvoidanceManager_RobustWait_ReactiveDeadlockResolve()
		: m_AgentInfosIsDirty(false)
	{
	}

	virtual void AutoAddAgents(AgentManager& agentMan)
	{
		m_AgentInfos.resize(agentMan.agents.size());

		for (int i = 0; i < agentMan.agents.size(); ++i)
		{
			m_AgentInfos[i].pAgent = agentMan.agents[i];
			m_AgentInfos[i].priority = i;
			m_AgentInfos[i].Reset();
		}
	}

	virtual void AddAgent(Agent* pAgent, int priority)
	{
		m_AgentInfos.push_back(AgentInfo(pAgent, priority));
		m_AgentInfosIsDirty = true;
	}

	virtual void ResetAgentState(Agent* pAgent)
	{
		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			if (m_AgentInfos[i].pAgent == pAgent)
			{
				m_AgentInfos[i].shouldWait = false;
				m_AgentInfos[i].isWaiting = false;
			}
		}
	}

	static bool HasHigherPriority(const AgentInfo& info, const AgentInfo& compInfo)
	{
		return compInfo.priority > info.priority;
	}

	void UpdateAgentInfos()
	{
		std::sort(m_AgentInfos.begin(), m_AgentInfos.end(), HasHigherPriority);
	}

	virtual void Update(float time, float dt)
	{
		UpdateAgentInfos();

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			m_AgentInfos[i].shouldWait = false;
		}

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			for (int j = 0; j < m_AgentInfos.size(); ++j)
			{
				if (i != j)
				{
					PerformCollisionAvoidance(m_AgentInfos[i], m_AgentInfos[j], time);
				}

				if (m_AgentInfos[i].shouldWait)
				{
					m_AgentInfos[i].obstacleInfoIndex = j;
					break;
				}
			}
		}

		int escapedWaitingCount = 0;

		for (int i = 0; i < m_AgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = m_AgentInfos[i];

			if (agentInfo.shouldWait || 
				(agentInfo.isWaiting && (time - agentInfo.startWaitTime) < 0.5f))
			{
				if (agentInfo.isWaiting == false)
				{
					agentInfo.vel = agentInfo.pAgent->GetVel();
					agentInfo.pAgent->SetVel(Vector2D::kZero);
					agentInfo.isWaiting = true;
					agentInfo.startObstacleNotMovingTime = 0.0f;
				}
				
				if (m_AgentInfos[agentInfo.obstacleInfoIndex].pAgent->GetVel() == Vector2D::kZero)
				{
					agentInfo.startObstacleNotMovingTime += dt;
					
					if (agentInfo.isWaiting && agentInfo.startObstacleNotMovingTime > 0.5f)
					{
						agentInfo.pAgent->Agitate();

						if (escapedWaitingCount == 0 && agentInfo.startObstacleNotMovingTime > 2.0f)
						{
							Vector2D avoidVel;

							GetAvoidVel(agentInfo, m_AgentInfos[agentInfo.obstacleInfoIndex], avoidVel);
							
							agentInfo.vel = avoidVel;

							if (DetectCollisionAvoidance(agentInfo, m_AgentInfos[agentInfo.obstacleInfoIndex], time) == NONE)
							{
								++escapedWaitingCount;

								agentInfo.isWaiting = false;
								agentInfo.pAgent->SetVel(agentInfo.vel);
							}
						}
					}
				}
			}
			else
			{
				if (agentInfo.isWaiting)
				{
					agentInfo.isWaiting = false;
					agentInfo.pAgent->SetVel(agentInfo.vel);
				}
			}
		}
	}

	Vector2D& GetAvoidVel(AgentInfo& agent, AgentInfo& avoided, Vector2D& avoidVel)
	{
		// Very simple random conditions here

		float minSpeed = std::max(1.0f, GetOriginalVel(agent).Length()); 
		float speed = minSpeed * Randf(1.0f, 1.5f);

		Vector2D dir;
		
		while (dir == Vector2D::kZero)
		{
			dir = Vector2D(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f));
		}

		avoidVel = dir.Normalized() * speed;
		return avoidVel;
	}

	void PerformCollisionAvoidance(AgentInfo& lowAgent, AgentInfo& highAgent, float time)
	{
		AvoidStrategy strategy = DetectCollisionAvoidance(lowAgent, highAgent, time);

		switch (strategy)
		{
		case LOW_WAIT:
			{
				Wait(lowAgent, time);
			}
			break;

		case HIGH_WAIT:
			{
				Wait(highAgent, time);
			}
			break;

		case BOTH_WAIT:
			{
				Wait(lowAgent, time);
				Wait(highAgent, time);
			}
			break;
		}
	}

	void Wait(AgentInfo& agent, float time)
	{
		if (agent.shouldWait == false)
		{
			agent.shouldWait = true;
			agent.startWaitTime = time;
		}
	}

	const Vector2D& GetOriginalVel(AgentInfo& agent)
	{
		if (agent.isWaiting)
		{
			return agent.vel;
		}

		return agent.pAgent->GetVel();
	}

	const Vector2D& GetNewVel(AgentInfo& agent)
	{
		if (agent.shouldWait)
		{
			return Vector2D::kZero;
		}

		return agent.pAgent->GetVel();
	}

	bool WillCollide(AgentInfo& testAgent, AgentInfo& obstacleAgent, float lookAheadTime)
	{
		return WillCollide(testAgent, GetOriginalVel(testAgent), obstacleAgent, lookAheadTime);
	}

	bool WillCollide(AgentInfo& testAgent, const Vector2D& testAgentVel, AgentInfo& obstacleAgent, float lookAheadTime)
	{
		float timeUntilCollision;

		return (GetCollisionTime(testAgent, testAgentVel, 
			obstacleAgent, GetNewVel(obstacleAgent), timeUntilCollision)
			&& (timeUntilCollision <= lookAheadTime));
	}

	AvoidStrategy DetectCollisionAvoidance(AgentInfo& lowAgent, AgentInfo& highAgent, float time)
	{
		const float lookAheadTime = 0.25f;

		AvoidStrategy strategy = NONE;

		// the 1.5 multiplier is needed to make sure we dont wait if the only effect is 
		// delaying a collision, in that case the other agent will end up waiting for us anyway
		// to test this make 2 perpendicular slow moving agents going into a collision
		// at one point if the 1.5 multiplier is not there they will both wait for a split second
		// then one of them will continue moving. There must be a more elegant solution to this
		if (WillCollide(lowAgent, highAgent, lookAheadTime)
			&& !WillCollide(lowAgent, Vector2D::kZero, highAgent, (lookAheadTime * 1.5f)))
		{
			strategy = LOW_WAIT;
		}

		return strategy;
	}

	bool GetCollisionTime(AgentInfo& lowAgent, const Vector2D& lowAgentVel, 
		AgentInfo& highAgent, const Vector2D& highAgentVel, float& timeUntilCollision)
	{
		AvoidStrategy strategy = NONE;

		Vector2D relVel = lowAgentVel - highAgentVel;
		float relSpeed = relVel.Length();

		if (relSpeed > 0.0f)
		{
			Vector2D relVelDir = relVel.Normalized();
			float t, u;

			if (IntersectLineCircle(lowAgent.pAgent->GetPos(), relVelDir, highAgent.pAgent->GetPos(), lowAgent.pAgent->GetRadius() + highAgent.pAgent->GetRadius(), t, u) > 0) 
			{
				if (t < 0.0f)
					t = u;

				float intersectionT = std::min(t, u);

				timeUntilCollision = intersectionT / relSpeed;

				if (timeUntilCollision >= 0.0f)
				{
					return true;
				}
			}
		}

		return false;
	}
};

class SceneController
{
public:

	AgentManager& m_AgentManager;
	ICollisionAvoidanceManager* m_pAvoidanceManager;
	Agent* m_pFocusAgent;
	Agent* m_pLeftMouseControlledAgent;
	Agent* m_pRightMouseControlledAgent;
	bool m_IsLeftPressed;
	bool m_IsRightPressed;
	Uint32 m_StartPressTime;
	Vector2D m_MoveAvgVel;
	Vector2D m_AgentCenterOffset;
	Vector2D m_StartRightClickPos;
	Vector2D m_DrawVelVector;
	
	SceneController(AgentManager& agentManager, ICollisionAvoidanceManager* pAvoidanceMan)
	:	m_AgentManager(agentManager)
	,	m_pAvoidanceManager(pAvoidanceMan)
	,	m_pLeftMouseControlledAgent(NULL)
	,	m_pRightMouseControlledAgent(NULL)
	,	m_IsLeftPressed(false)
	,	m_IsRightPressed(false)
	,	m_pFocusAgent(NULL)
	{
	}

	void Update()
	{
		int x;
		int y;
		int relX;
		int relY;

		if (m_pLeftMouseControlledAgent)
		{
			if (SDL_GetMouseState(&x, &y)&SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				Vector2D worldPos = ScreenToWorld(Vector2D((float) x, (float) y));

				m_pLeftMouseControlledAgent->SetPos(worldPos + m_AgentCenterOffset);
				if (m_pAvoidanceManager)
					m_pAvoidanceManager->ResetAgentState(m_pLeftMouseControlledAgent);

				SDL_GetRelativeMouseState(&relX, &relY);
				Vector2D worldPosDiff = ScreenToWorldDir(Vector2D((float) relX, (float) relY));

				if (m_IsLeftPressed)
				{
					m_MoveAvgVel += worldPosDiff;
					//printf("%f,%f\n", m_MoveAvgVel.x, m_MoveAvgVel.y);
				}
				else
				{
					m_IsLeftPressed = true;
					m_MoveAvgVel = Vector2D::kZero;
					m_StartPressTime = SDL_GetTicks();
				}
			}
			else
			{
				float pressTime = ((float) (SDL_GetTicks() - m_StartPressTime)) / 1000.0f;

				Vector2D vel = m_MoveAvgVel * (1.0f / pressTime);
				//m_pLeftMouseControlledAgent->SetVel(vel);
				if (m_pAvoidanceManager)
					m_pAvoidanceManager->ResetAgentState(m_pLeftMouseControlledAgent);
				
				m_pLeftMouseControlledAgent = NULL;
				m_IsLeftPressed = false;
			}
		}

		if (m_pRightMouseControlledAgent)
		{
			if (SDL_GetMouseState(&x, &y)&SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				Vector2D worldPos = ScreenToWorld(Vector2D((float) x, (float) y));

				m_DrawVelVector = worldPos - m_pRightMouseControlledAgent->GetPos();
			}
			else
			{
				m_pRightMouseControlledAgent->SetVel(m_DrawVelVector * (1.0f / kVelDrawScale));
				if (m_pAvoidanceManager)
					m_pAvoidanceManager->ResetAgentState(m_pRightMouseControlledAgent);
				m_pRightMouseControlledAgent = NULL;
				m_IsRightPressed = false;
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
					m_pLeftMouseControlledAgent = pAgent;

					m_AgentCenterOffset = m_pLeftMouseControlledAgent->GetPos() - worldPos;
					m_pLeftMouseControlledAgent->SetVel(Vector2D::kZero);
					if (m_pAvoidanceManager)
						m_pAvoidanceManager->ResetAgentState(m_pLeftMouseControlledAgent);
				}
			}

			if (evt.button.button == SDL_BUTTON_RIGHT)
			{
				int x = evt.motion.x;
				int y = evt.motion.y;

				m_StartRightClickPos = ScreenToWorld(Vector2D((float) x, (float) y));
				Agent* pAgent = m_AgentManager.PickAgent(m_StartRightClickPos);

				if (pAgent != NULL)
				{
					m_pRightMouseControlledAgent = pAgent;

					m_pRightMouseControlledAgent->SetVel(Vector2D::kZero);
					if (m_pAvoidanceManager)
						m_pAvoidanceManager->ResetAgentState(m_pRightMouseControlledAgent);
				}
			}

			if (evt.button.button == SDL_BUTTON_MIDDLE)
			{
				int x = evt.motion.x;
				int y = evt.motion.y;

				Vector2D worldPos = ScreenToWorld(Vector2D((float) x, (float) y));

				Agent* pAgent = new Agent(worldPos, Vector2D(0.0f, 0.0f), 1.0f, Color::kWhite);
				m_AgentManager.Add(*(pAgent));

				if (m_pAvoidanceManager)
					m_pAvoidanceManager->AddAgent(pAgent, m_AgentManager.agents.size());
			}
		}
	}

	void Draw()
	{
		if (m_pRightMouseControlledAgent)
		{
			DrawArrow(WorldToScreen(m_pRightMouseControlledAgent->GetPos()), 
						WorldToScreen(m_pRightMouseControlledAgent->GetPos() + m_DrawVelVector), Color::kGreen, 0.5f);
		}
	}
};



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


void CreateTestCaseRandom(AgentManager& agents)
{

	agents.Add(*(new Agent(Vector2D::kZero, Vector2D(4.0f, -10.0f), 1.0f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(3.0f, 6.0f), Vector2D(16.5f, 1.5f), 1.1f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(-3.0f, 6.0f), Vector2D(-2.5f, -10.5f), 1.2f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(-6.0f, -3.0f), Vector2D(3.5f, 6.5f), 1.3f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(-3.0f, -6.0f), Vector2D(2.5f, 3.5f), 1.4f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(6.0f, 6.0f), Vector2D(12.5f, -5.5f), 1.5f, Color::kWhite)));
}

void CreateTestCase1(AgentManager& agents)
{
	agents.Add(*(new Agent(Vector2D(-6.0f, 0.0f), Vector2D(1.0f, 0.0f), 1.0f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D::kZero, Vector2D::kZero, 1.4f, Color::kWhite)));
}

void CreateTestCase2(AgentManager& agents)
{
	agents.Add(*(new Agent(Vector2D(-6.0f, 0.0f), Vector2D(1.5f, 0.0f), 1.0f, Color::kBlue)));
	agents.Add(*(new Agent(Vector2D(0.0f, -6.0f), Vector2D(0.0f, 1.0f), 1.0f, Color::kWhite)));
}

void CreateTestCrossing4(AgentManager& agents)
{
	agents.Add(*(new Agent(Vector2D(-10.0f, -15.0f), (Vector2D(10.0f, 5.0f) - Vector2D(-10.0f, -15.0f)).Normalized() * 3.0f, 1.0f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(-5.0f, -15.0f),(Vector2D(0.0f, 5.0f) - Vector2D(-5.0f, -15.0f)).Normalized() * 3.0f, 1.0f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(0.0f, -15.0f), (Vector2D(-5.0f, 5.0f) - Vector2D(0.0f, -15.0f)).Normalized() * 3.0f, 1.0f, Color::kWhite)));
	agents.Add(*(new Agent(Vector2D(5.0f, -15.0f), (Vector2D(-10.0f, 5.0f) - Vector2D(5.0f, -15.0f)).Normalized() * 3.0f, 1.0f, Color::kWhite)));
}


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
	
	CreateTestCrossing4(agents);
	
	ICollisionAvoidanceManager& avoidanceManager = *(new CollisionAvoidanceManager_RobustWait_ReactiveDeadlockResolve());
	avoidanceManager.AutoAddAgents(agents);

	SceneController sceneController(agents, &avoidanceManager);

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
			sceneController.Draw();
			SDL_GL_SwapBuffers();
		}

		if (updateTimer.Update())
		{
			unsigned int frames = updateTimer.GetDeltaFrameIndex();

			//printf ("%f\n", updateTimer.GetFrameLockedTime());
			for (unsigned int i = 0; i < frames; ++i)
			{
				avoidanceManager.Update(updateTimer.GetFrameLockedTime(), updateTimer.GetFrameTime());
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

	delete &(avoidanceManager);

	return 0;
}
