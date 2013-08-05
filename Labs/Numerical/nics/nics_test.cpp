#include "nics.h"

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
	};

	
	Rrt rrt;
	rrt.init(-100.0f, 100.0f, 0.01f);
	Rrt::Vertex v = rrt.nextRand();
	double d = m_abs(Instrum::diff(v));

	Rrt::Type cube[16*4];
	Rrt::build_cube(cube);

	return 0;
}

int main()
{
	nics_test1();

	return 0;
}