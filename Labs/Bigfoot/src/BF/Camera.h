#ifndef _CAMERA_H
#define _CAMERA_H

#include "Rendering.h"
#include "BFMath.h"
#include "BE/Widgets.h"
#include "BE/MainWindow.h"

namespace BF
{

struct ViewportSetup
{
	glm::vec2 mWindowSize;
};

class CameraSetup
{
public:

	CameraSetup()
	:	mDepthPlanes(0.001f, 10000.0f)
	{
	}

	void SetupFOVY(const ViewportSetup& inViewport)
	{
		mAspectRatio = (inViewport.mWindowSize.x/inViewport.mWindowSize.y);
		mFOV.y = BF::gDegToRad(45.0f);
		mFOV.x = mFOV.y * mAspectRatio;
	}

	void SetGlProjectionMatrix()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(BF::gRadToDeg(mFOV.y),(GLfloat) mAspectRatio, mDepthPlanes.x, mDepthPlanes.y);

		glGetDoublev( GL_PROJECTION_MATRIX, mProjectionMatrix );
	}

	void SetupDepthPlanes(const glm::vec2& inDepthPlanes)
	{
		mDepthPlanes = inDepthPlanes;
	}

	const glm::vec2& GetDepthPlanes() const { return mDepthPlanes; }
	const glm::vec2& GetFOV() const { return mFOV; }
	const GLdouble* GetGlPorjectionMatrix() const { return mProjectionMatrix; }

protected:

	glm::vec2 mFOV;
	glm::vec2 mDepthPlanes;
	float mAspectRatio;
	GLdouble mProjectionMatrix[16];
};


class Camera
{
public:

	Camera()
	:	mCameraWorldMatrixIsDirty(false)
	{
	}

	void SetWorldMatrix(const glm::mat4& inCameraWorldMatrix)
	{
		mCameraWorldMatrix = inCameraWorldMatrix;
		mCameraWorldMatrixIsDirty = true;
	}

	void SetLookAtWorldMatrix(const glm::vec3& inEye, const glm::vec3& inCenter, const glm::vec3& inUp)
	{
		SetWorldMatrix(glm::lookAt2(inEye, inCenter, inUp));
	}


	const glm::mat4& GetWorldMatrix() const { return mCameraWorldMatrix; }

	const glm::mat4& GetViewMatrix() 
	{ 
		if (mCameraWorldMatrixIsDirty) 
			mCameraViewMatrix = glm::inverse(mCameraWorldMatrix); 
		
		mCameraWorldMatrixIsDirty = false; 

		return mCameraViewMatrix; 
	}

protected:

	bool mCameraWorldMatrixIsDirty;
	glm::mat4 mCameraWorldMatrix;
	glm::mat4 mCameraViewMatrix;
};



class CameraFollowSphereAutoSetup
{
public:

	CameraFollowSphereAutoSetup()
	:	mFollowDist(0.0f)
	{
	}


	void SetFollowParams(const glm::vec3& inOffsetDirection)
	{
		mOffsetDirection = glm::normalize(inOffsetDirection);
	}

	void SetupCamera(const Sphere& inTarget, const glm::vec2& inFOV, glm::vec2& inDepthPlanes, Camera& inCamera, glm::vec2& outDepthPlanes)
	{
		float min_dist_fov_x = GetMinDist(inTarget, inDepthPlanes, inFOV.x);
		float min_dist_fov_y = GetMinDist(inTarget, inDepthPlanes, inFOV.y);
		float min_dist = std::max(min_dist_fov_x, min_dist_fov_y);
		mFollowDist = std::max(min_dist, (1.001f) * inDepthPlanes.x);
		mFollowDist = std::min(min_dist, (0.9f) * inDepthPlanes.y);

		glm::vec3 default_up(0.0f, 1.0f, 0.0f);
		glm::vec3 up = glm::dot(default_up, mOffsetDirection) <= 0.99f ? default_up : glm::vec3(1.0f, 0.0f, 0.0f);

		inCamera.SetLookAtWorldMatrix(inTarget.mPosition + (mOffsetDirection * mFollowDist), inTarget.mPosition, up);

		{
			outDepthPlanes.y = 1.5f * GetFollowDist();
			outDepthPlanes.x = outDepthPlanes.y / 500.0f;
		}
	}

	float GetFollowDist() { return mFollowDist; }

protected:


	float GetMinDist(const Sphere& inTarget, glm::vec2& inDepthPlanes, float inFOV)
	{
		float rn = inDepthPlanes.x * atanf(0.5f * inFOV);
		return (inTarget.mRadius * inDepthPlanes.x) / rn;
	}

	glm::vec3 mOffsetDirection;
	float mFollowDist;
};


class CameraTurnTableRotationController
{
public:

	void Render(Camera& inCamera)
	{
		glMatrixMode(GL_MODELVIEW);
		glm::mat4 model_view_mat = inCamera.GetViewMatrix() * glm::translate(glm::mat4(), mLastPickedPoint);
		glLoadMatrixf(glm::value_ptr(model_view_mat));

		glColor3f(1.0f, 1.0f, 1.0f);
		glutSolidSphere(0.0001f, 10, 10);
	}


	void Update(const BE::WidgetContext& inContext, BE::SimpleRenderToTextureWidget& inWidget, const ViewportSetup& inViewportSetup, const CameraSetup& inCameraSetup, Camera& inCamera)
	{
		if (inContext.mMainWindow.GetInputState(BE::INPUT_MOUSE_MIDDLE) != 0.0f)
		{
			glm::vec2 pixel_pos;
			
			if (inWidget.IsMainWindowPosInViewport(inContext, inContext.mMainWindow.GetMousePos(), pixel_pos))
			{
				glm::vec3 pick_pos;
				inWidget.GetScene()->Unproject(pixel_pos, pick_pos, NULL);
				mLastPickedPoint = pick_pos;

				printf("click %d, %d\n", (int) pixel_pos[0], (int) pixel_pos[1]);
			}
		}
	}

	/*
	void gCameraGlUnProject(const ViewportSetup& inViewportSetup, int inViewportX, int inViewportY, const CameraSetup& inCameraSetup, const glm::mat4& inCamWorldMarix, const glm::mat4& inViewMarix, glm::vec3& outPoint, glm::vec3* outRayDir)
	{
		GLint viewport[4] = { 0, 0, (int) inViewportSetup.mWindowSize.x, (int) inViewportSetup.mWindowSize.y };
		GLdouble modelview[16];
		const GLdouble* projection = inCameraSetup.GetGlPorjectionMatrix();
		GLdouble posX, posY, posZ;

		for (int i=0; i<16; ++i)
			modelview[i] = (GLdouble) (glm::value_ptr(inViewMarix))[i];
		
		gluUnProject(inViewportX, inViewportY, 0.0f/, modelview, projection, viewport, &posX, &posY, &posZ);

		outPoint = glm::vec3(posX, posY, posZ);

		if (outRayDir)
		{
			glm::vec3 dir = outPoint - glm::vec3(inCamWorldMarix[3]);
			*outRayDir = glm::normalize(dir);
		}
	}
	*/

protected:

	glm::vec3 mLastPickedPoint;
};




}



#endif