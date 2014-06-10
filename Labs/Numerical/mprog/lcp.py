from sets import Set
import itertools
from fractions import Fraction

def g_numDefault(x):
	return x
g_num = g_numDefault

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

def print_tab(list, pref='', sep=' ', post=''):
    col_width = [max(len(str(x)) for x in col) for col in itertools.izip_longest(*list, fillvalue='')]
    for line in list:
        print pref + sep.join("{:>{}}".format(x, col_width[i]) for i,x in enumerate(line)) + post

def mat_print(inMatrix, inName):
	rows = len(inMatrix)
	print inName, '[',rows,'x',len(inMatrix[0]),']'
	for r in range(rows):
		row = inMatrix[r]; cols = len(row); print '|',
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

def vec_divs(a,s):
	return [x/s for x in a]		

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

def mat_col(M, c):
	v = [None]*len(M); 
	for i in range(len(M)): v[i] = M[i][c]; 
	return v;

def mat_swapr(M, i, j):
	r=M[i]; M[i]=M[j]; M[j]=r

def mat_copy(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]; rn = N[i]
		for j in range(c1):
			rn[j] = rm[j]
	return N		

def mat_copyTo(M, N):
	r1 = len(M);c1 = len(M[0]);
	for i in range(r1):
		rm = M[i]; rn = N[i];
		for j in range(c1):
			rn[j] = rm[j]
	return N	

def mat_neg(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]; rn = N[i];
		for j in range(c1):
			rn[j] = -rm[j]
	return N		

def mat_add(M, N):
	r1 = len(M);c1 = len(M[0]);
	S = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]; rn = N[i]; rs = S[i];
		for j in range(c1):
			rs[j] = rm[j]+rn[j]
	return S	

def mat_sub(M, N):
	r1 = len(M);c1 = len(M[0]);
	S = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]; rn = N[i]; rs = S[i];
		for j in range(c1):
			rs[j] = rm[j]-rn[j]
	return S	

def mat_mul(M1, M2):
	r1 = len(M1);c1 = len(M1[0]);r2 = len(M2);c2 = len(M2[0]);r3 = r1;c3 = c2;
	M3 = [None]*r3;
	for i in range(r3):
		M3[i] = [None]*c3; row_3 = M3[i]; row_1 = M1[i];
		for j in range(c3):
			dot = g_num(0)
			for d in range(c1):
				dot = dot + row_1[d] * M2[d][j]
			row_3[j] = dot
	return M3		

def mat_mulv(M1, V2):
	r1 = len(M1);c1 = len(M1[0]);r3 = r1;c3 = 1;M3 = [None]*r3;
	for i in range(r3):
		row_3 = M3[i]; row_1 = M1[i]; dot = g_num(0);
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
	r = len(M); c = len(M[0]); T = [None]*c;
	for i in range(c):
		T[i] = [None]*r; row = T[i]
		for j in range(r):
			row[j] = M[j][i]
	return T 		

def mat_diagInv(M):
	r = len(M);c = len(M[0]); I = [g_num(0)]*r;
	for i in range(r):
		I[i] = [g_num(0)]*c; I[i][i] = g_num(1)/M[i][i];
	return I

def lcp_tbl_create(n):
	return { 'n':n, 'L':[i for i in range(n)], 'Mq':mat_create(n, (2*n)+1, g_num(0)) }

def lcp_tbl_pivot(tbl, plr, pec):
	Mq = tbl['Mq']; 
	Mq[plr] = vec_divs(Mq[plr], Mq[plr][pec])
	for i in [x for x in range(len(Mq)) if (not x == plr) and (not Mq[x][pec] == 0)]:
		Mq[i] = vec_add(Mq[i], vec_muls(Mq[plr], g_num(-Mq[i][pec])))
	tbl['L'][plr] = pec

def lcp_tbl_qcol(tbl):
	 return mat_col(tbl['Mq'], -1)

def lcp_solve_bard_tableau(tbl):
	#TLCP p.239
	while True:
		Mq = tbl['Mq']; q = lcp_tbl_qcol(tbl);
		# Test for termination
		if all(x >= 0 for x in q):
			return True
		for i in range(len(q)):
			if (q[i] < 0):
				lcp_tbl_pivot(tbl, i, i)
				break;

def lcp_solve_principal_tableau(tbl, opts = {}):
	#LC_L_NP p.255
	maxit = opts.get('maxit', 0); it = 0
	while (maxit == 0 or it < maxit):
		it = it + 1
		Mq = tbl['Mq']; q = lcp_tbl_qcol(tbl);
		# Test for termination
		if all(x >= 0 for x in q):
			return True
		cands = [x for x in range(len(q)) if q[x] < 0]
		r,c = cands[-1], (tbl['L'][cands[-1]] + tbl['n']) % (2*tbl['n'])
		print r,c
		lcp_tbl_pivot(tbl, r, c)
	return False

if 0:
	tbl = lcp_tbl_create(3)
	Mq = tbl['Mq']
	Mq[0] = [1, 0, 0, -1, 1, -1, 3]
	Mq[1] = [0, 1, 0, -1, 1, -1, 5]
	Mq[2] = [0, 0, 1, -1, -1, -1, -9]
	print tbl['Mq']
	lcp_tbl_pivot(tbl, 5, 2)
	print tbl['Mq']

if 1:
	tbl = lcp_tbl_create(3)
	Mq = tbl['Mq']
	Mq[0] = [1, 0, 0, -1, 0, 0, -1]
	Mq[1] = [0, 1, 0, -2, -1, 0, -1]
	Mq[2] = [0, 0, 1, -2, -2, -1, -1]
	print tbl['Mq']
	lcp_solve_principal_tableau(tbl)
	print tbl['Mq']


