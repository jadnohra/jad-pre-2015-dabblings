#ifndef RandomizedFlowFieldPlanner_h
#define RandomizedFlowFieldPlanner_h

#include "MathInc.h"
#include "Assert.h"
#include "TestVehicle.h"

namespace planner
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

	void ModelSafeSteer(const Vector2D& inPos, const Vector2D& inVel, float steer_dir, float inTime, Vector2D& outPos, Vector2D& outVel)
	{
		float turn_radius = mVehicle->mSafeSteerTurnRadiusForSpeedRC.Get(inVel.Length(), 5.0f);
		float arc_length = inVel.Length() * inTime;
		float angle = arc_length / turn_radius;

		Vector2D circle_normal = rotate90(inVel.Normalized()) * steer_dir;
		Vector2D circle_center = inPos + circle_normal*turn_radius;

		Vector2D new_normal = rotate(circle_normal*-1.0f, angle);
		outPos = circle_center + new_normal*turn_radius;
		outVel = rotate90(new_normal) * steer_dir * inVel.Length();
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
		int mIndex;
		int mParentState;

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

		int mIndex;
		int mParentState;
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

		ControllerState& CopyValues(const ControllerState& inRefState)
		{
			int index = mIndex;
			std::memcpy(this, &inRefState, sizeof(ControllerState));
			mIndex = index;

			return *this;
		}
	};

	struct SearchState
	{
		SearchState()
		{
			mIsClosed = false;
		}

		bool mIsClosed;
		ModelState mModelState;
		ControllerState mControllerState;
	};
	typedef std::vector<SearchState> SearchStates;

	class Skill
	{
	public:

		virtual bool IsInterruptible(float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState) = 0;

		virtual bool Continue(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
			ModelState& outModelState, ControllerState& outControllerState, bool& outClose) = 0;

		virtual bool NewState(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
			ModelState& outModelState, ControllerState& outControllerState) = 0;


		virtual bool Execute(RandomizedFlowFieldPlanner& inPlanner, float inTime, float inDt, VehicleModel& inModel, const ModelState& inModelState, TestVehicle& inVehicle)
		{
			return false;
		}
	};


	class EasyDriveSkill : public Skill
	{
	public:

		VehicleController_LearnedNoSlideSteer mController;

		virtual bool IsInterruptible(float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState) 
		{
			return (inTime - inControllerState.mCurrentSkillStartTime) > 0.7f;
		}

		virtual bool Continue(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
							  ModelState& outModelState, ControllerState& outControllerState, bool& outClose) 
		{
			outModelState.mVel = inModelState.mVel;
			outModelState.mPos = inControllerState.mStartState.mPos + (inControllerState.mStartState.mVel * (inTime-inControllerState.mCurrentSkillStartTime));
			outModelState.mParentState = inModelState.mIndex;
			
			outControllerState.CopyValues(inControllerState);
			outControllerState.mParentState = inControllerState.mIndex;

			outClose = false;
			return true;
		}

		virtual bool NewState(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
								ModelState& outModelState, ControllerState& outControllerState)
		{
			ControllerState temp_controller_state = inControllerState;
			temp_controller_state.mIndex = inControllerState.mIndex;

			temp_controller_state.mStartState = inModelState;
			temp_controller_state.mCurrentSkill = this;
			temp_controller_state.mCurrentSkillStartTime = inTime;
			
			bool close;
			return Continue(inPlanner, inTime, inModel, inModelState, temp_controller_state, outModelState, outControllerState, close);
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
		virtual bool IsInterruptible(float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState)
		{
			return (inTime - inControllerState.mCurrentSkillStartTime) > 3.0f;
		}

		virtual bool Continue(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
			ModelState& outModelState, ControllerState& outControllerState, bool& outClose)
		{
			float speed = inModelState.mVel.Length();
			float prev_speed = inModelState.mParentState >= 0 ? inPlanner.GetModelState(inModelState.mParentState).mVel.Length() : speed;

			if (speed >= prev_speed)
			{
				if (speed >= inControllerState.mAccelTargetSpeed)
					return false;
			}
			else
			{
				if (speed <= inControllerState.mAccelTargetSpeed)
					return false;
			}

			// We can continue
			float curr_speed;
			float curr_dist;
			float time_left;
			inModel.ModelSwitchSpeed(inControllerState.mStartState.mVel.Length(), inControllerState.mAccelTargetSpeed, inTime - inControllerState.mCurrentSkillStartTime, curr_speed, curr_dist, time_left);

			Vector2D dir = inControllerState.mStartState.mVel.Normalized();
			outModelState.mVel = (dir) * curr_speed;
			outModelState.mPos = inControllerState.mStartState.mPos + (dir * curr_dist);
			outModelState.mParentState = inModelState.mIndex;

			outControllerState.CopyValues(inControllerState);
			outControllerState.mParentState = inControllerState.mIndex;

			gAssert(outControllerState.mParentState != outControllerState.mIndex);

			outClose = time_left<=0.0f;
			return true;
		}

		virtual bool NewState(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
			ModelState& outModelState, ControllerState& outControllerState)
		{
			if (inControllerState.mAccelChangeCount > 3)
				return false;

			Vector2D flow_dir = inPlanner.GetFlowDirection(inModelState.mPos);
			float is_in_flow_dir = Dot(inModel.GetDir(inModelState.mVel), flow_dir) >= 0.0f;
			float min_allowed;
			float max_allowed;

			if (is_in_flow_dir)
			{
				if (inControllerState.mAccelChangeCount > 0 && inControllerState.mLastAccelDir > 0.0f 
					&& (inControllerState.mCurrentSkill != NULL && inControllerState.mCurrentSkill == this)
					&& (std::abs(inModelState.mVel.Length()) >= inModel.GetMinLocomotionSpeed()))
					return false;

				max_allowed = inModel.GetMaxSpeed();
				min_allowed = std::min(max_allowed, inModelState.mVel.Length() + 1.0f);
			}
			else
			{
				if (inControllerState.mAccelChangeCount > 0 && inControllerState.mLastAccelDir < 0.0f
					&& (inControllerState.mCurrentSkill != NULL && inControllerState.mCurrentSkill == this)
					&& (std::abs(inModelState.mVel.Length()) >= inModel.GetMinLocomotionSpeed()))
					return false;

				min_allowed = inModel.GetMinSpeed();
				max_allowed = std::max(min_allowed, inModelState.mVel.Length() - 1.0f);
			}

			if (min_allowed >= max_allowed)
				return false;

			ControllerState temp_controller_state = inControllerState;

			++temp_controller_state.mAccelChangeCount;
			temp_controller_state.mParentState = inControllerState.mIndex;
			temp_controller_state.mLastAccelDir = min_allowed > inModelState.mVel.Length() ? 1.0f : -1.0f;
			temp_controller_state.mStartState = inModelState;

			if (temp_controller_state.mStartState.mVel == Vector2D::kZero)
				temp_controller_state.mStartState.mVel = inModel.GetDir(outControllerState.mStartState.mVel);

			temp_controller_state.mAccelTargetSpeed = Randf(min_allowed, max_allowed);
			temp_controller_state.mCurrentSkill = this;
			temp_controller_state.mCurrentSkillStartTime = inModelState.mTime;

			bool close;
			return Continue(inPlanner, inTime, inModel, inModelState, temp_controller_state, outModelState, outControllerState, close);
		}
	};


	class TurnSkill : public Skill
	{
	public:

		//http://www.visualexpert.com/Resources/reactiontime.html
		virtual bool IsInterruptible(float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState)
		{
			return (inTime - inControllerState.mCurrentSkillStartTime) > 3.0f;
		}

		virtual bool Continue(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
			ModelState& outModelState, ControllerState& outControllerState, bool& outClose)
		{
			float turned_dot = Dot(inControllerState.mStartState.mVel.Normalized(), inModelState.mVel.Normalized());
			float pos_dot = Dot(inControllerState.mStartState.mVel, inModelState.mPos-inControllerState.mStartState.mPos);

			// We can continue
			inModel.ModelSafeSteer(inControllerState.mStartState.mPos, inControllerState.mStartState.mVel, inControllerState.mTurnDir, inTime - inControllerState.mCurrentSkillStartTime, outModelState.mPos, outModelState.mVel);

			outModelState.mParentState = inModelState.mIndex;
			outControllerState.CopyValues(inControllerState);
			outControllerState.mParentState = inControllerState.mIndex;
			outClose = (turned_dot >= 0.0f && pos_dot < 0.0f);
			return true;
		}

		virtual bool NewState(RandomizedFlowFieldPlanner& inPlanner, float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
			ModelState& outModelState, ControllerState& outControllerState)
		{
			if (inControllerState.mTurnChangeCount > 3)
				return false;

			if (inModelState.mVel.Length() < inModel.GetMinLocomotionSpeed())
				return false;

			Vector2D flow_dir = inPlanner.GetFlowDirection(inModelState.mPos);

			ControllerState temp_controller_state = inControllerState;

			if (outControllerState.mIndex == 47)
			{
				int x=0;
				++x;
			}

			++temp_controller_state.mTurnChangeCount;
			temp_controller_state.mParentState = inControllerState.mIndex;
			temp_controller_state.mStartState = inModelState;
			temp_controller_state.mCurrentSkill = this;
			temp_controller_state.mCurrentSkillStartTime = inModelState.mTime;
			temp_controller_state.mTurnDir = Randf(0.0f, 1.0f) > 0.5f ? 1.0f : -1.0f;

			bool close;
			return Continue(inPlanner, inTime, inModel, inModelState, temp_controller_state, outModelState, outControllerState, close);
		}
	};

	VehicleModel& GetModel() { return mModel; }
	ModelState& GetModelState(int inIndex) { return mSearchStates[inIndex].mModelState; }
	ControllerState& GetControllerState(int inIndex) { return mSearchStates[inIndex].mControllerState; }

	Vector2D GetFlowDirection(Vector2D inPos)
	{
		return Vector2D(0.5f, 0.5f);
	}

	bool NewState(float inTime, VehicleModel& inModel, const ModelState& inModelState, const ControllerState& inControllerState,
		ModelState& outModelState, ControllerState& outControllerState)
	{
	}

	int curr_free_state;
	float t;
	float dt;

	void TestSearch()
	{
		if (mSearchStates.empty())
		{
			mSearchSkills[0] = &mAccelerateSkill;
			mSearchSkills[1] = &mTurnSkill;
			mSearchSkills[2] = NULL;

			mSearchStates.resize(1000);
			ModelState& model_state = mSearchStates.front().mModelState;
			model_state.mPos = mVehicle->GetPos();
			model_state.mVel = mVehicle->GetVel();
			model_state.mIndex = 0;
			model_state.mParentState = -1;
			model_state.mTime = 0.0f;

			ControllerState& controller_state = mSearchStates.front().mControllerState;
			controller_state.mAccelChangeCount = 0;
			controller_state.mTurnChangeCount = 0;
			controller_state.mCurrentSkill = NULL;
			controller_state.mIndex = 0;
			controller_state.mParentState = -1;

			for (int i=0; i<mSearchStates.size(); ++i)
			{
				mSearchStates[i].mModelState.mIndex = i;
				mSearchStates[i].mControllerState.mIndex = i;
			}

			curr_free_state=1;
			dt = 0.2f;
			t = 0.0f + dt;
		}

		while (curr_free_state < mSearchStates.size())
		{
			int loop_currfree_state = curr_free_state;

			for (int i=0; i<loop_currfree_state; ++i)
			{
				SearchState& search_state = mSearchStates[i];

				if (!search_state.mIsClosed)
				{
					if (curr_free_state >= mSearchStates.size())
						break;

				
					bool do_branch = false;

					if (search_state.mControllerState.mCurrentSkill != NULL)
					{
						if (search_state.mControllerState.mCurrentSkill->IsInterruptible(t, mModel, search_state.mModelState, search_state.mControllerState))
						{
							if (search_state.mControllerState.mCurrentSkill->Continue(*this, t, mModel, 
								search_state.mModelState, search_state.mControllerState, 
								mSearchStates[curr_free_state].mModelState, mSearchStates[curr_free_state].mControllerState, search_state.mIsClosed))
							{
								mSearchStates[curr_free_state].mModelState.mTime = t;
								mSearchStates[curr_free_state].mIsClosed = false;

								++curr_free_state;
								if (curr_free_state >= mSearchStates.size())
									break;
							}
						}
						else
						{
							if (search_state.mControllerState.mCurrentSkill->Continue(*this, t, mModel, 
								search_state.mModelState, search_state.mControllerState, 
								mSearchStates[curr_free_state].mModelState, mSearchStates[curr_free_state].mControllerState, search_state.mIsClosed))
							{
								mSearchStates[curr_free_state].mModelState.mTime = t;
								mSearchStates[curr_free_state].mIsClosed = true;

								++curr_free_state;
								if (curr_free_state >= mSearchStates.size())
									break;
							}
							else
							{
								search_state.mIsClosed = true;
							}

							do_branch = search_state.mIsClosed;
						}
					}
					else
						do_branch = true;

					if (do_branch)
					{
						for (int j=0; j<3; ++j)
						{
							if (mSearchSkills[j] != NULL)
							{
								if (mSearchSkills[j]->NewState(*this, t, mModel, 
									search_state.mModelState, search_state.mControllerState, 
									mSearchStates[curr_free_state].mModelState, mSearchStates[curr_free_state].mControllerState))
								{
									mSearchStates[curr_free_state].mModelState.mTime = t;

									++curr_free_state;
									if (curr_free_state >= mSearchStates.size())
										break;
								}
							}
						}
					}
				}

				if (!search_state.mIsClosed)
				{
					if (i==0)
						search_state.mIsClosed = true;
				}
			}
			t+=dt;

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
			SearchState* pParentState = mSearchStates[i].mModelState.mParentState >= 0 ? &mSearchStates[mSearchStates[i].mModelState.mParentState] : NULL;

			renderer.DrawCircle(renderer.WorldToScreen(state.mModelState.mPos), renderer.WorldToScreen(0.2f), Color::kGreen, -1.0f, true);
			renderer.DrawArrow(renderer.WorldToScreen(state.mModelState.mPos), renderer.WorldToScreen(state.mModelState.mPos+(state.mModelState.mVel*0.2f)), Color::kGreen, 0.5f);

			if (Distance(worldPos, state.mModelState.mPos) <= 0.2f)
				printf("%d\n", i);


			if (pParentState)
				renderer.DrawLine(renderer.WorldToScreen(state.mModelState.mPos), renderer.WorldToScreen(pParentState->mModelState.mPos), Color::kBlue, -1.0f, 0.5f);
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