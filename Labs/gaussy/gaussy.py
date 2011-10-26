import math


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
	
def v2_muls(v1, s):
	return [v1[0]*s, v1[1]*s]	

def v2_normalize(v1):
	l = v2_len(v1)
	if l != 0.0:
		return v2_muls(v1, 1.0/l)
	return v1



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


def Matrix(r,c,v):
	M = [v]*r
	for i in range(r):
		M[i] = [v]*c
	return M	


def Eye(r,c):
	M = Matrix(r,c,v)
	for i in range(r):
		M[i][i] = 1.0
	return M



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
			#Now A[i,j] has the value of 1
			row_i = A[i]
			for u in range(i+1, m):
				row_u = A[u]
				scale = row_u[j]
				for c in range(n):
					row_u[c] = row_u[c] - scale * row_i[c]
				# Now A[u,j] will be zero
			i = i + 1
		j = j + 1	
	return ioMatrix


def GaussElim(ioMatrix):
	
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
			#Now A[i,j] has the value of will1
			row_i = A[i]
			for u in range(i+1, m):
				row_u = A[u]
				scale = row_u[j]
				for c in range(n):
					row_u[c] = row_u[c] - scale * row_i[c]
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


def Test():
	#PrintMatrix( [[1, 2, 3], [4, 5, 6]] )	

	M = [[1, 1, 2], [1, -1, 0]] 
	x = GaussSolve(M)
	PrintV(x)

	PrintV(GaussSolve([ [0, 1] ]))
	PrintV(GaussSolve([ [0, 0] ]))
	PrintV(GaussSolve([ [2, -1] ]))
	PrintV(GaussSolve([ [1, 0, 1], [0, 1, 2] ]))

	PrintM( MulM( [ [1.0, 0.0], [0.0, 1.0] ], [ [2.0, 3.0], [4.0, 5.0] ] ), '' )
	PrintM( MulM( [ [0.5, 0.0], [0.0, -1.0] ], [ [2.0, 3.0], [4.0, 5.0] ] ), '' )

#Test()	
