#include "Main.h"


int main()
{
#ifdef MAIN_RETARGET
	return RetargetMain();
#endif

#ifdef MAIN_VIEWER
	return ViewerMain();
#endif

#ifdef MAIN_PLANNER
	return PlannerMain();
#endif

}