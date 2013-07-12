#ifndef LAMBY_GUASSY_H
#define LAMBY_GUASSY_H

#include <math.h>

typedef float Rl;
typedef Rl Rlp;

struct V2 
{ 
	Rl x[2]; 
	V2(Rl x1 = 0, Rl x2 = 0) 
		{ x[0]=x1; x[1]=x2; } 

	Rl& operator()(int i) { return x[i]; }
	const Rl& operator()(int i) const { return x[i]; }
};
typedef const V2& V2p;

struct V3 
{
	Rl x[3]; 
	V3(Rl x1 = 0, Rl x2 = 0, Rl x3 = 0) 
		{ x[0]=x1; x[1]=x2; x[2]=x3; }

	Rl& operator()(int i) { return x[i]; }
	const Rl& operator()(int i) const { return x[i]; }
};
typedef const V3& V3p;

struct M3 
{ 
	V3 r[3]; 

	M3() 
		{} 
	M3(V3p r1, V3p r2, V3p r3) 
		{ r[0]=r1; r[1]=r2; r[2]=r3; }

	Rl& operator()(int i, int j) { return r[i].x[j]; }
	const Rl& operator()(int i, int j) const { return r[i].x[j]; }
};
typedef const M3& M3p;

Rl rl_pi() { return Rl(3.141592653589793238462643); }
Rl m_min(Rlp v1, Rlp v2) 
	{ return v1 <= v2 ? v1 : v2; }
Rl m_max(Rlp v1, Rlp v2) 
	{ return v1 >= v2 ? v1 : v2; }
Rl m_clamp(Rlp v, Rlp v1, Rlp v2) 
	{ return v <= v1 ? v1 : (v >= v2 ? v2 : v); }
Rl m_sqrt(Rlp v) 
	{ return sqrt(v); }
Rl m_atan(Rlp y, Rlp x) 
	{ return atan2(y, x); }
Rl m_cos(Rlp x) 
	{ return cos(x); }
Rl m_sin(Rlp x) 
	{ return sin(x); }
Rl m_rad(Rlp d)
	{ return d*rl_pi()/Rl(180); }

V2 v2_z() 
	{ return V2(0, 0); }
V2 add(V2p v1, V2p v2) 
	{ return V2(v1.x[0]+v2.x[0], v1.x[1]+v2.x[1]); }
V2 sub(V2p v1, V2p v2) 
	{ return V2(v1.x[0]-v2.x[0], v1.x[1]-v2.x[1]); }
V2 mul(V2p v1, Rlp s) 
	{ return V2(v1.x[0]*s, v1.x[1]*s); }
V2 neg(V2p v1) 
	{ return V2(-v1.x[0], -v1.x[1]); }
Rl dot(V2p v1, V2p v2) 
	{ return v1.x[0]*v2.x[0] + v1.x[1]*v2.x[1]; }
Rl lenSq(V2p v1) 
	{ return dot(v1, v1); }
Rl len(V2p v1) 
	{ return m_sqrt(lenSq(v1)); }
Rl distSq(V2p v1, V2p v2) 
	{ return lenSq(sub(v1, v2)); }
Rl dist(V2p v1, V2p v2) 
	{ return m_sqrt(distSq(v1, v2)); }
V2 normalize(V2p v1)
	{ Rl l = len(v1); return l==0? v1 : mul(v1, Rl(1)/l); }
Rl angle(V2p v1, V2p v2)
	{ V2 c = sub(v2, v1); return m_atan(c.x[1], c.x[0]); }
V2 rot(V2p v1, Rlp a)
	{ Rl c = m_cos(a); Rl s = m_sin(a); return V2(v1.x[0]*c + v1.x[1]*(-s), v1.x[0]*s + v1.x[1]*(c)); }
V2 rot90(V2p v1)
	{ return V2(-v1.x[1], v1.x[0]); }
V2 rotm90(V2p v1)
	{ return neg(rot90(v1)); }
V2 orth(V2p v1)
	{ return rot90(v1); }
Rl projs(V2p v1, V2p a)
	{ return dot(v1, a) / dot(a, a); }
V2 proj(V2p v1, V2p a)
	{ return mul(a, projs(v1, a)); }
V2 proj_rest(V2p v1, V2p a)
	{ return sub(v1, proj(v1, a)); }
V2 proj_points(V2p a, V2p b, V2p c)
	{ return add(a, proj(sub(c, a), sub(b, a))); }
V2 proj_points_rest(V2p a, V2p b, V2p c)
	{ return proj_rest(sub(c, a), sub(b, a)); }
V2 lin_comb(const V2* v, Rl* l, int c)
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
M3 rigid(V2p transl, Rl a)
	{
		Rl c = m_cos(a); Rl s = m_sin(a);
		M3 m; m(0,2) = transl(0);m(1,2) = transl(1);
		m(0,0) = c; m(0,1) = -s; m(1,0) = s; m(1,1) = c;
		return m;
	}



#endif // LAMBY_GUASSY_H