#ifndef _Random_hpp
#define _Random_hpp

#include "SuperHeader.hpp"

class Random {
public:

	Random(float seed = 0.0f) {

		reset(seed);
	}

	void reset(float seed) {

		mSeed = seed;
		mGenerator.reset(mSeed);
	}

	unsigned int next() {

		return (int) mGenerator.next();
	}

	int next(int min, int max) {

		unsigned int raw = next();
		return min + raw % ((max + 1) - min );
	}

private:

	class Generator_MultWithCarry {
	public:

		Generator_MultWithCarry() {

			reset(0);
		}

		void reset(float seed) {
			
			x = (unsigned long)(seed * 4294967296.0f);
			crry = 1234567;
		}

		unsigned long next() {
			
			unsigned long  mult = 2083801278;
			unsigned long  m_hi = mult >> 16;
			unsigned long  m_lo = mult & 0xFFFF;

			unsigned long  x_hi = x >> 16;
			unsigned long  x_lo = x & 0xFFFF;

			unsigned long  c_hi = crry >> 16;
			unsigned long  c_lo = crry & 0xFFFF;

			x = x_lo * m_lo + c_lo;
			unsigned long axc = x_lo * m_hi + x_hi * m_lo + c_hi + (x >> 16);
			crry = x_hi * m_hi + (axc >> 16);

			x = (x & 0xFFFF) + ((axc << 16) & 0xFFFFFFFF);

			return x;
		}

	private:
		
		unsigned long x;
		unsigned long crry;
	};

private:

	float mSeed;
	Generator_MultWithCarry mGenerator;
};

#endif