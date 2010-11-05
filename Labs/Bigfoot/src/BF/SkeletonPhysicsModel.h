#ifndef _INCLUDED_BIGFOOT_SKELETON_PHYSICS_MODEL_H
#define _INCLUDED_BIGFOOT_SKELETON_PHYSICS_MODEL_H

#include "Skeleton.h"

namespace BF
{
	struct JointPhysicsInfo
	{
		float mMass;
	};

	struct SkeletonPhysicsParticle
	{
		bool mIsValid;

		float mMass;
		glm::vec3 mPosition;
		
		bool mIsValidVelocity;
		glm::vec3 mVelocity;
		bool mIsValidAcceleration;
		glm::vec3 mAcceleration;
	};

	typedef std::vector<JointPhysicsInfo> JointPhysicsInfos;

	class SkeletonPhysicsModel
	{
	public:

		SkeletonPhysicsModel() : mSkeleton(NULL), mTotalMass(0.0f), mRenderMassToLengthScale(1.0f), mFrameTime(1.0f), mFrameIndex(-2)
		{
			for (int i = Frame_m2; i <= Frame_2; ++i)
			{
				mIsValidModelSpaceJoints[i] = false;
			}
		}

		void Init(bool inIncludeRootTranslation, bool inIncludeRootAnimTranslation)
		{
			mIncludeRootTranslation = inIncludeRootTranslation;
			mIncludeRootAnimTranslation = inIncludeRootAnimTranslation;
		}

		void Build(Skeleton& inSkeleton, float inMassPerMeter = 1.0f)
		{
			mSkeleton = &inSkeleton;
			mJointPhysicsInfos.resize(mSkeleton->mJoints.size());
			mMassJointCount = 0;
			mTotalMass = 0.0f;
			
			for (size_t i = 0; i < mJointPhysicsInfos.size(); ++i)
			{
				int normal_child_count =  mSkeleton->mJointHierarchy.mJointChildrenInfos[i].mNormalChildCount;
				mJointPhysicsInfos[i].mMass = 0.0f;
				
				if (normal_child_count > 0)
				{
					int first_child_index = mSkeleton->mJointHierarchy.mJointChildrenInfos[i].mFirstChildIndex;
					int child_count =  mSkeleton->mJointHierarchy.mJointChildrenInfos[i].mChildCount;
		
					float total_length = 0.0f;

					int child_index = first_child_index;
					for (int ci=0; ci<child_count; ++ci)
					{
						int child_joint_index = mSkeleton->mJointHierarchy.mJointChildren[child_index++];
						if (inSkeleton.mJointInfos[child_joint_index].mType == Joint_Normal)
						{
							const Joint& child_joint = mSkeleton->mJoints[child_index];
							total_length += glm::length(child_joint.mLocalTransform.mPosition);
						}
					}

					mJointPhysicsInfos[i].mMass = total_length * inMassPerMeter;
				}
				else
				{
					if (mSkeleton->mJointInfos[i].mType == Joint_Normal)
						mJointPhysicsInfos[i].mMass = 0.1f * inMassPerMeter;
					else
						mJointPhysicsInfos[i].mMass = 0.0f;
				}

				mTotalMass += mJointPhysicsInfos[i].mMass;

				if (mJointPhysicsInfos[i].mMass > 0.0f)
				++mMassJointCount;
			}

			mRenderMassToLengthScale = 1.0f / inMassPerMeter;
		}

		void ResetStep()
		{
			mFrameIndex = -2;

			for (int i = Frame_m1; i <= Frame_1; ++i)
			{
				mIsValidModelSpaceJoints[i] = false;
			}
		}

		void Step(int inAnimFrame, SkeletonAnimationFrames* inSkeletonAnimFrames)
		{
			if (inAnimFrame < 0)
			{
				ResetStep();
				return;
			}

			//if (mFrameIndex != inAnimFrame)
			{
				mFrameTime = (inSkeletonAnimFrames != NULL ? inSkeletonAnimFrames->mFrameTime : 1.0f);

				for (int i = Frame_m1; i <= Frame_1; ++i)
				{
					mIsValidModelSpaceJoints[(EFrameType) i] = MakeFrame(inAnimFrame, inSkeletonAnimFrames, (EFrameType) i);
				}

				mFrameIndex = inAnimFrame;
			}
		}

		void AnalyzeCenterOfMass(SkeletonPhysicsParticle& outCOM)
		{
			outCOM.mIsValid = false;
			outCOM.mMass = mTotalMass;
			outCOM.mIsValidVelocity = false;
			outCOM.mIsValidAcceleration = false;

			glm::vec3 alt_pos[2];

			if (mIsValidModelSpaceJoints[Frame_0])
			{
				outCOM.mIsValid = true;
				outCOM.mPosition = CalcCOMPosition(mModelSpaceJoints[Frame_0]);
			}

			if (mIsValidModelSpaceJoints[Frame_0] && mIsValidModelSpaceJoints[Frame_1])
			{
				outCOM.mIsValidVelocity = true;
				alt_pos[0] = CalcCOMPosition(mModelSpaceJoints[Frame_1]);
				outCOM.mVelocity = (alt_pos[0] - outCOM.mPosition) / mFrameTime;
			}

			if (mFrameIndex > 1 // T pose
				&& mIsValidModelSpaceJoints[Frame_0] && mIsValidModelSpaceJoints[Frame_1] && mIsValidModelSpaceJoints[Frame_m1])
			{
				outCOM.mIsValidAcceleration = true;
				alt_pos[1] = CalcCOMPosition(mModelSpaceJoints[Frame_m1]);
				glm::vec3 prev_velocity = (outCOM.mPosition - alt_pos[1]) / mFrameTime;
				outCOM.mAcceleration = (outCOM.mVelocity - prev_velocity) / mFrameTime;
			}
		}

		float GetRenderMassToLengthScale() const
		{
			return mRenderMassToLengthScale;
		}

		int GetMassJointCount() const
		{
			return mMassJointCount;
		}

	protected:

		enum EFrameType
		{
			Frame_m2, Frame_m1, Frame_0, Frame_1, Frame_2, FrameTypeCount 
		};

		glm::vec3 CalcCOMPosition(const JointTransforms& inTransforms)
		{
			glm::vec3 pos;

			for (size_t i = 0; i < mJointPhysicsInfos.size(); ++i)
			{
				pos += inTransforms[i].mPosition * (mJointPhysicsInfos[i].mMass / mTotalMass);
			}

			return pos;
		}

		glm::vec3 CalcCOMVelocity(const JointTransforms& inTransforms1, const JointTransforms& inTransform2, glm::vec3& inPos1)
		{
			glm::vec3 pos2 = CalcCOMPosition(inTransform2);
			return pos2 - inPos1;
		}

		bool MakeFrame(int inAnimFrame, SkeletonAnimationFrames* inSkeletonAnimFrames, EFrameType inFrameType)
		{
			int fame_index = inAnimFrame;

			switch (inFrameType)
			{
				case Frame_m1: fame_index -= 1; break;
				case Frame_m2: fame_index -= 2; break;
				case Frame_1: fame_index += 1; break;
				case Frame_2: fame_index += 2; break;
			}

			if (inAnimFrame < -1 || inSkeletonAnimFrames == NULL)
			{
				if (inFrameType == Frame_0)
				{
					mSkeleton->ToModelSpace(mSkeleton->mDefaultPose, mIncludeRootTranslation, mIncludeRootAnimTranslation, mModelSpaceJoints[Frame_0]);
					return true;
				}
			}
			else
			{
				if (fame_index < 0 || fame_index >= (int) inSkeletonAnimFrames->mSkeletonAnimationFrames.size())
				{
					mModelSpaceJoints[inFrameType].clear();
					return false;
				}
				else
				{
					mSkeleton->ToModelSpace(inSkeletonAnimFrames->mSkeletonAnimationFrames[fame_index], mIncludeRootTranslation, mIncludeRootAnimTranslation, mModelSpaceJoints[inFrameType]);
					return true;
				}
			}

			return false;
		}

		Skeleton* mSkeleton;
		JointPhysicsInfos mJointPhysicsInfos;
		int mMassJointCount;
		float mTotalMass;
		float mRenderMassToLengthScale;
		bool mIncludeRootTranslation;
		bool mIncludeRootAnimTranslation;
		int mFrameIndex;
		float mFrameTime;
		JointTransforms mModelSpaceJoints[FrameTypeCount];
		bool mIsValidModelSpaceJoints[FrameTypeCount];
	};


	class SkeletonPhysicsModelRenderer
	{
	public:

		void Render(const Skeleton& inSkeleton, 
					const SkeletonPhysicsModel& inModel, const SkeletonPhysicsParticle& inCOM,
					const glm::mat4& inViewMatrix, 
					const SkeletonTreeInfo* pTreeInfo)
		{
			if (inCOM.mIsValid)
			{
				glEnable(GL_COLOR_MATERIAL);
				glMatrixMode(GL_MODELVIEW);
				glColor4f(0.0f, 0.0f, 1.0f, 0.5f);

				glm::mat4 model_view_mat = inViewMatrix * glm::translate(glm::mat4(), inCOM.mPosition);
				glLoadMatrixf(glm::value_ptr(model_view_mat));

				glutSolidSphere((inModel.GetRenderMassToLengthScale() * inCOM.mMass) / (float) (inModel.GetMassJointCount()), 10, 10);

				if (inCOM.mIsValidVelocity)
				{
					glColor4f(0.0f, 0.0f, 1.0f, 0.5f);

					glLineWidth(2.0f); 
					glBegin(GL_LINES);
					glVertex3fv(glm::value_ptr(glm::vec3())); 
					glVertex3fv(glm::value_ptr(inCOM.mVelocity)); 
					glEnd();
					glLineWidth(1.0f); 
				}

				if (inCOM.mIsValidAcceleration)
				{
					glColor4f(0.0f, 1.0f, 1.0f, 0.5f);

					glLineWidth(1.5f); 
					glBegin(GL_LINES);
					glVertex3fv(glm::value_ptr(glm::vec3())); 
					glVertex3fv(glm::value_ptr(inCOM.mAcceleration)); 
					glEnd();
					glLineWidth(1.0f); 
				}


				glDisable(GL_COLOR_MATERIAL);
			}
		}
	};
}

#endif