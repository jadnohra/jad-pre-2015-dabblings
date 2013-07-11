#ifndef LAMBY_GUASSY_H
#define LAMBY_GUASSY_H

#include <math.h>

typedef float Rl;
typedef Rl Rlp;

struct Vec2 
{ 
	Rl x[2]; 
	Vec2(Rl x1 = 0, Rl x2 = 0) 
		{ x[0]=x1; x[1]=x2; } 
};
typedef const Vec2& Vec2p;

struct Vec 
{
	Rl x[3]; 
	Vec(Rl x1 = 0, Rl x2 = 0, Rl x3 = 0) 
		{ x[0]=x1; x[1]=x2; x[2]=x3; }
};
typedef const Vec& Vecp;

struct Mat3 
{ 
	Vec r[3]; 

	Mat3() 
		{} 
	Mat3(Vecp r1, Vecp r2, Vecp r3) 
		{ r[0]=r1; r[1]=r2; r[2]=r3; }
};
typedef const Mat3& Mat3p;


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

Vec2 Vec2_z() 
	{ return Vec2(0, 0); }
Vec2 Vec2_0() 
	{ return Vec2(0, 0); }
Vec2 add(Vec2p v1, Vec2p v2) 
	{ return Vec2(v1.x[0]+v2.x[0], v1.x[1]+v2.x[1]); }
Vec2 sub(Vec2p v1, Vec2p v2) 
	{ return Vec2(v1.x[0]-v2.x[0], v1.x[1]-v2.x[1]); }
Vec2 mul(Vec2p v1, Rlp s) 
	{ return Vec2(v1.x[0]*s, v1.x[1]*s); }
Vec2 neg(Vec2p v1) 
	{ return Vec2(-v1.x[0], -v1.x[1]); }
Rl dot(Vec2p v1, Vec2p v2) 
	{ return v1.x[0]*v2.x[0] + v1.x[1]*v2.x[1]; }
Rl lenSq(Vec2p v1) 
	{ return dot(v1, v1); }
Rl len(Vec2p v1) 
	{ return m_sqrt(lenSq(v1)); }
Rl distSq(Vec2p v1, Vec2p v2) 
	{ return lenSq(sub(v1, v2)); }
Rl dist(Vec2p v1, Vec2p v2) 
	{ return m_sqrt(distSq(v1, v2)); }
Vec2 normalize(Vec2p v1)
	{ Rl l = len(v1); return l==0? v1 : mul(v1, Rl(1)/l); }
Rl angle(Vec2p v1, Vec2p v2)
	{ Vec2 c = sub(v2, v1); return m_atan(c.x[1], c.x[0]); }
Vec2 rot(Vec2p v1, Rlp a)
	{ Rl c = m_cos(a); Rl s = m_sin(a); return Vec2(v1.x[0]*c + v1.x[1]*(-s), v1.x[0]*s + v1.x[1]*(c)); }
Vec2 rot90(Vec2p v1)
	{ return Vec2(-v1.x[1], v1.x[0]); }
Vec2 rotm90(Vec2p v1)
	{ return neg(rot90(v1)); }
Vec2 orth(Vec2p v1)
	{ return rot90(v1); }
Rl projs(Vec2p v1, Vec2p a)
	{ return dot(v1, a) / dot(a, a); }
Vec2 proj(Vec2p v1, Vec2p a)
	{ return mul(a, projs(v1, a)); }
Vec2 proj_rest(Vec2p v1, Vec2p a)
	{ return sub(v1, proj(v1, a)); }
Vec2 proj_points(Vec2p a, Vec2p b, Vec2p c)
	{ return add(a, proj(sub(c, a), sub(b, a))); }
Vec2 proj_points_rest(Vec2p a, Vec2p b, Vec2p c)
	{ return proj_rest(sub(c, a), sub(b, a)); }

Vec Vec_z() 
	{ return Vec(0, 0, 0); }
Vec uI()
	{ return Vec(1, 0, 0); }
Vec uJ()
	{ return Vec(0, 1, 0); }
Vec uK()
	{ return Vec(0, 0, 1); }

Mat3 Mat3_z()
	{ static Mat3 mz = Mat3(Vec_z(), Vec_z(), Vec_z()); return mz; }
Mat3 Mat3_id()
	{ static Mat3 mid = Mat3(uI(), uJ(), uK()); return mid; }

/*
def m2_tr(off, a):
	m = Eye(3,3)
	m[0][2] = off[0]
	m[1][2] = off[1]
	c = math.cos(a)
	s = math.sin(a)
	m[0][0] = c
	m[0][1] = -s
	m[1][0] = s
	m[1][1] = c
	return m

def m2_mul(m1, m2):
	p = Eye(3, 3)
	for i in range(3):
		for j in range(3):
			p[i][j] = m1[i][0]*m2[0][j]+m1[i][1]*m2[1][j]+m1[i][2]*m2[2][j]
	return p		

def m2_mulp(m, v):
	p = [0.0, 0.0]
	p[0] = m[0][0]*v[0]+m[0][1]*v[1]+m[0][2]
	p[1] = m[1][0]*v[0]+m[1][1]*v[1]+m[1][2]
	return p

def m2_mulv(m, v):
	p = [0.0, 0.0]
	p[0] = m[0][0]*v[0]+m[0][1]*v[1]
	p[1] = m[1][0]*v[0]+m[1][1]*v[1]
	return p

def m2_inv(m):
	mi = Eye(3, 3)
	mi[0][0]=m[1][1]
	mi[0][1]=m[1][0]
	mi[0][2]=m[0][2]*m[1][1]+m[1][2]*m[1][0]
	mi[1][0]=m[0][1]
	mi[1][1]=m[0][0]
	mi[1][2]=m[0][2]*m[0][1]+m[1][2]*m[0][0]
	return mi

def m2_inv_mulp(m, v):
	p = [0.0, 0.0]
	p[0] = m[1][1]*v[0]+m[1][0]*v[1]-m[0][2]
	p[1] = m[0][1]*v[0]+m[0][0]*v[1]-m[1][2]
	return p

def m2_orth(m):
	orth = v2_orth([m[0][0], m[0][1]])
	m[1][0] = orth[0]
	m[1][1] = orth[1]
	return m

def m2_get_trans(m):
	return [m[0][2], m[1][2]]

def m2_set_trans(m, off):
	m[0][2] = off[0] 
	m[1][2] = off[1]
*/

#endif