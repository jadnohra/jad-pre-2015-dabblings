#ifndef COLLISION_AVOIDANCE_MANAGER_H
#define COLLISION_AVOIDANCE_MANAGER_H

#include "World.h"
#include "Agent.h"

class ICollisionAvoidanceManager
{
public:

	virtual ~ICollisionAvoidanceManager() {}

	virtual void AddAgent(Agent* pAgent, int priority) = 0;
	virtual void RemoveAgent(Agent* pAgent) = 0;
	virtual void ResetAgentState(Agent* pAgent) = 0;
	virtual void Update(float time, float dt) = 0;
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

	
	virtual void AddAgent(Agent* pAgent, int priority)
	{
		m_AgentInfos.push_back(AgentInfo(pAgent, priority));
		m_AgentInfosIsDirty = true;
	}

	virtual void RemoveAgent(Agent* pAgent) 
	{
		ResetAgentState(pAgent);
		for (size_t i = 0; i < m_AgentInfos.size(); ++i)
		{
			if (m_AgentInfos[i].pAgent == pAgent)
			{
				m_AgentInfos[i].pAgent = NULL;
			}
		}
	}

	virtual void ResetAgentState(Agent* pAgent)
	{
		for (size_t i = 0; i < m_AgentInfos.size(); ++i)
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

		for (size_t i = 0; i < m_AgentInfos.size(); ++i)
		{
			m_AgentInfos[i].shouldWait = false;
		}

		for (size_t i = 0; i < m_AgentInfos.size(); ++i)
		{
			for (size_t j = 0; j < m_AgentInfos.size(); ++j)
			{
				if (i != j)
				{
					PerformCollisionAvoidance(m_AgentInfos[i], m_AgentInfos[j], time);
				}

				if (m_AgentInfos[i].shouldWait)
				{
					m_AgentInfos[i].obstacleInfoIndex = (int) j;
					break;
				}
			}
		}

		int escapedWaitingCount = 0;

		for (size_t i = 0; i < m_AgentInfos.size(); ++i)
		{
			AgentInfo& agentInfo = m_AgentInfos[i];

			if (agentInfo.shouldWait || 
				(agentInfo.isWaiting && (time - agentInfo.startWaitTime) < 0.5f))
			{
				agentInfo.pAgent->NotifyControlledByAvoidance();

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
		float speed = std::min(15.0f, minSpeed * Randf(0.8f, 1.2f));


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

#endif