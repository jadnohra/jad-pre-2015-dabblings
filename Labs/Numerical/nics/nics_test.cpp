#include "nics.h"
#include "stdio.h"

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

template<typename T>
T det_2d(T a, T b, T c, T d) { return a*d-b*c; }

template<typename T>
T m_abs(T v) { return v >= T(0) ? v : -v; }


int nics_test1()
{
	typedef Rrt_Rn<float, 4> Rrt; 

	struct Instrum
	{
		static double diff(Rrt::Vertex v) 
		{
			double vf = (double) det_2d<float>(v[0], v[1], v[2], v[3]);
			double vd = (double) det_2d<double>(double(v[0]), double(v[1]), double(v[2]), double(v[3]));
			return vd-vf;
		}

		static double descend(Rrt::Vertex v, Rrt::Scalar* cube)
		{
			Rrt::Scalar center[Rrt::D];
			Rrt::copy(v, center);

			
			double diffs[16];
			double center_diff;
			
			int max_diff = 0;
			int descend_count = 0;

			center_diff = m_abs(diff(center));
			
			while(1) 
			{
				for (int i=0; i<16; ++i)
				{
					Rrt::Scalar corner[Rrt::D];
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
		}

		static double descend(Rrt::Vertex v, int* choices)
		{
			Rrt::Scalar center[Rrt::D];
			Rrt::copy(v, center);
			
			double diffs[81];
			double center_diff;
			
			int max_diff = 0;
			int descend_count = 0;

			center_diff = m_abs(diff(center));
			
			while(1) 
			{
				for (int i=0; i<81; ++i)
				{
					Rrt::Scalar corner[Rrt::D];
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
					Rrt::Scalar corner[Rrt::D];
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
		}
	};

	
	Rrt rrt;
	float test_radius = 1000.0f;
	rrt.init(-test_radius, test_radius, 0.01f);
	Rrt::Vertex v = rrt.nextRand();
	double d = m_abs(Instrum::diff(v));

	Rrt::Scalar cube[16*4];
	Rrt::build_cube(cube, 1.e-2f);
	Instrum::descend(v, cube);

	{
		float gamma4 = (4.0f * fp754::floatMachineEps() / ( 1.0f -  4.0f * fp754::floatMachineEps()));
		// This is assuming the four values are inexact, fix this to obtain a tighter bound.
		//float abse = (fabs(ax*by)+fabs(bx*ay)) * gamma4; // what is the maximum of abse given the range?

		float max_abse = (fabs(test_radius*test_radius)+fabs(test_radius*test_radius)) * gamma4;
		printf("test_radius:%f, max_abse: %f\n", test_radius, float(max_abse));
	}

	{
		Rrt::Scalar cube[16*4];
		Rrt::build_cube(cube, 1.e-4f);

		int choices[81*4];
		Rrt::build_choices(choices);

		double largest_diff1 = 0.0;
		double largest_diff2 = 0.0;
		double largest_diff3 = 0.0;

		int pt_count = 1024*64;
		for (int i=0;i<pt_count; ++i) 
		{
			Rrt::Vertex v;
			if (1) 
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

	return 0;
}

int main()
{
	nics_test1();

	return 0;
}