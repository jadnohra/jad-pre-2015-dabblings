#ifndef TIME_H
#define TIME_H

#include "SDL.h"

class GlobalTime
{
public:

	unsigned int pauseTime;
	bool isPaused;
	unsigned int millisAtPauseStart;
	unsigned int currPauseStartTime;

	GlobalTime()
	:	pauseTime(SDL_GetTicks())
	,	isPaused(false)
	{

	}

	unsigned int GetMillis()
	{
		if (isPaused)
			return millisAtPauseStart;

		return SDL_GetTicks() - pauseTime;
	}

	void pause()
	{
		if (!isPaused)
		{
			millisAtPauseStart = GetMillis();
			isPaused = true;
			currPauseStartTime = SDL_GetTicks();
		}
	}

	void resume()
	{
		if (isPaused)
		{
			isPaused = false;
			pauseTime += SDL_GetTicks() - currPauseStartTime;
		}
	}

	void stepPaused(unsigned int millis)
	{
		currPauseStartTime += millis;
		millisAtPauseStart += millis;
	}
};



class Timer
{
public:

	GlobalTime* pGlobalTime;
	unsigned int sdl_time;
	unsigned int frame_time;
	unsigned int frame_index;
	unsigned int rest_time;
	unsigned int delta_frame_index;
	float delta_time;
	float last_returned_time;

	void Start(GlobalTime& globalTime, unsigned int framesPerSec)
	{
		pGlobalTime = &globalTime;
		frame_time = 1000 / framesPerSec;
		sdl_time = pGlobalTime->GetMillis();
		frame_index = 0;
		rest_time = 0;
		delta_frame_index = 0;
		delta_time = 0.0f;
		last_returned_time = 0.0f;
	}

	bool Update()
	{
		unsigned int new_sdl_time = pGlobalTime->GetMillis();
		unsigned int elapsed = new_sdl_time - sdl_time;

		if (elapsed >= frame_time)
		{
			delta_frame_index = elapsed / frame_time;
			frame_index += delta_frame_index;
			rest_time = elapsed - (delta_frame_index * frame_time);

			sdl_time += delta_frame_index * frame_time;

			return true;
		}

		return false;
	}

	float GetFrameLockedTime()
	{
		return (float) (frame_index * frame_time) / 1000.0f;
	}

	float GetTime()
	{
		float curr_time = (GetFrameLockedTime() + ((float) rest_time / 1000.0f));

		delta_time = last_returned_time - curr_time;
		last_returned_time = curr_time;
		return curr_time;
	}

	float GetDeltaTime()
	{
		return delta_time;
	}

	unsigned int GetDeltaFrameIndex()
	{
		return delta_frame_index;
	}

	float GetFrameTime()
	{
		return (float) (frame_time) / 1000.0f;
	}
};


#endif