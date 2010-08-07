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
	,	mNearDepth(1.0f)
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

	void SetFollowParams(const glm::vec3& inOffsetDirection, const glm::vec2& inFOV, float inNearDepth)
	{
		mOffsetDirection = inOffsetDirection;
		mFOV = inFOV;
		mNearDepth = inNearDepth;
	}

	void Update()
	{
		if (mCamera != NULL)
		{
			float min_dist_fov_x = GetMinDist(mFOV.x);
			float min_dist_fov_y = GetMinDist(mFOV.y);
			float min_dist = std::max(min_dist_fov_x, min_dist_fov_y);
			min_dist = std::max(min_dist, (1.001f) * mNearDepth);

			mCamera->SetLookAtWorldMatrix(mTarget.mPosition + (mOffsetDirection * min_dist), mTarget.mPosition, glm::vec3(0.0f, 1.0f, 0.0f));
		}
	}

protected:


	float GetMinDist(float inFOV)
	{
		float rn = mNearDepth * atanf(0.5f * inFOV);
		return (mTarget.mRadius * mNearDepth) / rn;
	}

	Camera* mCamera;
	Sphere mTarget;
	glm::vec3 mOffsetDirection;
	glm::vec2 mFOV;
	float mNearDepth;
};

}

#endif