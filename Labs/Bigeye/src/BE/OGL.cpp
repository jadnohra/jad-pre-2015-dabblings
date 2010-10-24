#include "OGL.h"

namespace BE
{

bool OGLLibInit::Init()
{
	static bool is_inited = false;
	
	if (!is_inited)
	{
		is_inited = true;

		GLenum err = glewInit();

		int argc = 0;
		char* argv = "";
		glutInit(&argc, &argv);
		
		if (err != 0)
		{
			printf("GLEW Failed to initialize, Error: %s\n", glewGetErrorString(err));
			return false;
		}

		if (glGenRenderbuffersEXT == NULL)
		{
			printf("OpenGL extension: 'OpenGL Frame Buffer Object (FBO)' not found");
			return false;
		}
	}

	return true;
}

}