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

		V2 v;
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
		V2 d;
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

		out.d = n;
		out.p.p1 = cvx_vertex(m1, v1, lv1, r1, n, scr.cvx1.ami[0]);
		out.p.p2 = cvx_vertex(m2, v2, lv2, r2, neg(n), scr.cvx2.ami[0]);
		out.h = max1+max2; out.s = sub(out.p.p1, out.p.p2); out.cvx1 = &scr.cvx1; out.cvx2 = &scr.cvx2;
		return true;
	}

	struct Out_gjk_subdist
	{
		V2 v;
		const int* Isp;
		Rl* Li;
		int lLi;
	};

	template<typename Vert>
	bool gjk_subdist_fallback(Out_gjk_subdist& out, GjkScratch& scr, const Vert& Vk, int lVk)
	{
		const Perm& perm = scr.perm[lVk-1];
		Rl* Di = scr.Di; scr.lDi = perm.Di_count;
		for (int i=0;i<perm.Di_count; ++i) Di[i]=Rl(1.0);

		int best = -1;
		Rl bestDistSq;
		
		int i_Is = 0; int i_Isp = 0; int i_Union_index = 0;
		for (int pi=0; pi<perm.count; ++pi)
		{
			int d=0; { int i=i_Is; while(perm.Is[i++] != EL) d++; } 
			int di_index = perm.Di_index[pi];

			Rl D = Rl(0.0);
			for (int i=0; i<d; ++i) D = D + Di[di_index+i];

			const int* Is = perm.Is + i_Is;
			const int* Isp = perm.Isp + i_Isp;
			const int* Union_index = perm.Union_index + i_Union_index;

			for (int j=0; Isp[j] != EL; ++j)
			{
				Rl Dj = Rl(0.0); for (int i=0; i<d; ++i) Dj = Dj + (Di[di_index+i] * ( dot(Vk[Is[i]].v, sub(Vk[Is[0]].v, Vk[Isp[j]].v)) ) );
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
					for (int j=0; Isp[j] != EL; ++j) Li[Isp[j]] = Rl(0);
					for (int i=0; i<d; ++i)
					{
						Rl l = Di[di_index+i] * iD;
						v = add(v, muls(Vk[Is[i]].v, l));
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

	template<typename Vert>
	bool gjk_subdist(Out_gjk_subdist& out, GjkScratch& scr, const Vert& Vk, int lVk)
	{
		const Perm& perm = scr.perm[lVk-1];
		Rl* Di = scr.Di; scr.lDi = perm.Di_count;
		for (int i=0;i<perm.Di_count; ++i) Di[i]=Rl(1.0);

		int i_Is = 0; int i_Isp = 0; int i_Union_index = 0;
		for (int pi=0; pi<perm.count; ++pi)
		{
			int d=0; { int i=i_Is; while(perm.Is[i++] != EL) d++; } 
			int di_index = perm.Di_index[pi];

			Rl D = Rl(0.0);
			for (int i=0; i<d; ++i) D = D + Di[di_index+i];

			const int* Is = perm.Is + i_Is;
			const int* Isp = perm.Isp + i_Isp;
			const int* Union_index = perm.Union_index + i_Union_index;

			bool cond3 = true;
			for (int j=0; Isp[j] != EL; ++j)
			{
				Rl Dj = Rl(0.0); for (int i=0; i<d; ++i) Dj = Dj + (Di[di_index+i] * ( dot(Vk[Is[i]].v, sub(Vk[Is[0]].v, Vk[Isp[j]].v)) ) );
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
					for (int j=0; Isp[j] != EL; ++j) Li[Isp[j]] = Rl(0);
					for (int i=0; i<d; ++i)
					{
						Rl l = Di[di_index+i] * iD;
						v = add(v, muls(Vk[Is[i]].v, l));
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

	void gjk_copy_vert(const Out_gjk_support_mink_cvx& supp, GjkVert& simpl)
	{
		simpl.v = supp.s;
		simpl.Pi = supp.p;
		for (int i=0;i<supp.cvx1->lami; ++i) simpl.IndI.ami1[i] = supp.cvx1->ami[i]; simpl.IndI.lami1 = supp.cvx1->lami;
		for (int i=0;i<supp.cvx2->lami; ++i) simpl.IndI.ami2[i] = supp.cvx2->ami[i]; simpl.IndI.lami2 = supp.cvx2->lami;
		simpl.DirI = supp.d;
	}

	Out_gjk_distance gjk_distance(GjkScratch& scr, M3p m1, V2pc v1, int lv1, Rl r1, M3p m2, V2pc v2, int lv2, Rl r2, Rl eps)
	{
		V2 d = V2(Rl(-1.0), Rl(0.0));
		int& lsimpl = scr.lsimpl; GjkVert* simpl = scr.simpl; 
		{
			Out_gjk_support_mink_cvx supp; if (!gjk_support_mink_cvx(supp, scr, m1, v1, lv1, r1, m2, v2, lv2, r2, d)) return Out_gjk_distance(false, 0.0f, 0.0f, v2_z(), v2_z());
			lsimpl = 1;
			gjk_copy_vert(supp, simpl[0]);
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

			if (1)
			{
				Rl lisum = Rl(0); for (int i=0; i<subd.lLi; ++i) { lisum += subd.Li[i]; }
				if (lisum < 0.95f || lisum > 1.05f)
					return Out_gjk_distance(false, 0.0f, 0.0f, v2_z(), v2_z());
			}

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
				gjk_copy_vert(supp, simpl[nsi]);
				lsimpl = m_tmin(3, lsimpl+1);
			}
		}
	}

	bool epa_closest_on_edge(Out_gjk_subdist& out, GjkScratch& scr, const GjkVert& v1, const GjkVert& v2 )
	{
		struct GetVert
		{
			const GjkVert** Vk;
			GetVert(const GjkVert** _Vk) : Vk(_Vk) {}

			const GjkVert& operator[](int i) const { return *Vk[i]; }
		};

		const GjkVert* Vk[] = { &v1, &v2 };
		return gjk_subdist(out, scr, GetVert(Vk), 2);
	}

	struct EpaScratch
	{
		GjkScratch gjk;
		int maxdim;
		int maxV;
		GjkVert* simpl;
		int lsimpl;

		struct LiPair
		{
			Rl i1;
			Rl i2;

			Rl& operator[](int i) { return i==0?i1:i2; }
		};

		Rl* Dk;
		V2* Ck;
		LiPair* Li;

		EpaScratch(int maxVertices, bool _use_fallbacks = true) 
		: gjk(_use_fallbacks)
		{
			maxV = maxVertices;
			Dk = (Rl*) malloc(maxVertices*sizeof(Rl));
			Ck = (V2*) malloc(maxVertices*sizeof(V2));
			Li = (LiPair*) malloc(maxVertices*sizeof(LiPair));

			maxdim = gjk.perm[2].dim;
			simpl = (GjkVert*) malloc(maxVertices*sizeof(GjkVert));
			for (int i=0;i<maxVertices; ++i) 
			{
				simpl[i].IndI.ami1 = (int*) malloc(maxdim*sizeof(int));
				simpl[i].IndI.ami2 = (int*) malloc(maxdim*sizeof(int));
			}
		}

		~EpaScratch()
		{
			free(Dk);
			free(Ck);
			free(Li);

			for (int i=0;i<maxdim; ++i) 
			{
				free(simpl[i].IndI.ami1); 
				free(simpl[i].IndI.ami2); 
			}
			free(simpl);
		}
	};

	struct Out_epa_distance
	{
		Out_gjk_distance gjk;
		Rl dist;
		V2 v1;
		V2 v2;
		bool success;
	};

	bool epa_copy_vert(const Out_gjk_support_mink_cvx& supp, GjkVert& simpl, int maxdim)
	{
		if (supp.cvx1->lami > maxdim || supp.cvx2->lami > maxdim) return false;

		simpl.v = supp.s;
		simpl.Pi = supp.p;
		for (int i=0;i<supp.cvx1->lami; ++i) simpl.IndI.ami1[i] = supp.cvx1->ami[i]; simpl.IndI.lami1 = supp.cvx1->lami;
		for (int i=0;i<supp.cvx2->lami; ++i) simpl.IndI.ami2[i] = supp.cvx2->ami[i]; simpl.IndI.lami2 = supp.cvx2->lami;
		simpl.DirI = supp.d;

		return true;
	}

	bool epa_copy_vert(const GjkVert& src, GjkVert& simpl, int maxdim)
	{
		if (src.IndI.lami1 > maxdim || src.IndI.lami2 > maxdim) return false;

		simpl.v = src.v;
		simpl.Pi = src.Pi;
		for (int i=0;i<src.IndI.lami1; ++i) simpl.IndI.ami1[i] = simpl.IndI.ami1[i]; simpl.IndI.lami1 = src.IndI.lami1;
		for (int i=0;i<src.IndI.lami2; ++i) simpl.IndI.ami2[i] = simpl.IndI.ami2[i]; simpl.IndI.lami2 = src.IndI.lami2;
		simpl.DirI = src.DirI;

		return true;
	}

	bool epa_insert(EpaScratch& scr, int lV, int min_i, const Out_gjk_support_mink_cvx& supp)
	{
		if (scr.lsimpl == scr.maxV) return false; 

		int ii = (min_i+1) % lV;
		for (int i=ii+1, j=lV-1; i<=lV; ++i, j--)
		{
			epa_copy_vert(scr.simpl[j], scr.simpl[j+1], scr.maxdim);
			scr.Dk[j+1] = scr.Dk[j];
			scr.Ck[j+1] = scr.Ck[j];
			scr.Li[j+1] = scr.Li[j];
		}

		epa_copy_vert(supp, scr.simpl[ii], scr.maxdim);
		scr.Dk[ii] = Rl(-1.0);
		scr.Dk[min_i] = Rl(-1.0);
		scr.lsimpl++;

		return true;
	}

	bool epa_distance(Out_epa_distance& out, EpaScratch& scr, M3p m1, V2pc v1, int lv1, Rl r1, M3p m2, V2pc v2, int lv2, Rl r2, Rl epsEpa, Rl epsGjk)
	{
		out.success = false;
		out.gjk = gjk_distance(scr.gjk, m1, v1, lv1, r1, m2, v2, lv2, r2, epsGjk);
		if (!out.gjk.success) 
			return false;	
		if (out.gjk.dist > out.gjk.eps) 
		{
			out.success = true;
			out.dist = out.gjk.dist;
			out.v1 = out.gjk.v1;
			out.v2 = out.gjk.v2;
			return true; 
		}

		GjkVert* simpl = scr.simpl;
		int& lsimpl = scr.lsimpl;

		lsimpl = 0;
		for (int i=0; i<scr.gjk.lsimpl; ++i) 
		{
			if (!epa_copy_vert(scr.gjk.simpl[i], simpl[i], scr.maxdim)) return false;
			scr.Dk[i] = Rl(-1.0);
			lsimpl++;
		}

		// Treat degenerate case
		if (lsimpl == 1)
		{
			V2 d(Rl(1.0), Rl(0.0));
			Out_gjk_support_mink_cvx supp; if (!gjk_support_mink_cvx(supp, scr.gjk, m1, v1, lv1, r1, m2, v2, lv2, r2, d)) return false;
			
			if (!epa_copy_vert(supp, simpl[lsimpl], scr.maxdim)) return false; scr.Dk[lsimpl] = Rl(-1.0); lsimpl++;
		}

		// Treat degenerate case
		if (lsimpl == 2)
		{
			V2 d = sub(simpl[1].v, simpl[0].v);
			Out_gjk_support_mink_cvx supp1; if (!gjk_support_mink_cvx(supp1, scr.gjk, m1, v1, lv1, r1, m2, v2, lv2, r2, rot90(d))) return false;
			Out_gjk_support_mink_cvx supp2; if (!gjk_support_mink_cvx(supp2, scr.gjk, m1, v1, lv1, r1, m2, v2, lv2, r2, rotm90(d))) return false;

			Rl dist1 = lenSq( proj_rest(supp1.s, d));
			Rl dist2 = lenSq( proj_rest(supp2.s, d));

			if (m_isnz(dist1) && m_isnz(dist2))
			{
				Out_gjk_support_mink_cvx* supp = m_abs(dist2) > m_abs(dist1) ? &supp2 : &supp1;
				if (!epa_copy_vert(*supp, simpl[lsimpl], scr.maxdim)) return false; scr.Dk[lsimpl] = Rl(-1.0); lsimpl++;
			}
		}

		const int max_iter = 3 + (lv1+lv2)*5;
		int iter = 0;
		
		while (true)
		{
			iter++;
			const int& lV = lsimpl;
			int min_i = -1;

			for (int i=0; i<lV; ++i)
			{
				if (scr.Dk[i] < Rl(0))
				{
					GjkVert& v1 = simpl[i];
					GjkVert& v2 = simpl[(i+1)%lV];
					Out_gjk_subdist cl; if (!epa_closest_on_edge(cl, scr.gjk, v1, v2)) return false;

					if (cl.Li[0] > Rl(0) && cl.Li[0] < Rl(1)) // get a better estimate (better numerics) of the vertex when is not an extremity.
						cl.v = proj_points(v1.v, v2.v, v2_z() ); 
					
					scr.Ck[i] = cl.v;
					scr.Li[i].i1 = cl.Li[0];
					scr.Li[i].i2 = cl.Li[1];
					scr.Dk[i] = lenSq(cl.v);
				}

				if ((min_i < 0) || (scr.Dk[i] < scr.Dk[min_i]))
					min_i = i;
			}

			V2 n = normalize(scr.Ck[min_i]);
			// The epsilon is needed because of the numerical accuracy of gjk_epa_closest_on_edge (REASON1) when used without 
			if (lenSq(scr.Ck[min_i]) <= Rl(1.e-5f)) // We can't use the closest point as a direction, use segment normal. 
			{
				V2 v = simpl[min_i].v;
				V2 vp = simpl[(min_i-1+lV)%lV].v;
				V2 vn = simpl[(min_i+1)%lV].v;
				n = normalize(orth(sub(vn, v)));
				V2 t = sub(v, vp);
				if (dot(n, t) < Rl(0))
					n = neg(n);
			}

			Out_gjk_support_mink_cvx supp; if (!gjk_support_mink_cvx(supp, scr.gjk, m1, v1, lv1, r1, m2, v2, lv2, r2, n)) return false;
			if ((dot(n, supp.s) - dot(n, scr.Ck[min_i]) < epsEpa) || (iter >= max_iter) || (lV == scr.maxV) )
			{
				V2 v1 = v2_z();
				V2 v2 = v2_z();

				for (int i=0; i<2; ++i)
				{
					int j = (min_i+i)%lV;
					v1 = add(v1, muls(simpl[j].Pi.p1, scr.Li[min_i][i]));
					v2 = add(v2, muls(simpl[j].Pi.p2, scr.Li[min_i][i]));
				}

				out.success = true;
				out.v1 = v1;
				out.v2 = v2;
				out.dist = -len(scr.Ck[min_i]);

				return true;
			}
		
			if (!epa_insert(scr, lV, min_i, supp)) return false;
		}

		return true;
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

namespace SAT
{

}