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


class CollisionAvoidanceManager_DiscereteSearch : public ICollisionAvoidanceManager
{
public:

	struct AgentInfo
	{
		Agent* pAgent;
		int priority;
		int obstacleInfoIndex;
		int avoidanceGroupIndex;
		bool hasModifiedVel;
		Vector2D modifiedVel;

		AgentInfo(Agent* pAgent_ = NULL, int priority_ = -1, int obstacleInfoIndex_ = -1)
			:	pAgent(pAgent_)
			,	priority(priority_)
			,	obstacleInfoIndex(obstacleInfoIndex_)
		{
		}

		void Reset()
		{
		}
	};

	struct AvoidanceOption
	{
		Vector2D point;
		bool hasVel;
		Vector2D vel;
	};

	struct AgentAvoidanceOption : public AvoidanceOption
	{
		int agentIndex;
		bool replacePointInPath;
	};

	struct AvoidanceGroup
	{
		typedef std::vector<int> Agents;
		typedef std::vector<AgentAvoidanceOption> Options;

		Agents agents;
		Agents newResolveAgents;
		Options avoidanceSolutions;

		AvoidanceGroup()
		{

		}

		AvoidanceGroup(const AvoidanceGroup& ref)
		{
			agents = ref.agents;
			newResolveAgents = ref.newResolveAgents;
			avoidanceSolutions = ref.avoidanceSolutions;
		}

		void AddAgent(int index)
		{
			if (std::find(agents.begin(), agents.end(), index) == agents.end())
			{
				agents.push_back(index);
			}
		}
	};

	typedef std::vector<AgentInfo> AgentInfos;
	typedef std::vector<AvoidanceGroup> AvoidanceGroups;

	World& mWorld;
	AgentInfos m_AgentInfos;
	bool m_AgentInfosIsDirty;

	CollisionAvoidanceManager_DiscereteSearch(World& world)
		: mWorld(world)
	{
	}

	virtual ~CollisionAvoidanceManager_DiscereteSearch() {}

	virtual void AddAgent(Agent* pAgent, int priority)
	{
		m_AgentInfos.push_back(AgentInfo(pAgent, priority, (int) m_AgentInfos.size()));
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
				m_AgentInfosIsDirty = true;
			}
		}
	}

	virtual void ResetAgentState(Agent* pAgent)
	{
	}

	void UpdateAgentInfos()
	{
		if (m_AgentInfosIsDirty)
		{
			//std::sort(m_AgentInfos.begin(), m_AgentInfos.end(), HasHigherPriority);
			m_AgentInfosIsDirty = false;
		}
	}

	virtual void Update(float time, float dt)
	{
		UpdateAgentInfos();
		AvoidanceGroups avoidanceGroups;

		const float lookAheadTime = 0.25f;

		for (size_t i = 0; i < m_AgentInfos.size(); ++i)
		{
			m_AgentInfos[i].avoidanceGroupIndex = -1;
			m_AgentInfos[i].hasModifiedVel = false;
		}

		for (size_t i = 0; i < m_AgentInfos.size(); ++i)
		{
			if (m_AgentInfos[i].pAgent)
			{
				m_AgentInfos[i].avoidanceGroupIndex = -1;

				for (size_t j = 0; j < m_AgentInfos.size(); ++j)
				{
					if (i != j && m_AgentInfos[j].pAgent)
					{
						if (WillCollide(m_AgentInfos[i], m_AgentInfos[i].pAgent->GetVel(),
										m_AgentInfos[j], m_AgentInfos[j].pAgent->GetVel(),
										lookAheadTime))
						{
							if (m_AgentInfos[i].avoidanceGroupIndex == -1
								&& m_AgentInfos[j].avoidanceGroupIndex == -1)
							{
								int groupIndex = (int) avoidanceGroups.size();
								avoidanceGroups.push_back(AvoidanceGroup());
								avoidanceGroups[groupIndex].AddAgent((int)i);
								avoidanceGroups[groupIndex].AddAgent((int)j);
								m_AgentInfos[i].avoidanceGroupIndex = groupIndex;
								m_AgentInfos[j].avoidanceGroupIndex = groupIndex;
							}
							else if (m_AgentInfos[i].avoidanceGroupIndex != -1
									&& m_AgentInfos[j].avoidanceGroupIndex != -1)
							{
								if (m_AgentInfos[i].avoidanceGroupIndex != m_AgentInfos[j].avoidanceGroupIndex)
								{
									AvoidanceGroup* pMergeGroup;
									AvoidanceGroup* pMergedGroup;
									int mergeGroupIndex;

									// TODO duplicates!!

									if (avoidanceGroups[m_AgentInfos[i].avoidanceGroupIndex].agents.size() > 
										avoidanceGroups[m_AgentInfos[j].avoidanceGroupIndex].agents.size())
									{
										mergeGroupIndex = m_AgentInfos[i].avoidanceGroupIndex;
										pMergeGroup = &avoidanceGroups[m_AgentInfos[i].avoidanceGroupIndex];
										pMergedGroup = &avoidanceGroups[m_AgentInfos[j].avoidanceGroupIndex];
									}
									else
									{
										mergeGroupIndex = m_AgentInfos[j].avoidanceGroupIndex;
										pMergeGroup = &avoidanceGroups[m_AgentInfos[j].avoidanceGroupIndex];
										pMergedGroup = &avoidanceGroups[m_AgentInfos[i].avoidanceGroupIndex];
									}
									
									pMergeGroup->agents.insert(pMergeGroup->agents.end(), pMergedGroup->agents.begin(), pMergedGroup->agents.end());
									pMergedGroup->agents.clear();

									m_AgentInfos[i].avoidanceGroupIndex = mergeGroupIndex;
									m_AgentInfos[j].avoidanceGroupIndex = mergeGroupIndex;
								}
							}
							else
							{
								if (m_AgentInfos[i].avoidanceGroupIndex != -1)
								{
									m_AgentInfos[j].avoidanceGroupIndex = m_AgentInfos[i].avoidanceGroupIndex;
									avoidanceGroups[m_AgentInfos[i].avoidanceGroupIndex].AddAgent((int) j);
								}
								else
								{
									m_AgentInfos[i].avoidanceGroupIndex = m_AgentInfos[j].avoidanceGroupIndex;
									avoidanceGroups[m_AgentInfos[j].avoidanceGroupIndex].AddAgent((int) i);
								}
							}
						}
					}
				}
			}
		}

		bool groupsHaveChanged = false;
		int iterationCount = 0;

		do 
		{
			for (size_t i = 0; i < avoidanceGroups.size(); ++i)
			{
				AvoidanceGroup& group = avoidanceGroups[i];

				ResolveGroup(group, lookAheadTime);

				if (!group.agents.empty())
				{
					if (!group.newResolveAgents.empty())
					{
						// TODO
						//groupsHaveChanged = true;
					}
				}
			}

		} while (groupsHaveChanged && iterationCount < 10);

		for (size_t i = 0; i < avoidanceGroups.size(); ++i)
		{
			AvoidanceGroup& group = avoidanceGroups[i];

			for (size_t j = 0; j < group.avoidanceSolutions.size(); ++j)
			{
				const AgentAvoidanceOption& solution = group.avoidanceSolutions[j];

				m_AgentInfos[solution.agentIndex].pAgent->AddAvoidanceSolutionToPath(solution.point, solution.hasVel ? & solution.vel : NULL, solution.replacePointInPath);
			}
		}
	}


	void ResolveGroup(AvoidanceGroup& group, float lookAheadTime)
	{
		for (size_t i = 0; i < group.agents.size(); ++i)
		{
			AgentInfo& agent = m_AgentInfos[group.agents[i]];
			
			int collider;
			Vector2D collisionPoint;
			
			if (FindEarliestCollider(agent, group, lookAheadTime, collider, collisionPoint))
			{
				ResolveCollision(agent, m_AgentInfos[group.agents[collider]], collisionPoint, group, lookAheadTime);
			}
		}
	}

	class CollisionResolveOptionGenerator
	{
	public:

		CollisionAvoidanceManager_DiscereteSearch& manager;
		AgentInfo& agent;
		AgentInfo& collider;
		AvoidanceGroup& group;
		float lookAheadTime;
		Vector2D collisionPoint;
		int nextOptionIndex;
		int nextOptionLeftIndex;
		int nextOptionRightIndex;
		int optionCountPerSide;
		Vector2D optionSegmentStart;
		Vector2D optionSegmentEnd;

		CollisionResolveOptionGenerator(CollisionAvoidanceManager_DiscereteSearch& manager_, AgentInfo& agent_, AgentInfo& collider_, const Vector2D& collisionPoint_, AvoidanceGroup& group_, float lookAheadTime_)
			:	manager(manager_)
			,	agent(agent_)
			,	collider(collider_)
			,	group(group_)
			,	lookAheadTime(lookAheadTime_)
			,	collisionPoint(collisionPoint_)
			,	nextOptionIndex(0)
			,	nextOptionLeftIndex(0)
			,	nextOptionRightIndex(0)
			,	optionCountPerSide(0)
		{
			const Terrain::AgentInfo* pAgentInfo = manager.mWorld.mTerrain->FindAgentInfo(agent.pAgent);

			if (pAgentInfo)
			{
				SetOptionSegment(pAgentInfo->location);
			}
			else
			{
				optionCountPerSide = 0;
			}
		}

		void SetOptionSegment(const Terrain::AgentLocation& loc)
		{
			Vector2D dir = rotate90(collisionPoint - agent.pAgent->GetPos());

			if (manager.mWorld.mTerrain->IntersectLineFromInside(loc, collisionPoint, dir, optionSegmentStart, optionSegmentEnd, agent.pAgent->GetRadius()) != 2)
			{
				optionCountPerSide = 0;
				return;
			}

			optionCountPerSide = (int) (Distance(optionSegmentEnd, optionSegmentStart) / (0.5f * agent.pAgent->GetRadius()));

			if (optionCountPerSide > 5)
				optionCountPerSide = 5;
		}

		virtual bool GetNextOption(AvoidanceOption& option)
		{
			if (nextOptionIndex >= (2 * optionCountPerSide) + 1)
				return false;

			option.hasVel = false;
			const Terrain::AgentInfo* pAgentInfo = manager.mWorld.mTerrain->FindAgentInfo(agent.pAgent);

			Vector2D optionCenter = (optionSegmentStart + optionSegmentEnd) * 0.5f;

			if (nextOptionRightIndex == 0 && nextOptionLeftIndex == 0 && nextOptionIndex == 0)
			{
				++nextOptionIndex;
				option.point = optionCenter;
				return true;
			}

			if (nextOptionRightIndex == nextOptionLeftIndex)
			{
				option.point = optionCenter + ((optionSegmentEnd - optionCenter) * ((float) (nextOptionRightIndex + 1) / (float) (optionCountPerSide + 1)));
				++nextOptionRightIndex;
			}
			else
			{
				option.point = optionCenter + ((optionSegmentStart - optionCenter) * ((float) (nextOptionRightIndex + 1) / (float) (optionCountPerSide + 1)));
				++nextOptionLeftIndex;
			}

			++nextOptionIndex;
			return true;
		}
	};

	class CollisionResolveOptionScorer
	{
	public:

		CollisionAvoidanceManager_DiscereteSearch& manager;
		AgentInfo& agent;
		AgentInfo& collider;
		AvoidanceGroup& group;
		float lookAheadTime;
		Vector2D collisionPoint;
		
		CollisionResolveOptionScorer(CollisionAvoidanceManager_DiscereteSearch& manager_, AgentInfo& agent_, AgentInfo& collider_, const Vector2D& collisionPoint_, AvoidanceGroup& group_, float lookAheadTime_)
			:	manager(manager_)
			,	agent(agent_)
			,	collider(collider_)
			,	group(group_)
			,	lookAheadTime(lookAheadTime_)
			,	collisionPoint(collisionPoint_)
		{
		}

		virtual float ScoreOption(const AvoidanceOption& option)
		{
			Vector2D option_agent_vel = ((option.point - agent.pAgent->GetPos()).Normalized() * agent.pAgent->GetVel().Length());
			Vector2D collider_vel = collider.hasModifiedVel ? collider.modifiedVel : collider.pAgent->GetVel();

			float collision_score = 0.0f;
			float safety_score = 0.0f;
			
			for (size_t i = 0; i < group.agents.size(); ++i)
			{
				AgentInfo& collider = manager.m_AgentInfos[group.agents[i]];

				if (&collider != &agent)
				{	
					if (collider_vel != Vector2D::kZero)
					{
						// we could use the unnormalized + response curve
						safety_score += 5.0f * (1.0f - Dot((option.point - collider.pAgent->GetPos()).Normalized(), collider_vel.Normalized()));
					}
					else
					{
						// use orientation
					}

					{
						float timeUntilCollision;

						if (manager.GetCollisionTime(agent, option_agent_vel, 
													collider, collider_vel, timeUntilCollision))
						{
							float penalty = 10.0f * (1.0f / timeUntilCollision);
							if (penalty > 100.0f)
								penalty = 100.0f;

							collision_score -= penalty;
						}
					}
				}
			}

			// we could use the unnormalized + response curve
			float deviation_score = 5.0f * (Dot(option_agent_vel.Normalized(), agent.pAgent->GetVel().Normalized()));
			
			return collision_score + safety_score + deviation_score;
		}
	};

	bool ResolveCollision(AgentInfo& agent, AgentInfo& collider, const Vector2D& collisionPoint, AvoidanceGroup& group, float lookAheadTime)
	{
		CollisionResolveOptionGenerator generator(*this, agent, collider, collisionPoint, group, lookAheadTime);
		CollisionResolveOptionScorer scorer(*this, agent, collider, collisionPoint, group, lookAheadTime);

		AgentAvoidanceOption option;
		option.agentIndex = -1;

		// Lazy hack
		for (size_t i = 0; i < group.agents.size(); ++i)
		{
			if (&m_AgentInfos[group.agents[i]] == &agent)
			{
				option.agentIndex = (int) i;
				break;
			}
		}

		option.replacePointInPath = false;
		if (agent.pAgent->mHasPath && agent.pAgent->mIndexInPath >= 0)
		{
			float test_dist = agent.pAgent->GetRadius() + collider.pAgent->GetRadius();
			test_dist += test_dist * 0.1f;

			if (Distance(agent.pAgent->mPath.GetPoint(agent.pAgent->mIndexInPath), collisionPoint) <= test_dist)
			{
				option.replacePointInPath = true;
			}
		}
		
		bool has_best_option = false;
		float best_score = -FLT_MAX;
		AgentAvoidanceOption best_option;
		
		while (generator.GetNextOption(option))
		{
			float score = scorer.ScoreOption(option);

			if (!has_best_option || score > best_score)
			{
				has_best_option = true;
				best_score = score;
				best_option = option;
			}
		}

		if (has_best_option)
		{
			group.avoidanceSolutions.push_back(best_option);
			m_AgentInfos[best_option.agentIndex].hasModifiedVel = true;
			m_AgentInfos[best_option.agentIndex].modifiedVel = ((best_option.point - agent.pAgent->GetPos()).Normalized() * agent.pAgent->GetVel().Length());
		}

		return has_best_option;
	}

	bool FindEarliestCollider(AgentInfo& agent, AvoidanceGroup& group, float lookAheadTime, int& earliestCollider, Vector2D& collisionPoint)
	{
		earliestCollider = -1;
		float earliestCollisionTime;

		for (size_t i = 0; i < group.agents.size(); ++i)
		{
			AgentInfo& collider = m_AgentInfos[group.agents[i]];

			if (&collider != &agent)
			{	
				float timeUntilCollision;

				if (GetCollisionTime(agent, agent.pAgent->GetVel(), 
					collider, collider.pAgent->GetVel(), timeUntilCollision)
					&& (timeUntilCollision <= lookAheadTime))
				{
					if (earliestCollider == -1 || timeUntilCollision < earliestCollisionTime)
					{
						earliestCollider = (int) i;
						earliestCollisionTime = timeUntilCollision;
						collisionPoint = agent.pAgent->GetPos() + (agent.pAgent->GetVel() * timeUntilCollision);
					}
				}
			}
		}

		return (earliestCollider != -1);
	}


	bool WillCollide(AgentInfo& testAgent, const Vector2D& testAgentVel, AgentInfo& obstacleAgent, const Vector2D& obstacleAgentVel, float lookAheadTime)
	{
		float timeUntilCollision;

		return (GetCollisionTime(testAgent, testAgentVel, 
				obstacleAgent, obstacleAgentVel, timeUntilCollision)
				&& (timeUntilCollision <= lookAheadTime));
	}


	bool GetCollisionTime(AgentInfo& lowAgent, const Vector2D& lowAgentVel, 
						  AgentInfo& highAgent, const Vector2D& highAgentVel, float& timeUntilCollision)
	{
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