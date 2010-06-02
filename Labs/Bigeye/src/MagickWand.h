#ifndef _INCLUDED_BIGEYE_MAGICKWAND_H
#define _INCLUDED_BIGEYE_MAGICKWAND_H

#include "OGL.h"

namespace BE
{
	class MagicWand
	{
	public:

		static bool ReadImageToGLTexture(const char* inPath, GLuint& outTexture, GLsizei& outWidth, GLsizei& outHeight);
	};
}

#endif