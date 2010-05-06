#ifndef _CAMERA_H
#define _CAMERA_H

#include "Rendering.h"
#include "stdio.h"
#include <algorithm>

class Camera
{
public:

	Camera()
	:	mCameraWorldMatrixIsDirty(false)
	,	mWindow(NULL)
	,	mArcBallIsEnabled(false)
	,	mArcBallIsDragging(false)
	{
	}


	void Init(GL_Window& inWindow)
	{
		mWindow = &inWindow;
	}

	void SetArcBallIsEnabled(bool inValue) { mArcBallIsEnabled = inValue; mArcBallIsDragging = false; }


	void SetWorldMatrix(const glm::mat4& inCameraWorldMatrix)
	{
		mCameraWorldMatrix = inCameraWorldMatrix;
		mCameraWorldMatrixIsDirty = true;
	}

	const glm::mat4& GetWorldMatrix() const { return mCameraWorldMatrix; }

	const glm::mat4& GetViewMatrix() 
	{ 
		if (mCameraWorldMatrixIsDirty) 
			mCameraViewMatrix = glm::inverse(mCameraWorldMatrix); 
		
		mCameraWorldMatrixIsDirty = false; 

		return mCameraViewMatrix; 
	}


	void Update(unsigned int inMillis)
	{
		UpdateArcBall(inMillis);
	}


	bool Unproject(int inWindowX, int inWindowY, glm::vec3& outPoint, glm::vec3* outRayDir)
	{
		return Unproject(inWindowX, inWindowY, mCameraWorldMatrix, GetViewMatrix(), outPoint, outRayDir);
	}

	bool Unproject(int inWindowX, int inWindowY, const glm::mat4& inCamWorldMarix, const glm::mat4& inViewMarix, glm::vec3& outPoint, glm::vec3* outRayDir)
	{
		RECT rect;

		GetClientRect(mWindow->hWnd, &rect);		
		int window_width=rect.right-rect.left;		
		int window_height=rect.bottom-rect.top;		

		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLdouble posX, posY, posZ;
		
		//glMatrixMode(GL_MODELVIEW);
		//glLoadMatrixf(glm::value_ptr(inViewMarix));

		for (int i=0; i<16; ++i)
			modelview[i] = (GLdouble) (glm::value_ptr(inViewMarix))[i];

		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );

		GLfloat winX, winY;

		winX = (float) inWindowX;					// Holds The Mouse X Coordinate
		winY = (float)window_height -(float) inWindowY;		
		
		gluUnProject(winX, winY, 0.0f/*mCameraWorldMatrix[3].z*/, modelview, projection, viewport, &posX, &posY, &posZ);

		outPoint = glm::vec3(posX, posY, posZ);

		if (outRayDir)
		{
			glm::vec3 dir = outPoint - glm::vec3(inCamWorldMarix[3]);
			*outRayDir = glm::normalize(dir);
		}

		return true;
	}

protected:

	void UpdateArcBall(unsigned int inMillis)
	{
		if (mArcBallIsEnabled)
		{
			if (mWindow->IsKeyDown('H'/*VK_LSHIFT*//*VK_LMENU*/))
			{
				POINT mouse;						
				GetCursorPos(&mouse);				
				ScreenToClient(mWindow->hWnd, &mouse);

				if (mArcBallIsDragging)
				{
					glm::vec3 unproj_point;
					glm::vec3 dir3;
					Unproject(mouse.x, mouse.y, mArcBallStartWorldMatrix, mArcBallStartViewMatrix, unproj_point, &dir3);
					glm::vec4 dir4 = mArcBallStartViewMatrix * glm::vec4(dir3, 0.0f);
					mArcBallCurrDir = glm::vec3(dir4.x, dir4.y, dir4.z);

					glm::vec3 axis = glm::cross(mArcBallStartDir, mArcBallCurrDir);

					if (glm::dot(axis, axis) > 0.0f)
					{
						axis = glm::normalize(axis);
						float dot_val = glm::dot(mArcBallStartDir, mArcBallCurrDir);

						dot_val = std::max(dot_val, -1.0f);
						dot_val = std::min(dot_val, 1.0f);

						float angle = acosf(dot_val);

						glm::quat rot_quat = glm::normalize(glm::angleAxis(glm::degrees(angle), axis));

						mArcBallCurrWorldMatrix = mArcBallStartWorldMatrix * glm::toMat4(rot_quat);
						SetWorldMatrix(mArcBallCurrWorldMatrix);
					}
				}
				else
				{
					mArcBallStartWorldMatrix = mCameraWorldMatrix;
					mArcBallStartViewMatrix = GetViewMatrix();

					glm::vec3 unproj_point;
					glm::vec3 dir3;
					Unproject(mouse.x, mouse.y, mArcBallStartWorldMatrix, mArcBallStartViewMatrix, unproj_point, &dir3);
					glm::vec4 dir4 = mArcBallStartViewMatrix * glm::vec4(dir3, 0.0f);
					mArcBallStartDir = glm::vec3(dir4.x, dir4.y, dir4.z);

					mArcBallIsDragging = true;
				}
			}
			else
			{
				mArcBallIsDragging = false;
			}
		}
	}


protected:

	GL_Window*	mWindow;

	bool mCameraWorldMatrixIsDirty;
	glm::mat4 mCameraWorldMatrix;
	glm::mat4 mCameraViewMatrix;

	bool mArcBallIsEnabled;
	bool mArcBallIsDragging;
	glm::vec3 mArcBallStartDir;
	glm::mat4 mArcBallStartWorldMatrix;
	glm::mat4 mArcBallStartViewMatrix;

	glm::vec3 mArcBallCurrDir;
	glm::mat4 mArcBallCurrWorldMatrix;
};

#endif