#include "OGL.h"

namespace BE
{

bool OGLLibInit::Init()
{
	static bool is_inited = false;

	if (!is_inited)
	{
		GLenum err = glewInit();

		int argc = 0;
		char* argv = "";
		glutInit(&argc, &argv);
		is_inited = true;
	}

	return is_inited;
}

}