#ifndef _INCLUDED_BIGFOOT_SKELETON_RENDERER_H
#define _INCLUDED_BIGFOOT_SKELETON_RENDERER_H

#include "Skeleton.h"
#include "SkeletonAnalyzer.h"
#include "Rendering.h"
#include "BFMath.h"
#include "ColorUtil.h"

namespace BF
{

class SkeletonRenderer
{
public:

	struct RenderSettings
	{
		SkeletonTreeInfo* pTreeInfo;
		bool showSemantics;
	};

	SkeletonRenderer()
	{
	}

	void GetRenderBounds(const Skeleton& inSkeleton, int inAnimFrame, SkeletonAnimationFrames* inSkeletonAnimFrames, 
						bool inIncludeRootTranslation, bool inIncludeRootAnimTranslation, 
						AAB& ioBounds)
	{
		if (inAnimFrame == -1 || inSkeletonAnimFrames == NULL)
			inSkeleton.ToModelSpace(inSkeleton.mDefaultPose, inIncludeRootTranslation, inIncludeRootAnimTranslation, mModelSpaceJoints);
		else
			inSkeleton.ToModelSpace(inSkeletonAnimFrames->mSkeletonAnimationFrames[inAnimFrame], inIncludeRootTranslation, inIncludeRootAnimTranslation, mModelSpaceJoints);

		if (mModelSpaceJoints.empty())
			return;


		for (size_t i=0; i<mModelSpaceJoints.size(); ++i)
			ioBounds.Include(mModelSpaceJoints[i].mPosition);
	}

	void Render(const Skeleton& inSkeleton, int inAnimFrame, SkeletonAnimationFrames* inSkeletonAnimFrames,
				const glm::mat4& inViewMatrix, 
				bool inIncludeRootTranslation, bool inIncludeRootAnimTranslation, 
				const RenderSettings& settings,
				AAB& ioBounds)
	{
		BE::OGLLibInit::Init();

		if (settings.pTreeInfo != NULL
			&& mBranchColors.size() < settings.pTreeInfo->mBranches.size())
		{
			ColorGen color_gen;
			mBranchColors.resize(settings.pTreeInfo->mBranches.size());

			for (size_t i=0; i<mBranchColors.size(); ++i)
			{
				mBranchColors[i] = color_gen.Next();
			}
		}

		if (inAnimFrame == -1 || inSkeletonAnimFrames == NULL)
			inSkeleton.ToModelSpace(inSkeleton.mDefaultPose, inIncludeRootTranslation, inIncludeRootAnimTranslation, mModelSpaceJoints);
		else
			inSkeleton.ToModelSpace(inSkeletonAnimFrames->mSkeletonAnimationFrames[inAnimFrame], inIncludeRootTranslation, inIncludeRootAnimTranslation, mModelSpaceJoints);


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

		DrawSkeletonJoint(inSkeleton, inViewMatrix, -1, 0, ioBounds, 1.0f, settings);
	
		glDisable(GL_COLOR_MATERIAL);
	}

	

	void DrawBone(const glm::mat4& inModelViewMat, const glm::vec3& inDir, const glm::vec3& inLocalTranslation, 
					int inParentJointIndex, int inJointIndex, 
					float inLength, float inLengthFraction, float inRadius, float inSphereRadius, 
					const glm::quat& inOrientation, bool inInRenderOnlyBone, const RenderSettings& settings)
	{
		//if (inJointIndex != 15)
		//	return;

		glm::mat3 orientation = glm::toMat3(inOrientation);

		int max_index = 0;
		float max_value = std::abs(inLocalTranslation[max_index]);
		for (int i=1; i<3; ++i)
		{
			if (std::abs(inLocalTranslation[i]) > max_value)
			{
				max_index = i;
				max_value = std::abs(inLocalTranslation[max_index]);
			}
		}

		int normal1_index = (max_index+1)%3;
		int normal2_index = (normal1_index+1)%3;
		glm::vec3 draw_normal = 0.5f * (orientation[normal1_index] + orientation[normal2_index]);
		glm::vec3 normal2 = glm::cross(inDir, draw_normal);

		glm::vec3 base_point = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 end_point = base_point + inDir * (inLength);
		glm::vec3 center_point = inDir * (inLengthFraction * inLength);
		glm::vec3 side_points[] = { center_point + draw_normal * inRadius, center_point + normal2 * inRadius, center_point + draw_normal * -inRadius, center_point + normal2 * -inRadius };

		

		glLoadMatrixf(glm::value_ptr(inModelViewMat));
		
		glEnable(GL_POLYGON_OFFSET_FILL); // Avoid Stitching!
		glPolygonOffset(1.0, 1.0);

		{
			glm::vec3 color(89.0f/255.0f, 89.0f/255.0f, 89.0f/255.0f);

			if (settings.pTreeInfo != NULL)
			{
				int branch_index = settings.pTreeInfo->GetLinkBranch(inParentJointIndex, inJointIndex);

				if (branch_index >= 0)
				{
					color = mBranchColors[branch_index];

					if (settings.showSemantics)
					{
						if (settings.pTreeInfo->mBranches[branch_index].mMirrorBranch >= 0
							&& branch_index > settings.pTreeInfo->mBranches[branch_index].mMirrorBranch)
							color = mBranchColors[settings.pTreeInfo->mBranches[branch_index].mMirrorBranch];
					}
				}
			}
			else
			{
				if (inInRenderOnlyBone)
					color = glm::vec3(120.0f/255.0f, 0.0f/255.0f, 220.0f/255.0f);
			}
			
			glColor3f(color.r, color.g, color.b);
		}

		glBegin(GL_TRIANGLE_FAN);
		glVertex3fv( glm::value_ptr(base_point) );
		glVertex3fv( glm::value_ptr(side_points[0]) );
		glVertex3fv( glm::value_ptr(side_points[1]) );
		glVertex3fv( glm::value_ptr(side_points[2]) );
		glVertex3fv( glm::value_ptr(side_points[3]) );
		glVertex3fv( glm::value_ptr(side_points[0]) );
		glEnd();

		glBegin(GL_TRIANGLE_FAN);
		glVertex3fv( glm::value_ptr(end_point) );
		glVertex3fv( glm::value_ptr(side_points[0]) );
		glVertex3fv( glm::value_ptr(side_points[1]) );
		glVertex3fv( glm::value_ptr(side_points[2]) );
		glVertex3fv( glm::value_ptr(side_points[3]) );
		glVertex3fv( glm::value_ptr(side_points[0]) );
		glEnd();

		glDisable(GL_POLYGON_OFFSET_FILL); 

		//TODO: http://www.allegro.cc/forums/thread/590317
		glColor3f(0.0f, 0.0f,0.0f);
		glBegin(GL_LINES);
		glVertex3fv( glm::value_ptr(base_point) );
		glVertex3fv( glm::value_ptr(side_points[0]) );
		glVertex3fv( glm::value_ptr(base_point) );
		glVertex3fv( glm::value_ptr(side_points[1]) );
		glVertex3fv( glm::value_ptr(base_point) );
		glVertex3fv( glm::value_ptr(side_points[2]) );
		glVertex3fv( glm::value_ptr(base_point) );
		glVertex3fv( glm::value_ptr(side_points[3]) );
		
		glVertex3fv( glm::value_ptr(side_points[0]) );
		glVertex3fv( glm::value_ptr(side_points[1]) );

		glVertex3fv( glm::value_ptr(side_points[1]) );
		glVertex3fv( glm::value_ptr(side_points[2]) );

		glVertex3fv( glm::value_ptr(side_points[2]) );
		glVertex3fv( glm::value_ptr(side_points[3]) );

		glVertex3fv( glm::value_ptr(side_points[3]) );
		glVertex3fv( glm::value_ptr(side_points[0]) );

		glEnd();

		glBegin(GL_LINES);
		glVertex3fv( glm::value_ptr(end_point) );
		glVertex3fv( glm::value_ptr(side_points[0]) );
		glVertex3fv( glm::value_ptr(end_point) );
		glVertex3fv( glm::value_ptr(side_points[1]) );
		glVertex3fv( glm::value_ptr(end_point) );
		glVertex3fv( glm::value_ptr(side_points[2]) );
		glVertex3fv( glm::value_ptr(end_point) );
		glVertex3fv( glm::value_ptr(side_points[3]) );
	
		glEnd();

		

		glutSolidSphere(inLength * 0.025f, 10, 10);
	}


	void DrawSkeletonJoint(const Skeleton& inSkeleton, const glm::mat4& inViewMatrix, int inParentJointIndex, int inJointIndex, AAB& ioBounds, float inLastJointLength, const RenderSettings& settings)
	{
		ioBounds.Include(mModelSpaceJoints[inJointIndex].mPosition);
		glm::mat4 model_view_mat = inViewMatrix * glm::translate(glm::mat4(), mModelSpaceJoints[inJointIndex].mPosition);
		//glLoadMatrixf(glm::value_ptr(model_view_mat));
		//glutSolidSphere(0.25f, 10, 10);
		
		int child_count = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mChildCount;
		int normal_child_count = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mNormalChildCount;
		float bone_length = inLastJointLength;

		if (child_count > 0)
		{
			{
				int first_child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;
				int child_index = first_child_index;
				glm::vec3 bone_tail_pos;
				bool include_normal_children_only = normal_child_count > 0;
				float weight = 1.0f / (float) (include_normal_children_only ? normal_child_count : child_count);

				for (int i=0; i<child_count; ++i)
				{
					int child_joint_index = inSkeleton.mJointHierarchy.mJointChildren[child_index++];
					if (!include_normal_children_only || inSkeleton.mJointInfos[child_joint_index].mType == Joint_Normal)
						bone_tail_pos += weight * mModelSpaceJoints[child_joint_index].mPosition;
				}

				bone_length = glm::distance(bone_tail_pos, mModelSpaceJoints[inJointIndex].mPosition);
				glm::vec3 bone_dir = glm::normalize(bone_tail_pos - mModelSpaceJoints[inJointIndex].mPosition);

				int first_child_joint_index = inSkeleton.mJointHierarchy.mJointChildren[first_child_index];
				DrawBone(model_view_mat, bone_dir, inSkeleton.mJoints[first_child_joint_index].mLocalTransform.mPosition, 
							inJointIndex, first_child_joint_index, 
							bone_length, 0.2f, bone_length*0.15f, bone_length*0.05f, 
							mModelSpaceJoints[inJointIndex].mOrientation, normal_child_count == 0 && child_count > 0, settings);

				if (child_count > 1)
				{
					int child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;

					glColor3f(0.0f, 0.0f, 0.0f);
					glLoadMatrixf(glm::value_ptr(inViewMatrix));
					glPushAttrib(GL_ENABLE_BIT);
					glLineStipple(5, 0xAAAA); 
					glEnable(GL_LINE_STIPPLE);
					for (int i=0; i<child_count; ++i)
					{
						int child_joint_index = inSkeleton.mJointHierarchy.mJointChildren[child_index++];
						
						glBegin(GL_LINES);
						glVertex3fv(glm::value_ptr(bone_tail_pos)); 
						glVertex3fv(glm::value_ptr(mModelSpaceJoints[child_joint_index].mPosition)); 
						glEnd();
					}

					glPopAttrib();
				}
			}

			int child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;

			
			for (int i=0; i<child_count; ++i)
			{
				int child_joint_index = inSkeleton.mJointHierarchy.mJointChildren[child_index++];

				//TODO enable both modes
				// normal line to every child
				//glLoadMatrixf(glm::value_ptr(inViewMatrix));
				//glBegin(GL_LINES);
				//glVertex3fv(glm::value_ptr(mModelSpaceJoints[inJointIndex].mPosition)); 
				//glVertex3fv(glm::value_ptr(mModelSpaceJoints[child_joint_index].mPosition)); 
				//glEnd();

				DrawSkeletonJoint(inSkeleton, inViewMatrix, inJointIndex, child_joint_index, ioBounds, bone_length, settings);
			}
		}
		else
		{
			glLoadMatrixf(glm::value_ptr(model_view_mat));
			glutSolidSphere(bone_length * 0.025f, 10, 10);
		}
	}

	typedef std::vector<glm::vec3> BranchColors;
	BranchColors mBranchColors;

	JointTransforms mModelSpaceJoints;
};

}

#endif