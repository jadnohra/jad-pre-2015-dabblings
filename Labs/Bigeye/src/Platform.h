#ifndef _BIGEYE_PLATFORM_H
#define _BIGEYE_PLATFORM_H

#ifdef WIN32
	#include "windows.h"

	class TimerMillis
	{
	public:

		TimerMillis()
		{
			Reset();
		}

		void Reset()
		{
			mLastMillis = GetTickCount();
			mTimeMillis = 0;
			mTimeSecs = 0.0f;
		}

		void CacheTime()
		{
			DWORD tick_count = GetTickCount();
			
			mTimeMillis += tick_count - mLastMillis;
			mLastMillis = tick_count;

			mTimeSecs = (float) mTimeMillis / 1000.0f;
		}

		int GetTimeMillis() { return mTimeMillis; } 
		float GetTimeSecs() { return mTimeSecs; } 

	private:

		DWORD mLastMillis;
		int mTimeMillis;
		float mTimeSecs;
	};

#endif

#endif