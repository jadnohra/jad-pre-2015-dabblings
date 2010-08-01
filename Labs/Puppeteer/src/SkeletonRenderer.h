#ifndef _SKELETON_RENDERER_H
#define _SKELETON_RENDERER_H

#include "Skeleton.h"
#include "Rendering.h"

class SkeletonRenderer
{
public:

	void Render(const BF::Skeleton& inSkeleton, int inAnimFrame, const glm::mat4& inViewMatrix, bool inIncludeRootTranslation, bool inIncludeRootAnimTranslation)
	{
		inSkeleton.ToModelSpace(inSkeleton.mDefaultPose, inIncludeRootTranslation, inIncludeRootAnimTranslation, mModelSpaceJoints);

		if (mModelSpaceJoints.empty())
			return;

		/*
		if (inAnimFrame >= 0
			&& !mTestSkeletonAnimFrames.mSkeletonAnimationFrames.empty())
		{
			mTestSkeleton.ToModelSpace(mTestSkeletonAnimFrames.mSkeletonAnimationFrames[mTestAnimFrame], true, mTestSkeletonJoints);
		}
		*/

		glEnable(GL_COLOR_MATERIAL);
		glMatrixMode(GL_MODELVIEW);
		glColor4f(0.0f, 0.0f, 0.0f, 0.5f);

		DrawSkeletonJoint(inSkeleton, inViewMatrix, 0);
	
		glDisable(GL_COLOR_MATERIAL);
	}

	void DrawSkeletonJoint(const BF::Skeleton& inSkeleton, const glm::mat4& inViewMatrix, int inJointIndex)
	{
		glLoadMatrixf(glm::value_ptr(inViewMatrix * glm::translate(glm::mat4(), mModelSpaceJoints[inJointIndex].mPosition)));
		glutSolidSphere(0.25f, 10, 10);
		
		int child_count = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mChildCount;

		if (child_count > 0)
		{
			int child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;

			for (int i=0; i<child_count; ++i)
			{
				int child_joint_index = inSkeleton.mJointHierarchy.mJointChildren[child_index++];

				glLoadMatrixf(glm::value_ptr(inViewMatrix));
				glBegin(GL_LINES);
				//glVertex3f(mModelSpaceJoints[inJointIndex].mPosition.x, mModelSpaceJoints[inJointIndex].mPosition.y, mModelSpaceJoints[inJointIndex].mPosition.z); 
				//glVertex3f(mModelSpaceJoints[child_joint_index].mPosition.x, mModelSpaceJoints[child_joint_index].mPosition.y, mModelSpaceJoints[child_joint_index].mPosition.z); 
				glVertex3fv(glm::value_ptr(mModelSpaceJoints[inJointIndex].mPosition)); 
				glVertex3fv(glm::value_ptr(mModelSpaceJoints[child_joint_index].mPosition)); 
				glEnd();

				DrawSkeletonJoint(inSkeleton, inViewMatrix, child_joint_index);
			}
		}
	}

	BF::JointTransforms mModelSpaceJoints;
};

#endif