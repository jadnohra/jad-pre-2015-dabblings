#ifndef LAMBY_TOUCHY_H
#define LAMBY_TOUCHY_H

#include "gaussy.h"

namespace gjk
{
	typedef const V2* V2pc;

	#define EL -1
	struct GJK_PERM_0
	{
		static int dim()			{ return 1; }
		static int count()			{ return 1; }
		static int Di_count()		{ return 1; }
		static int* Is()			{ static int l[] =	{0,EL,EL};		return (int*)l; }
		static int* Isp()			{ static int l[] =	{EL,EL};		return (int*)l; }
		static int* Di_index()		{ static int l[] =	{0,EL};			return (int*)l; }
		static int* Union_index()	{ return 0; }
	};
	struct GJK_PERM_1
	{
		static int dim()			{ return 2; }
		static int count()			{ return 3; }
		static int Di_count()		{ return 4; }
		static int* Is()			{ static int l[] =	{0,EL,1,EL,0,1,EL,EL};		return (int*)l; }
		static int* Isp()			{ static int l[] =	{1,EL,0,EL,EL,EL};			return (int*)l; }
		static int* Di_index()		{ static int l[] =	{0,1,2,EL};					return (int*)l; }
		static int* Union_index()	{ static int l[] =	{3,EL,2,EL,EL,EL};			return (int*)l; }
	};
	struct GJK_PERM_2
	{
		static int dim()			{ return 3; }
		static int count()			{ return 7; }
		static int Di_count()		{ return 12; }
		static int* Is()			{ static int l[] =	{0,EL,1,EL,2,EL,0,1,EL,0,2,EL,1,2,EL,0,1,2,EL};		return (int*)l; }
		static int* Isp()			{ static int l[] =	{1,2,EL,0,2,EL,0,1,EL,2,EL,1,EL,0,EL,EL};			return (int*)l; }
		static int* Di_index()		{ static int l[] =	{0,1,2,3,5,7,9,EL};									return (int*)l; }
		static int* Union_index()	{ static int l[] =	{4,6,EL,3,8,EL,5,7,EL,11,EL,10,EL,9,EL,EL};			return (int*)l; }
	};
	
	struct Perm
	{
		int dim;
		int count;
		int Di_count;
		int* Is;
		int* Isp;
		int* Di_index;
		int* Union_index;

		template<typename T> static Perm make()
		{
			Perm p; p.dim=T::dim(); p.count=T::count(); p.Di_count=T::Di_count(); p.Is=T::Is(); p.Isp=T::Isp(); p.Di_index=T::Di_index(); p.Union_index=T::Union_index();
			return p;
		}
	};

	struct CvxScratch
	{
		int* ami;
		int lami;
	};

	struct V2Pair
	{
		V2 p1;
		V2 p2;
	};

	struct GjkVert
	{
		struct amiPair
		{
			int* ami1;
			int lami1;
			int* ami2;
			int lami2;
		};

		V2 Vk;
		V2Pair Pi;
		amiPair IndI;
		V2 DirI;
	};

	struct GjkScratch
	{
		bool use_fallbacks;
		Perm perm[3];
		int maxdim;
		
		CvxScratch cvx1;
		CvxScratch cvx2;
		Rl* Di;
		int lDi;
		Rl* Li;
		int lLi;
		GjkVert* simpl;
		int lsimpl;

		GjkScratch(bool _use_fallbacks = true) 
		{
			use_fallbacks = _use_fallbacks;
			perm[0] = (Perm::make<GJK_PERM_0>());
			perm[1] = (Perm::make<GJK_PERM_1>());
			perm[2] = (Perm::make<GJK_PERM_2>());
			maxdim = perm[2].dim;
			cvx1.ami = (int*) malloc(maxdim*sizeof(int));
			cvx2.ami = (int*) malloc(maxdim*sizeof(int));
			Di = (Rl*) malloc(perm[2].Di_count*sizeof(Rl));
			Li = (Rl*) malloc(maxdim*sizeof(Rl));
			simpl = (GjkVert*) malloc(perm[2].dim*sizeof(GjkVert));
			for (int i=0;i<maxdim; ++i) 
			{
				simpl[i].IndI.ami1 = (int*) malloc(maxdim*sizeof(int));
				simpl[i].IndI.ami2 = (int*) malloc(maxdim*sizeof(int));
			}
		}

		~GjkScratch() 
		{
			free(cvx1.ami);
			free(cvx2.ami);
			free(Di);
			free(Li);
			for (int i=0;i<maxdim; ++i) 
			{
				free(simpl[i].IndI.ami1); 
				free(simpl[i].IndI.ami2); 
			}
			free(simpl);
		}
	};

	V2 cvx_vertex(M3p m, V2pc v, int lv, Rl r, V2p n, int i)
	{
		return add(mulp(m, v[i]), muls(n, r));
	}

	bool support_cvx(CvxScratch& scr, M3p m, V2pc v, int lv, Rl r, V2p d, V2p nd, int maxdim, Rl& out)
	{
		int mi=0;
		Rl& max = out;
		max = dot( cvx_vertex(m, v, lv, r, nd, 0), d);

		if (lenSq(d) != Rl(0))
		{
			for (int i=1;i<lv;++i)
			{
				Rl dp = dot( cvx_vertex(m, v, lv, r, nd, i), d);
				if (dp > max) { max = dp; mi = i; }
			}

			// Collect extra points (with the same dot product)
			scr.ami[0] = mi;
			scr.lami = 1;
			int j = (mi+1)%lv;
			int c = 1;
			while (c < lv)
			{
				Rl dp = dot( cvx_vertex(m, v, lv, r, nd, j), d);
				if (dp == max)
				{
					if (scr.lami+1 == maxdim) return false;
					scr.ami[scr.lami++] = j;
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
				Rl dp = dot( cvx_vertex(m, v, lv, r, nd, j), d);
				if (dp == max)
				{
					if (scr.lami+1 == maxdim) return false;
					scr.ami[scr.lami++] = j;
				}
				else
					break;
				j = (j+lv-1)%lv;
				c = c+1;
			}
		}

		return true;
	}

	struct Out_gjk_support_mink_cvx
	{
		Rl h;
		V2 s;
		V2Pair p;
		CvxScratch* cvx1;
		CvxScratch* cvx2; 
	};

	bool gjk_support_mink_cvx(Out_gjk_support_mink_cvx& out, GjkScratch& scr, M3p m1, V2pc v1, int lv1, Rl r1, M3p m2, V2pc v2, int lv2, Rl r2, V2p d)
	{
		V2 n = normalize(d);
		Rl max1; if (!support_cvx(scr.cvx1, m1, v1, lv1, r1, d, n, scr.maxdim, max1)) return false;
		Rl max2; if (!support_cvx(scr.cvx2, m2, v2, lv2, r2, neg(d), neg(n), scr.maxdim, max2)) return false;

		out.p.p1 = cvx_vertex(m1, v1, lv1, r1, n, scr.cvx1.ami[0]);
		out.p.p2 = cvx_vertex(m2, v2, lv2, r2, neg(n), scr.cvx2.ami[0]);
		out.h = max1+max2; out.s = sub(out.p.p1, out.p.p2); out.cvx1 = &scr.cvx1; out.cvx2 = &scr.cvx2;
		return true;
	}

	struct Out_gjk_subdist
	{
		V2 v;
		int* Isp;
		Rl* Li;
		int lLi;
	};

	bool gjk_subdist_fallback(Out_gjk_subdist& out, GjkScratch& scr, GjkVert* Vk, int lVk)
	{
		const Perm& perm = scr.perm[lVk-1];
		Rl* Di = scr.Di; scr.lDi = perm.Di_count;
		for (int i=0;i<perm.Di_count; ++i) Di[i]=Rl(1.0);
		for (int i=0;i<lVk; ++i)  scr.Li[i]=Rl(0.0);

		int best = -1;
		Rl bestDistSq;
		
		int i_Is = 0; int i_Isp = 0; int i_Union_index = 0;
		for (int pi=0; pi<perm.count; ++pi)
		{
			int d=0; { int i=i_Is; while(perm.Is[i++] != EL) d++; } 
			int di_index = perm.Di_index[pi];

			Rl D = Rl(0.0);
			for (int i=0; i<d; ++i) D = D + Di[di_index+i];

			int* Is = perm.Is + i_Is;
			int* Isp = perm.Isp + i_Isp;
			int* Union_index = perm.Union_index + i_Union_index;

			for (int j=0; Isp[j] != EL; ++j)
			{
				Rl Dj = Rl(0.0); for (int i=0; i<d; ++i) Dj = Dj + (Di[di_index+i] * ( dot(Vk[Is[i]].Vk, sub(Vk[Is[0]].Vk, Vk[Isp[j]].Vk)) ) );
				Di[Union_index[j]] = Dj;
			}

			bool cond1 = D > Rl(0.0);
			if (cond1)
			{
				bool cond2 = true;
				for (int i=0; i<d; ++i) if (Di[di_index+i] <= Rl(0.0)) { cond2 = false; break; }

				if (cond2)
				{
					V2 v = v2_z();
					Rl* Li = scr.Li; scr.lLi = lVk;
					Rl iD = Rl(1.0) / D;
					for (int i=0; i<d; ++i)
					{
						Rl l = Di[di_index+i] * iD;
						v = add(v, muls(Vk[Is[i]].Vk, l));
						Li[Is[i]] = l;
					}

					Rl distSq = lenSq(v);
					if (best < 0 || distSq < bestDistSq)
					{
						best = pi;
						bestDistSq = distSq;
						out.v = v; out.Isp = Isp; out.Li = Li; out.lLi = scr.lLi; 
					}
				}
			}

			while(perm.Is[i_Is++] != EL); while(perm.Isp[i_Isp++] != EL); while(perm.Union_index[i_Union_index++] != EL);
		}

		return true;
	}

	bool gjk_subdist(Out_gjk_subdist& out, GjkScratch& scr, GjkVert* Vk, int lVk)
	{
		const Perm& perm = scr.perm[lVk-1];
		Rl* Di = scr.Di; scr.lDi = perm.Di_count;
		for (int i=0;i<perm.Di_count; ++i) Di[i]=Rl(1.0);
		for (int i=0;i<lVk; ++i)  scr.Li[i]=Rl(0.0);

		int i_Is = 0; int i_Isp = 0; int i_Union_index = 0;
		for (int pi=0; pi<perm.count; ++pi)
		{
			int d=0; { int i=i_Is; while(perm.Is[i++] != EL) d++; } 
			int di_index = perm.Di_index[pi];

			Rl D = Rl(0.0);
			for (int i=0; i<d; ++i) D = D + Di[di_index+i];

			int* Is = perm.Is + i_Is;
			int* Isp = perm.Isp + i_Isp;
			int* Union_index = perm.Union_index + i_Union_index;

			bool cond3 = true;
			for (int j=0; Isp[j] != EL; ++j)
			{
				Rl Dj = Rl(0.0); for (int i=0; i<d; ++i) Dj = Dj + (Di[di_index+i] * ( dot(Vk[Is[i]].Vk, sub(Vk[Is[0]].Vk, Vk[Isp[j]].Vk)) ) );
				Di[Union_index[j]] = Dj;
				if (Dj > Rl(0.0)) cond3 = false;
			}

			bool cond1 = D > Rl(0.0);
			if (cond1)
			{
				bool cond2 = true;
				for (int i=0; i<d; ++i) if (Di[di_index+i] <= Rl(0.0)) { cond2 = false; break; }

				if (cond2 && cond3)
				{
					V2 v = v2_z();
					Rl* Li = scr.Li; scr.lLi = lVk;
					Rl iD = Rl(1.0) / D;
					for (int i=0; i<d; ++i)
					{
						Rl l = Di[di_index+i] * iD;
						v = add(v, muls(Vk[Is[i]].Vk, l));
						Li[Is[i]] = l;
					}
					out.v = v; out.Isp = Isp; out.Li = Li; out.lLi = scr.lLi; 
					return true;
				}
			}

			while(perm.Is[i_Is++] != EL); while(perm.Isp[i_Isp++] != EL); while(perm.Union_index[i_Union_index++] != EL);
		}

		// Add failure case support from original paper and from VanDenBergen 'A Fast and Robust GJK Implementation for Collision Detection of Convex Objects'
		return scr.use_fallbacks ? gjk_subdist_fallback(out, scr, Vk, lVk) : false;
	}

	void gjk_find_features() {}

	struct Out_gjk_distance
	{
		bool success;
		Rl dist;
		Rl eps;
		V2 v1;
		V2 v2;

		Out_gjk_distance() {}
		Out_gjk_distance(bool success_, Rlp dist_, Rlp eps_, V2p v1_, V2p v2_) : success(success_), dist(dist_), eps(eps_), v1(v1_), v2(v2_) {}
	};

	Out_gjk_distance gjk_distance(GjkScratch& scr, M3p m1, V2pc v1, int lv1, Rl r1, M3p m2, V2pc v2, int lv2, Rl r2, Rl eps)
	{
		V2 d = V2(Rl(-1.0), Rl(0.0));
		int& lsimpl = scr.lsimpl; GjkVert* simpl = scr.simpl; 
		{
			Out_gjk_support_mink_cvx supp; if (!gjk_support_mink_cvx(supp, scr, m1, v1, lv1, r1, m2, v2, lv2, r2, d)) return Out_gjk_distance(false, 0.0f, 0.0f, v2_z(), v2_z());
			lsimpl = 1;
			simpl[0].Vk = supp.s;
			simpl[0].Pi = supp.p;
			for (int i=0;i<supp.cvx1->lami; ++i) simpl[0].IndI.ami1[i] = supp.cvx1->ami[i]; simpl[0].IndI.lami1 = supp.cvx1->lami;
			for (int i=0;i<supp.cvx2->lami; ++i) simpl[0].IndI.ami2[i] = supp.cvx2->ami[i]; simpl[0].IndI.lami2 = supp.cvx2->lami;
			simpl[0].DirI = d;
		}
	
		int max_iter = 3 + (lv1+lv2)*5;
		int iter = 0;

		while(1)
		{
			iter = iter + 1;

			//#if (fallback):
			//#	subd = gjk_subdist_fallback(ctx, Vk)
			//#else:
			Out_gjk_subdist subd;
			if (!gjk_subdist(subd, scr, simpl, lsimpl)) return Out_gjk_distance(false, 0.0f, 0.0f, v2_z(), v2_z());


			V2 vk = subd.v;
			V2 nvk = neg(vk);
			Out_gjk_support_mink_cvx supp; if (!gjk_support_mink_cvx(supp, scr, m1, v1, lv1, r1, m2, v2, lv2, r2, nvk)) return Out_gjk_distance(false, 0.0f, 0.0f, v2_z(), v2_z());
			Rl g = dot(vk, vk) + supp.h;

			Rl dist = len(vk);
			bool eps_cond = (lsimpl==3 && subd.Isp[0]==EL);
			if ( (m_abs(g) < eps) || (iter > max_iter) || eps_cond)
			{
				V2 v1 = v2_z();
				V2 v2 = v2_z();
				Rl* li = subd.Li;
				for (int i=0; i<subd.lLi; ++i)
				{
					v1 = add(v1, muls(simpl[i].Pi.p1, li[i]));
					v2 = add(v2, muls(simpl[i].Pi.p2, li[i]));
				}

				//features = 	gjk_find_features(m1, cvx1, r1, m2, cvx2, r2, li, IndI)
				//return [dist, eps, v1, v2, features, Vk, Pi, IndI]
				return Out_gjk_distance(true, dist, eps_cond ? dist : eps, v1, v2);
			}

			int nsi = lsimpl < 3 ? lsimpl : subd.Isp[0];
			if (nsi == EL) return Out_gjk_distance(false, 0.0f, 0.0f, v2_z(), v2_z());
			{
				simpl[nsi].Vk = supp.s;
				simpl[nsi].Pi = supp.p;
				for (int i=0;i<supp.cvx1->lami; ++i) simpl[nsi].IndI.ami1[i] = supp.cvx1->ami[i]; simpl[nsi].IndI.lami1 = supp.cvx1->lami;
				for (int i=0;i<supp.cvx2->lami; ++i) simpl[nsi].IndI.ami2[i] = supp.cvx2->ami[i]; simpl[nsi].IndI.lami2 = supp.cvx2->lami;
				simpl[nsi].DirI = nvk;
				lsimpl = m_tmin(3, lsimpl+1);
			}
		}
	}

	#undef EL

	int gjk_test1()
	{
		int err = 0;
		GjkScratch gjk;
		
		if (1)
		{
			V2 v1[] = { V2(0.0f, 1.0f), V2(0.0f, 2.0f), V2(-1.0f, 1.0f) };
			V2 v2[] = { V2(0.0f, 1.5f) };
			Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 3, Rl(0), m3_id(), v2, 1, Rl(0), 1.e-7f);

			if (!dist.success || dist.dist > dist.eps) { err++; }
		}

		if (1)
		{
			V2 v1[] = { V2(0.0f, 1.0f), V2(0.0f, 2.0f), V2(-1.0f, 1.0f) };
			V2 v2[] = { V2(0.0f, 1.5f) };

			for (int x=0; x<512; x+=11) 
				for (int y=0; y<512; y+=11)
			{
				Rl l[] = { Rl(x)/Rl(512), Rl(y)/Rl(512), Rl(0)};
				l[2] = Rl(1) - l[0] - l[1];
				if (l[2] >= Rl(0))
				{
					v2[0] = lin_comb(v1, l, 3);

					Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 3, Rl(0), m3_id(), v2, 1, Rl(0), 1.e-7f);
					if (!dist.success || dist.dist > dist.eps) 
					{ 
						err++; 
					}
				}

			}
		}

		if (1)
		{
			V2 v1[] = { V2(0.0f, 1.0f), V2(0.0f, 2.0f), V2(-1.0f, 1.0f) };
			V2 v2[] = { V2(0.0f, 1.5f) };

			for (int x=0; x<512; x+=11) 
				for (int y=0; y<512; y+=11)
			{
				Rl l[] = { Rl(x)/Rl(512), Rl(1.1) + Rl(y)/Rl(512), Rl(0)};
				l[2] = Rl(1) - l[0] - l[1];
				v2[0] = lin_comb(v1, l, 3);

				Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 3, Rl(0), m3_id(), v2, 1, Rl(0), 1.e-7f);
				if (!dist.success || dist.dist <= dist.eps) 
				{ 
					err++; 
				}
			}
		}

		if (1)
		{
			V2 v1[] = { V2(-1.0f, -5.0f), V2(4.0f, -5.0f), V2(4.0f, 5.0f), V2(-1.0f, 5.0f) };
			V2 v2[] = { V2(0.0f, 0.0f), V2(-3.0f, 3.0f), V2(-3.0f, -3.0f) };
			Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 4, Rl(0), m3_id(), v2, 3, Rl(0), 1.e-7f);

			if (!dist.success || dist.dist > dist.eps) { err++; }
		}
		if (1)
		{
			V2 v1[] = { V2(1.0f, 0.0f), V2(3.0f, 3.0f), V2(0.0f, -5.0f) };
			V2 v2[] = { V2(0.0f, 0.0f) };
			Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 3, Rl(0), m3_id(), v2, 1, Rl(0), 1.e-7f);

			if (!dist.success || dist.dist > 1.0f || dist.dist < 0.9f) { err++; }
		}
		if (1)
		{
			V2 v1[] = { V2(-1.0f, 0.0f), V2(3.0f, 3.0f), V2(0.0f, -5.0f) };
			V2 v2[] = { V2(0.0f, 0.0f) };
			Out_gjk_distance dist = gjk_distance(gjk, m3_id(), v1, 3, Rl(0), m3_id(), v2, 1, Rl(0), 1.e-7f);

			if (!dist.success || dist.dist > dist.eps) { err++; }
		}

		return err;
	}

	int gjk_tests()
	{
		return 
			gjk_test1();
	}

}

/*

	def TestGJK():

	p1 = [0.0, 0.0]
	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	p2 = [0.0, 0.0]
	cvx2 = [[0.0,1.5]]
	
	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,1.5]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,1.5]]
	cvx2 = [linComb(cvx1, [1.0/256.0, 1.0/256.0, 1.0-(2.0/256.0)])]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,1.5]]
	off = linComb(cvx1, [1.0/256.0, 1.0/256.0, 1.0-(2.0/256.0)])
	cvx1.v = [ v2_sub(cvx1[0], off), v2_sub(cvx1[1], off), v2_sub(cvx1[2], off) ]
	cvx2.v = [[0.0,0.0]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist
	
	
	# Test convex combinations inside cvx1
	if (False):
		cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
		cvx2 = [[0.0,1.5]]
		for x in range(512):
			for y in range(512):
				l = [x/512.0, y/512.0, 0.0]
				l[2] = 1.0 - l[0] - l[1]
				if (l[2] >= 0.0):
					cvx2 = [linComb(cvx1, l)]
					dd = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)
					if (dd[0] > dd[1]):
						print 'Fail'

		# Test combinations outside cvx1
		cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
		cvx2 = [[0.0,1.5]]
		for x in range(512):
			l = [x/512.0, 1.1+x/512.0, 0.0]
			cvx2 = [linComb(cvx1, l)]
			dd = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)
			if (dd[0] == 0.0):
				print 'Fail'		

	return 0
*/

#endif

/*
#if 0
import os, inspect
import math
import random

def absPath(relPath):
	return os.path.join(os.path.split(inspect.getfile(inspect.currentframe()))[0], relPath)

execfile(absPath('./gaussy.py'))


#----------------------------------------------------------------------------
#------------ Convex --------------------------------------------------------
#----------------------------------------------------------------------------


def convexVertex(m, cvx, r, n, i):
	v = m2_mulp(m, cvx[i])
	v = v2_add(v, v2_muls(n, r))
	return v


def convexNormal(m, cvx, r, d, i):
	p1 = convexVertex(m, cvx, 0.0, v2_z(), i)
	p2 = convexVertex(m, cvx, 0.0, v2_z(), (i+1)%len(cvx))
	n = v2_orth(v2_sub(p2, p1))
	if (v2_dot(d, n) < 0.0):
		n = v2_neg(n)
	return n	


def linComb(v, l):
	vc = [0.0, 0.0]
	for i in range(len(l)):
		vc = v2_add( vc, v2_muls(v[i], l[i]) )
	return vc	


def randConvex(r, vc):
	v = [None] * vc
	
	t = [None] * vc
	for i in range(vc):
		t[i] = random.uniform(0.0, 2.0*math.pi)
	t.sort()
	
	for i in range(vc):
		v[i] = [math.cos(t[i])*r, math.sin(t[i])*r]

	return v	


#----------------------------------------------------------------------------
#------------ GJK -----------------------------------------------------------
#----------------------------------------------------------------------------

gGJK_eps = 0.0000001

class GJK_Perm_0:
	count = 1
	Di_count = 1;
	Is = [[0]]
	Isp = [[]]
	Dis = [[1]]
	Di_index = [0]
	Union_Index = None

class GJK_Perm_1:
	count = 3
	Di_count = 4;
	Is = [[0], [1], [0, 1]]
	Isp = [[1], [0], []]
	Di_index = [0, 1, 2]
	Union_index = [[3], [2]]

class GJK_Perm_2:
	count = 7
	Di_count = 12;
	Is = [[0], [1], [2], [0,1], [0,2], [1,2], [0,1,2]]
	Isp = [[1,2], [0,2], [0,1], [2], [1], [0], []]
	Di_index = [0, 1, 2, 3, 5, 7, 9]
	Union_index = [[4,6], [3,8], [5,7], [11] ,[10], [9]]

class GJK_Context:
	perms = [GJK_Perm_0(), GJK_Perm_1(), GJK_Perm_2()]


def gjk_support_cvx(m, cvx, r, d, nd):

	lv = len(cvx)
	mi = 0
	max = v2_dot( convexVertex(m, cvx, r, nd, 0), d)	

	for i in range(1, lv):
		dot = v2_dot( convexVertex(m, cvx, r, nd, i), d)
		if (dot > max):
			max = dot
			mi = i

	# collect extra points (with the same dot product)		
	ami = [mi]		
	j = (mi+1)%lv
	c = 0		
	while (c < lv):
		dot = v2_dot( convexVertex(m, cvx, r, nd, j), d)
		if (dot == max):
			ami.append(j)
		else:
			break
		j = (j+1)%lv
		c = c+1

	j = (mi+lv-1)%lv
	c = 0		
	while (c < lv):
		dot = v2_dot( convexVertex(m, cvx, r, nd, j), d)
		if (dot == max):
			ami.append(j)
		else:
			break
		j = (j+lv-1)%lv
		c = c+1

	return [max, ami]



def gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, d):

	n = v2_normalize(d)

	i1 = gjk_support_cvx(m1, cvx1, r1, d, n)
	i2 = gjk_support_cvx(m2, cvx2, r2, v2_neg(d), v2_neg(n))

	h = i1[0]+i2[0]
	p1 = convexVertex(m1, cvx1, r1, n, i1[1][0])
	p2 = convexVertex(m2, cvx2, r2, v2_neg(n), i2[1][0])
	s = v2_sub(p1, p2)
	return [ h, s, [p1, p2], [i1[1], i2[1]] ] # h, s, points, indices


def gjk_subdist_fallback(ctx, Vk):

	perm = ctx.perms[len(Vk)-1]
	Di = [1.0] * perm.Di_count;	

	best = -1
	best_v = None
	best_dist = None
	best_Isp = None
	best_Li = None

	for pi in range(perm.count):

		d = len(perm.Is[pi])
		di_index = perm.Di_index[pi]

		D = 0.0
		for i in range(d):
			D = D + Di[di_index+i]

		Is = perm.Is[pi]	
		Isp = perm.Isp[pi]

		for j in range(len(Isp)):
			Dj = 0.0
			for i in range(d):
				Dj = Dj + ( Di[di_index+i] * ( v2_dot(Vk[Is[i]], v2_sub(Vk[Is[0]], Vk[Isp[j]]) )  ) )
			Di[perm.Union_index[pi][j]] = Dj

		cond1 = D > 0.0
		if (cond1):

			cond2 = True
			for i in range(d):
				if (Di[di_index+i] <= 0.0):
					cond2 = False
					break
		
			if (cond2):

				v = [0.0, 0.0]
				Li = [0.0] * len(Vk)
				iD = 1.0 / D
				for i in range(d):
					l = Di[di_index+i] * iD
					v = v2_add(v, v2_muls(Vk[Is[i]], l))
					Li[Is[i]] = l

				dist = v2_len(v)
				if (best < 0 or dist < best_dist):
					best = pi
					best_v = v
					best_dist = dist
					best_Isp = Isp
					best_Li = Li


	if (best >= 0):
		return [best_v, best_Isp, best_Li]
					
	return None


def gjk_subdist(ctx, Vk):

	perm = ctx.perms[len(Vk)-1]
	Di = [1.0] * perm.Di_count;	

	for pi in range(perm.count):

		d = len(perm.Is[pi])
		di_index = perm.Di_index[pi]

		D = 0.0
		for i in range(d):
			D = D + Di[di_index+i]

		Is = perm.Is[pi]	
		Isp = perm.Isp[pi]

		cond3 = True	
		for j in range(len(Isp)):
			Dj = 0.0
			for i in range(d):
				Dj = Dj + ( Di[di_index+i] * ( v2_dot(Vk[Is[i]], v2_sub(Vk[Is[0]], Vk[Isp[j]]) )  ) )
			Di[perm.Union_index[pi][j]] = Dj
			if (Dj > 0.0):
				cond3 = False

		cond1 = D > 0.0
		if (cond1):

			cond2 = True
			for i in range(d):
				if (Di[di_index+i] <= 0.0):
					cond2 = False
					break

			if (cond2 and cond3):

				v = [0.0, 0.0]
				Li = [0.0] * len(Vk)
				iD = 1.0 / D
				for i in range(d):
					l = Di[di_index+i] * iD
					v = v2_add(v, v2_muls(Vk[Is[i]], l))
					Li[Is[i]] = l

				return [v, Isp, Li]	# vertex, unused Vk's, lambdas
					
	# Add failure case support from original paper and from VanDenBergen 'A Fast and Robust GJK Implementation for Collision Detection of Convex Objects'
	return gjk_subdist_fallback(ctx, Vk)


def gjk_find_features(m1, cvx1, r1, m2, cvx2, r2, li, IndI):
	
	ui1 = []
	ui2 = []
	for i in range(len(li)):
		if (li[i] != 0.0):
			# consider all extra vertices
			for ind in IndI[i][0]:	
				ui1.append(ind)
			for ind in IndI[i][1]:
				ui2.append(ind)

	f1 = None
	f2 = None

	lv = len(cvx1)
	lu = len(ui1)
	if (lv == 1):
		f1 = [ui1[0]]
	else:	
		ui1 = sorted(ui1)
		f1 = [ui1[0]]
		for i in range(len(ui1)):
			if ((ui1[i]+1)%lv == ui1[(i+1)%lu]):
				f1 = [ui1[i], (ui1[i]+1)%lv]
				break


	lv = len(cvx2)
	lu = len(ui2)
	if (lv == 1):
		f2 = [ui2[0]]
	else:	
		ui2 = sorted(ui2)
		f2 = [ui2[0]]
		for i in range(len(ui2)):
			if ((ui2[i]+1)%lv == ui2[(i+1)%lu]):
				f2 = [ui2[i], (ui2[i]+1)%lv]
				break
	
	return [f1, f2]


#2d,3d in principle
def gjk_distance(m1, cvx1, r1, m2, cvx2, r2, eps=gGJK_eps, dbg = None):

	ctx = GJK_Context()

	d = [-1.0, 0.0]
	supp = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, d) 
	Vk = [ supp[1] ]
	Pi = [ supp[2] ]
	IndI = [ supp[3] ]
	DirI = [ d ]

	max_iter = 3 + (len(cvx1) + len(cvx2))*5
	iter = 0
	#last_dist = v2_len( Vk[0] )
	#fallback = False

	while (True):
		iter = iter + 1	
		
		#if (fallback):
		#	subd = gjk_subdist_fallback(ctx, Vk)
		#else:
		subd = gjk_subdist(ctx, Vk)

		vk = subd[0]
		nvk = v2_neg(vk)	
		supp = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, nvk) 
		g = v2_dot(vk, vk) + supp[0]

		if dbg != None:
			dbg.append( [copy.deepcopy(Vk), copy.deepcopy(vk), copy.deepcopy(d)] )

		dist = v2_len(vk)	
		if ((math.fabs(g) < eps) or (iter > max_iter)):
			
			v1 = [0.0, 0.0]
			v2 = [0.0, 0.0]
			li = subd[2]
			for i in range(len(li)):
				v1 = v2_add(v1, v2_muls(Pi[i][0], li[i]))
				v2 = v2_add(v2, v2_muls(Pi[i][1], li[i]))

			features = 	gjk_find_features(m1, cvx1, r1, m2, cvx2, r2, li, IndI)
			
			return [dist, eps, v1, v2, features, Vk, Pi, IndI]


		#if (dist > last_dist):
		#	fallback = True

		#last_dist = dist
		Vk.append( supp[1] )
		Pi.append( supp[2] )
		IndI.append( supp[3] )
		DirI.append( nvk )
		if (len(Vk)>3):
			nVi = subd[1]
			if (len(nVi) > 0):
				popi = nVi[0]
				Vk.pop(popi)
				Pi.pop(popi)
				IndI.pop(popi)
				DirI.pop(popi)
			else:
				return None


#----------------------------------------------------------------------------
#------------ EPA -----------------------------------------------------------
#----------------------------------------------------------------------------

gEPA_eps = 0.0001

def gjk_epa_closest_on_edge(ctx, v1, v2):
	Vk = [v1, v2]
	subd = gjk_subdist(ctx, Vk)
	return [subd[0], subd[2]] # vert, lambdas


#only 2D
def gjk_epa_distance(m1, cvx1, r1, m2, cvx2, r2, epa_eps=gEPA_eps, gjk_eps=gGJK_eps, dbg = None):

	gjkOut = gjk_distance(m1, cvx1, r1, m2, cvx2, r2, gjk_eps, dbg) 
	if (gjkOut[0] >= gjkOut[1]):
		return gjkOut

	ctx = GJK_Context()

	Vk = gjkOut[5]
	Pi = gjkOut[6]
	IndI = gjkOut[7]
	
	# treat degenerate case
	if (len(Vk)==1):
		supp = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, [1.0, 0.0])
		Vk.append(supp[1])
		Pi.append(supp[2])
		IndI.append(supp[3])

	# treat degenerate case
	if (len(Vk)==2):	
		d = v2_sub(Vk[1], Vk[0])
		supp1 = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, [-d[1], d[0]])
		supp2 = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, [d[1], -d[0]])

		dist1 = v2_lenSq(v2_proj_rest(supp1[1], d))
		dist2 = v2_lenSq(v2_proj_rest(supp2[1], d))

		if (dist1 != 0.0 and dist2 != 0.0):
			supp = supp1
			if (math.fabs(dist2) > math.fabs(dist1)):
				supp = supp2

			Vk.append(supp[1])
			Pi.append(supp[2])
			IndI.append(supp[3])
		

	Dk = [-1.0] * len(Vk)
	Ck = [None] * len(Vk)
	Li = [None] * len(Vk)

	max_iter = 3 + (len(cvx1) + len(cvx2))*5
	iter = 0

	while (True):
		iter = iter+1

		min_i = -1

		lV = len(Vk)
		for i in range(lV):
			if (Dk[i] < 0.0):
				v1 = Vk[i]
				v2 = Vk[(i+1)%lV]
				cl = gjk_epa_closest_on_edge(ctx, v1, v2)

				if (cl[1][0] > 0.0 and cl[1][0] < 1.0):	# get a better estimate (better numerics) of the vertex in the case it is not an extremity.
					cl[0] = v2_points_proj(v1, v2, v2_z())

				Ck[i] = cl[0]
				Li[i] = cl[1]
				Dk[i] = v2_lenSq(cl[0])

			if ((min_i < 0) or (Dk[i] < Dk[min_i])):
				min_i = i

		n = v2_normalize(Ck[min_i])
		# The epsilon is needed because of the numerical accuracy of gjk_epa_closest_on_edge (REASON1) when used without 
		if (v2_lenSq(Ck[min_i]) <= 0.00000001):	# We can't use the closest point as a direction, use segment normal. 
		#if (v2_lenSq(Ck[min_i]) == 0.0):	
			v = Vk[min_i]
			vp = Vk[(min_i-1+lV)%lV]
			vn = Vk[(min_i+1)%lV]
			n = v2_normalize(v2_orth(v2_sub(vn, v)))
			t = v2_sub(v, vp)
			if (v2_dot(n, t) < 0.0):
				n = v2_neg(n)

		supp = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, n)
		if ((v2_dot(n, supp[1]) - v2_dot(n, Ck[min_i]) < epa_eps) or (iter > max_iter)):

			v1 = [0.0, 0.0]
			v2 = [0.0, 0.0]
			for i in range(2):
				j = (min_i+i)%lV
				v1 = v2_add(v1, v2_muls(Pi[j][0], Li[min_i][i]))
				v2 = v2_add(v2, v2_muls(Pi[j][1], Li[min_i][i]))
			
			indI = [IndI[min_i], IndI[(min_i+1)%lV]]
			features = 	gjk_find_features(m1, cvx1, r1, m2, cvx2, r2, Li[min_i], indI)	

			return [-v2_len(Ck[min_i]), 0.0, v1, v2, features]

		ii = (min_i+1) % lV
		Vk.insert(ii, supp[1])
		Pi.insert(ii, supp[2])
		IndI.insert(ii, supp[3])
		Dk[min_i] = -1.0
		Dk.insert(ii, -1.0)
		Ck.insert(ii, None)
		Li.insert(ii, None)


#----------------------------------------------------------------------------
#------------ GJK Output ----------------------------------------------------
#----------------------------------------------------------------------------

def cfeature_vertices(m, cvx, r, fi, gjkOut):
	
	feature = gjkOut[4][fi]
	fj = (fi+1)%2
	lf = len(feature)

	if (lf not in [1,2]):
		return None

	p1 = convexVertex(m, cvx, 0.0, v2_z(), feature[0])
	vertices = None
	n = None

	# TODO: calculate normal
	if (r <= 0.0):
		if (lf == 1):
			vertices = [p1]
		elif (lf == 2):
			p2 = convexVertex(m, cvx, 0.0, v2_z(), feature[1])
			vertices = [p1, p2]
	else:
		n = v2_sub(gjkOut[2+fj], gjkOut[2+fi])
		if (gjkOut[0] < 0.0):
			n = v2_neg(n)
		if (lf == 1):
			if (v2_lenSq(n) == 0.0):
				n = v2_sub(gjkOut[2+fi], p1)
			if (v2_lenSq(n) == 0.0):
				n = None	
			vertices = [v2_add(p1, v2_muls(v2_normalize(n), r))]
		elif (lf == 2):
			p2 = convexVertex(m, cvx, 0.0, v2_z(), feature[1])
			if (v2_lenSq(n) == 0.0):
				n = v2_points_proj_rest(p1, p2, gjkOut[2+fi])
			if (v2_lenSq(n) == 0.0):
				n = None	
			vertices = [v2_add(p1, v2_muls(v2_normalize(n), r)), v2_add(p2, v2_muls(v2_normalize(n), r))]

	return [vertices, n]		


def cfeature_mfold_2d(f1, f2):

	fl1 = len(f1)
	fl2 = len(f2)

	seg1 = v2_sub(f1[1%fl1], f1[0])
	seg2 = v2_sub(f2[1%fl2], f2[0])
	
	def safe_projs(v, a):
		if v2_lenSq(a) > 0.0:
			return v2_projs(v, a)
		return 0.0	

	proj1 = sorted([ safe_projs(v2_sub(f2[0], f1[0]), seg1), safe_projs(v2_sub(f2[1%fl2], f1[0]), seg1) ])
	proj2 = sorted([ safe_projs(v2_sub(f1[0], f2[0]), seg2), safe_projs(v2_sub(f1[1%fl1], f2[0]), seg2) ])

	def interval_intersect(int):
		if int[0] <= 0.0:
			if int[1] <= 0.0:
				return [0.0, 0.0]
			else:
				return [0.0, m_min(1.0, int[1])]
		elif int[0] <= 1.0:	
			return [int[0], m_min(1.0, int[1])]
		return [1.0, 1.0]

	proj1 = interval_intersect(proj1)
	proj2 = interval_intersect(proj2)
	
	if (proj1[0] == proj1[1]):
		mf1 = [ v2_add(f1[0], v2_muls(seg1, proj1[0])) ]
		mf2 = [ v2_add(f2[0], v2_muls(seg2, proj2[0])) ]
	else:	
		mf1 = [ v2_add(f1[0], v2_muls(seg1, proj1[0])), v2_add(f1[0], v2_muls(seg1, proj1[1])) ]
		mf2 = [ v2_add(f2[0], v2_muls(seg2, proj2[0])), v2_add(f2[0], v2_muls(seg2, proj2[1])) ]

	return [mf1, mf2]


#----------------------------------------------------------------------------
#------------ GJK Output: Pertrubed Manifold --------------------------------
#----------------------------------------------------------------------------

def pmfold_2d(m1, cvx1, r1, m2, cvx2, r2, gjkOut, epa_eps=gEPA_eps, gjk_eps=gGJK_eps):
	pert_count = 2
	sc = 1.0
	if (gjkOut[0] < 0.0):
		sc = 50.0
	min_pert = sc* math.pi*0.0001/180.0
	max_pert = sc* math.pi*0.0001/180.0
	#todo: base on shape characteristics?

	points1 = [None] * (pert_count+1)
	points2 = [None] * (pert_count+1)

	points1[0] = gjkOut[2]
	points2[0] = gjkOut[3]

	for pi in range(pert_count):
		pert_a = random.uniform(min_pert, max_pert)
		if (pi%2==0):
			pert_a = -pert_a
		pert_m = m2_tr(v2_z(), pert_a)
		pert_m1 = m2_mul(m1, pert_m)
		#pert_im1 = m2_inv(pert_m1)
		ed = gjk_epa_distance(pert_m1, cvx1, r1, m2, cvx2, r2, epa_eps, gjk_eps)
		#points1[pi+1] = m2_mulp(m1, m2_mulp(pert_im1, ed[2]))
		points1[1+pi] = ed[2]
		points2[1+pi] = ed[3]

	np = len(points1)	
	sd = v2_z() 
	i = 1
	while (v2_lenSq(sd)==0.0 and i < np):
		sd = v2_sub(points2[i], points2[0])
		i = i+1
	
	ind = [i for i in range(np)]
	def sort_dir(ix, iy):
	 	diff = v2_dot(sd, points2[ix])-v2_dot(sd, points2[iy])
	 	if diff < 0.0:
	 		return -1
	 	if diff > 0.0:
	 		return 1
	 	return 0	
	ind = sorted(ind, cmp=sort_dir)

	mfold1 = [points1[ind[0]], points1[ind[-1]]]
	mfold2 = [points2[ind[0]], points2[ind[-1]]]

	ext1 = points1[ind[0]]
	ext2 = points1[ind[-1]]
	if (v2_dist(ext1, ext2) <= 0.01):
		mfold1.pop()
		mfold2.pop()

	return [mfold1, mfold2]	
	#return [points1, points2]


#----------------------------------------------------------------------------
#------------ Test ----------------------------------------------------------
#----------------------------------------------------------------------------

def TestGJK():

	p1 = [0.0, 0.0]
	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	p2 = [0.0, 0.0]
	cvx2 = [[0.0,1.5]]
	
	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,1.5]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,1.5]]
	cvx2 = [linComb(cvx1, [1.0/256.0, 1.0/256.0, 1.0-(2.0/256.0)])]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
	cvx2 = [[0.0,1.5]]
	off = linComb(cvx1, [1.0/256.0, 1.0/256.0, 1.0-(2.0/256.0)])
	cvx1.v = [ v2_sub(cvx1[0], off), v2_sub(cvx1[1], off), v2_sub(cvx1[2], off) ]
	cvx2.v = [[0.0,0.0]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist
	
	
	# Test convex combinations inside cvx1
	if (False):
		cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
		cvx2 = [[0.0,1.5]]
		for x in range(512):
			for y in range(512):
				l = [x/512.0, y/512.0, 0.0]
				l[2] = 1.0 - l[0] - l[1]
				if (l[2] >= 0.0):
					cvx2 = [linComb(cvx1, l)]
					dd = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)
					if (dd[0] > dd[1]):
						print 'Fail'

		# Test combinations outside cvx1
		cvx1 = [[0.0,1.0], [0.0,2.0], [-1.0,1.0]]
		cvx2 = [[0.0,1.5]]
		for x in range(512):
			l = [x/512.0, 1.1+x/512.0, 0.0]
			cvx2 = [linComb(cvx1, l)]
			dd = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)
			if (dd[0] == 0.0):
				print 'Fail'		

	return 0


def TestGJK1():

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.0]

	#cvx1 = [[0.0,0.0], [5.0,0.0], [5.0,5.0], [0.0,5.0]]
	#cvx2 = [[-1.0,-1.0], [1.0,-1.0], [1.0,1.0], [-1.0,1.0]]
	#dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	#print dist

	cvx1 = [[0.0-1.0,-5.0], [5.0-1.0,-5.0], [5.0-1.0,5.0], [0.0-1.0,5.0]]
	cvx2 = [[0.0,0.0], [-3.0,3.0], [-3.0,-3.0]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	return 0


def TestGJK2():

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.0]
	cvx1 = [[1.0,0.0], [3.0,3.0], [0.0,-5.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.0]
	cvx1 = [[-1.0,0.0], [3.0,3.0], [0.0,-5.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_distance(p1, cvx1, 0.0, p2, cvx2, 0.0)[0]
	print dist

	return 0	


def TestEPA():

	p1 = [0.0, 0.0]
	p2 = [0.1, 0.0]
	cvx1 = [[-2.0,-2.0], [2.0,-2.0], [2.0,2.0], [-2.0, 2.0] ]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)
	print dist[0]
	print dist[2]
	print dist[3]

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.0]
	cvx1 = [[-2.0,-2.0], [2.0,-2.0], [2.0,2.0], [-2.0, 2.0] ]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.0]
	cvx1 = [[-2.0,-2.0], [2.0,-2.0], [2.0,2.0], [-2.0, 2.0] ]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.1]
	cvx1 = [[-2.0,-2.0], [2.0,-2.0], [2.0,2.0], [-2.0, 2.0] ]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 =[21.25,22.15]
	cvx1 = [[20.0,20.0], [24.0,20.0], [24.0,24.0], [20.0,24.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 =[22.0,22.0]
	cvx1 = [[20.0,20.0], [24.0,20.0], [24.0,24.0], [20.0,24.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 =[22.0,21.9]
	cvx1 = [[20.0,20.0], [24.0,20.0], [24.0,24.0], [20.0,24.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.0]
	cvx1 = [[1.0,0.0], [3.0,3.0], [0.0,-5.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	p1 = [0.0, 0.0]
	p2 = [0.0, 0.0]
	cvx1 = [[-1.0,0.0], [3.0,3.0], [0.0,-5.0]]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(p1, cvx1, 0.0, p2, cvx2, 0.0, 0.001)[0]
	print dist

	return 0	


def TestGJKr():

	m1 = m2_tr([0.0, 0.0], 0.0)
	m2 = m2_tr([0.0, 0.0], 0.0)
	cvx1 = [[-1.0,-1.0], [1.0,-1.0], [1.0,1.0], [-1.0, 1.0] ]
	cvx2 = [[0.0,-1.0]]
	dist = gjk_epa_distance(m1, cvx1, 0.0, m2, cvx2, 0.0, 0.001)
	print dist[0]
	print dist[2]
	print dist[3]

	m1 = m2_tr([0.0, 0.0], 0.0)
	m2 = m2_tr([0.0, 0.0], 0.0)
	cvx1 = [[-1.0,-1.0], [1.0,-1.0], [1.0,1.0], [-1.0, 1.0] ]
	cvx2 = [[0.0,1.0]]
	dist = gjk_epa_distance(m1, cvx1, 0.0, m2, cvx2, 0.0, 0.001)
	print dist[0]
	print dist[2]
	print dist[3]

	m1 = m2_tr([0.0, 0.0], 0.0)
	m2 = m2_tr([2.0, 0.0], 0.0)
	cvx1 = [[-1.0,-1.0], [1.0,-1.0], [1.0,1.0], [-1.0, 1.0] ]
	cvx2 = [[0.0,0.0]]
	dist = gjk_epa_distance(m1, cvx1, 0.5, m2, cvx2, 0.0, 0.001)
	print dist[0]
	print dist[2]
	print dist[3]


def TestGJKFeature():

	# REASON1
	cvx0v = [[0.0, 0.0], [2.0, 0.0], [2.0, 2.0], [0.0, 2.0]]
	cvx0r = 1.5
	cvx0m = [[1.0, -0.0, 15.250000000000004], [0.0, 1.0, 0.0], [0.0, 0.0, 1.0]]	
	gjkOut = gjk_epa_distance(cvx0m, cvx0v, cvx0r, m2_id(), [[16.6, 0.0]], 0.0, 0.0001)
	features = gjkOut[4]
	vs = cfeature_vertices(cvx0m, cvx0v, cvx0r, 0, gjkOut)

#TestGJK()
#TestGJK1()
#TestGJK2()
#TestEPA()
#TestGJKr()
#TestGJKFeature()
#endif
	*/