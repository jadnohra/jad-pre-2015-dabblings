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

		ELimbType
		{
			Limb_Unknown, Limb_Arm, Limb_Leg
		};

		typedef std::vector<int> JointIndices;

		class BodyPart
		{
		public:

			EBodyPartType mType;
			ELimbType mLimbType;
			int mMirrorBodyPart;
			JointIndices mJoints;
		};

		typedef std::vector<BodyPart> BodyParts;

		BodyParts mBodyParts;
	};


	class SkeletonSemanticAnalyzer
	{
	public:

		//spine vs head: spine has multiple branching symmetries (head probably only one), spine is longer
		void Analyze(const Skeleton& inSkeleton, SemanticSkeletonInfo& outInfo)
		{

		}

	protected:

		class Segment
		{
		public:

			SemanticSkeletonInfo::JointIndices mJointIndices;
			int mMirrorSegment;
		};

		typedef std::vector<SkeletonSegment> SkeletonSegments;

		void AnalyzeSegments(const Skeleton& inSkeleton, SkeletonSegments& outSegments)
		{
		}
	};
}

#endif