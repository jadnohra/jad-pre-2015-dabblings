#include "render.h"
#include "mesh.h"

class App
{
public:

	Mesh mMesh;

	bool Load()
	{
		mMesh.LoadWavefrontObj("../media/monkey_smooth.obj", false, false, false);

		return true;
	}

	bool DrawScene()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer
		glLoadIdentity();									// Reset The Current Modelview Matrix

		glTranslatef(-1.5f,0.0f,-5.0f);						// Move Left 1.5 Units And Into The Screen 6.0
		mMesh.Draw();

		return true;
	}

};

