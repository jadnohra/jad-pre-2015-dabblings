#include "Main.h"


int main()
{
#ifdef MAIN_RETARGET
	return RetargetMain();
#else
	return ViewerMain();
#endif

}