#ifndef LAB_SPATIAL2D_H
#define LAB_SPATIAL2D_H

#include "math_ops.h"
#include <vector>

struct ConvexShape2d
{
	std::vector<V2> v;
	Sc r;

	ConvexShape2d() : r(0.0f) {}
	const V2* vp() const { return &(*v.begin()); }
	int vl() const { return (int) v.size(); }
};

V2 cvx_vertex(M3p m, V2ptr v, int i)
{
	return (m * v[i]);
}

V2 cvx_vertex(M3p m, V2ptr v, Sc r, V2p d, int i)
{
	return (m * v[i]) + (d * r);
}

namespace sat
{
	bool support(M3p m, V2ptr v, int lv, Sc r, V2p d, Sc& projOut, int& featureDimOut, int featureOut[2])
	{
		int mi=0;
		Sc& max = projOut;
		max = dot( cvx_vertex(m, v, r, d, 0), d);

		if (lenSq(d) != Sc(0))
		{
			for (int i=1;i<lv;++i)
			{
				Sc dp = dot( cvx_vertex(m, v, r, d, i), d);
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
				Sc dp = dot( cvx_vertex(m, v, r, d, j), d);
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
				Sc dp = dot( cvx_vertex(m, v, r, d, j), d);
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

	struct FeatureRef2D
	{
		int dim;
		int index[2]; 
	};

	struct HyperplaneSep2d
	{
		FeatureRef2D minFeature[2];
		FeatureRef2D maxFeature[2];
		Sc dist;
	};

	bool hyperplane_separation2d(M3p m1, V2ptr v1, int lv1, Sc r1, M3p m2, V2ptr v2, int lv2, Sc r2, V2p d, HyperplaneSep2d& out)
	{
		Sc projMin[2], projMax[2];
		bool ok = true;

		ok &= support(m1, v1, lv1, r1, d, projMax[0], out.maxFeature[0].dim, out.maxFeature[0].index );
		ok &= support(m2, v2, lv2, r2, d, projMax[1], out.maxFeature[1].dim, out.maxFeature[1].index );
		ok &= support(m1, v1, lv1, r1, -d, projMin[0], out.minFeature[0].dim, out.minFeature[0].index ); projMin[0] = -projMin[0];
		ok &= support(m2, v2, lv2, r2, -d, projMin[1], out.minFeature[1].dim, out.minFeature[1].index ); projMin[1] = -projMin[1];

		if (!ok) return false;

		// Sort min/max
		if (projMin[0] > projMax[0])
		{
			m_swap(projMin[0], projMax[0]);
			m_swap(out.minFeature[0], out.maxFeature[0]);
		}
		if (projMin[1] > projMax[1])
		{
			m_swap(projMin[1], projMax[1]);
			m_swap(out.minFeature[1], out.maxFeature[1]);
		}

		// Find shape order
		int shape1 = projMin[0] <= projMin[1] ? 0 : 1;
		int shape2 = 1-shape1;

		// Determin configuration
		if (projMin[shape2] > projMax[shape1])
		{
			out.dist = projMin[shape2] - projMax[shape1];
		}
		else
		{
			if (projMax[shape2] > projMax[shape1])
			{
				out.dist = projMin[shape1] - projMax[shape2];
			}
			else
			{
				Sc dleft = projMax[shape2] - projMin[shape1];
				Sc dright = projMax[shape1] - projMin[shape2];

				out.dist = -m_min(dleft, dright);
			}
		}

		return true;
	}

	void poly_dist_chooseBestSep(bool& hasBestSep, HyperplaneSep2d& bestSep, const HyperplaneSep2d& sep)
	{
		if (!hasBestSep)
		{
			bestSep = sep;
			hasBestSep = true;
		}
		else
		{
			if (bestSep.dist > 0.0f)
			{
				if (sep.dist > 0.0f && sep.dist < bestSep.dist)
					bestSep = sep;
			}
			else
			{
				if (sep.dist > bestSep.dist)
					bestSep = sep;
			}
		}
	}

	bool poly_dist(M3p m1, V2ptr v1, int lv1, Sc r1, M3p m2, V2ptr v2, int lv2, Sc r2, HyperplaneSep2d& sepOut)
	{
		HyperplaneSep2d& bestSep = sepOut;
		bool hasBestSep = false;

		const M3* m[2] = {&m1, &m2};
		V2ptr v[2] = {v1, v2};
		int lv[2] = {lv1, lv2};

		for (int si=0; si<2; ++si)
		{
			const M3& ms = *m[si];
			V2ptr vs = v[si];
			int lvs = lv[si];

			V2 dv0, dv1;
			dv1 = cvx_vertex(ms, vs, 0);

			for (int i=0; i<lvs; ++i)
			{
				int ni = (i+1)%lvs;

				dv0 = dv1;
				dv1 = cvx_vertex(ms, vs, ni);
				V2 d = rot90(normalize(dv1-dv0));

				HyperplaneSep2d sep;
				if (!hyperplane_separation2d(m1, v1, lv1, r1, m2, v2, lv2, r2, d, sep)) return false;

				poly_dist_chooseBestSep(hasBestSep, bestSep, sep);
			}
		}
		
		return true;
	}

	bool poly_circle_dist(M3p m1, V2ptr v1, int lv1, Sc r1, M3p m2, V2ptr v2, int lv2, Sc r2, HyperplaneSep2d& sepOut)
	{
		HyperplaneSep2d& bestSep = sepOut;
		bool hasBestSep = false;

		const M3* m[2] = {&m1, &m2};
		V2ptr v[2] = {v1, v2};
		int lv[2] = {lv1, lv2};
		Sc r[2] = {r1, r2};

		int poly = lv1 > 1 ? 0 : 1;
		int circle = 1-poly;

		const M3& ms = *m[poly];
		V2ptr vs = v[poly];
		int lvs = lv[poly];

		V2 center = cvx_vertex(*m[circle], v[circle], 0);
		Sc radius = r[circle];
		
		for (int i=0; i<lvs; ++i)
		{
			// TODO tangents (correct features).
			V2 v = cvx_vertex(ms, vs, i);
			Sc dist = len(v-center)-radius;

			if (!hasBestSep || dist < bestSep.dist)
			{
				hasBestSep = true;
				bestSep.dist = dist;
				bestSep.minFeature[poly].index[0] = i;
			}
		}

		sepOut.minFeature[poly].dim = 0;
		sepOut.minFeature[circle].dim = 0;
		sepOut.minFeature[circle].index[0] = 0;
		
		return true;
	}

	bool circle_dist(M3p m1, V2ptr v1, int lv1, Sc r1, M3p m2, V2ptr v2, int lv2, Sc r2, HyperplaneSep2d& sepOut)
	{
		V2 cd = cvx_vertex(m1, v1, 0) - cvx_vertex(m2, v2, 0);
		V2 d = (cd == v2_z() ? V2(1.0f, 0.0f) : normalize(cd));
		if (!hyperplane_separation2d(m1, v1, lv1, r1, m2, v2, lv2, r2, d, sepOut)) return false;

		return true;
	}


	bool dist(M3p m1, const ConvexShape2d& s1, M3p m2, const ConvexShape2d& s2, HyperplaneSep2d& sepOut)
	{
		if (s1.vl()>1 && s2.vl()>1)
		{
			return poly_dist(m1, s1.vp(), s1.vl(), s1.r, m2, s2.vp(), s2.vl(), s2.r, sepOut);
		}
		else if (s1.vl()>1 || s2.vl()>1)
		{
			return poly_circle_dist(m1, s1.vp(), s1.vl(), s1.r, m2, s2.vp(), s2.vl(), s2.r, sepOut);
		}
		else
		{
			return circle_dist(m1, s1.vp(), s1.vl(), s1.r, m2, s2.vp(), s2.vl(), s2.r, sepOut);
		}
	}

}

#endif