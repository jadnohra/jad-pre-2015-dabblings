#include "OGL.h"

namespace BE
{

bool OGLewInit::Init()
{
	static bool is_inited = false;

	if (!is_inited)
	{
		GLenum err = glewInit();
		is_inited = true;
	}

	return is_inited;
}

}