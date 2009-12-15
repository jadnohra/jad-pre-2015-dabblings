#ifndef PLANNER_H
#endif PLANNER_H

#include "SuperHeader.h"

namespace planner
{

class PlanBranchManager
{
public:
};

class PlannerContext
{
public:

	int mFrame;
};

class WorldState
{
public:

	virtual ~WorldState() {}
};

class ActionState
{
public:

	virtual ~ActionState() {}

	int mStartFrame;
};

class Action
{
public:
	virtual ~Action() {}
	
	virtual bool ContinueFrom(const PlannerContext& context, WorldState* pWorldState, ActionState* pState, bool& isInterruptible) { return false;  }
};

class Manoeuvre
{
public:

	virtual ~Manoeuvre() {}
	virtual bool ContinueFrom(const PlannerContext& context, WorldState* pWorldState, ActionState* pManoeuvreState, ActionState* pState, PlanBranchManager& branchManager) { return false;  }
};

class Planner
{
public:

};

class VehicleMotionModel
{
public:

	bool ApplyDeceleration(float mBrakeAmount, VehicleWorldState& worldState)
	{
		if (worldState.mSteer != 0.0f)
			return false;

		//learn speed+brake-amount -> decel
		return true;
	}


	bool ApplySteering(float mBrakeAmount, VehicleWorldState& worldState)
	{
		// check stability
		//if (worldState.mSteer != 0.0f)
		//	return false;

		//learn speed+steer-amount -> new state
		return true;
	}

	bool ApplyGoStraight(VehicleWorldState& worldState)
	{
		// 
		return true;
	}
};

class VehiclePlannerContext : public PlannerContext
{
public:

	VehicleMotionModel mModel;
};

class VehicleWorldState
{
public:

	int mFrame;

	Vector2D mPosition;
	Vector2D mForward;
	float mSpeed;	// assumed stability
	float mSteer;
};



class VehicleActionGoStraight : public Action
{
public:

	int mMinFrameCount;

	void Setup(int minFrameCount)
	{
		mMinFrameCount = minFrameCount;
	}

	virtual bool ContinueFrom(const PlannerContext& context, WorldState* pWorldState, ActionState* pState, bool& isInterruptible) 
	{ 
		VehicleWorldState& worldState = *((VehicleWorldState*) pWorldState);

		if (worldState.mSpeed == 0.0f)
			return false;

		isInterruptible = (pState->mFrame - worldState.mFrame) > mMinFrameCount;
		if (!((VehiclePlannerContext&) context).mModel.ApplyGoStraight(*pWorldState))
			return false;
		
		return true; 
	}
};

class VehicleActionConstantBrake : public Action
{
public:

	float mBrakeAmount;
	int mMinFrameCount;

	class State : public ActionState
	{
	public:
	};

	void Setup(float brakeAmount, int minFrameCount)
	{
		mBrakeAmount = decel;
		mMinFrameCount = minFrameCount;
	}
	
	virtual bool ContinueFrom(const PlannerContext& context, WorldState* pWorldState, ActionState* pState, bool& isInterruptible) 
	{ 
		VehicleWorldState& worldState = *((VehicleWorldState*) pWorldState);

		if (worldState.mSpeed <= 0.0f)
			return false;

		isInterruptible = (pState->mFrame - worldState.mFrame) > mMinFrameCount;
		if (!((VehiclePlannerContext&) context).mModel.ApplyDeceleration(mBrakeAmount, *pWorldState))
			return false;
		
		return true; 
	}
};

class VehicleActionConstantSteer : public Action
{
public:

	float mSteerAmount;
	int mMinFrameCount;
	int mMaxFrameCount;

	class State : public ActionState
	{
	public:


	};

	void Setup(float steerAmount, int minFrameCount)
	{
		mSteerAmount = steerAmount;
		mMinFrameCount = minFrameCount;
		//mMaxFrameCount = from model, full loop
	}
	
	virtual bool ContinueFrom(const PlannerContext& context, WorldState* pWorldState, ActionState* pState, bool& isInterruptible) 
	{ 
		VehicleWorldState& worldState = *((VehicleWorldState*) pWorldState);

		if (worldState.mSpeed <= 0.0f)
			return false;

		if ((pState->mFrame - worldState.mFrame) > mMaxFrameCount)
		{
			return false;
		}

		isInterruptible = (pState->mFrame - worldState.mFrame) > mMinFrameCount;
		if (!((VehiclePlannerContext&) context).mModel.ApplySteering(mSteerAmount, *pWorldState))
			return false;
		
		return true; 
	}
};

class VehicleManoeuvre_TurnStraightTurn : public Manoeuvre
{
public:

	enum ActionType
	{
		Invalid, Steer, Brake, Go
	};

	public State : public ActionState
	{
	public:

		ActionType mActionType[4];
		int mActionCount;
	};

	VehicleActionConstantSteer mActionConstantSteer;
	VehicleActionConstantBrake mActionConstantBrake;
	VehicleActionGoStraight mActionGoStraight;

	Action* GetAction(ActionType type)
	{
		switch (type)
		{
			case Steer: return &mActionConstantSteer;
			case Brake: return &mActionConstantBrake;
			case Go: return &mActionGoStraight;
		}

		return NULL;
	}

	bool BrancheFrom(const PlannerContext& context, WorldState* pWorldState, State& manoeuvreState, PlanBranchManager& branchManager) 
	{
		if (mActionCount == 4)
			return false;

		int steer_count = 0;

		for (int i=0;i<manoeuvreState.mActionCount;++i)
		{
			if (mActionType[i]==Steer)
				++steer_count;
		}

		if (steer_count >= 2)
			return false;
	}

	virtual bool ContinueFrom(const PlannerContext& context, WorldState* pWorldState, ActionState* pManoeuvreState, ActionState* pState, PlanBranchManager& branchManager) 
	{ 
		State& state = *((State*) pManoeuvreState);
		Action* pAction = GetAction(state.mActionType);

		bool isInterruptible;

		if (!pAction->ContinueFrom(context, pWorldState, pActionState, isInterruptible))
			isInterruptible = true;

		if (isInterruptible)
		{
			if (BrancheFrom(context, pWorldState, state, branchManager))
				return true;
		}

		return false;  
	}
};


}

#endif