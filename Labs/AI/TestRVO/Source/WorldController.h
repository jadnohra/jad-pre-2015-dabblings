#ifndef WORLD_CONTROLLER_H
#define WORLD_CONTROLLER_H

#include "World.h"
#include "CollisionAvoidanceManager.h"
#include "Agent.h"

class WorldController
{
public:

	World& mWorld;
	ICollisionAvoidanceManager* mpAvoidanceManager;
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
	bool mRightDragShift;

	
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
			mpFocusAgent = mpLeftMouseControlledAgent;

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
		else
		{
			mpFocusAgent = NULL;
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

				Agent* pAgent = new Agent(&mWorld, worldPos, Vector2D(0.0f, 0.0f), 1.0f, Color::kWhite);
				mWorld.Add(*(pAgent));

				if (mpAvoidanceManager)
					mpAvoidanceManager->AddAgent(pAgent, (int) mWorld.mAgents.size());
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
	}
};


#endif