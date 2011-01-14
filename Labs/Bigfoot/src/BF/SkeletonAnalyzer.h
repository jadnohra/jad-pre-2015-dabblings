#ifndef _INCLUDED_BIGFOOT_SKELETON_ANALYZER_H
#define _INCLUDED_BIGFOOT_SKELETON_ANALYZER_H

#include <vector>
#include <map>
#include "Skeleton.h"
#include "BFMath.h"

namespace BF
{

	class SkeletonTreeInfo
	{
	public:

		typedef std::vector<int> Indices;
		typedef Indices JointIndices;

		class Branch
		{
		public:

			Branch() : mIsValid(false) {}

			bool mIsValid;
			Indices mJointIndices;
			Indices mChildBranches;
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
		typedef std::map<int, int> JointToBranchMap;

		Branches mBranches;
		LinkToBranchMap mLinkToBranchMap;
		JointToBranchMap mJointChildToBranchMap;

		int GetLinkBranch(int inFromJointIndex, int inToJointIndex) const
		{
			LinkToBranchMap::const_iterator it = mLinkToBranchMap.find(Link(inFromJointIndex, inToJointIndex));
			if (it != mLinkToBranchMap.end())
				return it->second;

			return -1;
		}

		int GetChildBranch(int inToJointIndex) const
		{
			JointToBranchMap::const_iterator it = mJointChildToBranchMap.find(inToJointIndex);
			if (it != mJointChildToBranchMap.end())
				return it->second;

			return -1;
		}

		void MapJointToBranch(int inFromJointIndex, int inToJointIndex, int inBranchIndex)
		{
			mLinkToBranchMap[Link(inFromJointIndex, inToJointIndex)] = inBranchIndex;
			mJointChildToBranchMap[inToJointIndex] = inBranchIndex;
		}

		void Build(const Skeleton& inSkeleton)
		{
			mBranches.clear();
			mLinkToBranchMap.clear();
			mJointChildToBranchMap.clear();
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
			MapJointToBranch(inParentJointIndex, inJointIndex, inBranchIndex); 
			mBranches[inBranchIndex].mJointIndices.push_back(inJointIndex);
			mBranches[inBranchIndex].mIsValid = true;

			int child_count = inSkeleton.mJointHierarchy.mJointChildrenInfos[inJointIndex].mChildCount;
		
			if (child_count == 0)
			{
			
			} else if (child_count == 1)
			{
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


	class SkeletonSemanticInfo
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

		typedef SkeletonTreeInfo::JointIndices JointIndices;
		
		class BodyPart
		{
		public:

			EBodyPartType mType;
			ELimbType mLimbType;
			int mMirrorBodyPart;
			JointIndices mJoints;
		};

		typedef std::vector<BodyPart> BodyParts;
		typedef std::vector<int> BranchBodyParts;

		BodyParts mBodyParts;
		BranchBodyParts mBranchBodyParts;


		//spine vs head: spine has multiple branching symmetries (head probably only one), spine is longer
		void Build(const Skeleton& inSkeleton, SkeletonTreeInfo& ioTreeInfo)
		{
			JointTransforms model;
			AAB bounds;

			inSkeleton.ToModelSpace(inSkeleton.mDefaultPose, false, false, model);
			
			for (size_t i=0; i<model.size(); ++i)
				bounds.Include(model[i].mPosition);

			float tolerance = (bounds.GetExtents()[0] + bounds.GetExtents()[1] + bounds.GetExtents()[2]) * 0.05f;

			for (size_t i=0; i<ioTreeInfo.mBranches.size(); ++i)
			{
				ioTreeInfo.mBranches[i].mMirrorBranch = -1;
			}

			for (size_t i=0; i<ioTreeInfo.mBranches.size(); ++i)
			{
				for (size_t j=i+1; j<ioTreeInfo.mBranches.size(); ++j)
				{
					if (ioTreeInfo.mBranches[j].mMirrorBranch == -1)
					{
						if (IsMirrorOf(model, ioTreeInfo.mBranches[i], ioTreeInfo.mBranches[j], tolerance))
						{
							ioTreeInfo.mBranches[i].mMirrorBranch = j;
							ioTreeInfo.mBranches[j].mMirrorBranch = i;
							break;
						}
					}
				}
			}

			mBodyParts.resize(1);
			mBranchBodyParts.resize(ioTreeInfo.mBranches.size());

			mBodyParts[0].mType = BodyPart_Spine;

			for (size_t i=0; i<ioTreeInfo.mBranches.size(); ++i)
			{
				if (ioTreeInfo.mBranches[i].mMirrorBranch == -1)
				{
					mBranchBodyParts[i] = 0;
				}
				else
				{
					mBranchBodyParts[i] = -1;
				}
			}
		}

		bool IsMirrorOf(const JointTransforms& model, 
						const SkeletonTreeInfo::Branch& branch1, const SkeletonTreeInfo::Branch& branch2, float tolerance)
		{
			if ((!branch1.mIsValid) 
				|| (!branch2.mIsValid)
				|| (branch1.mJointIndices.empty())
				|| (branch2.mJointIndices.empty())
				|| (branch1.mJointIndices.size() != branch2.mJointIndices.size()))
				return false;

			size_t index = 0;
			for (; index<branch1.mJointIndices.size(); ++index)
			{
				if (glm::distance(model[branch1.mJointIndices[index]].mPosition, model[branch2.mJointIndices[index]].mPosition) != 0.0f)
					break;
			}

			int mirror_dim = FindMirrorDim(model[branch1.mJointIndices[index]], model[branch2.mJointIndices[index]], tolerance);

			if (mirror_dim < 0)
				return false;

			float test_tolerance = 2.0f * (std::abs(model[branch1.mJointIndices[index]].mPosition[mirror_dim] - model[branch2.mJointIndices[index]].mPosition[mirror_dim]));
			
			for (size_t i=index+1; i<branch1.mJointIndices.size(); ++i)
			{
				float dist = std::abs(model[branch1.mJointIndices[i]].mPosition[mirror_dim] - model[branch2.mJointIndices[i]].mPosition[mirror_dim]);

				if (dist >= test_tolerance)
				{
					int test_mirror_dim = FindMirrorDim(model[branch1.mJointIndices[index]], model[branch2.mJointIndices[index]], tolerance);

					if (test_mirror_dim != mirror_dim)
						return false;
				}
			}

			return true;
		}
		
		int FindMirrorDim(const JointTransform& xfm1, const JointTransform& xfm2, float tolerance)
		{
			float min_dist = glm::distance(xfm1.mPosition[0], xfm2.mPosition[0]);
			int best_dim = 0;

			for (int i=1; i<3; ++i)
			{
				float dist = glm::distance(xfm1.mPosition[i], xfm2.mPosition[i]);

				if (dist < min_dist)
				{
					min_dist = dist;
					best_dim = i;
				}
			}

			if (min_dist >= tolerance)
				return -1;

			return best_dim;
		}
	};
}

#endif