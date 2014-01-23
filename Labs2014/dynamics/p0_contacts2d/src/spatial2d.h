#ifndef LAB_SPATIAL2D_H
#define LAB_SPATIAL2D_H

#include "math_ops.h"
#include <vector>

struct ConvexShape2d
{
	std::vector<V2> v;
	Sc r;

	const V2* vp() const { return &(*v.begin()); }
};


V2 cvx_vertex(M3p m, V2ptr v, int lv, Sc r, V2p d, int i)
{
	return (m * v[i]) + (d * r);
}

bool support_cvx(M3p m, V2ptr v, int lv, Sc r, V2p d, V2p nd, Sc& suppOut, int& featureDimOut, int featureOut[2])
{
	int mi=0;
	Sc& max = suppOut;
	max = dot( cvx_vertex(m, v, lv, r, nd, 0), d);

	if (lenSq(d) != Sc(0))
	{
		for (int i=1;i<lv;++i)
		{
			Sc dp = dot( cvx_vertex(m, v, lv, r, nd, i), d);
			if (dp > max) { max = dp; mi = i; }
		}

		// Collect extra points (with the same dot product)
		const int maxdim = 2;
		featureOut[0] = mi;
		featureOut[1] = -1;
		featureDimOut = 0;
		int j = (mi+1)%lv;
		int c = 1;
		while (c < lv)
		{
			Sc dp = dot( cvx_vertex(m, v, lv, r, nd, j), d);
			if (dp == max)
			{
				if (featureDimOut+1 == maxdim) return false;
				featureOut[featureDimOut++] = j;
			}
			else
				break;
			j = (j+1)%lv;
			c = c+1;
		}

		j = (mi+lv-1)%lv;
		c = 1;
		while (c < lv)
		{
			Sc dp = dot( cvx_vertex(m, v, lv, r, nd, j), d);
			if (dp == max)
			{
				if (featureDimOut+1 == maxdim) return false;
				featureOut[featureDimOut++] = j;
			}
			else
				break;
			j = (j+lv-1)%lv;
			c = c+1;
		}
	}

	return true;
}
#endif