#ifndef RandomizedFlowFieldPlanner2_h
#define RandomizedFlowFieldPlanner2_h

#include "MathInc.h"
#include "Assert.h"
#include "TestVehicle.h"

namespace planner2
{

class VehicleModel
{
public:

	TestVehicle* mVehicle;

	float GetMinSpeed()
	{
		//return -10.0f;
		return 3.0f; // for now cannot drive back, need to model right
	}

	float GetMaxSpeed()
	{
		return 29.0f;
	}

	float GetMinLocomotionSpeed()
	{
		return 3.0f;
	}

	void ModelSwitchSpeed(float inSpeed, float inTargetSpeed, float inTime, float& outSpeed, float& outDist, float& outTimeLeft)
	{
		float time_to_reach = mVehicle->mSwitchSpeedTimeCurvesRC.Get(inSpeed, inTargetSpeed, 0.5f);
		float dist_to_reach = mVehicle->mSwitchSpeedDistCurvesRC.Get(inSpeed, inTargetSpeed, 1.0f);

		outTimeLeft = time_to_reach - inTime;
		outDist = dist_to_reach * (inTime / time_to_reach);
		outSpeed = inSpeed + ((inTargetSpeed - inSpeed) * (inTime / time_to_reach));
	}

	

	void ModelSafeSteerAngle(const Vector2D& inPos, const Vector2D& inVel, float steer_dir, float inAngle, float& outTime, Vector2D& outPos, Vector2D& outVel)
	{
		float turn_radius = mVehicle->mSafeSteerTurnRadiusForSpeedRC.Get(inVel.Length(), 5.0f);
		float angle = inAngle;

		outTime = (angle * turn_radius) / inVel.Length();

		Vector2D circle_normal = rotate90(inVel.Normalized()) * steer_dir;
		Vector2D circle_center = inPos + circle_normal*turn_radius;

		Vector2D new_normal = rotate(circle_normal*-1.0f, angle*steer_dir);
		outPos = circle_center + new_normal*turn_radius;
		outVel = rotate90(new_normal) * steer_dir * inVel.Length();
	}

	void ModelSafeSteerTime(const Vector2D& inPos, const Vector2D& inVel, float steer_dir, float inTime, Vector2D& outPos, Vector2D& outVel)
	{
		float turn_radius = mVehicle->mSafeSteerTurnRadiusForSpeedRC.Get(inVel.Length(), 5.0f);
		float arc_length = inVel.Length() * inTime;
		float angle = arc_length / turn_radius;

		float outTime;

		ModelSafeSteerAngle(inPos, inVel, steer_dir, angle, outTime, outPos, outVel);
	}

	Vector2D GetDir(const Vector2D& inVel)
	{
		if (inVel == Vector2D::kZero)
			return mVehicle->GetDir();
		else
			return inVel.Normalized();
	}
};

class RandomizedFlowFieldPlanner : public VehicleController
{
public:

	struct ModelState
	{
		float mTime;
		Vector2D mPos;
		Vector2D mVel;
	};

	class Skill;

	struct ControllerState
	{
		ControllerState()
		{
			mCurrentSkill = NULL;
		}

		Skill* mCurrentSkill;
		float mCurrentSkillStartTime;

		ModelState mStartState;

		float mAccelTargetSpeed;
		int mAccelChangeCount;
		float mLastAccelDir;

		float mTurn;
		float mTurnRadius;
		float mTurnDir;
		int mTurnChangeCount;
	};

	struct SearchState
	{
		SearchState()
		{
		}

		int mIndex;
		int mParentIndex;

		float mTime;

		ModelState mModelState;
		ControllerState mControllerState;

		SearchState& CopyValues(const SearchState& inRefState)
		{
			mModelState = inRefState.mModelState;
			mControllerState = inRefState.mControllerState;
			mTime = inRefState.mTime;

			return *this;
		}

		SearchState& InitFromParent(const SearchState& inRefState)
		{
			CopyValues(inRefState);
			mParentIndex = inRefState.mIndex;

			return *this;
		}
	};
	typedef std::vector<SearchState> SearchStates;

	class Skill
	{
	public:

		virtual bool Plan(RandomizedFlowFieldPlanner& inPlanner, const SearchState& inState, SearchState& outState)
		{
			return false;
		}
	};


	class EasyDriveSkill : public Skill
	{
	public:

		VehicleController_LearnedNoSlideSteer mController;


		virtual bool Plan(RandomizedFlowFieldPlanner& inPlanner, const SearchState& inState, SearchState& outState)
		{
			if (inState.mModelState.mVel.Length() < inPlanner.GetModel().GetMinLocomotionSpeed())
				return false;

			outState.InitFromParent(inState);
			
			float time = Randf(0.5f, 1.0f);

			outState.mModelState.mVel = inState.mModelState.mVel;
			outState.mModelState.mPos = inState.mModelState.mPos + (inState.mModelState.mVel * time);
			outState.mTime = inState.mTime + time;

			return true;
		}

		virtual bool Execute(RandomizedFlowFieldPlanner& inPlanner, float inTime, float inDt, TestVehicle& inVehicle)
		{
			Vector2D flow_dir = inPlanner.GetFlowDirection(inVehicle.GetPos());
			Vector2D curr_dir = inVehicle.GetDir();

			if (Dot(curr_dir, flow_dir) > 0.8f)
			{
				mController.SetVehicle(&inVehicle);
				mController.SetFollowMouse(false);
				mController.SetTarget(inVehicle.GetPos() + flow_dir);
				mController.SetMaxSpeed(inPlanner.GetModel().GetMaxSpeed());

				mController.Update(inTime, inDt);

				return true;
			}
			else
			{
				mController.SetVehicle(&inVehicle);
				mController.SetFollowMouse(false);
				mController.SetTarget(inVehicle.GetPos() + flow_dir);
				mController.SetMaxSpeed(inPlanner.GetModel().GetMaxSpeed());

				mController.Update(inTime, inDt);

				return false;
			}
		}
	};

	class AccelerateSkill : public Skill
	{
	public:

		//http://www.visualexpert.com/Resources/reactiontime.html


		virtual bool Plan(RandomizedFlowFieldPlanner& inPlanner, const SearchState& inState, SearchState& outState)
		{
			outState.InitFromParent(inState);

			float actual_speed = inState.mModelState.mVel.Length();
			float target_speed = actual_speed;
			
			while(fabs(target_speed-actual_speed) < 3.0f)
				target_speed = Randf(inPlanner.GetModel().GetMinSpeed(), inPlanner.GetModel().GetMaxSpeed() * 0.4f);

			float time = Randf(0.5f, 2.0f);

			float curr_speed;
			float curr_dist;
			float time_left;
			inPlanner.GetModel().ModelSwitchSpeed(inState.mModelState.mVel.Length(), target_speed, time, curr_speed, curr_dist, time_left);

			Vector2D dir = inPlanner.GetModel().GetDir(inState.mModelState.mVel);
			outState.mModelState.mVel = (dir) * curr_speed;
			outState.mModelState.mPos = inState.mModelState.mPos + (dir * curr_dist);
			outState.mTime = inState.mTime + time;

			return true;
		}
	};


	class TurnSkill : public Skill
	{
	public:

		//http://www.visualexpert.com/Resources/reactiontime.html

		virtual bool Plan(RandomizedFlowFieldPlanner& inPlanner, const SearchState& inState, SearchState& outState)
		{
			outState.InitFromParent(inState);

			float curr_speed = inState.mModelState.mVel.Length();
			float target_speed = curr_speed;

			while(fabs(target_speed-curr_speed) < 3.0f)
				target_speed = Randf(inPlanner.GetModel().GetMinSpeed(), inPlanner.GetModel().GetMaxSpeed());

			float time;
			float turn_dir = Randf(0.0f, 1.0f) > 0.5f ? 1.0f : -1.0f;
			float angle = Randf(DegToRad(20.0f), DegToRad(180.0f));

			inPlanner.GetModel().ModelSafeSteerAngle(inState.mModelState.mPos, inState.mModelState.mVel, turn_dir, angle, time, outState.mModelState.mPos, outState.mModelState.mVel);
			outState.mTime = inState.mTime + time;

			return true;
		}
	};

	VehicleModel& GetModel() { return mModel; }
	ModelState& GetModelState(int inIndex) { return mSearchStates[inIndex].mModelState; }
	ControllerState& GetControllerState(int inIndex) { return mSearchStates[inIndex].mControllerState; }

	Vector2D GetFlowDirection(Vector2D inPos)
	{
		return Vector2D(0.5f, 0.5f);
	}

	int curr_free_state;
	
	void TestSearch()
	{
		if (mSearchStates.empty())
		{
			mSearchSkills[0] = &mAccelerateSkill;
			mSearchSkills[1] = &mTurnSkill;
			mSearchSkills[2] = NULL;

			mSearchStates.resize(10000);
			ModelState& model_state = mSearchStates.front().mModelState;
			model_state.mPos = mVehicle->GetPos();
			model_state.mVel = mVehicle->GetVel();
			model_state.mTime = 0.0f;

			for (int i=0; i<mSearchStates.size(); ++i)
			{
				mSearchStates[i].mIndex = i;
				mSearchStates[i].mParentIndex = -1;
			}

			curr_free_state=1;
		}

 		while (curr_free_state < mSearchStates.size())
		{
			int loop_currfree_state = curr_free_state;

			for (int i=0; i<loop_currfree_state; ++i)
			{
				SearchState& search_state = mSearchStates[i];

				for (int j=0; j<3; ++j)
				{
					if (mSearchSkills[j] != NULL)
					{
						if (mSearchSkills[j]->Plan(*this, search_state, mSearchStates[curr_free_state]))
							++curr_free_state;

						if (curr_free_state == mSearchStates.size())
							return;
					}
				}
			}

			break;
		}
	}

	virtual void Update(float inTime, float inDt)
	{
		if (mSearchStates.size() == 0)
			TestSearch();

		mEasyDriveSkill.Execute(*this, inTime, inDt, *mVehicle);
	}


	virtual void SetVehicle(TestVehicle* pVehicle) { mVehicle = pVehicle; mModel.mVehicle = pVehicle; }
	virtual bool IsFinished() { return false; }

	virtual void Draw(Renderer& renderer, float t) 
	{
		int x; int y;
		SDL_GetMouseState(&x, &y);
		Vector2D worldPos = renderer.ScreenToWorld(Vector2D((float) x, (float) y));

		for (int i=0; i<curr_free_state; ++i)
		{
			SearchState& state = mSearchStates[i];
			SearchState* pParentState = mSearchStates[i].mParentIndex >= 0 ? &mSearchStates[mSearchStates[i].mParentIndex] : NULL;

			renderer.DrawCircle(renderer.WorldToScreen(state.mModelState.mPos), renderer.WorldToScreen(0.2f), Color::kGreen, -1.0f, true);
			renderer.DrawArrow(renderer.WorldToScreen(state.mModelState.mPos), renderer.WorldToScreen(state.mModelState.mPos+(state.mModelState.mVel*0.2f)), Color::kGreen, 0.5f);

			if (Distance(worldPos, state.mModelState.mPos) <= 0.2f)
				printf("%d\n", i);


			//if (pParentState)
			//	renderer.DrawLine(renderer.WorldToScreen(state.mModelState.mPos), renderer.WorldToScreen(pParentState->mModelState.mPos), Color::kBlue, -1.0f, 0.5f);
		}
	}

	virtual void HandleEvent(const SDL_Event& evt) 
	{
		if (evt.type == SDL_KEYDOWN)
		{
			if (evt.key.keysym.sym == SDLK_SPACE)
			{
				TestSearch();
			}
		}
	}

	TestVehicle* mVehicle;
	VehicleModel mModel;
	EasyDriveSkill mEasyDriveSkill;
	AccelerateSkill mAccelerateSkill;
	TurnSkill mTurnSkill;
	Skill* mSearchSkills[3];
	SearchStates mSearchStates;
};

}

#endif