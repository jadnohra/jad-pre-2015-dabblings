#ifndef _INCLUDED_BIGEYE_OGL_H
#define _INCLUDED_BIGEYE_OGL_H

#include "LibInclude.h"

namespace BE
{

class OGLLibInit
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
		mDepthBufferObject = 0;
		mColorBufferObject = 0;
		mFrameBufferObject = 0;
		mMultiSampleFrameBufferObject = 0;
	}

	~OGLRenderToTexture()
	{
		if (mColorBufferObject != 0)
			glDeleteRenderbuffersEXT(1, &mColorBufferObject);

		if (mDepthBufferObject != 0)
			glDeleteRenderbuffersEXT(1, &mDepthBufferObject);

		if (mFrameBufferObject != 0)
			glDeleteFramebuffersEXT(1, &mFrameBufferObject);

		if (mMultiSampleFrameBufferObject != 0)
			glDeleteFramebuffersEXT(1, &mMultiSampleFrameBufferObject);
	}

	bool IsCreated()
	{
		return mTexture.IsCreated();
	}

	void BeginRender()
	{
		// set rendering destination to FBO
		if (mMultiSampleCount > 1)
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mMultiSampleFrameBufferObject);
		else
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

		if (mMultiSampleCount > 1)
		{
			glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, mMultiSampleFrameBufferObject);
			glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, mFrameBufferObject);
			glBlitFramebufferEXT(0, 0, mWidth, mHeight, 0, 0, mWidth, mHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	}

	void Bind()
	{
		glBindTexture(GL_TEXTURE_2D, mTexture.mTexture);
	}

	bool Create(GLsizei inWidth, GLsizei inHeight, int inMultisampleSamples = 0)
	{
		// http://www.stefanlienhard.ch/2008/08/27/gl_ext_framebuffer_object-with-multisampling/

		OGLLibInit::Init();

		if (mTexture.IsCreated())
			return false;

		mWidth = inWidth;
		mHeight = inHeight;

		if (inMultisampleSamples > 1 
			&& !glewIsSupported("GL_EXT_framebuffer_multisample")
			&& !glewIsSupported("GL_EXT_framebuffer_blit"))
		{
			inMultisampleSamples = 0;
		}

		if (inMultisampleSamples <= 1)
		{
			// create a texture object
			GLuint textureId = mTexture.Create();
			glBindTexture(GL_TEXTURE_2D, textureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, inWidth, inHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); // automatic mipmap
			glBindTexture(GL_TEXTURE_2D, 0);

			// create a renderbuffer object to store depth info
			GLuint& dboId = mDepthBufferObject;
			glGenRenderbuffersEXT(1, &dboId);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dboId);
			glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, inWidth, inHeight);

			// create a framebuffer object
			GLuint& fboId = mFrameBufferObject;
			glGenFramebuffersEXT(1, &fboId);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);

			// attach the texture to FBO color attachment point
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
									  GL_TEXTURE_2D, textureId, 0);

			// attach the renderbuffer to depth attachment point
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
										 GL_RENDERBUFFER_EXT, dboId);

			// check FBO status
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
			{
				mTexture.Destroy();
				glDeleteRenderbuffersEXT(1, &mDepthBufferObject);
				glDeleteFramebuffersEXT(1, &mFrameBufferObject);

				return false;
			}
		}
		else
		{
			// multi sampled color buffer
			GLuint& cboId = mColorBufferObject;
			glGenRenderbuffersEXT(1, &cboId);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, cboId);
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, inMultisampleSamples, GL_RGBA8, inWidth, inHeight);

			// multi sampled depth buffer
			GLuint& dboId = mDepthBufferObject;
			glGenRenderbuffersEXT(1, &dboId);
			glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, dboId);
			glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER_EXT, inMultisampleSamples, GL_DEPTH_COMPONENT, inWidth, inHeight);

			// create fbo for multi sampled content and attach depth and color buffers to it
			GLuint& mfboId = mMultiSampleFrameBufferObject;
			glGenFramebuffersEXT(1, &mfboId);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mfboId);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, cboId);
			glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, dboId);

			// create a texture object
			GLuint textureId = mTexture.Create();
			glBindTexture(GL_TEXTURE_2D, textureId);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, inWidth, inHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE); // automatic mipmap


			// create final fbo and attach texture to it
			GLuint& fboId = mFrameBufferObject;
			glGenFramebuffersEXT(1, &fboId);
			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fboId);
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0);

			// check FBO status
			GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
			if(status != GL_FRAMEBUFFER_COMPLETE_EXT)
			{
				mTexture.Destroy();
				glDeleteRenderbuffersEXT(1, &mMultiSampleFrameBufferObject);
				glDeleteRenderbuffersEXT(1, &mColorBufferObject);
				glDeleteRenderbuffersEXT(1, &mDepthBufferObject);
				glDeleteFramebuffersEXT(1, &mFrameBufferObject);

				return Create(inWidth, inHeight, 0);
			}
		}

		mMultiSampleCount = inMultisampleSamples;
		// switch back to window-system-provided framebuffer
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

		return true;
	}

private:

	int mMultiSampleCount;
	GLsizei mWidth;
	GLsizei mHeight;
	OGLTexture mTexture;
	GLuint mDepthBufferObject;
	GLuint mColorBufferObject;
	GLuint mFrameBufferObject;
	GLuint mMultiSampleFrameBufferObject;
};

}

#endif
