#ifndef _BIGEYE_ASSERT_H
#define _BIGEYE_ASSERT_H

#include "Platform.h"

namespace BE
{
#ifdef WIN32
	inline void gAssert(bool inCondition)
	{
		if (!inCondition)
			DebugBreak();
	}
#endif
}

#endif