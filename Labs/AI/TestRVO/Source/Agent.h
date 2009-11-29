#ifndef AGENT_H
#define AGENT_H

#include "Math.h"
#include "Renderer.h"
#include "World.h"
#include "Terrain.h"

class IAgent
{
public:

	virtual ~IAgent() {}

	virtual Circle GetTerrainShape() 
	{
		return Circle(GetPos(), GetRadius());
	}

	virtual const Path* GetPath(int& currToIndex, bool& hasTempAvoidPt, Vector2D& currTempAvoidancePt) 
	{ 
		return NULL;
	}

	virtual void Agitate()			 { }
	virtual const Vector2D& GetPos() { return Vector2D::kZero; }
	virtual const Vector2D& GetVel() { return Vector2D::kZero;; }
	virtual float GetRadius() { return 0.0f; }
	virtual void NotifyControlledByAvoidance() { }

	virtual void SetPos(const Vector2D& position) { }
	virtual void SetVel(const Vector2D& velocity) { }

	virtual void SetGoal(const Vector2D& pos, float speed) {  }
	
	virtual void SetPath(const Path& path, float speed) { }

	virtual void AddAvoidanceSolutionToPath(const Vector2D& point, const Vector2D* pVel, bool replacePathPoint)
	{
	}

	virtual void Update(float time, float dt) 
	{
	}


	virtual void Draw(float time, Agent* pFocusAgent) 
	{
	}

};

class Agent : public IAgent, public TerrainAgent
{
public:

	static const float kVelDrawScale;

	World* mWorld;
	Vector2D mPos;
	Vector2D mVel;
	float mRadius;
	bool mHasGoal;
	Vector2D mGoalPos;

	bool mHasPath;
	Path mPath;
	int mIndexInPath;
	float mPathSpeed;
	bool mHasPathTempAvoidancePos;
	Vector2D mPathTempAvoidancePos;
	Vector2D mPathTempAvoidanceOrigPos;

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
		mHasPath = false;
		mHasPathTempAvoidancePos = false;

		mIsControlledByAvoidance = false;
		mLastControlledByAvoidanceTime = -1.0f;
	}

	virtual Circle GetTerrainShape() 
	{
		return Circle(mPos, mRadius);
	}

	virtual const Path* GetPath(int& currToIndex, bool& hasTempAvoidPt, Vector2D& currTempAvoidancePt) 
	{ 
		currToIndex = mIndexInPath;
		hasTempAvoidPt = mHasPathTempAvoidancePos;
		currTempAvoidancePt = mPathTempAvoidancePos;
		return (mHasPath && mIndexInPath >= 0 )? &mPath : NULL; 
	}

	virtual void Agitate()			 { mIsAgitated = true; }
	virtual const Vector2D& GetPos() { return mPos; }
	virtual const Vector2D& GetVel() { return mVel; }
	virtual float GetRadius() { return mRadius; }
	virtual void NotifyControlledByAvoidance() { mIsControlledByAvoidance = true; }

	virtual void SetPos(const Vector2D& position) { mPos = position; }
	virtual void SetVel(const Vector2D& velocity) { mVel = velocity; }

	virtual void SetGoal(const Vector2D& pos, float speed) { mGoalPos = pos; mHasGoal = true; SetVel((mGoalPos - GetPos()).Normalized() * speed); }
	
	virtual void SetPath(const Path& path, float speed)
	{
		mPath = path;
		mHasPath = true;
		mIndexInPath = -1;
		mPathSpeed = speed;

		UpdatePath();
	}

	virtual void AddAvoidanceSolutionToPath(const Vector2D& point, const Vector2D* pVel, bool replacePathPoint)
	{
		if (mHasPath)
		{
			// TODO support pVel

			if (replacePathPoint)
			{
				mPath.mPathNodes.insert(mPath.mPathNodes.begin() + (mIndexInPath + 1), PathNode(-1, point));
				mHasPathTempAvoidancePos = false;
				mHasGoal = false;
				UpdatePath();
			}
			else
			{
				if (!mHasPathTempAvoidancePos)
					mPathTempAvoidanceOrigPos = mHasGoal ? mGoalPos : point;
				//else keep the orig pos

				mHasPathTempAvoidancePos = true;
				mPathTempAvoidancePos = point;
				
				SetGoal(mPathTempAvoidancePos, mPathSpeed);
			}
		}
	}

	void UpdatePath(float time = 0.0f)
	{
		if (mHasPath && (mIndexInPath == -1 || mLastControlledByAvoidanceTime < 0.0f || (time - mLastControlledByAvoidanceTime) > 1.0f))
		{
			if (mIndexInPath == -1)
			{
				if (mPath.Length() <= 1)
				{
					mHasPath = false;
					mHasGoal = false;
					mVel.SetZero();
				}
				else
				{
					if (mHasPathTempAvoidancePos)
					{
						SetGoal(mPathTempAvoidancePos, mPathSpeed);
					}
					else
					{
						mIndexInPath = 0;
						SetGoal(mPath.GetPoint(mIndexInPath), mPathSpeed);
					}
				}
			}
			else
			{
				if (!mHasGoal)
				{
					if (mIndexInPath + 1 < mPath.Length() || mHasPathTempAvoidancePos)
					{
						if (!mHasPathTempAvoidancePos)
							++mIndexInPath;

						SetGoalToVisiblePathPoint(mIndexInPath);
						//SetGoal(mPath.GetPoint(mIndexInPath), mPathSpeed);
					}
					else
					{
						mHasPath = false;
						mVel.SetZero();
					}

					mHasPathTempAvoidancePos = false;
				}
			}
		}
	}

	void SetGoalToVisiblePathPoint(int& index)
	{
		if (index+1 < mPath.Length())
		{
			Terrain::NavigableNodes nodes;
			nodes.push_back(Terrain::NavigableNode(mPath.GetNode(index).wptIndex));
			Terrain::LinkAddress linkAdress;
			mpTerrain->FindLinkAddress(mPath.GetNode(index).wptIndex, mPath.GetNode(index+1).wptIndex, linkAdress);
			nodes.push_back(Terrain::NavigableNode(linkAdress));
			nodes.push_back(Terrain::NavigableNode(mPath.GetNode(index+1).wptIndex));

			Vector2D intersectionPoint;
			int intersectionNode;

			if (mpTerrain->IntersectRayWithNavigableNodes(GetPos(), GetVel().Normalized(), GetRadius(),
													   nodes, intersectionPoint, intersectionNode))
			{
				if (intersectionNode > 0)
				{
					index = index+1;
				}
			}
		}

		SetGoal(mPath.GetPoint(index), mPathSpeed);
	}

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

		UpdatePath(time);

		do {

			if (mHasGoal && (time - mLastControlledByAvoidanceTime) > 1.0f)
			{
				Vector2D vect = mGoalPos - GetPos();
				float dist = vect.Length();

				if (dist <= 0.2f)
				{
					mHasGoal = false;

					if (mHasPath)
						UpdatePath(time);
					else
					{
						SetVel(Vector2D::kZero);
						
					}
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

		} while (mHasPath && !mHasGoal); // make sure we update goal and not set velocity to zero if we are following a path

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

			col.a *= 0.5f;
			renderer.DrawCircle(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(lerpRadius * 0.66f), col);

			col.a *= 0.5f;
			renderer.DrawCircle(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(lerpRadius * 0.33f), col);
		}

		if (!(mVel == Vector2D::kZero))
		{
			renderer.DrawArrow(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(lerpPos + (mVel * kVelDrawScale)), lerpColor);
		}

		

		if (mHasGoal)
		{
			Color col = lerpColor;

			col.a = (mHasPath && pFocusAgent == this) ? 0.7f : 0.2f;
			renderer.DrawLine(mWorld->WorldToScreen(lerpPos), mWorld->WorldToScreen(mGoalPos), col, -1.0f, (mHasPath && pFocusAgent == this) ? 1.5f : 1.0f);
		}
	}
};

#endif