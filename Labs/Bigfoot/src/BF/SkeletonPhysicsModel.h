#ifndef _INCLUDED_BIGFOOT_SKELETON_PHYSICS_MODEL_H
#define _INCLUDED_BIGFOOT_SKELETON_PHYSICS_MODEL_H

#include "Skeleton.h"
#include "3rdparty/tsavgol.h"

namespace BF
{
	struct JointPhysicsInfo
	{
		float mMass;
		bool mIsStatic;
		bool mIsStaticPos;
		bool mIsStaticSpeed;
	};

	struct JointScalarKinematicInfo
	{
		float mHeight;
		float mSpeed;
		float mAcceleration;
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

		SkeletonPhysicsParticle()
		{
			mIsValid = false;
		}

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
	typedef std::vector<JointScalarKinematicInfo> JointScalarKinematicInfos;
	

	class SkeletonPhysicsModel
	{
	public:

		SkeletonPhysicsModel() : mSkeleton(NULL), mTotalMass(0.0f), mRenderMassToLengthScale(1.0f), mFrameTime(1.0f), mFrameIndex(-2)
		{
			for (int i = Frame_m2; i <= Frame_2; ++i)
			{
				mIsValidModelSpaceJoints[i] = false;
			}

			mIsLearning = false;
			mMinLearnedSpeed = -1.0f;
			mMinLearnedAccel = -1.0f;
			mMaxLearnedSpeed = -1.0f;
			mMaxLearnedAccel = -1.0f;
			mIsDetectingStaticJoints = false;
			mStaticJointSpeed = 0.0f;
			mStaticJointAccel = 0.0f;
			mIncludeRootTranslation = true;
			mIncludeRootAnimTranslation = true;
		}

		void Init(bool inIncludeRootTranslation, bool inIncludeRootAnimTranslation)
		{
			mIncludeRootTranslation = inIncludeRootTranslation;
			mIncludeRootAnimTranslation = inIncludeRootAnimTranslation;
		}

		void getLPCoefficientsButterworth2Pole(const int samplerate, const float cutoff, float* const ax, float* const by)
		{
			float PI      = 3.1415926535897932385f;
			float sqrt2 = 1.4142135623730950488f;

			float QcRaw  = (2 * PI * cutoff) / samplerate; // Find cutoff frequency in [0..PI]
			float QcWarp = tan(QcRaw); // Warp cutoff frequency

			double gain = 1 / (1+sqrt2/QcWarp + 2/(QcWarp*QcWarp));
			by[2] = (1 - sqrt2/QcWarp + 2/(QcWarp*QcWarp)) * gain;
			by[1] = (2 - 2 * 2/(QcWarp*QcWarp)) * gain;
			by[0] = 1;
			ax[0] = 1 * gain;
			ax[1] = 2 * gain;
			ax[2] = 1 * gain;
		}

		void filter(float* samples, int count, float ax[3], float by[3])
		{
			float xv[3] = {0,0,0};
			float yv[3] = {0,0,0};

		   for (int i=0;i<count;i++)
		   {
			   xv[2] = xv[1]; xv[1] = xv[0];
			   xv[0] = samples[i];
			   yv[2] = yv[1]; yv[1] = yv[0];

			   yv[0] =   (ax[0] * xv[0] + ax[1] * xv[1] + ax[2] * xv[2]
							- by[1] * yv[0]
							- by[2] * yv[1]);

			   samples[i] = yv[0];
		   }
		}

		void filterMovingAvg(float* samples, int count, int n)
		{
			float inv_n = 1.0f/(float) n;

			for (int i=n; i<count; ++i)
			{
				samples[i] = samples[i-1] + inv_n * (samples[i] - samples[i-n]);
			}
		}

		void SetIsLearning(bool isLearning)
		{
			if (mIsLearning && !isLearning)
			{
				mStaticJointSpeed = mMinLearnedSpeed + 0.002f * (mMaxLearnedSpeed - mMinLearnedSpeed);
				mStaticJointAccel = mMinLearnedAccel + 0.005f * (mMaxLearnedAccel - mMinLearnedAccel);

				size_t joint_count = mSkeleton->mJoints.size();
				size_t frame_count = mJointScalarKinematicInfos.size() / mSkeleton->mJoints.size();

				float ax[3];
				float by[3];
				getLPCoefficientsButterworth2Pole(44100, 1000, ax, by);

				std::vector<float> samples;
				std::vector<float> height_samples;
				samples.resize(frame_count);
				height_samples.resize(frame_count);

				for (size_t ji=0; ji<joint_count; ++ji)
				{
					int si = ji;
					for (size_t fi=0; fi<frame_count; ++fi)
					{
						JointScalarKinematicInfo& info = mJointScalarKinematicInfos[si];
						samples[fi] = info.mSpeed;
						height_samples[fi] = info.mHeight;
						si += joint_count;
					}

					
					//filter(&(samples[0]), frame_count, ax, by);
					//http://jean-pierre.moreau.pagesperso-orange.fr/Cplus/tsavgol_cpp.txt
					//http://www.chem.uoa.gr/applets/appletsmooth/appl_smooth2.html  nice demo
					//http://lorien.ncl.ac.uk/ming/filter/filmav.htm
					//filter(&(height_samples[0]), frame_count, ax, by);
					//filterMovingAvg(&(samples[0]), frame_count, 6);
					//filterMovingAvg(&(height_samples[0]), frame_count, 6);

					si = ji;
					for (size_t fi=0; fi<frame_count; ++fi)
					{
						JointScalarKinematicInfo& info = mJointScalarKinematicInfos[si];
						info.mSpeed = samples[fi];
						info.mHeight = height_samples[fi];
						si += joint_count;
					}
				}

				/*
				int si = 0;
				for (size_t fi=0; fi<frame_count; ++fi)
				{
					for (size_t ji=0; ji<joint_count; ++ji)
					{
						JointScalarKinematicInfo& info = mJointScalarKinematicInfos[si++];
						//http://baumdevblog.blogspot.com/2010/11/butterworth-lowpass-filter-coefficients.html
					}
				}
				*/
			}
			
			mIsLearning = isLearning;
		}

		void SetIsDetectingStaticJoints(bool isDetectingStaticJoints)
		{
			mIsDetectingStaticJoints = isDetectingStaticJoints;
		}

		bool IsJointStatic(int inIndex) const
		{
			return mJointPhysicsInfos[inIndex].mIsStatic;
		}

		bool IsJointStaticPos(int inIndex) const
		{
			return mJointPhysicsInfos[inIndex].mIsStaticPos;
		}

		bool IsJointStaticSpeed(int inIndex) const
		{
			return mJointPhysicsInfos[inIndex].mIsStaticSpeed;
		}

		void Build(Skeleton& inSkeleton, float inMassPerMeter = 1.0f)
		{
			mSkeleton = &inSkeleton;
			mJointPhysicsInfos.resize(mSkeleton->mJoints.size());
			
			for (int i=0; i<FrameTypeCount; ++i)
			{
				mIsValidModelSpaceJoints[i] = false;
				mModelSpaceJoints[i].clear();
			}

			mMassJointCount = 0;
			mTotalMass = 0.0f;
			
			for (size_t i = 0; i < mJointPhysicsInfos.size(); ++i)
			{
				int normal_child_count =  mSkeleton->mJointHierarchy.mJointChildrenInfos[i].mNormalChildCount;
				mJointPhysicsInfos[i].mMass = 0.0f;
				mJointPhysicsInfos[i].mIsStatic = false;
				
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

		void AnalyzeJoint(int inFrameIndex, int inJointIndex, SkeletonPhysicsParticle& outJoint)
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

			if (mIsLearning)
			{
				mJointScalarKinematicInfos.push_back(JointScalarKinematicInfo());
				JointScalarKinematicInfo& info = mJointScalarKinematicInfos.back();

				if (mIsValidModelSpaceJoints[Frame_0])
				{
					info.mHeight = outJoint.mPosition.y;
				}
				else
					info.mHeight = 0.0f;


				if (outJoint.mIsValidVelocity)
				{
					float speed = glm::length(outJoint.mVelocity);
					info.mSpeed = speed;

					if (speed != 0.0f && (mMinLearnedSpeed == -1.0f || speed < mMinLearnedSpeed))
						mMinLearnedSpeed = speed;

					if (speed > mMaxLearnedSpeed)
						mMaxLearnedSpeed = speed;
				}
				else
					info.mSpeed = -1.0f;

				if (outJoint.mIsValidAcceleration)
				{
					float acc = glm::length(outJoint.mAcceleration);
					info.mAcceleration = acc;

					if (acc != 0.0f && (mMinLearnedAccel == -1.0f || acc < mMinLearnedAccel))
						mMinLearnedAccel = acc;

					if (acc > mMaxLearnedAccel)
						mMaxLearnedAccel = acc;
				}
				else
					info.mAcceleration = -1.0f;
			}

			if (!mIsLearning && mIsDetectingStaticJoints/* && mJointPhysicsInfosAnalysisFrameIndex != mFrameIndex*/)
			{
				if (outJoint.mIsValidVelocity && outJoint.mIsValidAcceleration)
				{
					float speed = glm::length(outJoint.mVelocity);
					float acc = glm::length(outJoint.mAcceleration);
					
					if (acc < mStaticJointAccel && speed < mStaticJointSpeed)
						mJointPhysicsInfos[inJointIndex].mIsStatic = true;
					else
						mJointPhysicsInfos[inJointIndex].mIsStatic = false;
				}

				size_t joint_count = mSkeleton->mJoints.size();
				size_t frame_count = mJointScalarKinematicInfos.size() / mSkeleton->mJoints.size();

				int si = (inFrameIndex * joint_count) + inJointIndex;
				int psi = si - joint_count;
				int nsi = si + joint_count;

				if (psi >= 0 && psi < mJointScalarKinematicInfos.size()
					&& nsi < mJointScalarKinematicInfos.size())
				{
					float ssi = mJointScalarKinematicInfos[si].mSpeed;
					float spsi = mJointScalarKinematicInfos[psi].mSpeed;
					float snsi = mJointScalarKinematicInfos[nsi].mSpeed;

					if (mJointScalarKinematicInfos[si].mSpeed <= mJointScalarKinematicInfos[psi].mSpeed
						&& mJointScalarKinematicInfos[si].mSpeed <= mJointScalarKinematicInfos[nsi].mSpeed)
					{
						mJointPhysicsInfos[inJointIndex].mIsStaticSpeed = true;
					}
					else
						mJointPhysicsInfos[inJointIndex].mIsStaticSpeed = false;
				}

				if (psi >= 0 && psi < mJointScalarKinematicInfos.size()
					&& nsi < mJointScalarKinematicInfos.size())
				{
					float ssi = mJointScalarKinematicInfos[si].mHeight;
					float spsi = mJointScalarKinematicInfos[psi].mHeight;
					float snsi = mJointScalarKinematicInfos[nsi].mHeight;

					if (mJointScalarKinematicInfos[si].mHeight <= mJointScalarKinematicInfos[psi].mHeight
						&& mJointScalarKinematicInfos[si].mHeight <= mJointScalarKinematicInfos[nsi].mHeight)
					{
						mJointPhysicsInfos[inJointIndex].mIsStaticPos = true;
					}
					else
						mJointPhysicsInfos[inJointIndex].mIsStaticPos = false;
				}
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
		bool mIsLearning;
		float mMinLearnedSpeed;
		float mMinLearnedAccel;
		float mMaxLearnedSpeed;
		float mMaxLearnedAccel;
		float mStaticJointSpeed;
		float mStaticJointAccel;
		bool mIsDetectingStaticJoints;
		float mRenderMassToLengthScale;
		bool mIncludeRootTranslation;
		bool mIncludeRootAnimTranslation;
		int mFrameIndex;
		float mFrameTime;
		JointTransforms mModelSpaceJoints[FrameTypeCount];
		bool mIsValidModelSpaceJoints[FrameTypeCount];
		JointScalarKinematicInfos mJointScalarKinematicInfos;
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
					const SkeletonPhysicsModel& inModel, 
					int inJointIndex,
					const SkeletonPhysicsParticle& inParticle,
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
				//glm::mat4 model_view_mat = glm::translate(glm::mat4(), inParticle.mPosition) * inViewMatrix;
				glLoadMatrixf(glm::value_ptr(model_view_mat));

				float mass = (inParticle.mMass < 1.0f ? 1.0f :  inParticle.mMass);

				if (mRenderMas)
					glutSolidSphere((inModel.GetRenderMassToLengthScale() * mass) / (float) (inModel.GetMassJointCount()), 10, 10);

				if (inJointIndex >= 0 && (inModel.IsJointStaticPos(inJointIndex) || inModel.IsJointStaticSpeed(inJointIndex)))
				{
					if (!inModel.IsJointStaticSpeed(inJointIndex))
						glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
					else if (!inModel.IsJointStaticPos(inJointIndex))
						glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
					else
						glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

					glEnable(GL_BLEND);
					glutSolidSphere((inModel.GetRenderMassToLengthScale() * mass * 5.0f) / (float) (inModel.GetMassJointCount()), 10, 10);
					glDisable(GL_BLEND);
				}


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

		void AnalyzeParticles(int inFrameIndex, SkeletonPhysicsModel& inModel, const SkeletonTreeInfo* pTreeInfo)
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
					inModel.AnalyzeJoint(inFrameIndex, i, mParticles[i]);
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
				inRenderer.RenderParticle(inSkeleton, inModel, -1, mCOM,
											inViewMatrix, pTreeInfo, &mCOMTrail);
			}

			if (inRenderNodes)
			{
				for (size_t i = 0; i < mParticles.size(); ++i)
				{
					//if (mParticles[i].mMass > 0.0f)
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
							
							inRenderer.RenderParticle(inSkeleton, inModel, i, mParticles[i],
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