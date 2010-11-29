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

		class Trail
		{
		public:

			typedef std::vector<glm::vec3> Vecs;
			Vecs mPositions;
			int mStartPosition;
			int mPositionCount;

			Trail() : mPositionCount(0) {}

			void Reset(int inSize)
			{
				mPositions.resize(inSize);
				mPositionCount = 0;
			}

			void Reset()
			{
				mPositionCount = 0;
			}
			
			void Add(const glm::vec3& inPos)
			{
				if (mPositions.empty())
					return;

				if (mPositionCount == 0)
					mStartPosition = 0;
				else
				{
					if (glm::areSimilar(mPositions[(mStartPosition+(mPositionCount-1))  % (int) mPositions.size()], inPos, 0.0f))
						return;
				}
				
				mPositions[(mStartPosition+mPositionCount) % (int) mPositions.size()] = inPos;
				++mPositionCount;

				if (mPositionCount > (int) mPositions.size())
				{
					mPositionCount = mPositions.size();
					mStartPosition = (mStartPosition + 1) % (int) mPositions.size();
				}
			}
		};
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
							const Joint& child_joint = mSkeleton->mJoints[child_joint_index];
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

			if (mFrameIndex != inAnimFrame)
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

		void AnalyzeJoint(int inJointIndex, SkeletonPhysicsParticle& outJoint)
		{
			outJoint.mIsValid = false;
			outJoint.mMass = mJointPhysicsInfos[inJointIndex].mMass;
			outJoint.mIsValidVelocity = false;
			outJoint.mIsValidAcceleration = false;

			glm::vec3 alt_pos[2];

			if (mIsValidModelSpaceJoints[Frame_0])
			{
				outJoint.mIsValid = true;
				outJoint.mPosition = CalcJointPosition(inJointIndex, mModelSpaceJoints[Frame_0]);
			}

			if (mIsValidModelSpaceJoints[Frame_0] && mIsValidModelSpaceJoints[Frame_1])
			{
				outJoint.mIsValidVelocity = true;
				alt_pos[0] = CalcJointPosition(inJointIndex, mModelSpaceJoints[Frame_1]);
				outJoint.mVelocity = (alt_pos[0] - outJoint.mPosition) / mFrameTime;
			}

			if (mFrameIndex > 1 // T pose
				&& mIsValidModelSpaceJoints[Frame_0] && mIsValidModelSpaceJoints[Frame_1] && mIsValidModelSpaceJoints[Frame_m1])
			{
				outJoint.mIsValidAcceleration = true;
				alt_pos[1] = CalcJointPosition(inJointIndex, mModelSpaceJoints[Frame_m1]);
				glm::vec3 prev_velocity = (outJoint.mPosition - alt_pos[1]) / mFrameTime;
				outJoint.mAcceleration = (outJoint.mVelocity - prev_velocity) / mFrameTime;
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

		glm::vec3 CalcJointPosition(int inJointIndex, const JointTransforms& inTransforms)
		{
			return inTransforms[inJointIndex].mPosition;
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

	public:

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

		bool mRenderMas;
		bool mRenderVelocity;
		bool mRenderAcceleration;
		bool mRenderTrails;

		SkeletonPhysicsModelRenderer()
		{
			mRenderMas = false;
			mRenderVelocity = true;
			mRenderAcceleration = false;
			mRenderTrails = true;
		}

		void Setup(bool inRenderMass, bool inRenderVelocity, bool inRenderAcceleration, bool inRenderTrails)
		{
			mRenderMas = inRenderMass;
			mRenderVelocity = inRenderVelocity;
			mRenderAcceleration = inRenderAcceleration;
			mRenderTrails = inRenderTrails;
		}

		void BeginRender(const Skeleton& inSkeleton, 
						const SkeletonPhysicsModel& inModel,
						const glm::mat4& inViewMatrix)
		{
			glEnable(GL_COLOR_MATERIAL);
		}

		void EndRender(const Skeleton& inSkeleton, 
						const SkeletonPhysicsModel& inModel,
						const glm::mat4& inViewMatrix)
		{
			glDisable(GL_COLOR_MATERIAL);
		}

		void RenderParticle(const Skeleton& inSkeleton, 
					const SkeletonPhysicsModel& inModel, const SkeletonPhysicsParticle& inParticle,
					const glm::mat4& inViewMatrix, 
					const SkeletonTreeInfo* pTreeInfo,
					SkeletonPhysicsParticle::Trail* ioTrail = NULL,
					glm::vec3* pColor = NULL)
		{
			if (inParticle.mIsValid)
			{
				glMatrixMode(GL_MODELVIEW);

				if (pColor != NULL)
					glColor4f(pColor->r, pColor->g, pColor->b, 0.5f);
				else
					glColor4f(0.0f, 0.0f, 1.0f, 0.5f);


				glm::mat4 model_view_mat = inViewMatrix * glm::translate(glm::mat4(), inParticle.mPosition);
				glLoadMatrixf(glm::value_ptr(model_view_mat));

				if (mRenderMas)
					glutSolidSphere((inModel.GetRenderMassToLengthScale() * inParticle.mMass) / (float) (inModel.GetMassJointCount()), 10, 10);

				if (mRenderVelocity && inParticle.mIsValidVelocity)
				{
					if (pColor != NULL)
						glColor4f(pColor->r, pColor->g, pColor->b, 0.5f);
					else
						glColor4f(0.0f, 0.0f, 1.0f, 0.5f);

					glLineWidth(2.0f); 
					glBegin(GL_LINES);
					glVertex3fv(glm::value_ptr(glm::vec3())); 
					glVertex3fv(glm::value_ptr(inParticle.mVelocity * 0.2f)); 
					glEnd();
					glLineWidth(1.0f); 
				}

				if (mRenderAcceleration && inParticle.mIsValidAcceleration)
				{
					glColor4f(0.0f, 1.0f, 1.0f, 0.5f);

					glLineWidth(1.5f); 
					glBegin(GL_LINES);
					glVertex3fv(glm::value_ptr(glm::vec3())); 
					glVertex3fv(glm::value_ptr(inParticle.mAcceleration)); 
					glEnd();
					glLineWidth(1.0f); 
				}


				if (mRenderTrails && ioTrail)
				{
					ioTrail->Add(inParticle.mPosition);

					if (ioTrail->mPositionCount > 1)
					{
						glLoadMatrixf(glm::value_ptr(inViewMatrix));
						if (pColor != NULL)
							glColor4f(pColor->r, pColor->g, pColor->b, 0.5f);
						else
							glColor4f(0.0f, 0.0f, 1.0f, 0.5f);

						glLineWidth(0.5f); 
						glBegin(GL_LINES);

						for (int i = ioTrail->mStartPosition, c = 0; c+1< ioTrail->mPositionCount; ++c, ++i)
						{
							if (i >= (int)ioTrail->mPositions.size())
								i = 0;
							int j = i+1;
							if (j >= (int)ioTrail->mPositions.size())
								j = 0;
							glVertex3fv(glm::value_ptr(ioTrail->mPositions[i])); 
							glVertex3fv(glm::value_ptr(ioTrail->mPositions[j])); 
						}
						
						glEnd();
						glLineWidth(1.0f); 
					}
				}
			}
		}
	};


	class SkeletonPhysicsParticles
	{
	public:
		
		typedef std::vector<SkeletonPhysicsParticle> Particles;
		typedef std::vector<SkeletonPhysicsParticle::Trail> ParticleTrails;
		Particles mParticles;
		ParticleTrails mParticleTrails;
		SkeletonPhysicsParticle mCOM;
		SkeletonPhysicsParticle::Trail mCOMTrail;
		
		typedef std::vector<glm::vec3> Colors;
		
		Colors mBranchColors;
		Colors mJointColors;

		int mTrailSize;

		SkeletonPhysicsParticles()
		{
			mTrailSize = 10;
		}

		void ResetTrails(int inSize)
		{
			mTrailSize = inSize;
		}

		void ResetTrails()
		{
			for (size_t i = 0; i < mParticleTrails.size(); ++i)
			{
				mParticleTrails[i].Reset();
			}

			mCOMTrail.Reset();
		}

		void Reset()
		{
			mCOM.mIsValid = false;
			mParticles.clear();
			mParticleTrails.clear();
		}

		void AnalyzeParticles(SkeletonPhysicsModel& inModel, const SkeletonTreeInfo* pTreeInfo)
		{
			if (inModel.mSkeleton != NULL)
			{
				if (inModel.mSkeleton->mJoints.size() != mParticles.size())
				{
					mParticles.resize(inModel.mSkeleton->mJoints.size());
					mParticleTrails.resize(mParticles.size());

					for (size_t i = 0; i < mParticles.size(); ++i)
					{
						mParticles[i].mIsValid = false;
						mParticleTrails[i].Reset();
					}

					mCOM.mIsValid = false;
					mCOMTrail.Reset();

					if (pTreeInfo != NULL)
					{
						ColorGen color_gen;
						mBranchColors.resize(pTreeInfo->mBranches.size());

						for (size_t i=0; i<mBranchColors.size(); ++i)
						{
							mBranchColors[i] = color_gen.Next();
						}
					}
					else
					{
						mBranchColors.clear();
					}

					{
						ColorGen color_gen(10);
						mJointColors.resize(mParticles.size());

						for (size_t i=0; i<mJointColors.size(); ++i)
						{
							mJointColors[i] = color_gen.Next();
						}
					}


					for (size_t i = 0; i < mParticleTrails.size(); ++i)
					{
						mParticleTrails[i].Reset(mTrailSize);
					}

					mCOMTrail.Reset(mTrailSize);
				}
				
				inModel.AnalyzeCenterOfMass(mCOM);
				
				for (size_t i = 0; i < mParticles.size(); ++i)
				{
					inModel.AnalyzeJoint(i, mParticles[i]);
				}
			}
			else
			{
				mCOM.mIsValid = false;
				mParticles.clear();
				mParticleTrails.clear();
			}
		}

		void RenderParticles(bool inRenderCOM, bool inRenderNodes, bool inRenderLeafNodesOnly,
								SkeletonPhysicsModelRenderer& inRenderer,
								const Skeleton& inSkeleton, 
								const SkeletonPhysicsModel& inModel, 
								const glm::mat4& inViewMatrix, 
								const SkeletonTreeInfo* pTreeInfo)
		{
			if (mParticles.empty())
				return;

			inRenderer.BeginRender(inSkeleton, inModel, inViewMatrix);

			if (inRenderCOM)
			{
				inRenderer.RenderParticle(inSkeleton, inModel, mCOM,
											inViewMatrix, pTreeInfo, &mCOMTrail);
			}

			if (inRenderNodes)
			{

				for (size_t i = 0; i < mParticles.size(); ++i)
				{
					if (mParticles[i].mMass > 0.0f)
					{
						if (!inRenderLeafNodesOnly
							|| inSkeleton.mJointHierarchy.mJointChildrenInfos[i].mNormalChildCount == 0)
						{

							glm::vec3 color = mJointColors[i];
							
							if (pTreeInfo != NULL)
							{
								int branch_index = pTreeInfo->GetChildBranch(i);

								if (branch_index >= 0)
									color = mBranchColors[branch_index];
							}

							
							
							inRenderer.RenderParticle(inSkeleton, inModel, mParticles[i],
														inViewMatrix, pTreeInfo, &(mParticleTrails[i]),
														&color);
						}
					}
				}
			}

			inRenderer.EndRender(inSkeleton, inModel, inViewMatrix);
		}

	};
}

#endif