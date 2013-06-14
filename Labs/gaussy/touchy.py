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

	mi = 0
	max = v2_dot( convexVertex(m, cvx, r, nd, 0), d)	

	for i in range(1, len(cvx)):
		dot = v2_dot( convexVertex(m, cvx, r, nd, i), d)	
		if (dot > max):
			max = dot
			mi = i
	return [max, mi]



def gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, d):

	n = v2_normalize(d)

	i1 = gjk_support_cvx(m1, cvx1, r1, d, n)
	i2 = gjk_support_cvx(m2, cvx2, r2, v2_neg(d), v2_neg(n))

	h = i1[0]+i2[0]
	p1 = convexVertex(m1, cvx1, r1, n, i1[1])
	p2 = convexVertex(m2, cvx2, r2, v2_neg(n), i2[1])
	s = v2_sub(p1, p2)
	return [ h, s, [p1, p2] ] # h, s, points


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


def gjk_distance(m1, cvx1, r1, m2, cvx2, r2, eps=gGJK_eps, dbg = None):

	ctx = GJK_Context()

	d = [-1.0, 0.0]
	supp = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, d) 
	Vk = [ supp[1] ]
	Pi = [ supp[2] ]

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

			return [dist, eps, v1, v2, Vk, Pi]


		#if (dist > last_dist):
		#	fallback = True

		#last_dist = dist
		Vk.append( supp[1] )
		Pi.append( supp[2] )
		if (len(Vk)>3):
			nVi = subd[1]
			if (len(nVi) > 0):
				Vk.pop(nVi[0])
				Pi.pop(nVi[0])
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

	out = gjk_distance(m1, cvx1, r1, m2, cvx2, r2, gjk_eps, dbg) 
	if (out[0] >= out[1]):
		return out

	ctx = GJK_Context()

	Vk = out[4]
	Pi = out[5]
	
	# treat degenerate case
	if (len(Vk)==1):
		supp = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, [1.0, 0.0])
		Vk.append(supp[1])
		Pi.append(supp[2])

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
				cl = gjk_epa_closest_on_edge(ctx, Vk[i], Vk[(i+1)%lV])
				Ck[i] = cl[0]
				Li[i] = cl[1]
				Dk[i] = v2_lenSq(cl[0])

			if ((min_i < 0) or (Dk[i] < Dk[min_i])):
				min_i = i

		n = v2_normalize(Ck[min_i])
		if (v2_lenSq(Ck[min_i]) == 0.0):	# We can't use the closest point as a direction, use segment normal
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

			return [-v2_len(Ck[min_i]), 0.0, v1, v2]

		ii = (min_i+1) % lV
		Vk.insert(ii, supp[1])
		Pi.insert(ii, supp[2])
		Dk[min_i] = -1.0
		Dk.insert(ii, -1.0)
		Ck.insert(ii, None)
		Li.insert(ii, None)


#----------------------------------------------------------------------------
#------------ Pertrubed Manifold --------------------------------------------
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

#TestGJK()
#TestGJK1()
#TestGJK2()
#TestEPA()
#TestGJKr()
