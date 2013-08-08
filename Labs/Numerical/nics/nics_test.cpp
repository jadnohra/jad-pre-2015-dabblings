#include "nics.h"
#include "stdio.h"
#include "conio.h"

using namespace nics;

void rtt_test1()
{
	int pt_count = 1024*64;
	typedef Rrt_Rn<float, 2> Rrt;
	Rrt rrt;
	float min=-1.0f; float max=1.0f;
	rrt.init(min, max, 0.1f);

	for (int i=0;i<pt_count; ++i) 
	{
		if (1) 
			rrt.next();
		else
			rrt.nextRand();

	}
}

template<int MantissaBits>
struct float_reduce_chop
{
	enum { Mask = (unsigned int(-1))<< (23-MantissaBits) };
	static float reduce(float f) 
	{ 
		static int guard[(23-MantissaBits)+1];

		using namespace fp754;
		return h2f(f2h(f)&Mask); 
	}
};

template<int MantissaBits>
struct float_reduce_round
{
	enum { Mask = (unsigned int(-1))<< (23-MantissaBits) };
	enum { RoundEps = (unsigned int(1))<< (23-MantissaBits) };
	
	static float reduce(float f) 
	{ 
		using namespace fp754;
		
		static int guard[(23-MantissaBits)-1];

		if (f < 0.0f) return -reduce(-f);
		float rnd_mul = h2f(FLTCT_EXP0 | (RoundEps>>1));
		float r = h2f(f2h(f * rnd_mul) & Mask); 
		return r;
	}
};

template<typename Reduce>
struct float_reduce
{
	float v;

	static float reduce(float f) { return Reduce::reduce(f); }

	float_reduce() {}
	float_reduce(const float f) { v = reduce(f); }

	//operator float() { return v; }
	operator double() { return double(v); }

	float_reduce operator-() { return float_reduce(-v); }
 	float_reduce operator+(const float_reduce& v1) { float o = reduce(v+v1.v); return float_reduce(o); }
 	float_reduce operator-(const float_reduce& v1) { float o = reduce(v-v1.v); return float_reduce(o); }
 	float_reduce operator*(const float_reduce& v1) { float o = reduce(v*v1.v); return float_reduce(o); }
	float_reduce operator/(const float_reduce& v1) { float o = reduce(v/v1.v); return float_reduce(o); }

	float_reduce& operator*=(const float_reduce& v1) { v = reduce(v*v1.v); return *this; }
	float_reduce& operator+=(const float_reduce& v1) { v = reduce(v+v1.v); return *this; }
	float_reduce& operator/=(const float_reduce& v1) { v = reduce(v/v1.v); return *this; }
	
	bool operator==(const float_reduce& v1) const { return v >= v1.v; }
	bool operator>=(const float_reduce& v1) const { return v >= v1.v; }
	bool operator>(const float_reduce& v1) const { return v > v1.v; }
	bool operator<(const float_reduce& v1) const { return v < v1.v; }
};
template<typename Reduce>
float_reduce<Reduce> abs(float_reduce<Reduce> v) { return v.0 >= 0.0f ? v : -v; }


template<typename T>
T det_2d(T a, T b, T c, T d) 
{ 
	return a*d-b*c; 
}

template<typename T>
T m_abs(T v) { return v >= T(0) ? v : -v; }



template<typename Rrt, typename T>
struct Instrum
{
	static double diff(typename Rrt::Vertex v) 
	{
		double vf = (double) det_2d<T>(v[0], v[1], v[2], v[3]);
		double vd = (double) det_2d<double>(double(v[0]), double(v[1]), double(v[2]), double(v[3]));
		return vd-vf;
	}

	static double descend(typename Rrt::Vertex v, typename Rrt::Scalar* cube, int max_depth = -1)
	{
		typename Rrt::Scalar center[Rrt::D];
		Rrt::copy(v, center);
		
		double diffs[16];
		double center_diff;
		
		int max_diff = 0;
		int descend_count = 0;

		center_diff = m_abs(diff(center));
		
		while(descend_count < max_depth || max_depth < 0) 
		{
			for (int i=0; i<16; ++i)
			{
				typename Rrt::Scalar corner[Rrt::D];
				Rrt::add(cube+(i*Rrt::D), center, corner);
				diffs[i] = m_abs(diff(corner));

				if (diffs[i] > diffs[max_diff])
				{
					max_diff = i;
				}
			}

			descend_count++;
			double diff_inc = diffs[max_diff] - center_diff;
			if (diff_inc > 0.0)
			{
				Rrt::Scalar corner[Rrt::D];
				Rrt::add(cube+(max_diff*Rrt::D), center, corner);
				Rrt::copy(corner, center);
				center_diff = m_abs(diff(center));
			}
			else
			{
				return center_diff;
			}
		} 

		return center_diff;
	}

	static double descend(typename Rrt::Vertex v, int* choices, int max_depth = -1)
	{
		typename Rrt::Scalar center[Rrt::D];
		Rrt::copy(v, center);
		
		double diffs[81];
		double center_diff;
		
		int max_diff = 0;
		int descend_count = 0;

		center_diff = m_abs(diff(center));
		
		while(descend_count < max_depth || max_depth < 0) 
		{
			for (int i=0; i<81; ++i)
			{
				typename Rrt::Scalar corner[Rrt::D];
				int* v_choices = choices+(i*Rrt::D);
				for (int j=0;j<Rrt::D; ++j) corner[j] = fp754::walkFloats(center[j], v_choices[j]);
				diffs[i] = m_abs(diff(corner));

				if (diffs[i] > diffs[max_diff])
				{
					max_diff = i;
				}
			}

			descend_count++;
			double diff_inc = diffs[max_diff] - center_diff;
			if (diff_inc > 0.0)
			{
				typename Rrt::Scalar corner[Rrt::D];
				int* v_choices = choices+(max_diff*Rrt::D);
				for (int j=0;j<Rrt::D; ++j) corner[j] = fp754::walkFloats(center[j], v_choices[j]);
				Rrt::copy(corner, center);
				center_diff = m_abs(diff(center));
			}
			else
			{
				return center_diff;
			}
		} 

		return center_diff;
	}
};


int nics_test1()
{
	float test_radius = 10.0f;
	int test_pt_count = 1024*32;
	
	if (0)
	{
		float gamma4 = (4.0f * fp754::floatMachineEps() / ( 1.0f -  4.0f * fp754::floatMachineEps()));
		// This is assuming the four values are inexact, fix this to obtain a tighter bound.
		//float abse = (fabs(ax*by)+fabs(bx*ay)) * gamma4; // what is the maximum of abse given the range?
		float max_abse = (fabs(test_radius*test_radius)+fabs(test_radius*test_radius)) * gamma4;
		printf("test_radius:%f, max_abse: %f\n", test_radius, float(max_abse));
	}

	if (1)
	{
		float dd = fp754::floatMachineEps()*fp754::floatMachineEps() + 2.0f * fp754::floatMachineEps();
		float gamma2 = dd / (1.0f + dd);
		// This is assuming the four values are inexact, fix this to obtain a tighter bound.
		//float abse = (fabs(ax*by)+fabs(bx*ay)) * gamma4; // what is the maximum of abse given the range?
		float max_abse = (fabs(test_radius*test_radius)+fabs(test_radius*test_radius)) * gamma2;
		printf("test_radius:%f, max_abse: %f\n", test_radius, float(max_abse));
	}

	{
		typedef float_reduce<float_reduce_chop<12>> flt;
		typedef float_reduce<float_reduce_chop<23>> flt1;
		flt d = det_2d<flt>(1.0f, 0.0f, 2.0f, 3.0f);
		d = d;
		float df = det_2d(1.0f, 0.0f, 2.0f, 3.0f);

		flt a(1.0f);
		flt b(1.0f);
		flt c = a+flt(1.0f);
		c = a+b;
		flt t1 = flt(0.25f) + flt(0.5f);
		flt t2 = flt(0.25f) + flt(0.5f+fp754::floatMachineEps());
		float f = fp754::floatMachineEps();
		flt1 ee = flt1(fp754::floatMachineEps());
		flt1 t3 = flt1(0.25f) + flt1(0.5f+fp754::floatMachineEps());
		
		flt1 t3_1 = flt1(0.25f);
		flt1 t3_2 = flt1(0.5f+fp754::floatMachineEps());
		flt1 t3_3 = t3_1 + t3_2;
	}

	{
		float ff = fp754::h2f(0x3f800003);
		float nff = -ff;

		typedef float_reduce<float_reduce_chop<21>> flt1;
		typedef float_reduce<float_reduce_round<21>> flt2;

		flt1 cp(ff);
		flt2 rd(ff);

		flt1 ncp(nff);
		flt2 nrd(nff);

		int x=0;x;
	}

	{
		typedef Rrt_Rn<float, 4> Rrt; 
		typedef Instrum<Rrt, Rrt::Scalar> Instrum;

		Rrt rrt;
		rrt.init(-test_radius, test_radius, 0.01f);
		{
			Rrt::Vertex v = rrt.nextRand();
			double d = m_abs(Instrum::diff(v));

			Rrt::Scalar cube[16*4];
			Rrt::build_cube(cube, 1.e-2f);
			Instrum::descend(v, cube);
		}

		Rrt::Scalar cube[16*4];
		Rrt::build_cube(cube, 1.e-4f);

		int choices[81*4];
		Rrt::build_choices(choices);

		double largest_diff1 = 0.0;
		double largest_diff2 = 0.0;
		double largest_diff3 = 0.0;

		int pt_count = test_pt_count;
		for (int i=0;i<pt_count; ++i) 
		{
			Rrt::Vertex v;
			if (0) 
				v = rrt.next();
			else
				v = rrt.nextRand();

			double d1, d2, d3;
			d1 = m_abs(Instrum::diff(v));
			d2 = Instrum::descend(v, cube);
			d3 = Instrum::descend(v, choices);
			
			if (d1 > largest_diff1)
				largest_diff1 = d1;
			if (d2 > largest_diff2)
				largest_diff2 = d2;
			if (d3 > largest_diff3)
				largest_diff3 = d3;
		}
		// eps is at least half of largest_diff.
		// Is it true that eps must go down with less prec unconditionally? Then we can get upp. Bound.
		printf("%f,%f,%f\n", float(largest_diff1), float(largest_diff2), float(largest_diff3));
	}

	{
		typedef Rrt_Rn<float, 4> Rrt; 
		typedef Instrum<Rrt, float_reduce<float_reduce_chop<22>>> Instrum;

		Rrt rrt;
		rrt.init(-test_radius, test_radius, 0.01f);
		
		Rrt::Scalar cube[16*4];
		Rrt::build_cube(cube, 1.e-4f);

		int choices[81*4];
		Rrt::build_choices(choices);

		double largest_diff1 = 0.0;
		double largest_diff2 = 0.0;
		double largest_diff3 = 0.0;

		int pt_count = test_pt_count;
		for (int i=0;i<pt_count; ++i) 
		{
			Rrt::Vertex v;
			if (0)	// why does this not work in chop mode?!??!?!?
				v = rrt.next();
			else
				v = rrt.nextRand();

			double d1, d2, d3;
			d1 = m_abs(Instrum::diff(v));
			d2 = Instrum::descend(v, cube, 100);	// why do we need the limit?
			d3 = Instrum::descend(v, choices, 100);
			
			if (d1 > largest_diff1)
				largest_diff1 = d1;
			if (d2 > largest_diff2)
				largest_diff2 = d2;
			if (d3 > largest_diff3)
				largest_diff3 = d3;
		}
		// eps is at least half of largest_diff.
		// Is it true that eps must go down with less prec unconditionally? Then we can get upp. Bound.
		printf("%f,%f,%f\n", float(largest_diff1), float(largest_diff2), float(largest_diff3));
	}

	return 0;
}

int main()
{
	nics_test1();

	printf("done..."); _getch();
	return 0;
}