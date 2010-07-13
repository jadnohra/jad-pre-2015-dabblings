#ifndef _INCLUDED_BIGEYE_OGL_H
#define _INCLUDED_BIGEYE_OGL_H

// TODO!!!  (http://glew.sourceforge.net/install.html)
//#define GLEW_STATIC 
#include "GL/glew.h"
#ifdef GLEW_STATIC 
	#pragma comment (lib, "glew32s.lib")	
#else
	#pragma comment (lib, "glew32.lib")	
#endif

#ifdef WIN32
	#include <windows.h>
	#define FREEGLUT_STATIC
	#include "freeglut/GL/freeglut.h"
	#include "freeglut/GL/freeglut_ext.h"
#endif


class OGLewInit
{
public:

	static bool Init();
};


class OGLTexture
{
public:

	OGLTexture() 
	: mTexture(-1) 
	{}
	
	~OGLTexture()
	{
		Destroy();
	}

	void Destroy()
	{
		if (mTexture != -1)
		{
			glDeleteTextures(1, &mTexture);
			mTexture = -1;
		}
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


class OGLRenderToTexture
{
public:

	OGLRenderToTexture()
	{
	}

	~OGLRenderToTexture()
	{
	}

	bool IsCreated()
	{
		return mTexture.IsCreated();
	}

	void BeginRender()
	{
		// set rendering destination to FBO
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFrameBufferObject);

		//glPushAttrib(GL_VIEWPORT_BIT);
		glViewport(0, 0, mWidth, mHeight);

		// test
		glClearColor(255.0f/255.0f, 176.0f/255.0f, 176.0f/255.0f, 1.0f);

		//glClearDepth(1.0f);

		// clear buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0f, mWidth, mHeight, 0.0f, -1.0f, 1.0f);
		//glOrtho(0.0f, mWidth, 0.0f, mHeight, -1.0f, 1.0f);

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();
	}

	void EndRender()
	{
		//glPopAttrib();
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void Bind()
	{
		glBindTexture(GL_TEXTURE_2D, mTexture.mTexture);
	}

	bool Create(GLsizei inWidth, GLsizei inHeight, int inMultisampleSamples = 0)
	{
		OGLewInit::Init();

		if (mTexture.IsCreated())
			return false;

		mWidth = inWidth;
		mHeight = inHeight;

		// create a texture object
		GLuint textureId = mTexture.Create();
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); // automatic mipmap
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, inWidth, inHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		// create a renderbuffer object to store depth info
		GLuint& rboId = mRenderBufferObject;
		glGenRenderbuffersEXT(1, &rboId);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rboId);
		//glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT,
		//						 inWidth, inHeight);

		//TODO! multisample! embed scene in RenderToTextureWidget, fix color scheme, FPS counter
		//http://www.gamedev.net/community/forums/topic.asp?topic_id=468653
		//http://www.opengl.org/discussion_boards/ubbthreads.php?ubb=showflat&Number=233432 

		if (inMultisampleSamples > 0)
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, inMultisampleSamples, GL_DEPTH_COMPONENT, inWidth, inHeight);
		else
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, inWidth, inHeight);
		glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);

		// create a framebuffer object
		GLuint& fboId = mFrameBufferObject;
		glGenFramebuffersEXT(1, &fboId);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

		// attach the texture to FBO color attachment point
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
								  GL_TEXTURE_2D, textureId, 0);

		// attach the renderbuffer to depth attachment point
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
									 GL_RENDERBUFFER_EXT, rboId);

		// check FBO status
		GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
		if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
		{
			mTexture.Destroy();
			glDeleteRenderbuffersEXT(1, &mRenderBufferObject);
			glDeleteFramebuffersEXT(1, &mFrameBufferObject);

			return false;
		}

		// switch back to window-system-provided framebuffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		return true;
	}

private:

	GLsizei mWidth;
	GLsizei mHeight;
	OGLTexture mTexture;
	GLuint mRenderBufferObject;
	GLuint mFrameBufferObject;
};

#endif
