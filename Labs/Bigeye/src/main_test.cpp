#include "../include/Bigeye/Bigeye.h"
#include "BigEyeRenderer.h"

int main()
{
	BE::gTestRenderer();
	
	BE::App app;

	app.Create("Bigeye", 1024, 768);

	TimerMillis timer;

	while (app.Update(timer.GetTimeSecs()))
	{
		timer.CacheTime();
	}

	return 0;
}