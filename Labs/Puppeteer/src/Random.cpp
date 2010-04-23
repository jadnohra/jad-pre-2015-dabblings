#include "Random.h"

static Random gRandom;

Random::Random(RandomSeed seed)
:	m_Seed(seed)
{
	init(seed);
}
