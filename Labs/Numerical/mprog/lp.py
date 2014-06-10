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

def vec_neg(a):
	return [-x for x in a]

def vec_add(a,b):
	return [x+y for x, y in zip(a, b)]

def vec_sub(a,b):
	return [x-y for x, y in zip(a, b)]

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

def lp_smtp_tblPrint(tbl, log = True, toFloat = False):
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
			v = r[j]
			if toFloat:
				v = float(v)
			list[i][j+1] = v
	R = tbl['tblr']			
	for i in range(ld[0]-1):
		v = R[i]
		if toFloat:
				v = float(v)
		list[i][ld[1]-1] = v
	mat_swapr(list, -1, -2)	
	print_tab(list)

def lp_smtp_tblCreate(A,B,C):
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
	
def lp_smtp_tblMul(tbl, x):
	return mat_mulv(tbl['tbl'], x)

def lp_smtp_feasible(tbl):
	return all(x >= 0 for x in tbl['tblr'])

def lp_smtp_zrow(tbl):
	return tbl['tbl'][-1]

def lp_smtp_findEntering(tbl):
	# Will use minimum index in case of tie
	# TODO: bland's rule
	M = tbl['tbl']
	zr = lp_smtp_zrow(tbl)
	maxi = 0
	for i in range(len(zr)):
		if zr[i] > zr[maxi]:
			maxi = i
	return maxi

def lp_smtp_findLeaving_simple(tbl, pe):
	# Will use minimum index in case of tie
	M = tbl['tbl']
	R = tbl['tblr']
	pl = -1; minsr = 0;
	for ri in range(len(M)-1):
		r = M[ri][pe]
		if (r > 0):
			sr = R[ri]/r
			if (pl == -1 or sr < minsr):
				minsr = sr
				pl = ri
	return pl

def lp_lexiPos(v):
	for e in v:
		if e > 0:
			return 1
		if e < 0:
			return -1
	return 0		

def lp_lexiComp(v1, v2):
	return lp_lexiPos(vec_sub(v2, v1))

def lp_smtp_findLeaving_lexi(tbl, pe):
	M = tbl['tbl']
	R = tbl['tblr']
	lexiinfo = []
	for ri in range(len(M)-1):
		r = M[ri][pe]
		if (r > 0):
			v = list(M[ri])
			v.append(R[ri])
			v = vec_muls(v, 1/r)
			lexiinfo.append([ri,v])
	lexiinfo = sorted(lexiinfo, cmp=lambda x,y: lp_lexiComp(x[1], y[1]))
	return lexiinfo[0][0]

def lp_smtp_findLeaving(tbl, pe, opts):
	lexi = opts.get('lexi', False)
	if lexi:
		return lp_smtp_findLeaving_lexi(tbl, pe)
	else:
		return lp_smtp_findLeaving_simple(tbl, pe)
		
def lp_smtp_swapBasis(tbl, pe, pl):
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

def lp_smtp_opt(tbl, opts, log = False):
	it = 0
	maxit = opts.get('maxit', 0)
	while (it < maxit or maxit <= 0):
		it = it + 1
		pc = lp_smtp_findEntering(tbl)
		if (lp_smtp_zrow(tbl)[pc] <= 0):
			print '*** optimal'; return True
		pr = lp_smtp_findLeaving(tbl, pc, opts)
		if (pr == -1):
			print '*** unbounded'; return False
		lp_smtp_swapBasis(tbl, pc, pr)
		s_print('', log); lp_smtp_tblPrint(tbl, log);
	print '*** maxiter'; return False

def lp_smtp_initFeed(tbl, tblp, opts, log = False):
	if (not lp_smtp_feasible(tblp)):
		print '*** error 1'; return False
	if (not lp_smtp_opt(tblp, opts, log)):
		print '*** no init 0'; return False
	if (tblp['tblr'][-1] != 0):
		print '*** no init 1'; return False

	# Transfer T	
	R = tbl['tblr']; Rp = tblp['tblr'];
	T = tbl['tbl']; Tp = tblp['tbl'];
	for i in range(len(R)-1):
		R[i] = Rp[i]
		for j in range(len(T[i])):
			T[i][j]=Tp[i][j+1]

	# Transfer L
	L = tbl['tbll']; Lp = tblp['tbll']
	for i in range(len(L)):
		L[i] = Lp[i]-1

	# Transfer z		
	z = lp_smtp_zrow(tbl)
	tv = [None]*len(z)
	tvr = [g_num(0)]*len(z)
	for i in range(len(z)):
		tv[i] = [g_num(0)]*len(z)
		tv[i][i] = g_num(1)
	for i in range(len(Lp)):
		xi = Lp[i]-1
		if (xi >= 0 and xi < len(z)):
			tv[xi] = vec_neg(T[i])
			tv[xi][xi] = g_num(0)
			tvr[xi] = Rp[i]
	nz = [g_num(0)]*len(z)
	nzr = g_num(0)
	for i in range(len(z)):
		nz = vec_add(nz, vec_muls(tv[i], z[i]))
		nzr = nzr + tvr[i] * z[i]
	R[-1] = nzr
	T[-1] = nz

	s_print('', log); lp_smtp_tblPrint(tbl, log);
	s_print('*** main', log)		
	return True		

def lp_smtp_init(A,B,C, tbl, opts = {}, log = False):
	# Chvatal [p41]
	if (lp_smtp_feasible(tbl)):
		return True
	else:	
		s_print('*** aux', log)
		Cp = [g_num(0)]*(len(C)+1);	Cp[0] = g_num(-1)
		Ap = mat_create(mat_rows(A), mat_cols(A)+1, g_num(-1))
		for i in range(len(A)):
			for j in range(len(A[i])):
				Ap[i][j+1] = A[i][j]
		tblp = lp_smtp_tblCreate(Ap,B,Cp)
		pc = 0; pr = 0;
		for i in range(len(B)):
			if (B[i] < B[pr]):
				pr = i
		lp_smtp_swapBasis(tblp, pc, pr)
		s_print('', log); lp_smtp_tblPrint(tblp, log);
		return lp_smtp_initFeed(tbl, tblp, opts, log)


def lp_simplex_maximum_tableau_pivoting(A,B,C, tbl, opts = {}, log = False):
	# Chvatal [p24-25]
	s_print('+++ lp_smtp', log); lp_smtp_tblPrint(tbl, log);
	if (not lp_smtp_init(A,B,C, tbl, opts, log)):
		return False
	return lp_smtp_opt(tbl, opts, log)

# maximize Cx, with Ax <= B. x >= 0.
def lp_simplex_maximum_pivoting(A,B,C, opts = {}, log = False):
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
	tableau = {'n':n, 'nb':nb, 'tbll':tbll, 'tbl':tbl, 'tblr':tblr}
	return lp_simplex_maximum_tableau_pivoting(A,B,C, tableau, opts, log)

# # maximize Cx, with Ax = B, x >= 0.
# def lp_simplex_canonical_pivoting(A,B,C, opts = {}, log = False):
# 	nb = vec_dim(B) 
# 	n = len(C)
# 	tbll = range(nnb, nnb+nb, 1)
# 	tblr = vec_create(nb+1, g_num(0)); 
# 	tbl = mat_create(nb+1, n, g_num(0))
# 	mat_copyTo(A, tbl)
# 	for i in range(nb):
# 		tbl[i][(n-nb)+i] = g_num(1)
# 		tblr[i] = B[i]
# 	tblr[-1] = 0
# 	for i in range(len(C)):	
# 		tbl[nb][i] = C[i]
# 	tableau = {'n':n, 'nb':nb, 'tbll':tbll, 'tbl':tbl, 'tblr':tblr}
# 	return lp_simplex_maximum_tableau_pivoting(tableau, opts, log)



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
	lp_simplex_maximum_pivoting(mat_rational(A),vec_rational(B),vec_rational(C), {'maxit':10}, True)
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
	lp_simplex_maximum_pivoting(mat_rational(A),vec_rational(B),vec_rational(C), {'maxit':10}, True)
	g_num = g_numDefault	

if 0:
	g_num = g_numRational
	A = [ 	[2, -1, 2],
			[2, -3, 1],
			[-1, 1, -2],
		]
	B = [4, -5, -1]
	C = [1, -1, 1]
	lp_simplex_maximum_pivoting(mat_rational(A),vec_rational(B),vec_rational(C), {'maxit':10}, True)
	g_num = g_numDefault	

if 1:
	#cycles if nothing is done about it
	g_num = g_numDefault
	A = [ 	[0, 0, 0, 0, 1],
			[0.5, -5.5, -2.5, 9, 1],
			[0.5, -1.5, -0.5, 1, 1],
			[1, 0, 0, 0, 1]
		]
	B = [1, 1, 1, 2]
	C = [10, -57, -9, -24, 100]
	lp_simplex_maximum_pivoting(A, B, C, {'maxit':10}, True)
	lp_simplex_maximum_pivoting(A, B, C, {'maxit':10, 'lexi':True}, True)
	g_num = g_numDefault

if 0:
	#5,5,z=25
	g_num = g_numRational
	A = [ [2, 1], [1, 3], ]; B = [15, 20]; C = [2, 3]
	lp_simplex_maximum_pivoting(mat_rational(A),vec_rational(B),vec_rational(C), {'maxit':10}, True)
	g_num = g_numDefault	

if 0:
	#3,6,z=24
	g_num = g_numRational
	A = [ [3, 1], [1, 1], [2, -1], ]; B = [15, 10, 0]; C = [6, 1]
	lp_simplex_maximum_pivoting(mat_rational(A),vec_rational(B),vec_rational(C), {'maxit':10}, True)
	g_num = g_numDefault		

if 0:
	#3,6,z=24
	g_num = g_numRational
	A = [ [3, 1], [1, 1], [2, -1], ]; B = [15, 10, 0]; C = [5, 1, 0, 0, 0]
	lp_simplex_maximum_pivoting(mat_rational(A),vec_rational(B),vec_rational(C), {'maxit':10}, True)
	g_num = g_numDefault		

if 0:
	#1,2,0
	g_num = g_numRational
	A = [ [-2, -2, -1], [-1, -2, -3], ]; B = [-6, -5]; C = [-3, -4, -5]
	lp_simplex_maximum_pivoting(mat_rational(A),vec_rational(B),vec_rational(C), {'maxit':10}, True)
	g_num = g_numDefault			