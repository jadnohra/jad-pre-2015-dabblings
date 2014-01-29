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
					featureOut[++featureDimOut] = j;
				}
				else
					break;
				j = (j+1)%lv;
				c = c+1;
			}

			j = (mi+lv-1)%lv;
			c = 1;
			while (c < lv && lv>2) // lv>2 : Segments are not closed.
			{
				Sc dp = dot( cvx_vertex(m, v, r, d, j), d);
				if (dp == max)
				{
					if (featureDimOut+1 == maxdim) return false;
					featureOut[++featureDimOut] = j;
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
		FeatureRef2D nearestFeature[2];
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
		int shape1 = (projMin[0] <= projMin[1] ? 0 : 1);
		int shape2 = 1-shape1;

		// Determine configuration
		if (projMin[shape2] > projMax[shape1])
		{
			out.nearestFeature[shape1] = out.maxFeature[shape1];
			out.nearestFeature[shape2] = out.minFeature[shape2];
			out.dist = projMin[shape2] - projMax[shape1];
		}
		else
		{
			if (projMax[shape2] > projMax[shape1])
			{
				out.nearestFeature[shape1] = out.maxFeature[shape1];
				out.nearestFeature[shape2] = out.minFeature[shape2];
				out.dist = projMin[shape2] - projMax[shape1];
			}
			else
			{
				Sc dleft = projMax[shape2] - projMin[shape1];
				Sc dright = projMax[shape1] - projMin[shape2];

				if (dleft < dright)
				{
					out.nearestFeature[shape1] = out.minFeature[shape1];
					out.nearestFeature[shape2] = out.maxFeature[shape2];
					out.dist = -dleft;
				}
				else
				{
					out.nearestFeature[shape1] = out.maxFeature[shape1];
					out.nearestFeature[shape2] = out.minFeature[shape2];
					out.dist = -dright;
				}
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
		Sc r[2] = {r1, r2};

		for (int si=0; si<2; ++si)
		{
			const M3& ms = *m[si];
			V2ptr vs = v[si];
			int lvs = lv[si];

			V2 dv0, dv1;
			dv1 = cvx_vertex(ms, vs, 0);

			if (lvs>1)
			{
				for (int i=0; i<lvs && (lvs>1); ++i)
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
			else
			{
				if (r[si] > 0.0f)
				{
					V2 center = cvx_vertex(ms, vs, 0);

					const M3& pms = *m[1-si];
					V2ptr pvs = v[1-si];
					int plvs = lv[1-si];

					int closest = 0;
					Sc closest_distSq = lenSq( cvx_vertex(pms, pvs, 0) - center );

					for (int i=1; i<plvs; ++i)
					{
						Sc distSq = lenSq( cvx_vertex(pms, pvs, i) - center );
						if (distSq < closest_distSq)
						{
							closest_distSq = distSq;
							closest = i;
						}
					}

					V2 cd = cvx_vertex(pms, pvs, closest) - center;
					V2 d = (cd == v2_z() ? V2(1.0f, 0.0f) : normalize(cd));

					HyperplaneSep2d sep;
					if (!hyperplane_separation2d(m1, v1, lv1, r1, m2, v2, lv2, r2, d, sep)) return false;

					poly_dist_chooseBestSep(hasBestSep, bestSep, sep);
				}
			}
		}
		
		return true;
	}

	bool dist(M3p m1, const ConvexShape2d& s1, M3p m2, const ConvexShape2d& s2, HyperplaneSep2d& sepOut)
	{
		return poly_dist(m1, s1.vp(), s1.vl(), s1.r, m2, s2.vp(), s2.vl(), s2.r, sepOut);
	}

	struct ContactProjection
	{
		int indices[2][2]; // [shape][vertex index]
		V2 vertices[2][2]; // [shape][vertex]
		int vertexCount[2];
	};

	bool contact_projection(M3p m1, const ConvexShape2d& s1, M3p m2, const ConvexShape2d& s2, const HyperplaneSep2d& sep, ContactProjection& proj)
	{
		const M3* m[2] = {&m1, &m2};
		V2ptr v[2] = {s1.vp(), s2.vp()};

		for (int si=0;si<2;++si) 
		{
			for (int i=0;i<2;++i) { proj.indices[si][i] = sep.nearestFeature[si].index[i]; proj.vertices[si][i] = cvx_vertex(*m[si], v[si], proj.indices[si][i]); }
			proj.vertexCount[si] = sep.nearestFeature[si].dim+1;
		}
		
		for (int si=0;si<2;++si) 
		{
			if ( proj.vertexCount[si] == 2)
			{
				if ( proj.vertexCount[1-si] == 2)
				{
					V2 d = proj.vertices[si][1] - proj.vertices[si][0];

					if (d == v2_z()) return false;
			
					Sc selfMin = dot(proj.vertices[si][0], d);
					Sc selfMax = dot(proj.vertices[si][1], d);

					if (selfMax < selfMin) {  d = -d; selfMin = -selfMin; selfMax = -selfMax; }

					int otherVertMinIndex = proj.indices[1-si][0];
					int otherVertMaxIndex = proj.indices[1-si][1];
					V2 otherVertMin = proj.vertices[1-si][0];
					V2 otherVertMax = proj.vertices[1-si][1];
					Sc otherMin = dot(otherVertMin, d);
					Sc otherMax = dot(otherVertMax, d);

					if (otherMax < otherMin) { m_swap(otherMin, otherMax); m_swap(otherVertMin, otherVertMax); m_swap(otherVertMinIndex, otherVertMaxIndex); }

					// Determine configuration
					if (otherMin >= selfMax)
					{
						proj.vertexCount[si]=1;
						proj.indices[si][0] = proj.indices[si][1];
						proj.vertices[si][0] = proj.vertices[si][1];
					}
					else
					{
						if (otherMin >= selfMin)
						{
							if (otherMax >= selfMax)
							{
								proj.vertexCount[si]= (otherMin<selfMax ? 2 : 1);
								proj.indices[si][0] = otherVertMinIndex;
								proj.vertices[si][0] = proj_points(proj.vertices[si][0], proj.vertices[si][1], otherVertMin);
							}
							else
							{
								proj.vertexCount[si]= (otherMin == otherMax ? 1 : 2);
								proj.indices[si][0] = otherVertMinIndex;
								proj.vertices[si][0] = proj_points(proj.vertices[si][0], proj.vertices[si][1], otherVertMin);
								proj.indices[si][1] = otherVertMaxIndex;
								proj.vertices[si][1] = proj_points(proj.vertices[si][0], proj.vertices[si][1], otherVertMax);
							}
						}
						else
						{
							if (otherMax >= selfMax)
							{
						
							}
							else if (otherMax >= selfMin)
							{
								proj.vertexCount[si]= (otherMax>selfMin ? 2 : 1);
								proj.indices[si][1] = otherVertMaxIndex;
								proj.vertices[si][1] = proj_points(proj.vertices[si][0], proj.vertices[si][1], otherVertMax);
							}
							else
							{
								proj.vertexCount[si]=1;
							}
						}
					}
				}
				else
				{
					proj.vertexCount[si]= 1;
					proj.indices[si][0] = proj.indices[1-si][0];
					proj.vertices[si][0] = proj_points(proj.vertices[si][0], proj.vertices[si][1], proj.vertices[1-si][0]);
				}
			}
		}

		return true;
	}
}

#endif