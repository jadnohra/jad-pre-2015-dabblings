#include <vector>
#include <string>

/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
Plans should really not extend to the future and shoudl be monitored very little.
A plan to go somewhere and attack somebody is a bit stupid, many things can change.
A better plan is to go somehwere while monitoring the threat (for death) and keeping it in memory, monitoring it differently for importance
**
goal monitor:

UTILITY
|
|    -
|     -
|             -
| -  
|-
|       --
|  -      -
|          -
|
------------------------------------------ GOAL

the current plan has an effect on a group of the current goals
a plan switch has a cost and affects a potentially different group of goals.
we only switch a plan if it improves the total score of all covered goals weighted by how much plan affects goal.

*/

class ArmedHumanoid;

class AttackPositionPicker
{
public:

	struct AttackPosition
	{
	};

	static AttackPosition* FindAttackPosition(ArmedHumanoid& inAttacker, ArmedHumanoid& inTarget)
	{
		return new AttackPosition();
	}
};


class Pathfinder
{
public:

	struct Path
	{
	};

	static Path* FindPath(ArmedHumanoid& inAttacker, AttackPositionPicker::AttackPosition& inTargetPosition)
	{
		return new Path();
	}
};


class PlannedAction
{
public:

	virtual ~PlannedAction() {}
};


class ArmedHumanoid
{
public:

	int GetThreatCount() { return 3; }
	int GetThreat(int inIndex) { return inIndex; }

	class ActionFireAtThreat : public PlannedAction
	{
	public:

		int mThreatIndex;

		ActionFireAtThreat(int inThreatIndex)
		:	mThreatIndex(inThreatIndex)
		{
		}

		virtual void Execute(ArmedHumanoid& inEntity) 
		{
			// do nothing	
			//inEntity.ExecuteActionFireAtThreat(*this); 
		}
	};

	PlannedAction* CreateActionFireAtThreat(int inIndex)
	{
		return new ActionFireAtThreat(inIndex);
	}

	class ActionGoto : public PlannedAction
	{
	public:

		Pathfinder::Path* mPath;

		ActionGoto(Pathfinder::Path* inPath)
		:	mPath(inPath)
		{
		}

		~ActionGoto()
		{
			delete mPath;
		}

		virtual void Execute(ArmedHumanoid& inEntity) 
		{
			// do nothing	
		}
	};

	PlannedAction* CreateActionGoto(Pathfinder::Path* inPath)
	{
		return new ActionGoto(inPath);
	}
};




template<typename taItemType>
class SimpleMemoryContainer
{
public:

	typedef std::vector<taItemType> Items;
	Items mItems;

	void				Add(const taItemType& inItem)			{ mItems.push_back(inItem); }
	size_t				GetItemCount() const					{ return mItems.size(); }	
	const taItemType&	GetItem(size_t inIndex) const			{ return mItems[inIndex]; }	
	taItemType&			GetItem(size_t inIndex)					{ return mItems[inIndex]; }	
};

typedef SimpleMemoryContainer<std::string> StringMemory;

class ArmedHumanoidMemory :
	public StringMemory
{
public:

	template <typename taContainerType> taContainerType& GetContainer()		{ return static_cast<taContainerType>(*this); }
};

class Plan
{
public:

	typedef std::vector<PlannedAction*> Actions;
	Actions mActions;
	bool mIsCommited;

	class Branch
	{
	public:
		
		Plan& mPlan;
		size_t mRollbackSize;
		
		Branch(Plan& inPlan)
		:	mPlan(inPlan)
		,	mRollbackSize(inPlan.mActions.size())
		{
		}

		Branch(Plan& inPlan, size_t inRollbackSize)
		:	mPlan(inPlan)
		,	mRollbackSize(inRollbackSize)
		{
		}

		void AddAction(PlannedAction* inAction)
		{
			mPlan.mActions.push_back(inAction);
		}

		bool Succeed()
		{
			mPlan.Commit();
			return true;
		}

		bool Fail()
		{
			return false;
		}

		~Branch()
		{
			mPlan.Unroll(*this);
		}
	};

	Plan()
	:	mIsCommited(false)
	{
	}

	~Plan()
	{
		mIsCommited = false;
		Unroll(Branch(*this, 0));
	}

	void Commit()
	{
		mIsCommited = true;
	}

protected:

	void Unroll(Branch& inRollback)
	{
		if (!mIsCommited)
		{
			for (size_t i=inRollback.mRollbackSize; i<mActions.size(); ++i)
			{
				delete mActions[i];
			}

			mActions.resize(inRollback.mRollbackSize);
		}
	}
};

class Domain1
{
public:

	// choose threat
	// choose attack position
	// find path
	// go there
	// otherwize choose weapon
	// fire if in range

	struct DecompositionContext
	{
	public:

		DecompositionContext(ArmedHumanoid& inEntity, ArmedHumanoidMemory& inMemory, Plan& outPlan)
		:	mEntity(inEntity)
		,	mMemory(inMemory)
		,	mPlan(outPlan)
		{
		}

		ArmedHumanoid& mEntity;
		ArmedHumanoidMemory& mMemory;
		Plan& mPlan;

		bool SucceedPlan()
		{
			mPlan.Commit();
			return true;
		}

		bool FailPlan()
		{
			return false;
		}
	};


	class ThreatImportanceMonitor
	{
	public:

		int mThreatIndex;

		ThreatImportanceMonitor(int inThreatIndex)
		:	mThreatIndex(mThreatIndex)
		{
		}
	};


	static bool PlanBehave(DecompositionContext& inContext)
	{
		int ordered_threats[4] = { -1, -1, -1, -1 };

		for (int i=0; i<inContext.mEntity.GetThreatCount(); ++i)
		{
			ordered_threats[i] = i;
		}

		// try to attack
		{
			for (int i=0; i<inContext.mEntity.GetThreatCount(); ++i)
			{
				Plan::Branch branch(inContext.mPlan);
				{
					if (PlanAttack(inContext, ordered_threats[i]))
					{
						branch.AddMonitor(new ThreatImportanceMonitor(i));
						return branch.Succeed();
					}
				}
			}
		}

		// try to move to better position
		{
			for (int i=0; i<inContext.mEntity.GetThreatCount(); ++i)
			{
				Plan::Branch branch(inContext.mPlan);
				{
					if (PlanMoveToGoodAttackPosition(inContext, ordered_threats[i]))
					{
						branch.AddMonitor(new ThreatImportanceMonitor(i));
						return branch.Succeed();
					}
				}
			}
		}

		return inContext.FailPlan();
	}

	static bool PlanMoveToGoodAttackPosition(DecompositionContext& inContext, int inThreat)
	{
		{
			AttackPositionPicker::AttackPosition* attack_position = AttackPositionPicker::FindAttackPosition(inContext.mEntity, inContext.mEntity);

			if (attack_position != NULL)
			{
				Pathfinder::Path* path = Pathfinder::FindPath(inContext.mEntity, *attack_position);
				delete attack_position;

				if (path != NULL)
				{
					Plan::Branch branch(inContext.mPlan);
					branch.AddAction(inContext.mEntity.CreateActionGoto(path));

					return inThreat >= 2 ? branch.Succeed() : branch.Fail();
				}
			}
		}

		return inContext.FailPlan();
	}

	static bool PlanAttack(DecompositionContext& inContext, int inThreat)
	{
		Plan::Branch branch(inContext.mPlan);
		branch.AddAction(inContext.mEntity.CreateActionFireAtThreat(inThreat));

		return inThreat > 5 ? branch.Succeed() : branch.Fail();
	}
};

int main()
{
	ArmedHumanoid entity;
	ArmedHumanoidMemory memory;
	Plan plan;

	Domain1::PlanBehave(Domain1::DecompositionContext(entity, memory, plan));

	return 0;
}