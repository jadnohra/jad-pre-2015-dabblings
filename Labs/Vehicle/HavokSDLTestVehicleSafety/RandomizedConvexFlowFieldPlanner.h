#ifndef RandomizedConvexFlowFieldPlanner_h
#define RandomizedConvexFlowFieldPlanner_h

#include "MathInc.h"

class RandomizedConvexFlowFieldPlanner
{
public:

	struct ModelState
	{
		Vector2D mPos;
		Vector2D mVel;
	};

	struct ControllerState
	{
		float mAccelTargetSpeed;
		float mAccelStartTime;
		float mAccelStopTime;
		int mAccelChangeCount;

		float mTurn;
		float mTurnRadius;
		float mTurnStartTime;
		float mTurnStopTime;
		int mTurnChangeCount;
	};
};

#endif