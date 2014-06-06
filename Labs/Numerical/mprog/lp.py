from sets import Set
import itertools
from fractions import Fraction

def g_numDefault(x):
	return x
g_num = g_numDefault

def print_tab(list, pref='', sep=' ', post=''):
    col_width = [max(len(str(x)) for x in col) for col in itertools.izip_longest(*list, fillvalue='')]
    for line in list:
        print pref + sep.join("{:>{}}".format(x, col_width[i]) for i,x in enumerate(line)) + post

def mat_print(inMatrix, inName):
	rows = len(inMatrix)
	print inName, '[',rows,'x',len(inMatrix[0]),']'
	for r in range(rows):
		row = inMatrix[r]
		cols = len(row)
		print '|',
		for c in range(cols):
			print row[c],
		print '|'	

def endl_print(log = True):
	if (not log):
		return
	print ''	

def s_print(str, log = True):
	if (not log):
		return
	print str	

def vec_print(row, log = True):
	if (not log):
		return
	cols = len(row)
	print '<',
	for c in range(cols):
		print row[c],
	print '>'	

def vec_create(n,v):
	return [v]*n

def vec_dim(v):
	return len(v)

def vec_add(a,b):
	return [x+y for x, y in zip(a, b)]

def vec_muls(a,s):
	return [x*s for x in a]	
	
def mat_rows(M):
	return len(M)

def mat_cols(M):
	return len(M[0])

def mat_dims(M):
	return [mat_rows(M), mat_cols(M)]

def mat_create(r,c,v):
	return [[v]*c for x in xrange(r)]

def mat_identity(r,c):
	M = mat_create(r,c, g_num(0))
	for i in range(r):
		M[i][i] = g_num(1)
	return M

def mat_swapr(M, i, j):
	r=M[i]; M[i]=M[j]; M[j]=r

def mat_copy(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = rm[j]
	return N		

def mat_copyTo(M, N):
	r1 = len(M);c1 = len(M[0]);
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = rm[j]
	return N	

def mat_neg(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = -rm[j]
	return N		

def mat_add(M, N):
	r1 = len(M);c1 = len(M[0]);
	S = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		rs = S[i]
		for j in range(c1):
			rs[j] = rm[j]+rn[j]
	return S	

def mat_sub(M, N):
	r1 = len(M);c1 = len(M[0]);
	S = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		rs = S[i]
		for j in range(c1):
			rs[j] = rm[j]-rn[j]
	return S	

def mat_mul(M1, M2):
	r1 = len(M1);c1 = len(M1[0]);r2 = len(M2);c2 = len(M2[0]);r3 = r1;c3 = c2;
	M3 = [None]*r3;
	for i in range(r3):
		M3[i] = [None]*c3
		row_3 = M3[i]
		row_1 = M1[i]
		for j in range(c3):
			dot = g_num(0)
			for d in range(c1):
				dot = dot + row_1[d] * M2[d][j]
			row_3[j] = dot
	return M3		

def mat_mulv(M1, V2):
	r1 = len(M1);c1 = len(M1[0]);r3 = r1;c3 = 1;M3 = [None]*r3;
	for i in range(r3):
		row_3 = M3[i]
		row_1 = M1[i]
		dot = g_num(0)
		for d in range(c1):
			dot = dot + row_1[d] * V2[d]
		M3[i] = dot
	return M3		

def mat_mulDiag1(M1, M2Diag):
	r1 = len(M1);c1 = len(M1[0]);M3 = mat_create(r1, c1, g_num(0))
	for i in range(r1):
		for j in range(c1):
			M3[i][j] = M1[i][j] * M2Diag[i]
	return M3

def mat_mulDiag2(M1Diag, M2):
	r1 = len(M2);c1 = len(M2[0]);M3 = mat_create(r1, c1, g_num(0))
	for i in range(r1):
		for j in range(c1):
			M3[i][j] = M2[i][j] * M1Diag[i]
	return M3

def mat_transp(M):
	r = len(M);
	c = len(M[0]);
	T = [None]*c;
	for i in range(c):
		T[i] = [None]*r
		row = T[i]
		for j in range(r):
			row[j] = M[j][i]
	return T 		

def mat_diagInv(M):
	r = len(M);c = len(M[0]);
	I = [g_num(0)]*r
	for i in range(r):
		I[i] = [g_num(0)]*c
		I[i][i] = g_num(1)/M[i][i]
	return I

def lp_scst_tblPrint(tbl, log = True):
	if (not log):
		return 
	ld = [tbl['nb']+2, tbl['n']+3]
	list = mat_create(ld[0], ld[1], '--')
	for i in range(ld[0]):
		list[i][-2] = '|'
	L = tbl['tbll']
	for i in range(ld[0]-2):	
		list[i][0] = 'x{}'.format(L[i]+1)
	list[-2][0] = '-z'
	M = tbl['tbl']	
	for i in range(len(M)):
		r = M[i]	
		for j in range(len(r)):
			list[i][j+1] = r[j]
	for i in range(ld[0]-1):
		list[i][ld[1]-1] = tbl['tblr'][i]
	mat_swapr(list, -1, -2)	
	print_tab(list)

def lp_scst_tblCreate(A,B,C):
	nb = vec_dim(B) # basic count (slack)
	nnb = len(C) # nonbasic count (original)
	n = nb + nnb
	tbll = range(nnb, nnb+nb, 1)
	tblr = vec_create(nb+1, g_num(0)); 
	tbl = mat_create(nb+1, n, g_num(0))
	mat_copyTo(A, tbl)
	for i in range(nb):
		tbl[i][(n-nb)+i] = g_num(1)
		tblr[i] = B[i]
	tblr[-1] = 0
	for i in range(len(C)):	
		tbl[nb][i] = C[i]
	return {'n':n, 'nb':nb, 'tbll':tbll, 'tbl':tbl, 'tblr':tblr}
	
def lp_scst_tblMul(tbl, x):
	return mat_mulv(tbl['tbl'], x)

def lp_scst_feasible(tbl):
	return all(x >= 0 for x in tbl['tblr'])

def lp_scst_zrow(tbl):
	return tbl['tbl'][-1]

def lp_scst_findEntering(tbl):
	zr = lp_scst_zrow(tbl)
	maxi = 0
	for i in range(len(zr)):
		if zr[i] > zr[maxi]:
			maxi = i
	return maxi

def lp_scst_findLeaving(tbl, pe):
	M = tbl['tbl']
	R = tbl['tblr']
	pl = -1; minsr = 0;
	for ri in range(len(M)-1):
		r = M[ri][pe]
		if (M[ri][pe] > 0):
			sr = R[ri]/r
			if (pl == -1 or sr < minsr):
				minsr = sr
				pl = ri
	return pl

def lp_scst_swapBasis(tbl, pe, pl):
	M = tbl['tbl']
	R = tbl['tblr']
	prow = M[pl]
	pn = prow[pe]	
	for i in range(len(prow)):
		prow[i] = prow[i] / pn
	R[pl] = R[pl] / pn
	rows = range(len(M)); rows.pop(pl);
	for ri in rows:
		row = M[ri]
		if (row[pe] != 0):
			s = g_num(-row[pe])
			M[ri] = vec_add(vec_muls(prow, s), row)
			R[ri] = (R[pl]*s)+R[ri]
	L = tbl['tbll']		
	L[pl] = pe

def lp_scst_opt(tbl, maxit = 0, log = False):
	it = 0
	while (it < maxit or maxit <= 0):
		it = it + 1
		pc = lp_scst_findEntering(tbl)
		if (lp_scst_zrow(tbl)[pc] <= 0):
			print '*** optimal'; return True
		pr = lp_scst_findLeaving(tbl, pc)
		if (pr == -1):
			print '*** unbounded'; return False
		lp_scst_swapBasis(tbl, pc, pr)
		s_print('', log); lp_scst_tblPrint(tbl, log);
	print '*** maxiter'; return False

def lp_scst_initFeed(tbl, tblp, maxit, log = False):
	if (not lp_scst_feasible(tblp)):
		print '*** error 1'; return False
	if (not lp_scst_opt(tblp, maxit, log)):
		print '*** no init 0'; return False
	if (tblp['tblr'][-1] != 0):
		print '*** no init 1'; return False

	R = tbl['tblr']; Rp = tblp['tblr'];
	T = tbl['tbl']; Tp = tblp['tbl'];
	for i in range(len(R)-1):
		R[i] = Rp[i]
		for j in range(len(T[i])):
			T[i][j]=Tp[i][j+1]

	z = lp_scst_zrow(tbl)
	nz = list(z)
	nzr = 0.0
	L = tblp['tbll']
	for i in range(len(L)):
		xi = L[i]-1
		if (xi >= 0 and xi < len(nz)):
			nz = vec_add(nz, vec_muls(T[i], z[xi]))
			nzr = nzr + (Rp[i] * z[xi])
			print z[xi], nz, nzr
	R[-1] = nzr
	T[-1] = nz
	print nzr, 'WRONG, fix lp_scst_initFeed [p41]'
	s_print('*** main', log)		
	return True		

def lp_scst_init(A,B,C, tbl, maxit = 0, log = False):
	# Chvatal [p41]
	if (lp_scst_feasible(tbl)):
		return True
	else:	
		s_print('*** aux', log)
		Cp = [g_num(0)]*(len(C)+1);	Cp[0] = g_num(-1)
		Ap = mat_create(mat_rows(A), mat_cols(A)+1, g_num(-1))
		for i in range(len(A)):
			for j in range(len(A[i])):
				Ap[i][j+1] = A[i][j]
		tblp = lp_scst_tblCreate(Ap,B,Cp)
		pc = 0; pr = 0;
		for i in range(len(B)):
			if (B[i] < B[pr]):
				pr = i
		lp_scst_swapBasis(tblp, pc, pr)
		s_print('', log); lp_scst_tblPrint(tblp, log);
		return lp_scst_initFeed(tbl, tblp, maxit, log)


def lp_scst(A,B,C, maxit = 0, log = False):
	# Chvatal [p24-25]
	tbl = lp_scst_tblCreate(A,B,C)
	s_print('+++ lp_scst', log); lp_scst_tblPrint(tbl, log);
	if (not lp_scst_init(A,B,C, tbl, maxit, log)):
		return False
	return lp_scst_opt(tbl, maxit, log)



# maximize Cx, with Ax = B, x >= 0, x in Rn.
def lp_standard_canonical_simplex_tableau(A,B,C, maxit = 0, log = True):
	return lp_scst(A,B,C, maxit, log)

# maximize Cx, with Ax <= B. x >= 0.
def lp_standard_simplex_dictionary(A,B,C):
	return 0

def rational(x, y=1):
	return Fraction(x, y)

def g_numRational(x):
	return rational(x)

def vec_rational(V):
	n = len(V);
	W = vec_create(n, None)
	for i in range(n):
		W[i] = rational(V[i])
	return W

def mat_rational(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = rational(rm[j])
	return N	


if 0:
	g_num = g_numRational
	A = [ 	[2, 3, 1],
			[4, 1, 2],
			[3, 4, 2]
		]
	B = [5, 11, 8]
	C = [5, 4, 3]
	lp_standard_canonical_simplex_tableau(mat_rational(A),vec_rational(B),vec_rational(C), 10, True)
	g_num = g_numDefault

if 0:
	g_num = g_numRational
	A = [ 	[1, 3, 1],
			[-1, 0, 3],
			[2, -1, 2],
			[2, 3, -1],
		]
	B = [3, 2, 4, 2]
	C = [5, 5, 3]
	lp_standard_canonical_simplex_tableau(mat_rational(A),vec_rational(B),vec_rational(C), 10, True)
	g_num = g_numDefault	

if 1:
	g_num = g_numRational
	A = [ 	[2, -1, 2],
			[2, -3, 1],
			[-1, 1, -2],
		]
	B = [4, -5, -1]
	C = [1, -1, 1]
	lp_standard_canonical_simplex_tableau(mat_rational(A),vec_rational(B),vec_rational(C), 10, True)
	g_num = g_numDefault	