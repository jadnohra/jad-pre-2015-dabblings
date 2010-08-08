#ifndef _CAMERA_H
#define _CAMERA_H

#include "Rendering.h"
#include "BFMath.h"

namespace BF
{

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



class CameraFollowSphereController
{
public:

	CameraFollowSphereController()
	:	mCamera(NULL)
	,	mFollowDist(0.0f)
	{
	}

	void SetCamera(Camera* inCamera)
	{
		mCamera = inCamera;
	}

	void SetTarget(const Sphere& inTarget)
	{
		mTarget = inTarget;
	}

	void SetFollowParams(const glm::vec3& inOffsetDirection, const glm::vec2& inFOV, glm::vec2 inDepthPlanes)
	{
		mOffsetDirection = inOffsetDirection;
		mFOV = inFOV;
		mDepthPlanes = inDepthPlanes;
	}

	void Update()
	{
		if (mCamera != NULL)
		{
			float min_dist_fov_x = GetMinDist(mFOV.x);
			float min_dist_fov_y = GetMinDist(mFOV.y);
			float min_dist = std::max(min_dist_fov_x, min_dist_fov_y);
			mFollowDist = std::max(min_dist, (1.001f) * mDepthPlanes.x);
			mFollowDist = std::min(min_dist, (0.9f) * mDepthPlanes.y);

			mCamera->SetLookAtWorldMatrix(mTarget.mPosition + (mOffsetDirection * mFollowDist), mTarget.mPosition, glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

	float GetFollowDist() { return mFollowDist; }

protected:


	float GetMinDist(float inFOV)
	{
		float rn = mDepthPlanes.x * atanf(0.5f * inFOV);
		return (mTarget.mRadius * mDepthPlanes.x) / rn;
	}

	Camera* mCamera;
	Sphere mTarget;
	glm::vec3 mOffsetDirection;
	glm::vec2 mFOV;
	glm::vec2 mDepthPlanes;
	float mFollowDist;
};

}

#endif