#ifndef _INCLUDED_BIGEYE_MAGICKWAND_H
#define _INCLUDED_BIGEYE_MAGICKWAND_H

#include "OGL.h"
#include "BEMath.h"
#include "smart_ptr.h"

namespace BE
{
	class MagickWandImpl;

	class MagicWand
	{
	public:

		typedef int FontID;

		MagicWand();
		~MagicWand();

		bool ReadImageToGLTexture(const char* inPath, GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeTestButtonTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeFrameTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeSliderFrameTexture(GLuint inTexture, GLsizei inLength, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeSliderFrameTexture2(GLuint inTexture, GLsizei inLength, const char* inText, FontID inFontID, float inPointSize, bool inBold, int inAdditionalHorizSpace, int inAdditionalVertSpace, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeSliderMarkerTexture2(GLuint inTexture, bool inIsPressed, GLsizei inFrameHeight, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeSliderMarkerTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeButtonTexture(GLuint inTexture, bool inIsPressed, const char* inText, FontID inFontID, float inPointSize, bool inBold, int inAdditionalHorizSpace, int inAdditionalVertSpace, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeTextTexture(GLuint inTexture, const char* inText, FontID inFontID, float inPointSize, bool inBold, int inAdditionalHorizSpace, int inAdditionalVertSpace, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeTestTexture(GLuint inTexture, int inWidth, int inHeight, GLsizei& outWidth, GLsizei& outHeight);

		FontID LoadFont(const char* inPath);

	protected:

		MagickWandImpl* mImpl;
	};
}

#endif