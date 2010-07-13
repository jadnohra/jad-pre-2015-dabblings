#include "../include/Bigeye/Bigeye.h"

int main()
{
	BE::App app;

	app.Create("Bigeye", 1024, 768);

	TimerMillis timer;

	while (app.Update(timer.GetTimeSecs()))
	{
		timer.CacheTime();
	}

	return 0;
}