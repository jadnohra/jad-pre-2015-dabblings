#ifndef _INCLUDED_BIGEYE_OGL_H
#define _INCLUDED_BIGEYE_OGL_H

#ifdef WIN32
	#include <windows.h>
	#define FREEGLUT_STATIC
	#include "freeglut/GL/freeglut.h"
	#include "freeglut/GL/freeglut_ext.h"
#endif

class OGLTexture
{
public:

	OGLTexture() 
	: mTexture(-1) 
	{}
	
	~OGLTexture()
	{
		if (mTexture != -1)
			glDeleteTextures(1, &mTexture);
	}

	GLuint AutoCreate()
	{
		if (mTexture == -1)
			Create();

		return mTexture;
	}

	GLuint Create(GLuint inTex = -1)
	{
		if (inTex == -1)
		{
			if (mTexture == -1)
			{
				glGenTextures(1, &mTexture);
			}
		}
		else
		{
			if (mTexture != inTex)
				glDeleteTextures(1, &mTexture);
			mTexture = inTex;
		}

		return mTexture;
	}

	bool IsCreated()
	{
		return mTexture != -1;
	}

	GLuint mTexture;
};

#endif
