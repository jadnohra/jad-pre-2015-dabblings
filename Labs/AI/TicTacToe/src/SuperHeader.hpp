#ifndef __SuperHeader_hpp
#define __SuperHeader_hpp

#include <stdlib.h>
#include <tchar.h>
#include <assert.h>
#include <algorithm>

#define NOMINMAX

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif	

#include "windows.h"


#ifdef _DEBUG

	inline void softAssert(bool cond) {

		if (!cond)
			DebugBreak();
	}

#else

	inline void softAssert(bool cond) {
	}

#endif


#endif