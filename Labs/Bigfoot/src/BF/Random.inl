#ifndef _INCLUDED_BIGFOOT_RANDOM_H
#error "Include from parent header only."
#else

inline void Random::init(RandomSeed seed)
{
	m_Seed = seed;
	dsfmt_init_gen_rand(&m_State, m_Seed);
}

inline RandomSeed Random::getSeed()
{
	return m_Seed;
}

inline float Random::randf()
{
	return (float) dsfmt_genrand_close_open(&m_State);
}

template <typename T>
T Random::randr(T range)
{
	return static_cast<T>(randf() * (float)range);
}

template <typename T>
T Random::randr(T min, T max)
{
	return min + randr<T>(max - min);
}

inline RandomSeed Random::rands()
{
	const unsigned int max = ~(unsigned int)0;
	return (RandomSeed)((double)max * dsfmt_genrand_close_open(&m_State));
}

#endif
