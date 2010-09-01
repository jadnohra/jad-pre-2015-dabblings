#ifndef _INCLUDED_BIGEYE_MAGICKWAND_H
#define _INCLUDED_BIGEYE_MAGICKWAND_H

#include <string>
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

		struct TextInfo
		{
			std::string mText;
			FontID mFontID;
			float mPointSize;
			bool mBold;
			glm::vec2 mMinEmptySpace;

			TextInfo();
			TextInfo(const char* inText, FontID inFontID, float inPointSize, bool inBold, const glm::vec2& inMinEmptySpace);
		};

		struct SizeConstraints
		{
			glm::vec2 mMinSize;
			glm::vec2 mMaxSize;

			SizeConstraints();
		};

		enum WidgetState
		{
			WIDGET_NORMAL,
			WIDGET_PRESSED,
			WIDGET_HIGHLIGHTED
		};

		enum FrameType
		{
			FRAME_NORMAL,
			FRAME_NORMAL_CUT_UPPER,
		};

		MagicWand();
		~MagicWand();

		bool ReadImageToGLTexture(const char* inPath, GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeTestButtonTexture(GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeFrameTexture(FrameType inType, bool inUseGradient, GLuint inTexture, GLsizei& outWidth, GLsizei& outHeight, glm::vec2& outInternalPos, glm::vec2& outInternalSize);
		bool MakeSliderFrameTexture(GLuint inTexture, GLsizei inLength, const TextInfo& inTextInfo, const SizeConstraints& inSizeConstraints, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeVerticalSliderFrameTexture(GLuint inTexture, GLsizei inWidth, GLsizei inHeight, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeSliderMarkerTexture(GLuint inTexture, WidgetState inWidgetState, GLsizei inFrameHeight, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeVerticalSliderMarkerTexture(GLuint inTexture, WidgetState inWidgetState, GLsizei inFrameWidth, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeButtonTexture(GLuint inTexture, WidgetState inWidgetState, const TextInfo& inTextInfo, const SizeConstraints& inSizeConstraints, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeTextTexture(GLuint inTexture, const TextInfo& inTextInfo, const SizeConstraints& inSizeConstraints, GLsizei& outWidth, GLsizei& outHeight);
		bool MakeTestTexture(GLuint inTexture, int inWidth, int inHeight, GLsizei& outWidth, GLsizei& outHeight);

		FontID LoadFont(const char* inPath, const char* inBoldPath = NULL); // bold font path is optional, it is also not needed if the font file supports font weights

	protected:

		MagickWandImpl* mImpl;
	};
}

#endif