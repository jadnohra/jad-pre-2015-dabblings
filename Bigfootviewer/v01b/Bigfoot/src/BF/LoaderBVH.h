#ifndef _INCLUDED_BIGFOOT_LOADER_BVH_H
#define _INCLUDED_BIGFOOT_LOADER_BVH_H

#include "Skeleton.h"

namespace BF
{
	class LoaderBVH
	{
	public:

		static bool Load(const char* inFilePath, Skeleton& outSkeleton, SkeletonAnimationFrames* outAnimFrames, bool inDebug = false);

		static glm::vec3 ToOpenGLTranslation(const glm::vec3& inPosBVH_xyz)
		{
			// http://www.cs.wisc.edu/graphics/Courses/cs-838-1999/Jeff/BVH.html
			// One last note about the BVH hierarchy, the world space is defined as a right handed coordinate system with the Y axis as the world up vector. 
			// Thus you will typically find that BVH skeletal segments are aligned along the Y or negative Y axis 
			// (since the characters are often have a zero pose where the character stands straight up with the arms straight down to the side).

			return glm::vec3(inPosBVH_xyz.x, inPosBVH_xyz.y, inPosBVH_xyz.z);
		}

		static glm::quat ToOpenGLOrientation(const glm::vec3& inDegRotBVH_xyz, int inEulerXYZChannelOrder[3])
		{
			// BVH order: RPY
			// concatenate the matrices from left to right Y, X and Z.
			// An alternative method is to compute the rotation matrix directly. A method for doing this is described in Graphics Gems II, p 322.
			//return glm::quat(glm::vec3(glm::radians(inDegRotBVH_xyz.x), glm::radians(inDegRotBVH_xyz.y), glm::radians(inDegRotBVH_xyz.z)));

			// inEulerXYZChannelOrder[0] is the channel order of X rotation in the file, [1] is for Y and [2] is for Z
			// we can choose to use this or ignore it. 


			glm::quat rotY = glm::rotate(glm::quat(), inDegRotBVH_xyz.y, glm::vec3(0.0f, 1.0f, 0.0f));
			glm::quat rotX = glm::rotate(glm::quat(), inDegRotBVH_xyz.x, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat rotZ = glm::rotate(glm::quat(), inDegRotBVH_xyz.z, glm::vec3(0.0f, 0.0f, 1.0f));

			glm::quat ordered_rot[3];

			for (int i=0; i<3; ++i)
			{
				if (inEulerXYZChannelOrder[0] == i)
					ordered_rot[i] = rotX;
				else if (inEulerXYZChannelOrder[1] == i)
					ordered_rot[i] = rotY;
				else if (inEulerXYZChannelOrder[2] == i)
					ordered_rot[i] = rotZ;
			}

			return glm::cross(glm::cross(ordered_rot[0], ordered_rot[1]), ordered_rot[2]);
			//return glm::cross(glm::cross(rotZ, rotX), rotY);
		}
	};
}

#endif