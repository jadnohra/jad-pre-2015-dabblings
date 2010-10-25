/******************************************************************************
* This file is part of The AI Sandbox
* 
* Copyright (c) 2008, AiGameDev.com
* 
* Credits:			See the PEOPLE file in the base distribution.
* License:			You may use this code for your own personal research and
*					education.  For more information, see the LICENSING file.
******************************************************************************/

#ifndef _INCLUDED_BIGFOOT_RANDOM_H
#define _INCLUDED_BIGFOOT_RANDOM_H

/// @file           math/Random.h
/// @brief          Random number generator based on the Mersenne Twister.

extern "C"
{
	#include "3rdparty/dSFMT/dSFMT.h"
}

namespace BF
{

typedef unsigned int RandomSeed;

class Random
{
public:

	Random(RandomSeed seed = 0);

	void init(RandomSeed seed = 0);
	RandomSeed getSeed();

	float randf();

	RandomSeed rands();

	template <typename T>
	T randr(T range);

	template <typename T>
	T randr(T min, T max);

private:

	RandomSeed m_Seed;
	dsfmt_t m_State;
};

#include "Random.inl"

}

#endif 
