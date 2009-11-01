#ifndef AGENT_H
#define AGENT_H

#include "Math.h"
#include "Renderer.h"
#include "World.h"
#include "Terrain.h"

class Agent : public TerrainAgent
{
public:

	static const float kVelDrawScale;

	World* mWorld;
	Vector2D mPos;
	Vector2D mVel;
	float mRadius;
	bool mHasGoal;
	Vector2D mGoalPos;

	float mUpdateTime;
	Vector2D mPrevUpdatePos;
	float mPrevUpdateTime;
	Color mColor;

	bool mIsAgitated;
	float mAgitation;

	bool mIsControlledByAvoidance;
	float mLastControlledByAvoidanceTime;

	Agent(World* pWorld)
	{
		mWorld = pWorld;
		mIsAgitated = false;
		mAgitation = 0.0f;

		init(Vector2D::kZero, Vector2D::kZero, 1.0f, Color::kWhite);
	}

	Agent(World* pWorld, const Vector2D& pos_, const Vector2D& vel_, float radius_, const Color color_)
	{
		mWorld = pWorld;

		init(pos_, vel_, radius_, color_);
	}

	void init(const Vector2D& pos_, const Vector2D& vel_, float radius_, const Color color_)
	{
		mUpdateTime = 0.0f;
		mPrevUpdateTime = 0.0f;
		
		mRadius = radius_;
		mVel = vel_; 
		mPos = pos_;
		mColor = color_;
		mHasGoal = false;

		mIsControlledByAvoidance = false;
		mLastControlledByAvoidanceTime = -1.0f;
	}

	virtual Circle GetTerrainShape() 
	{
		return Circle(mPos, mRadius);
	}

	virtual void Agitate()			 { mIsAgitated = true; }
	virtual const Vector2D& GetPos() { return mPos; }
	virtual const Vector2D& GetVel() { return mVel; }
	virtual float GetRadius() { return mRadius; }
	virtual void NotifyControlledByAvoidance() { mIsControlledByAvoidance = true; }


	virtual void SetPos(const Vector2D& position) { mPos = position; }
	virtual void SetVel(const Vector2D& velocity) { mVel = velocity; }

	virtual void SetGoal(const Vector2D& pos, float speed) { mGoalPos = pos; mHasGoal = true; SetVel((mGoalPos - GetPos()).Normalized() * speed); }

	virtual void Update(float time, float dt) 
	{
		mPrevUpdateTime = mUpdateTime;
		mPrevUpdatePos = mPos;

		if (mPrevUpdateTime == 0.0f)
			mPrevUpdateTime = time;

		mUpdateTime = time;
		mPos += mVel * dt;

		static const bool kLimitBounce = true;

		if (kLimitBounce)
		{
			Vector2D worldPos = mWorld->WorldToScreen(mPos);

			if (worldPos[0] > (float) mWorld->GetRenderer().GetScreenWidth())
				mVel[0] = -mVel[0];
			else if (worldPos[0] < 0.0f)
				mVel[0] = -mVel[0];

			if (worldPos[1] > (float) mWorld->GetRenderer().GetScreenHeight())
				mVel[1] = -mVel[1];
			else if (worldPos[1] < 0.0f)
				mVel[1] = -mVel[1];
		}

		if (mIsControlledByAvoidance)
		{
			mIsControlledByAvoidance = false;
			mLastControlledByAvoidanceTime = time;
		}

		if (mHasGoal && (time - mLastControlledByAvoidanceTime) > 1.0f)
		{
			Vector2D vect = mGoalPos - GetPos();
			float dist = vect.Length();

			if (dist <= 0.2f)
			{
				SetVel(Vector2D::kZero);
				mHasGoal = false;
			}
			else
			{
				Vector2D dir = vect.Normalized();

				if (Dot(dir, GetVel().Normalized()) < 0.98f)
				{
					float speed = std::max(2.0f, GetVel().Length());

					SetVel(dir * speed);
				}
			}
		}

		if (mIsAgitated)
		{
			mAgitation += dt;
		}
		else
		{
			mAgitation = 0.0f;
		}
		

		mIsAgitated = false;
	}


	virtual void Draw(float time, Agent* pFocusAgent) 
	{
		float lerp = (time - mPrevUpdateTime) / (mUpdateTime - mPrevUpdateTime);
		Renderer& renderer = mWorld->GetRenderer();

		if (mPrevUpdateTime == mUpdateTime || lerp < 0.0f)
			return;

		Vector2D lerpPos = mPrevUpdatePos + ((mPos - mPrevUpdatePos) * lerp);
		float lerpRadius = mRadius;
		Color lerpColor = mColor;

		if (mAgitation != 0.0f)
		{
			Vector2D agitationVector;
			
			while (agitationVector == Vector2D::kZero)
			{
				agitationVector = Vector2D(Randf(-1.0f, 1.0f), Randf(-1.0f, 1.0f));
			}

			agitationVector.Normalize();
			float factor = std::min(0.5f, (mAgitation * mAgitation * 0.5f));
			agitationVector *= GetRadius() * factor;

			lerpPos += agitationVector;

			float agitationRadius = GetRadius() * Randf(-0.1f, 0.1f);

			lerpRadius += agitationRadius;

			lerpColor.g = std::max(0.0f, mColor.g - factor);
			lerpColor.b = std::max(0.0f, mColor.b - factor);
			lerpColor.r = std::min(1.0f, mColor.r + factor);
		}

		renderer.DrawCircle(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(lerpRadius), lerpColor);

		if (pFocusAgent == this)
		{
			Color col = lerpColor;

			col.a *= 0.25f;
			renderer.DrawCircle(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(lerpRadius * 0.66f), col);

			col.a *= 0.25f;
			renderer.DrawCircle(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(lerpRadius * 0.33f), col);
		}

		if (!(mVel == Vector2D::kZero))
		{
			

			renderer.DrawArrow(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(lerpPos + (mVel * kVelDrawScale)), lerpColor);
		}

		

		if (mHasGoal)
		{
			Color col = lerpColor;

			col.a = 0.2f;
			renderer.DrawLine(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(mGoalPos), col);
		}
	}
};

#endif