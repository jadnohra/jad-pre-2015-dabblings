#ifndef _SKELETON_H
#define _SKELETON_H

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


	class Skeleton
	{
	public:

		void Destroy()
		{
			mJoints.clear();
			mJointHierarchy.mJointChildren.clear();
			mJointHierarchy.mJointChildrenInfos.clear();
			mJointInfos.clear();
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

			return -1;
		}

		void ToModelSpace(JointTransforms& outModelSpace, bool inIncludeRootTranslation)
		{
			outModelSpace.resize(mJoints.size());
			
			if (!mJoints.empty())
			{
				JointTransform identity;

				ToModelSpace(identity, 0, outModelSpace, inIncludeRootTranslation);
			}
		}

		void ToModelSpace(const JointTransform& inParentModelTransform, int inJointIndex, JointTransforms& outModelSpace, bool inIncludeRootTranslation)
		{
			const JointTransform& local_transform = mJoints[inJointIndex].mLocalTransform;

			if (inJointIndex != 0 || inIncludeRootTranslation)
			{
				outModelSpace[inJointIndex].mPosition = inParentModelTransform.mPosition + (inParentModelTransform.mOrientation * local_transform.mPosition);
			}
			else
				outModelSpace[inJointIndex].mPosition = glm::vec3();

			outModelSpace[inJointIndex].mOrientation = glm::cross(inParentModelTransform.mOrientation, local_transform.mOrientation);
			
			int child_count = mJointHierarchy.mJointChildrenInfos[inJointIndex].mChildCount;

			if (child_count > 0)
			{
				int child_index = mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;

				for (int i=0; i<child_count; ++i)
				{
					ToModelSpace(outModelSpace[inJointIndex], mJointHierarchy.mJointChildren[child_index++], outModelSpace, false);
				}
			}
		}

	public:

		Joints mJoints;
		JointHierarchy mJointHierarchy;
		JointInfos mJointInfos;
		ChannelInfos mChannelInfos;
	};
}

#endif