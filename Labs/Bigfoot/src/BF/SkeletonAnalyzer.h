#ifndef _INCLUDED_BIGFOOT_SKELETON_ANALYZER_H
#define _INCLUDED_BIGFOOT_SKELETON_ANALYZER_H

#include <vector>
#include <map>
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

		struct Link
		{
			union
			{
				struct 
				{
					__int16 mFromJointIndex;
					__int16 mToJointIndex;
				};

				struct 
				{
					__int32 mJointIndexHash;
				};
			};

			Link(int inFromJointIndex = -1, int inToJointIndex = -1)
			:	mFromJointIndex(inFromJointIndex)
			,	mToJointIndex(inToJointIndex)
			{
			}
			inline bool operator<(const Link& inComp) const { return mJointIndexHash < inComp.mJointIndexHash; }
		};


		typedef std::vector<Branch> Branches;
		typedef std::map<Link, int> LinkToBranchMap;

		Branches mBranches;
		LinkToBranchMap mLinkToBranchMap;


		void MapJointToBranch(int inFromJointIndex, int inToJointIndex, int inBranchIndex)
		{
			mLinkToBranchMap[Link(inFromJointIndex, inToJointIndex)] = inBranchIndex;
		}

		void Build(const Skeleton& inSkeleton)
		{
			mBranches.clear();
			mLinkToBranchMap.clear();
			mBranches.resize(inSkeleton.mJoints.size());

			int branch_count = 0;
			if (!inSkeleton.mJoints.empty())
			{
				Branch& branch = mBranches[branch_count++];
				RecurseAnalyzeBranch(inSkeleton, -1, 0, 0, branch_count);
			}

			for (int i=0; i<branch_count; ++i)
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

		void RecurseAnalyzeBranch(const Skeleton& inSkeleton, int inParentJointIndex, int inJointIndex, int inBranchIndex, int& ioBranchCount)
		{
			int child_count = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mChildCount;
		
			if (child_count == 0)
			{
				MapJointToBranch(inParentJointIndex, inJointIndex, inBranchIndex); 
				mBranches[inBranchIndex].mJointIndices.push_back(inJointIndex);
				mBranches[inBranchIndex].mIsValid = true;

			} else if (child_count == 1)
			{
				MapJointToBranch(inParentJointIndex, inJointIndex, inBranchIndex); 
				mBranches[inBranchIndex].mJointIndices.push_back(inJointIndex);
				mBranches[inBranchIndex].mIsValid = true;
				
				int first_child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;
				int child_joint = inSkeleton.mJointHierarchy.mJointChildren[first_child_index];
				RecurseAnalyzeBranch(inSkeleton, inJointIndex, child_joint, inBranchIndex, ioBranchCount);
			}
			else
			{
				int first_child_index = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mFirstChildIndex;
				int child_index = first_child_index;

				int first_branch_index = ioBranchCount;

				for (int i=0; i<child_count; ++i)
				{
					Branch& branch = mBranches[ioBranchCount++];
					branch.mIsValid = true;
					branch.mJointIndices.push_back(inJointIndex);
				}

				for (int i=0; i<child_count; ++i)
				{
					int child_joint = inSkeleton. mJointHierarchy.mJointChildren[child_index++];
					RecurseAnalyzeBranch(inSkeleton, inJointIndex, child_joint, first_branch_index+i, ioBranchCount);
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