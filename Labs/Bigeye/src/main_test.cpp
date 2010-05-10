#include "../include/Bigeye/Bigeye.h"


int main()
{
	BE::App app;

	app.Create("Bigeye", 640, 480);

	while (app.Update(0.0f))
	{
	}

	return 0;
}