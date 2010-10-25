#ifndef _INCLUDED_BIGFOOT_COLORUTIL_H
#define _INCLUDED_BIGFOOT_COLORUTIL_H

#include "BFMath.h"	
#include "Random.h"

namespace BF
{
	/*
	 *	inH	[0.0, 360.0]
	 *	inS	[0.0, 1.0]
	 *	inV	[0.0, 1.0]
	 */
	void gHSVToRGB(float inH, float inS, float inV, float& outR, float& outG, float& outB);


	class GoldenRatioRandomColorGenerator
	{
	public:

		GoldenRatioRandomColorGenerator(RandomSeed inSeed = 0);

		glm::vec3 Next();

	protected:

		float mValue;
	};


	typedef GoldenRatioRandomColorGenerator ColorGen;
}

#endif