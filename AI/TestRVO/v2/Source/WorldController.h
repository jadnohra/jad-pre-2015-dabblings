#ifndef WORLD_CONTROLLER_H
#define WORLD_CONTROLLER_H

#include "World.h"
#include "CollisionAvoidanceManager.h"
#include "Agent.h"
#include "App.h"

class WorldController
{
public:

	World& mWorld;
	ICollisionAvoidanceManager* mpAvoidanceManager;
	int mFocusAgentIndex;
	Agent* mpFocusAgent;
	Agent* mpLeftMouseControlledAgent;
	Agent* mpRightMouseControlledAgent;
	bool mIsLeftPressed;
	bool mIsRightPressed;
	Uint32 mStartPressTime;
	Vector2D mMoveAvgVel;
	Vector2D mAgentCenterOffset;
	Vector2D mStartRightClickPos;
	Vector2D mRightDragPos;
	Vector2D mStartMiddleDragPos;
	bool mRightDragShift;
	bool mMiddleDragShift;
	bool mDoDrawBox;
	b2AABB mDrawBox;

	
	Color mDrawArrowColor;
	Vector2D mDrawArrow;
	
	WorldController(World& world, ICollisionAvoidanceManager* pAvoidanceMan)
	:	mWorld(world)
	,	mpAvoidanceManager(pAvoidanceMan)
	,	mpLeftMouseControlledAgent(NULL)
	,	mpRightMouseControlledAgent(NULL)
	,	mIsLeftPressed(false)
	,	mIsRightPressed(false)
	,	mRightDragShift(false)
	,	mpFocusAgent(NULL)
	,	mFocusAgentIndex(-1)
	,	mMiddleDragShift(false)
	,	mDoDrawBox(false)
	{
	}

	void Update()
	{
		int x;
		int y;
		int relX;
		int relY;

		if (mpLeftMouseControlledAgent)
		{
			if (SDL_GetMouseState(&x, &y)&SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				Vector2D worldPos = mWorld.ScreenToWorld(Vector2D((float) x, (float) y));

				mpLeftMouseControlledAgent->SetPos(worldPos + mAgentCenterOffset);
				if (mpAvoidanceManager)
					mpAvoidanceManager->ResetAgentState(mpLeftMouseControlledAgent);

				SDL_GetRelativeMouseState(&relX, &relY);
				Vector2D worldPosDiff = mWorld.ScreenToWorldDir(Vector2D((float) relX, (float) relY));

				if (mIsLeftPressed)
				{
					mMoveAvgVel += worldPosDiff;
					//printf("%f,%f\n", mMoveAvgVel.x, mMoveAvgVel.y);
				}
				else
				{
					mIsLeftPressed = true;
					mMoveAvgVel = Vector2D::kZero;
					mStartPressTime = SDL_GetTicks();
				}
			}
			else
			{
				float pressTime = ((float) (SDL_GetTicks() - mStartPressTime)) / 1000.0f;

				Vector2D vel = mMoveAvgVel * (1.0f / pressTime);
				//mpLeftMouseControlledAgent->SetVel(vel);
				if (mpAvoidanceManager)
					mpAvoidanceManager->ResetAgentState(mpLeftMouseControlledAgent);
				
				mpLeftMouseControlledAgent = NULL;
				mIsLeftPressed = false;
			}
		}
		

		if (mpRightMouseControlledAgent)
		{
			if (SDL_GetMouseState(&x, &y)&SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				Vector2D worldPos = mWorld.ScreenToWorld(Vector2D((float) x, (float) y));
				mRightDragPos = worldPos;

				mDrawArrow = worldPos - mpRightMouseControlledAgent->GetPos();
				
				Uint8 *keystate = SDL_GetKeyState(NULL);
				if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
				{
					mRightDragShift = true;
					mDrawArrowColor = Color::kGreen;

					float length = std::min(mDrawArrow.Length(), 10.0f);
					mDrawArrow = mDrawArrow.Normalized() * length;
				}
				else
				{
					mRightDragShift = false;
					mDrawArrowColor = Color::kBlue;
				}
			}
			else
			{
				if (mRightDragShift)
				{
					mpRightMouseControlledAgent->SetVel(mDrawArrow * (1.0f / Agent::kVelDrawScale));
					
				}
				else
				{
					mpRightMouseControlledAgent->SetGoal(mRightDragPos, 5.0f);
				}
				
				if (mpAvoidanceManager)
					mpAvoidanceManager->ResetAgentState(mpRightMouseControlledAgent);
				mpRightMouseControlledAgent = NULL;
				mIsRightPressed = false;
			}
		}

		if (mMiddleDragShift)
		{
			if (SDL_GetMouseState(&x, &y))
			{
				if (mWorld.mTerrain)
				{
					Vector2D worldPos = mWorld.ScreenToWorld(Vector2D((float) x, (float) y));
					Vector2D size = worldPos - mStartMiddleDragPos;

					mDrawBox.lowerBound.Set(mStartMiddleDragPos.x, mStartMiddleDragPos.y);
					mDrawBox.upperBound.Set(mStartMiddleDragPos.x + size.x, mStartMiddleDragPos.y + size.y);

					mDoDrawBox = true;
				}
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
			if (evt.button.button == SDL_BUTTON_WHEELDOWN)
			{
				mWorld.GetRenderer().mWorldScale = std::max(1.0f, mWorld.GetRenderer().mWorldScale - 0.5f);
			}

			if (evt.button.button == SDL_BUTTON_WHEELUP)
			{
				mWorld.GetRenderer().mWorldScale = std::min(40.0f, mWorld.GetRenderer().mWorldScale + 0.5f);
			}

			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				int x = evt.motion.x;
				int y = evt.motion.y;

				Vector2D worldPos = mWorld.ScreenToWorld(Vector2D((float) x, (float) y));
				Agent* pAgent = mWorld.PickAgent(worldPos);

				if (pAgent != NULL)
				{
					mpLeftMouseControlledAgent = pAgent;

					mAgentCenterOffset = mpLeftMouseControlledAgent->GetPos() - worldPos;
					mpLeftMouseControlledAgent->SetVel(Vector2D::kZero);
					if (mpAvoidanceManager)
						mpAvoidanceManager->ResetAgentState(mpLeftMouseControlledAgent);
				}
			}

			if (evt.button.button == SDL_BUTTON_RIGHT)
			{
				int x = evt.motion.x;
				int y = evt.motion.y;

				mStartRightClickPos = mWorld.ScreenToWorld(Vector2D((float) x, (float) y));
				Agent* pAgent = mWorld.PickAgent(mStartRightClickPos);

				if (pAgent != NULL)
				{
					mpRightMouseControlledAgent = pAgent;

					mpRightMouseControlledAgent->SetVel(Vector2D::kZero);
					if (mpAvoidanceManager)
						mpAvoidanceManager->ResetAgentState(mpRightMouseControlledAgent);
				}
			}

			if (evt.button.button == SDL_BUTTON_MIDDLE)
			{
				int x = evt.motion.x;
				int y = evt.motion.y;

				Vector2D worldPos = mWorld.ScreenToWorld(Vector2D((float) x, (float) y));

				Uint8 *keystate = SDL_GetKeyState(NULL);
				if (keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT])
				{
					mStartMiddleDragPos = worldPos;
					mMiddleDragShift = true;
				}
				else
				{
					Agent* pAgent = new Agent(&mWorld, worldPos, Vector2D(0.0f, 0.0f), 1.0f, mWorld.mApp->GetDefaultAgentColor(mWorld));
					mWorld.Add(*(pAgent));

					if (mpAvoidanceManager)
						mpAvoidanceManager->AddAgent(pAgent, (int) mWorld.mAgents.size());

				}
			}
		}

		if (evt.type == SDL_MOUSEBUTTONUP)
		{
			if (evt.button.button == SDL_BUTTON_MIDDLE)
			{
				if (mMiddleDragShift)
				{
					if (mWorld.mTerrain)
					{
						mMiddleDragShift = false;
						mDoDrawBox = false;

						int x = evt.motion.x;
						int y = evt.motion.y;

						Vector2D worldPos = mWorld.ScreenToWorld(Vector2D((float) x, (float) y));
						Vector2D size = worldPos - mStartMiddleDragPos;

						if (fabs(size.x) < 1.0f)
						{
							if (size.x >= 0.0f)
								size.x = 1.0f;
							else
								size.x = -1.0f;
						}

						if (fabs(size.y) < 1.0f)
						{
							if (size.y >= 0.0f)
								size.y = 1.0f;
							else
								size.y = -1.0f;
						}

						b2AABB box;

						box.lowerBound.Set(mStartMiddleDragPos.x, mStartMiddleDragPos.y);
						box.upperBound.Set(mStartMiddleDragPos.x + size.x, mStartMiddleDragPos.y + size.y);

						mWorld.mTerrain->AddStaticObstacle(box, true);
					}
				}
			}
		}

		if (evt.type == SDL_KEYUP)
		{
			if (evt.key.keysym.sym == SDLK_KP_PLUS)
			{
				if (mFocusAgentIndex + 1 == mWorld.mAgents.size())
				{
					mFocusAgentIndex = -1;
					mpFocusAgent = NULL;
				}
				else
				{
					mFocusAgentIndex = ((mFocusAgentIndex + 1) % (int) mWorld.mAgents.size());
					mpFocusAgent = mWorld.mAgents[mFocusAgentIndex];
				}
			}
		}
	}

	void Draw()
	{
		if (mpRightMouseControlledAgent)
		{
			mWorld.GetRenderer().DrawArrow(mWorld.WorldToScreen(mpRightMouseControlledAgent->GetPos()), 
											mWorld.WorldToScreen(mpRightMouseControlledAgent->GetPos() + mDrawArrow), mDrawArrowColor, 0.5f);
		}

		if (mDoDrawBox)
		{
			Vector2D quad[4];
			CreateBoxQuad(mDrawBox, quad);

			mWorld.GetRenderer().DrawQuad(mWorld.WorldToScreen(quad[0]), 
										mWorld.WorldToScreen(quad[1]), 
										mWorld.WorldToScreen(quad[2]), 
										mWorld.WorldToScreen(quad[3]), Color::kGreen, -1.0f, 1.0f, true);
		}

		
	}
};


#endif