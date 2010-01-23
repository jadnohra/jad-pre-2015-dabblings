#ifndef _ASSERT_H
#define _ASSERT_H

inline void gAssert(bool inCondition)
{
	if (!inCondition)
		__asm { int 3 };
}

#endif