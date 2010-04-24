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
		glm::vec3 mPosition;
		glm::quat mOrientation;
	};

	struct Joint
	{
		JointTransform mLocalTransform;
	};

	typedef std::vector<Joint> Joints;

	class JointHierarchy
	{
		typedef std::vector<int> Indices;

		Indices mJointChildren;
		Indices mJointChildrenOffsets;
	};

	class Skeleton
	{
	public:

		Joints mJoints;
		JointHierarchy mJointHierarchy;
	};
}

#endif