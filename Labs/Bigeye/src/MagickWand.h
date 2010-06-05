#ifndef _INCLUDED_BIGEYE_MAGICKWAND_H
#define _INCLUDED_BIGEYE_MAGICKWAND_H

#include "OGL.h"
#include "BEMath.h"

namespace BE
{
	class MagicWand
	{
	public:

		static bool ReadImageToGLTexture(const char* inPath, GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		static bool MakeTestButtonTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		static bool MakeFrameTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		static bool MakeSliderFrameTexture(GLuint inTexture, GLsizei inLength, GLsizei& outWidth, GLsizei& outHeight);
		static bool MakeSliderMarkerTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
	};
}

#endif