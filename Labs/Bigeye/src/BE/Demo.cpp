#include "MainWindow.h"


int main()
{
	BE::MainWindow main_window;

	if (main_window.Create("Bigeye demo", 1024, 768))
	{
		TimerMillis timer;

		while (main_window.Update(timer.GetTimeSecs()))
		{
			timer.CacheTime();
		}
	}

	return 0;
}
