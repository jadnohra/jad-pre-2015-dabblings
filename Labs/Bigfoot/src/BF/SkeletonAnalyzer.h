#ifndef _INCLUDED_BIGFOOT_SKELETON_ANALYZER_H
#define _INCLUDED_BIGFOOT_SKELETON_ANALYZER_H

#include <vector>
#include "Skeleton.h"

namespace BF
{

	class SemanticSkeletonInfo
	{
	public:

		enum EBodyPartType
		{
			BodyPart_Unknown, BodyPart_Spine, BodyPart_Head, BodyPart_Tail, BodyPart_Limb, /*BodyPart_Hips*/
		};

		enum ELimbType
		{
			Limb_Unknown, Limb_Arm, Limb_Leg
		};

		typedef std::vector<int> Indices;
		typedef Indices JointIndices;

		class BodyPart
		{
		public:

			EBodyPartType mType;
			ELimbType mLimbType;
			int mMirrorBodyPart;
			JointIndices mJoints;
		};

		typedef std::vector<BodyPart> BodyParts;
		typedef std::vector<int> JointBodyParts;

		BodyParts mBodyParts;
		JointBodyParts mJointBodyParts;
	};


	class SkeletonTreeInfo
	{
	public:

		class Branch
		{
		public:

			Branch() : mIsValid(false) {}

			bool mIsValid;
			SemanticSkeletonInfo::Indices mJointIndices;
			SemanticSkeletonInfo::Indices mChildBranches;
			int mMirrorBranch;
			int mParentBranch;
		};

		typedef std::vector<Branch> Branches;

		Branches mBranches;
		SemanticSkeletonInfo::JointBodyParts mJointToBranchMap;


		void MapJointToBranch(int inJointIndex, int inBranchIndex)
		{
			mJointToBranchMap[inJointIndex] = inBranchIndex;
		}

		void Build(const Skeleton& inSkeleton)
		{
			mBranches.clear();
			mJointToBranchMap.resize(inSkeleton.mJoints.size());
			mBranches.resize(inSkeleton.mJoints.size());

			int child_count = inSkeleton.mJointHierarchy.mJointChildrenInfos[0].mChildCount;
			int first_child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[0].mFirstChildIndex;
			int child_index = first_child_index;

			int first_segment_index = (int) mBranches.size();

			for (int i=0; i<child_count; ++i)
			{
				inSkeleton.mJointHierarchy.mJointChildren[child_index++];

				mBranches.push_back(Branch());
				mBranches.back().mJointIndices.push_back(0);
				int child_joint = inSkeleton.mJointHierarchy.mJointChildren[child_index++];
				RecurseAnalyzeBranch(inSkeleton, child_joint, first_segment_index+1);
			}


			for (size_t i=0; i<mBranches.size(); ++i)
			{
				if (mBranches[i].mIsValid)
				{
					printf("branch %d\n", i);

					for (size_t j=0; j<mBranches[i].mJointIndices.size(); ++j)
					{
						printf(inSkeleton.mJointInfos[mBranches[i].mJointIndices[j]].mName.c_str());
						printf("\n");
					}
				}
			}
		}

	protected:

		void RecurseAnalyzeBranch(const Skeleton& inSkeleton, int inJointIndex, int inBranchIndex)
		{
			int child_count = inSkeleton.mJointHierarchy.mJointChildrenInfos[0].mChildCount;
			MapJointToBranch(inJointIndex, inBranchIndex); 
			mBranches[inBranchIndex].mJointIndices.push_back(inJointIndex);
			mBranches[inBranchIndex].mIsValid = true;

			if (child_count == 0)
			{
				return;

			} else if (child_count == 1)
			{
				int first_child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;
				int child_joint = inSkeleton.mJointHierarchy.mJointChildren[first_child_index];
				RecurseAnalyzeBranch(inSkeleton, child_joint, inBranchIndex);
			}
			else
			{
				int first_child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;
				int child_index = first_child_index;

				int first_segment_index = (int) mBranches.size();

				for (int i=0; i<child_count; ++i)
				{
					inSkeleton.mJointHierarchy.mJointChildren[child_index++];

					mBranches.push_back(Branch());
					mBranches.back().mJointIndices.push_back(0);
					int child_joint =inSkeleton. mJointHierarchy.mJointChildren[child_index++];
					RecurseAnalyzeBranch(inSkeleton, child_joint, first_segment_index+1);
				}
			}
		}
	};


	class SkeletonSemanticAnalyzer
	{
	public:

		//spine vs head: spine has multiple branching symmetries (head probably only one), spine is longer
		void Analyze(const Skeleton& inSkeleton, SemanticSkeletonInfo& outInfo)
		{
			SkeletonTreeInfo tree_info;
			tree_info.Build(inSkeleton);
		}

	protected:

		
	};
}

#endif