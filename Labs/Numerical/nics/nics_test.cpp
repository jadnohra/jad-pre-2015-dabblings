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

int main()
{
	return 0;
}