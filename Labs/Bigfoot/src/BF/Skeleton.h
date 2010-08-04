#ifndef _INCLUDED_BIGFOOT_SKELETON_H
#define _INCLUDED_BIGFOOT_SKELETON_H

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include <vector>

namespace BF
{

	struct JointTransform
	{
	public:

		glm::vec3 mPosition;
		glm::quat mOrientation;
	};
	typedef std::vector<JointTransform> JointTransforms;
	typedef std::vector<glm::quat> JointOrientations;



	struct Joint
	{
	public:

		JointTransform mLocalTransform;
	};
	typedef std::vector<Joint> Joints;



	struct JointHierarchyChildInfo
	{
		int mFirstChildIndex;
		int mChildCount;
	};
	typedef std::vector<JointHierarchyChildInfo> JointHierarchyChildInfos;



	class JointHierarchy
	{
	public:

		typedef std::vector<int> Indices;

		Indices mJointChildren;
		JointHierarchyChildInfos mJointChildrenInfos;
	};



	class JointInfo
	{
	public:

		JointInfo()
		:	mChannelInfoIndex(-1)
		{
		}

		std::string mName;
		int mChannelInfoIndex;
	};
	typedef std::vector<JointInfo> JointInfos;



	enum EChannelType
	{
		Channel_Invalid,
		Channel_TranslationX, Channel_TranslationY, Channel_TranslationZ,
		Channel_RotationEulerX, Channel_RotationEulerY, Channel_RotationEulerZ,
		Channel_User,
	};


	class ChannelInfo
	{
	public:

		std::vector<std::string> mChannelName;
		std::vector<EChannelType> mChannelTypes;
	};
	typedef std::vector<ChannelInfo> ChannelInfos;



	class SkeletonAnimationFrame
	{
	public:

		void Destroy()
		{
			mJointOrientations.clear();
		}

		JointOrientations mJointOrientations;
		glm::vec3 mRootTranslation;
	};
	typedef std::vector<SkeletonAnimationFrame> aSkeletonAnimationFrame;



	class SkeletonAnimationFrames
	{
	public:

		void Destroy()
		{
			mSkeletonAnimationFrames.clear();
		}
		
		float mFrameTime;
		aSkeletonAnimationFrame mSkeletonAnimationFrames;
	};



	class Skeleton
	{
	public:

		void Destroy()
		{
			mJoints.clear();
			mJointHierarchy.mJointChildren.clear();
			mJointHierarchy.mJointChildrenInfos.clear();
			mJointInfos.clear();
			mDefaultPose.Destroy();
		}

		int CreateChannelInfo(EChannelType inChannelType[6])
		{
			mChannelInfos.resize(mChannelInfos.size()+1);

			int valid_channel_count = 0;
			for (int j=0; j<6; ++j)
			{
				if (inChannelType[j] != Channel_Invalid)
					++valid_channel_count;
				else
					break;
			}

			mChannelInfos.back().mChannelTypes.resize(valid_channel_count);

			for (int j=0; j<valid_channel_count; ++j)
			{
				if (inChannelType[j] != Channel_Invalid)
					mChannelInfos.back().mChannelTypes[j] = inChannelType[j];
				else
					break;
			}

			return ((int) mChannelInfos.size()) - 1;
		}

		int FindChannelInfo(EChannelType inChannelType[6])
		{
			int valid_channel_count = 0;
			for (int j=0; j<6; ++j)
			{
				if (inChannelType[j] != Channel_Invalid)
					++valid_channel_count;
				else
					break;
			}

			for (size_t i=0; i<mChannelInfos.size(); ++i)
			{
				const ChannelInfo& channel_info = mChannelInfos[i];
				
				if (valid_channel_count == channel_info.mChannelTypes.size())
				{
					bool is_different = false;

					for (int j=0; j<valid_channel_count; ++j)
					{
						if (inChannelType[j] != channel_info.mChannelTypes[j])
						{
							is_different = true;
							break;
						}
					}

					if (!is_different)
						return i;
				}
			}

			return -1;
		}

		void ToModelSpace(const SkeletonAnimationFrame& inSkelAnimFrame, bool inIncludeRootTranslation, bool inIncludeRootAnimTranslation, JointTransforms& outModelSpace) const
		{
			outModelSpace.resize(mJoints.size());
			
			if (!mJoints.empty())
			{
				JointTransform identity;

				ToModelSpace(identity, 0, inSkelAnimFrame, inIncludeRootTranslation, inIncludeRootAnimTranslation, outModelSpace);
			}
		}

		void ToModelSpace(const JointTransform& inParentModelTransform, int inJointIndex, const SkeletonAnimationFrame& inSkelAnimFrame, bool inIncludeRootTranslation, bool inIncludeRootAnimTranslation, JointTransforms& outModelSpace) const
		{
			const JointTransform& local_transform = mJoints[inJointIndex].mLocalTransform;
			const glm::quat& pose_orientation = inSkelAnimFrame.mJointOrientations[inJointIndex];

			if (inJointIndex == 0)
			{
				glm::vec3 root_translation;

				if (inIncludeRootTranslation)
					root_translation += local_transform.mPosition;

				if (inIncludeRootAnimTranslation)
					root_translation += inSkelAnimFrame.mRootTranslation;

				outModelSpace[inJointIndex].mPosition = inParentModelTransform.mPosition + (inParentModelTransform.mOrientation * (root_translation));
			}
			else
				outModelSpace[inJointIndex].mPosition = inParentModelTransform.mPosition + (inParentModelTransform.mOrientation * (local_transform.mPosition));

			outModelSpace[inJointIndex].mOrientation = glm::cross(inParentModelTransform.mOrientation, pose_orientation);
			
			int child_count = mJointHierarchy.mJointChildrenInfos[inJointIndex].mChildCount;

			if (child_count > 0)
			{
				int child_index = mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;

				for (int i=0; i<child_count; ++i)
				{
					ToModelSpace(outModelSpace[inJointIndex], mJointHierarchy.mJointChildren[child_index++], inSkelAnimFrame, false, false, outModelSpace);
				}
			}
		}

	public:

		Joints mJoints;
		JointHierarchy mJointHierarchy;
		JointInfos mJointInfos;
		ChannelInfos mChannelInfos;
		SkeletonAnimationFrame mDefaultPose;
	};

	
}

#endif