import math
import random


def v2_z():
	return [0.0, 0.0]

def v2_zero(v):
	v[0] = 0.0
	v[1] = 0.0

def v2_add(v1, v2):
	return [v1[0]+v2[0], v1[1]+v2[1]]
	
def v2_sub(v1, v2):
	return [v1[0]-v2[0], v1[1]-v2[1]]

def v2_dot(v1, v2):
	return v1[0]*v2[0]+v1[1]*v2[1]

def v2_lenSq(v1):
	return v2_dot(v1, v1)

def v2_len(v1):
	return math.sqrt(v2_lenSq(v1))
	
def v2_distSq(p1, p2):
	vec = v2_sub(p2, p1)
	return v2_lenSq(vec)

def v2_dist(p1, p2):
	vec = v2_sub(p2, p1)
	return v2_len(vec)

def v2_muls(v1, s):
	return [v1[0]*s, v1[1]*s]	

def v2_neg(v1):
	return [ -v1[0], -v1[1] ]

def v2_copy(v):
	return [v[0], v[1]] 

def v2_normalize(v1):
	l = v2_len(v1)
	if l != 0.0:
		return v2_muls(v1, 1.0/l)
	return v1

def v2_angle(v1,v2):
	vec = v2_sub(v2, v1)
	return math.atan2(vec[1], vec[0])

def v2_rot(v1,a):
	c = math.cos(a)
	s = math.sin(a)
	ret = [ v1[0]*c + v1[1]*(-s), v1[0]*s + v1[1]*(c) ]
	return ret

def v2_orth(v1):
	return [ -v1[1], v1[0] ]

def v2_rot90(v1):
	return [ -v1[1], v1[0] ]
	
def v2_rotm90(v1):
	return [ v1[1], -v1[0] ]

def v2_proj(v, a):
	return v2_muls(a, v2_dot(v, a) / v2_dot(a, a))

def v2_proj_rest(v, a):
	return v2_sub(v, v2_proj(v, a))

def m2_zero():
	return Matrix(3, 3, 0.0)

def m2_id():
	return Eye(3, 3)

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

def la_inv_2(m):
	d = m[0][0]*m[1][1]-m[0][1]*m[1][0]
	if d == 0.0:
		return None
	id = 1.0 / d	
	return [[m[1][1]*id, -m[1][0]*id], -m[0][1]*id, m[0][0]*id]	

def la_mul_2(m1, m2):
	p = Eye(2, 2)
	for i in range(2):
		for j in range(2):
			p[i][j] = m1[i][0]*m2[0][j]+m1[i][1]*m2[1][j]
	return p

def la_mul_2v(m, v):
	p = [0.0, 0.0]
	p[0] = m[0][0]*v[0]+m[0][1]*v[1]
	p[1] = m[1][0]*v[0]+m[1][1]*v[1]
	return p

def la_solve_2(m, b):
	mi = la_inv_2(m)
	if (mi == None):
		return None
	return la_mul_2v(mi, b)	


def PrintM(inMatrix, inName):
	rows = len(inMatrix)
	print inName, '[',rows,'x',len(inMatrix[0]),']'
	for r in range(rows):
		row = inMatrix[r]
		cols = len(row)
		print '|',
		for c in range(cols):
			print row[c],
		print '|'	


def PrintV(row):
	cols = len(row)
	print '<',
	for c in range(cols):
		print row[c],
	print '>'	


def EmptySquareMatrix(d):
	return [[]*d for x in xrange(d)]


def EmptyMatrix(r,c):
	return [[]*c for x in xrange(r)]


def Matrix(r,c,v):
	return [[v]*c for x in xrange(r)]
	#M = [v]*r
	#for i in range(r):
	#	M[i] = [v]*c
	#return M	


def Eye(r,c):
	M = Matrix(r,c,0.0)
	for i in range(r):
		M[i][i] = 1.0
	return M


def CopyM(M):
	r1 = len(M)
	c1 = len(M[0])
	
	N = Matrix(r1, c1, None)

	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = rm[j]
	return N		


def NegM(M):
	r1 = len(M)
	c1 = len(M[0])
	
	N = Matrix(r1, c1, None)

	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = -rm[j]
	return N		


def AddM(M, N):
	r1 = len(M)
	c1 = len(M[0])
	
	S = Matrix(r1, c1, None)

	for i in range(r1):
		rm = M[i]
		rn = N[i]
		rs = S[i]
		for j in range(c1):
			rs[j] = rm[j]+rn[j]
	return S	


def SubM(M, N):
	r1 = len(M)
	c1 = len(M[0])
	
	S = Matrix(r1, c1, None)

	for i in range(r1):
		rm = M[i]
		rn = N[i]
		rs = S[i]
		for j in range(c1):
			rs[j] = rm[j]-rn[j]
	return S	


def MulM(M1, M2):
	r1 = len(M1)
	c1 = len(M1[0])
	r2 = len(M2)
	c2 = len(M2[0])

	r3 = r1
	c3 = c2

	M3 = [None]*r3

	for i in range(r3):
		M3[i] = [None]*c3
		row_3 = M3[i]
		row_1 = M1[i]
		for j in range(c3):
			dot = 0.0
			for d in range(c1):
				dot = dot + row_1[d] * M2[d][j]
			row_3[j] = dot
	
	return M3		


def MulMV(M1, V2):
	r1 = len(M1)
	c1 = len(M1[0])

	r3 = r1
	c3 = 1

	M3 = [None]*r3

	for i in range(r3):
		M3[i] = [None]*c3
		row_3 = M3[i]
		row_1 = M1[i]
		dot = 0.0
		for d in range(c1):
			dot = dot + row_1[d] * V2[d]
		row_3[0] = dot
	
	return M3		


def MulMDiag2(M1, M2Diag):
	r1 = len(M1)
	c1 = len(M1[0])

	M3 = Matrix(r1, c1, 0.0)

	for i in range(r1):
		for j in range(c1):
			M3[i][j] = M1[i][j] * M2Diag[i]

	return M3


def MulMDiag1(M1Diag, M2):
	r1 = len(M2)
	c1 = len(M2[0])

	M3 = Matrix(r1, c1, 0.0)

	for i in range(r1):
		for j in range(c1):
			M3[i][j] = M2[i][j] * M1Diag[i]

	return M3



def Transp(M):
	r = len(M)
	c = len(M[0])


	T = [None]*c

	for i in range(c):
		T[i] = [None]*r
		row = T[i]
		for j in range(r):
			row[j] = M[j][i]
	return T 		


def InvDiag(M):
	r = len(M)
	c = len(M[0])

	I = [0.0]*r

	for i in range(r):
		I[i] = [0.0]*c
		I[i][i] = 1.0/M[i][i]

	return I
	

def GaussElimNoPivot(ioMatrix):
# modified GaussElim	
	A = ioMatrix
	m = len(A)
	n = len(A[0])

	i = 0
	j = 0

	while (i < m and j < n):
		if (A[i][j] != 0):
			# Divide each entry in row i by A[i,j]
			scale = 1.0 / A[i][j]
			row_div = A[i]
			for c in range(n):
				row_div[c] = row_div[c] * scale
			#A[i][j] = 1.0
			#Now A[i,j] has the value of 1
			row_i = A[i]
			for u in range(i+1, m):
				row_u = A[u]
				scale = row_u[j]
				for c in range(n):
					row_u[c] = row_u[c] - scale * row_i[c]
				A[u][j] = 0.0 # needed for accuracy problems
				# Now A[u,j] will be zero
			i = i + 1
		j = j + 1	
	return ioMatrix


def GaussElim(ioMatrix):
# http://en.wikipedia.org/wiki/Gaussian_elimination

	A = ioMatrix
	m = len(A)
	n = len(A[0])

	i = 0
	j = 0

	while (i < m and j < n):
		#Find pivot in column j, starting in row i
		maxi = i
		for k in range(i+1, m):
			if math.fabs(A[k][j]) > math.fabs(A[maxi][j]):
				maxi = k
		
		if (A[maxi][j] != 0):
			# Swap rows i and maxi, without changing the value of i
			row_i = A[i]
			row_maxi = A[maxi]
			for c in range(n):
				temp = row_i[c]
				row_i[c] = row_maxi[c]
				row_maxi[c] = temp
			# Divide each entry in row i by A[i,j]
			scale = 1.0 / A[i][j]
			row_div = A[i]
			for c in range(n):
				row_div[c] = row_div[c] * scale
			#A[i][j] = 1.0
			#Now A[i,j] has the value of 1
			row_i = A[i]
			for u in range(i+1, m):
				row_u = A[u]
				scale = row_u[j]
				for c in range(n):
					row_u[c] = row_u[c] - scale * row_i[c]
				A[u][j] = 0.0 # needed for accuracy problems
				# Now A[u,j] will be zero
			i = i + 1
		j = j + 1	
	return ioMatrix


def BackSub(inMatrix):

	A = inMatrix
	m = len(A)
	n = len(A[0])

	x = [None]*(n-1)
	i = m - 1
	vars_solved = 0

	while (i >= 0):
		
		row = A[i]
		nzc = -1
		for c in range(n):
			if row[c] != 0.0:
				nzc = c
				break
		if (nzc == n-1):
			return [None, '\'No solution\''] 
		if (nzc != -1):
			if (nzc != n-vars_solved-2):
				return [None, '\'Infinite solutions\''] 
			sub_total = 0.0	
			for subi in range(nzc+1, n-1):
				sub_total = sub_total + row[subi] * x[subi]
			x[nzc] = (row[n-1] - sub_total) / row[nzc]
			vars_solved = vars_solved + 1	
		
		i = i-1
		
	if vars_solved == 0:
		return [None, '\'Infinite solutions\'']

	return x	


def GaussSolve(ioMatrix):
	return BackSub(GaussElim(ioMatrix))


def GaussSolve2(inLHS, inRHS):
	r1 = len(inLHS)
	c1 = len(inLHS[0])
	M = Matrix(r1, c1+1, None)

	for i in range(r1):
		for j in range(c1):
			M[i][j] = inLHS[i][j]
		M[i][c1] = inRHS[i][0]
	
	return GaussSolve(M)


def DebugGaussSolve(ioMatrix):
	M = GaussElim(ioMatrix)
	PrintM(M, '')
	return BackSub(M)



def GaussSeidelSolve(inMatrix, ioSolution, iterObj):
# http://www.cfd-online.com/Wiki/Gauss-Seidel_method
	it = 0

	x = ioSolution
	M = inMatrix
	r1 = len(inMatrix)
	c1 = len(inMatrix[0])

	while (iterObj.iterate(M, x, it)):
		
		for i in range(r1):
			row = M[i]
			dot = 0.0
			
			for j in range(i):
				dot = dot + row[j] * x[j]
			for j in range(i+1, c1-1):
				dot = dot + row[j] * x[j]	

			x[i] = ( row[c1-1] - dot ) / row[i]	

		it = it+1

	return x


def GaussSeidelSolveZeroGuess(inMatrix, iterObj):
	c1 = len(inMatrix[0])
	sol = [0.0] * (c1-1)
	return GaussSeidelSolve(inMatrix, sol, iterObj)


def GaussSeidelSolve2(inLHS, inRHS, ioSolution, iterObj):
	r1 = len(inLHS)
	c1 = len(inLHS[0])
	M = Matrix(r1, c1+1, None)

	for i in range(r1):
		for j in range(c1):
			M[i][j] = inLHS[i][j]
		M[i][c1] = inRHS[i][0]
	
	return GaussSolveSolve(M, ioSolution, iterObj)


def GaussSeidelSolve2ZeroGuess(inLHS, inRHS, iterObj):
	r1 = len(inLHS)
	c1 = len(inLHS[0])
	M = Matrix(r1, c1+1, None)

	for i in range(r1):
		for j in range(c1):
			M[i][j] = inLHS[i][j]
		M[i][c1] = inRHS[i][0]
	
	return GaussSeidelSolveZeroGuess(M, iterObj)


class IterObj:
	def __init__(self, maxit, maxerr):
		self.maxit = maxit
		self.maxerr = maxerr

	def iterate(self, M, x, it):
		self.it = it

		if it >= self.maxit:
			return False
		
		if (self.maxerr < 0.0):
			return True

		if (not hasattr(self, 'prevx')):
			self.prevx = [None] * len(x)
			for i in range(len(x)):
				self.prevx[i] = x[i]
			return True

		l = len(x)
		self.err = 0.0
		for i in range(l):
			e = math.fabs(x[i] - self.prevx[i])
			if e > self.err:
				self.err = e
			self.prevx[i] = x[i]

		return self.err > self.maxerr	

		


def GaussSeidelWillConverge(inMatrix, isOnlyLHS):
# http://www.cfd-online.com/Wiki/Gauss-Seidel_method
# http://en.wikipedia.org/wiki/Diagonally_dominant_matrix
	M = inMatrix
	r1 = len(inMatrix)
	c1 = len(inMatrix[0])

	if (not isOnlyLHS):
		c1 = c1 - 1
	
	for i in range(r1):
		row = M[i]
		sumr = 0.0

		for j in range(i):
			sumr = sumr + math.fabs(row[j])
		
		for j in range(i+1, c1):
			sumr = sumr + math.fabs(row[j])

		if (not ( math.fabs(row[i]) > sumr )):
			return False

	return True	


def GaussSeidelMightConverge(inMatrix, isOnlyLHS):
# http://www.cfd-online.com/Wiki/Gauss-Seidel_method
	M = inMatrix
	r1 = len(inMatrix)
	c1 = len(inMatrix[0])

	if (not isOnlyLHS):
		c1 = c1 - 1
	
	for i in range(r1):
		row = M[i]
		diag = math.fabs(row[i])

		for j in range(i):
			if (diag < math.fabs(row[j])):
				return False
		
		for j in range(i+1, c1):
			if (diag < math.fabs(row[j])):
				return False

	return True	


def TestLinAlg():
	#PrintMatrix( [[1, 2, 3], [4, 5, 6]] )	

	# Gaussian elim
	M = [[1, 1, 2], [1, -1, 0]] 
	x = GaussSolve(M)
	PrintV(x)

	PrintV(GaussSolve([ [0, 1] ]))
	PrintV(GaussSolve([ [0, 0] ]))
	PrintV(GaussSolve([ [2, -1] ]))
	PrintV(GaussSolve([ [1, 0, 1], [0, 1, 2] ]))

	# Basics
	PrintM( MulM( [ [1.0, 0.0], [0.0, 1.0] ], [ [2.0, 3.0], [4.0, 5.0] ] ), '' )
	PrintM( MulM( [ [0.5, 0.0], [0.0, -1.0] ], [ [2.0, 3.0], [4.0, 5.0] ] ), '' )
	
	
	#Gauss Seidel
	PrintV(GaussSeidelSolveZeroGuess([ [2.0, -1.0] ], IterObj(1, -1.0) ) )
	PrintV(GaussSeidelSolveZeroGuess([ [1, 0, 1], [0, 1, 2] ], IterObj(1, -1.0)))
	
	# Diverging example from http://numericalmethods.eng.usf.edu/mws/gen/04sle/mws_gen_sle_ppt_seidel.ppt
	sol = [1, 2, 5]
	M = [ [25.0, 5.0, 1.0, 106.8], [64, 8, 1, 177.2], [144, 12, 1, 279.2] ]
	print 'Will converge:', GaussSeidelWillConverge(M, False)
	PrintV(GaussSeidelSolve( M, sol, IterObj(1, -1.0) ) )
	PrintV(GaussSeidelSolve( M, sol, IterObj(4, -1.0) ) )

	# test IterObj converge
	sol = [1, 2, 5]
	M = [ [25.0, 5.0, 1.0, 106.8], [8, 64, 1, 177.2], [12, 1, 17, 279.2] ]
	print 'Will converge:', GaussSeidelWillConverge(M, False)
	iter = IterObj(50, 0.01)
	PrintV(GaussSeidelSolve( M, sol, iter) )
	print iter.err, 'error in', iter.it, 'iterations'


	# other tests
	PrintV(GaussSolve([ [1, -1, 10], [1, -2, 0] ]))
	PrintV(GaussSeidelSolveZeroGuess([ [1, -1, 10], [1, -2, 0] ], IterObj(1, -1.0)))
	PrintV(GaussSeidelSolveZeroGuess([ [1, -1, 10], [1, -2, 0] ], IterObj(2, -1.0)))
	PrintV(GaussSeidelSolveZeroGuess([ [1, -1, 10], [1, -2, 0] ], IterObj(3, -1.0)))
	PrintV(GaussSeidelSolveZeroGuess([ [1, -1, 10], [1, -2, 0] ], IterObj(4, -1.0)))
	PrintV(GaussSeidelSolveZeroGuess([ [1, -1, 10], [1, -2, 0] ], IterObj(5, -1.0)))
	
	for i in range(10):
		m1 = 1.0 / (1.0 + 10.0 * (i))
		m2 = 1.0
		g = 10.0
		dt = 1.0
		e = g * dt
		print 'Mass Ratio', m2 / m1
		lin_syst = [ [1.0/m1, -1.0/m1, e], [1.0/m1, -(1.0/m1 + 1.0/m2), 0] ]
		PrintV(GaussSolve( CopyM(lin_syst) ))
		iter = IterObj(1000, 0.001)
		PrintV(GaussSeidelSolveZeroGuess( CopyM(lin_syst) , iter))
		print iter.err, 'error in', iter.it, 'iterations'

#TestLinAlg()


class Convex:
	v = []

	def __init__(self):
		return

	def set4(self, v0, v1, v2, v3):	
		v = [v0, v1, v2, v3]


def linComb(v, l):
	vc = [0.0, 0.0]
	for i in range(len(l)):
		vc = v2_add( vc, v2_muls(v[i], l[i]) )
	return vc	
	

def convexVertexP(p, cvx, r, n, i):
	v = v2_add(p, cvx[i])
	v = v2_add(v, v2_muls(n, r))
	return v

def convexVertexM(m, cvx, r, n, i):
	v = m2_mulp(m, cvx[i])
	v = v2_add(v, v2_muls(n, r))
	return v

def randConvex(r, vc):
	v = [None] * vc
	
	t = [None] * vc
	for i in range(vc):
		t[i] = random.uniform(0.0, 2.0*math.pi)
	t.sort()
	
	for i in range(vc):
		v[i] = [math.cos(t[i])*r, math.sin(t[i])*r]

	return v	



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
	max = v2_dot( convexVertexM(m, cvx, r, nd, 0), d)	

	for i in range(1, len(cvx)):
		dot = v2_dot( convexVertexM(m, cvx, r, nd, i), d)	
		if (dot > max):
			max = dot
			mi = i
	return [max, mi]



def gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, d):

	n = v2_normalize(d)

	i1 = gjk_support_cvx(m1, cvx1, r1, d, n)
	i2 = gjk_support_cvx(m2, cvx2, r2, v2_neg(d), v2_neg(n))

	h = i1[0]+i2[0]
	p1 = convexVertexM(m1, cvx1, r1, n, i1[1])
	p2 = convexVertexM(m2, cvx2, r2, v2_neg(n), i2[1])
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


def gjk_distance(m1, cvx1, r1, m2, cvx2, r2, eps=0.0000001, dbg = None):

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


def gjk_epa_closest_on_edge(ctx, v1, v2):
	Vk = [v1, v2]
	subd = gjk_subdist(ctx, Vk)
	return [subd[0], subd[2]] # vert, lambdas


def gjk_epa_distance(m1, cvx1, r1, m2, cvx2, r2, epa_eps, eps=0.0000001, dbg = None):

	out = gjk_distance(m1, cvx1, r1, m2, cvx2, r2, eps, dbg) 
	if (out[0] >= out[1]):
		return out

	ctx = GJK_Context()

	Vk = out[4]
	Pi = out[5]
	
	if (len(Vk)==1):
		supp = gjk_support_mink_cvx(m1, cvx1, r1, m2, cvx2, r2, [1.0, 0.0])
		Vk.append(supp[1])
		Pi.append(supp[2])

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
