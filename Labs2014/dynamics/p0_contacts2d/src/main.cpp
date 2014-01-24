#include "window2d.h"
#include "simul2d.h"
#include "scenes.h"

int main(void)
{
	WindowSimul2d ws;
	createScene1(ws.simul);
	ws.updateFunc2 = updateScene1;

	window2d::loop(&ws);

	return 0;
}