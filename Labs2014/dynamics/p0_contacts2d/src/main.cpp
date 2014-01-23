#include "window2d.h"
#include "simul2d.h"
#include "scenes.h"

int main(void)
{
	WindowSimul2d ws;
	createScene0(ws.simul);

	window2d::loop(&ws);

	return 0;
}