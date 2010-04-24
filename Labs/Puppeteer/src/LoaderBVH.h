#ifndef _LOADER_BVH_H
#define _LOADER_BVH_H

#include "Skeleton.h"

namespace BF
{
	class LoaderBVH
	{
	public:

		static bool Load(const char* inFilePath, Skeleton& outSkeleton);
	};
}

#endif