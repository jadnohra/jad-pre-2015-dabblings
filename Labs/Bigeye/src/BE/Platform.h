#ifndef _BIGEYE_PLATFORM_H
#define _BIGEYE_PLATFORM_H

#ifdef WIN32
	
	//#define _BIND_TO_CURRENT_VCLIBS_VERSION 1


	#define WIN32_LEAN_AND_MEAN 1
	#define NOMINMAX
	#include "windows.h"
	#include "Shellapi.h" // drag and drop
	#include <string>

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


	class WideString
	{
	public:

		WideString::WideString()
		:	mBuffer(NULL)
		,	mSize(0)
		{
		}

		WideString::WideString(const char* inString)
		:	mBuffer(NULL)
		,	mSize(0)
		{
			Set(inString);
		}


		void WideString::Set(const char* inString)
		{
			int required_size = MultiByteToWideChar(CP_ACP, 0, inString, -1, NULL, 0);

			if (required_size == 0)
				return;

			++required_size;

			if (required_size > mSize)
			{
				mSize = required_size;
				mBuffer = (LPWSTR) (std::realloc(mBuffer, mSize * sizeof(WCHAR)));
			}

			if (!MultiByteToWideChar(CP_ACP, 0, inString, -1, mBuffer, mSize))
			{
				if (mSize > 0)
					mBuffer[0] = 0;
			}
		}

		operator LPCWSTR() const { return mBuffer; }

	private:

		LPWSTR mBuffer;
		int mSize;
	};

#endif

#endif