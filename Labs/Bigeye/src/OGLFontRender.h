#ifndef _BIGEYE_OGLFONTRENDER_H
#define _BIGEYE_OGLFONTRENDER_H

#include "OGLState.h"

namespace BE
{
	class OGLFontInstance
	{
	public:

		bool Create(const char* inFilePath, float inPixelHeight);
		void Render(const char* inText, float inPosX, float inPosY) const;

	protected:

		friend class OGLState_FontRender;
		
		GLuint mTexture;
	};


	class OGLState_FontRender : public OGLState
	{
		public:

						OGLState_FontRender(OGLFontInstance& inFont);

		virtual void	Set();

		void Render(const char* inText, float inPosX, float inPosY) const { mFont.Render(inText, inPosX, inPosY); }

	protected:

		OGLFontInstance& mFont;
	};
}

#endif