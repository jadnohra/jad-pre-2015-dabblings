#ifndef LAB_MATH_OPS_H
#define LAB_

#include <math.h>

typedef float Sc;
typedef Sc Scp;

struct V2 
{ 
	Sc x[2]; 
	V2(Sc x1 = 0, Sc x2 = 0) 
		{ x[0]=x1; x[1]=x2; } 

	Sc& operator()(int i) { return x[i]; }
	const Sc& operator()(int i) const { return x[i]; }
};
typedef const V2& V2p;

struct V3 
{
	Sc x[3]; 
	V3(Sc x1 = 0, Sc x2 = 0, Sc x3 = 0) 
		{ x[0]=x1; x[1]=x2; x[2]=x3; }

	Sc& operator()(int i) { return x[i]; }
	const Sc& operator()(int i) const { return x[i]; }
};
typedef const V3& V3p;

struct M3 
{ 
	V3 r[3]; 

	M3() 
		{} 
	M3(V3p r1, V3p r2, V3p r3) 
		{ r[0]=r1; r[1]=r2; r[2]=r3; }

	Sc& operator()(int i, int j) { return r[i].x[j]; }
	const Sc& operator()(int i, int j) const { return r[i].x[j]; }
};
typedef const M3& M3p;

Sc rl_pi() { return Sc(3.141592653589793238462643); }
Sc m_abs(Scp v) 
	{ return v >= Sc(0) ? v : -v; }
bool m_isz(Scp v) 
	{ return v == Sc(0); }
bool m_isnz(Scp v) 
	{ return v != Sc(0); }
template<typename T> T m_tmin(T v1, T v2) 
	{ return v1 <= v2 ? v1 : v2; }
template<typename T> T m_tmax(T v1, T v2) 
	{ return v1 >= v2 ? v1 : v2; }
Sc m_min(Scp v1, Scp v2) 
	{ return v1 <= v2 ? v1 : v2; }
Sc m_max(Scp v1, Scp v2) 
	{ return v1 >= v2 ? v1 : v2; }
Sc m_clamp(Scp v, Scp v1, Scp v2) 
	{ return v <= v1 ? v1 : (v >= v2 ? v2 : v); }
Sc m_sqrt(Scp v) 
	{ return sqrt(v); }
Sc m_atan(Scp y, Scp x) 
	{ return atan2(y, x); }
Sc m_cos(Scp x) 
	{ return cos(x); }
Sc m_sin(Scp x) 
	{ return sin(x); }
Sc m_rad(Scp d)
	{ return d*rl_pi()/Sc(180); }

V2 v2_z() 
	{ return V2(0, 0); }
V2 add(V2p v1, V2p v2) 
	{ return V2(v1.x[0]+v2.x[0], v1.x[1]+v2.x[1]); }
V2 sub(V2p v1, V2p v2) 
	{ return V2(v1.x[0]-v2.x[0], v1.x[1]-v2.x[1]); }
V2 muls(V2p v1, Scp s) 
	{ return V2(v1.x[0]*s, v1.x[1]*s); }
V2 mul(V2p v1, Scp s) 
	{ return muls(v1, s); }
V2 neg(V2p v1) 
	{ return V2(-v1.x[0], -v1.x[1]); }
Sc dot(V2p v1, V2p v2) 
	{ return v1.x[0]*v2.x[0] + v1.x[1]*v2.x[1]; }
Sc lenSq(V2p v1) 
	{ return dot(v1, v1); }
Sc len(V2p v1) 
	{ return m_sqrt(lenSq(v1)); }
Sc distSq(V2p v1, V2p v2) 
	{ return lenSq(sub(v1, v2)); }
Sc dist(V2p v1, V2p v2) 
	{ return m_sqrt(distSq(v1, v2)); }
V2 normalize(V2p v1)
	{ Sc l = len(v1); return l==0? v1 : mul(v1, Sc(1)/l); }
Sc angle(V2p v1, V2p v2)
	{ V2 c = sub(v2, v1); return m_atan(c.x[1], c.x[0]); }
V2 rot(V2p v1, Scp a)
	{ Sc c = m_cos(a); Sc s = m_sin(a); return V2(v1.x[0]*c + v1.x[1]*(-s), v1.x[0]*s + v1.x[1]*(c)); }
V2 rot90(V2p v1)
	{ return V2(-v1.x[1], v1.x[0]); }
V2 rotm90(V2p v1)
	{ return neg(rot90(v1)); }
V2 orth(V2p v1)
	{ return rot90(v1); }
Sc projs(V2p v1, V2p a)
	{ return dot(v1, a) / dot(a, a); }
V2 proj(V2p v1, V2p a)
	{ return mul(a, projs(v1, a)); }
V2 proj_rest(V2p v1, V2p a)
	{ return sub(v1, proj(v1, a)); }
V2 proj_points(V2p a, V2p b, V2p c)
	{ return add(a, proj(sub(c, a), sub(b, a))); }
V2 proj_points_rest(V2p a, V2p b, V2p c)
	{ return proj_rest(sub(c, a), sub(b, a)); }
V2 lin_comb(const V2* v, Sc* l, int c)
	{ V2 lc=v2_z(); for(int i=0;i<c;++i) lc=add(lc, mul(v[i], l[i])); return lc; }

V3 v_z() 
	{ return V3(0, 0, 0); }
V3 u_i()
	{ return V3(1, 0, 0); }
V3 u_j()
	{ return V3(0, 1, 0); }
V3 u_k()
	{ return V3(0, 0, 1); }
V3 u_ij()
	{ return V3(1, 1, 0); }
V3 u_ik()
	{ return V3(1, 0, 1); }
V3 u_jk()
	{ return V3(0, 1, 1); }
V3 u_ijk()
	{ return V3(1, 1, 1); }

V3 add(V3p v1, V3p v2) 
	{ return V3(v1.x[0]+v2.x[0], v1.x[1]+v2.x[1], v1.x[2]+v2.x[2]); }
V3 muls(V3p v1, Scp s) 
	{ return V3(v1.x[0]*s, v1.x[1]*s, v1.x[2]*s); }
V2p asV2(V3p v)
	{ return *((V2*)((void*) &v)); }
V2p asV2(Sc* x)
	{ return *((V2*)((void*) x)); }

M3 m3_z()
	{ static M3 mz = M3(v_z(), v_z(), v_z()); return mz; }
M3 m3_id()
	{ static M3 mid = M3(u_i(), u_j(), u_k()); return mid; }
M3 mul(M3p m1, M3p m2)
	{ M3 m; for(int i=0;i<3;++i)for(int j=0;j<3;++j) m(i,j)=m1(i,0)*m2(0,j)+m1(i,1)*m2(1,j)+m1(i,2)*m2(2,j); return m; }
V3 mul(M3p m, V3p v)
	{ V3 o; for(int i=0;i<3;++i) o(i)=m(i,0)*v(0)+m(i,1)*v(1)+m(i,2)*v(2); return o; }
V2 mulp(M3p m, V2p v)
	{ V2 o; for(int i=0;i<2;++i) o(i)=m(i,0)*v(0)+m(i,1)*v(1)+m(i,2); return o; }
V2 mulv(M3p m, V2p v)
	{ V2 o; for(int i=0;i<2;++i) o(i)=m(i,0)*v(0)+m(i,1)*v(1); return o; }
M3 rigid_inv(M3p m)
	{	M3 mi; 
		mi(0,0)=m(1,1); mi(0,1)=m(1, 0); mi(0,2)=m(0,2)*m(1,1)+m(1,2)*m(1,0);
		mi(1,0)=m(0,1);	mi(1,1)=m(0,0); mi(1,2)=m(0,2)*m(0,1)+m(1,2)*m(0,0);
		mi.r[2] = u_k();
		return mi;
	}
V2 rigid_inv_mulp(M3p m, V2p v)
	{ return V2(m(1,1)*v(0)+m(1,0)*v(1)-m(0,2), m(0,1)*v(0)+m(0,0)*v(1)-m(1,2)); }
void rigid_make_orth(M3& m)
	{ V2 v=orth(V2(m(0,0), m(0,1))); m(1,0)=v(0); m(1,1)=v(1); }
V2 rigid_get_transl(M3p m)
	{ return V2(m(0,2), m(1,2)); }
void rigid_set_transl(M3& m, V2p v)
	{ m(0,2)=v(0); m(1,2)=v(1); }
M3 rigid(V2p transl, Sc a)
	{
		Sc c = m_cos(a); Sc s = m_sin(a);
		M3 m; m(0,2) = transl(0);m(1,2) = transl(1);
		m(0,0) = c; m(0,1) = -s; m(1,0) = s; m(1,1) = c;
		return m;
	}



float h2f(unsigned int hex) { float f; *((unsigned int*) ((void*) &f)) = hex; return f; }
unsigned int f2h(float f) { unsigned int hex = *((unsigned int*) ((void*) &f)); return hex; }

#endif 