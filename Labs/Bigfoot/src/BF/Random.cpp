#include "Random.h"

namespace BF
{

static Random gRandom;

Random::Random(RandomSeed seed)
:	m_Seed(seed)
{
	init(seed);
}

}