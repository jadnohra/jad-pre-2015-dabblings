#ifndef _CAMERA_H
#define _CAMERA_H

#include "Rendering.h"
#include "BFMath.h"

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
	}

	void SetupDepthPlanes(const glm::vec2& inDepthPlanes)
	{
		mDepthPlanes = inDepthPlanes;
	}

	const glm::vec2& GetDepthPlanes() const { return mDepthPlanes; }
	const glm::vec2& GetFOV() const { return mFOV; }

protected:

	glm::vec2 mFOV;
	glm::vec2 mDepthPlanes;
	float mAspectRatio;
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
		//printf("%f, %f, %f - %f\n", inCameraWorldMatrix[0].x, inCameraWorldMatrix[0].y, inCameraWorldMatrix[0].z, glm::length(inCameraWorldMatrix[0]));
		//printf("%f, %f, %f - %f\n", inCameraWorldMatrix[1].x, inCameraWorldMatrix[1].y, inCameraWorldMatrix[1].z, glm::length(inCameraWorldMatrix[1]));
		//printf("%f, %f, %f - %f\n", inCameraWorldMatrix[2].x, inCameraWorldMatrix[2].y, inCameraWorldMatrix[2].z, glm::length(inCameraWorldMatrix[2]));
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

		inCamera.SetLookAtWorldMatrix(inTarget.mPosition + (mOffsetDirection * mFollowDist), inTarget.mPosition, glm::vec3(0.0f, 1.0f, 0.0f));

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

}

#endif