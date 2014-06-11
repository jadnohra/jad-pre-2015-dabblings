import itertools
from fractions import Fraction
from operator import itemgetter 

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
		row = inMatrix[r]; cols = len(row); print '|',
		for c in range(cols):
			print row[c],
		print '|'	
def log_print(str, log = True):
	if (log):
		print str	
def opt_print(str, opt = {}):
 	log_print(str, opt.get('log', False))	
def vec_print(row, log = True):
	if (log):
		print '({})'.format(', '.join(str(x) for x in row))

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
def vec_argmin2(v):
	return min(enumerate(v), key=itemgetter(1)); 
def vec_argmax2(v):
	return max(enumerate(v), key=itemgetter(1)); 
def vec_argmin(v):
	return vec_argmin2(v)[0]
def vec_argmax(v):
	return vec_argmax2(v)[0]
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
def mat_blockIdentity(M, r,c, n):
	for i in range(n): 
		for j in range(n):
			M[r+i][c+j] = g_num(0)
		M[r+i][c+i] = g_num(1)		
def mat_blockSetCol(M, r,c, n, v):
	for j in range(n): M[r+j][c] = v
def mat_col(M, c):
	v = [None]*len(M);
	for i in range(len(M)): v[i] = M[i][c]
	return v
def mat_rput(M, r, coff, v):
	for i in range(len(v)): M[r][coff+i] = v[i]
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
def mat_float(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = float(rm[j])
	return N	


def lcp_tbl_pivot(tbl, plr, pec):
	M = tbl['M']; 
	M[plr] = vec_divs(M[plr], M[plr][pec])
	for i in [x for x in range(len(M)) if (not x == plr) and (not M[x][pec] == 0)]:
		M[i] = vec_add(M[i], vec_muls(M[plr], g_num(-M[i][pec])))
	tbl['L'][plr] = pec

def lcp_tbl_struct(nc, off, lbl, init):
	return { 'nc':nc, 'off':off, 'lbl':lbl, 'init':init }

def lcp_tbl_sorted_structs(tbl):
	return sorted(tbl['structs'].values(), key=lambda x: x['off'])

def lcp_tbl_create(n, structs):
	nc = 0
	for s in structs.values():
		nc = nc + s['nc']
	tbl = { 'n':n, 'nc':nc, 'L':[i for i in range(n)], 'M':mat_create(n, nc, g_num(0)), 'structs':structs }	
	for s in structs.values():
		if (s['init'] == 'id'):
			mat_blockIdentity(tbl['M'], 0, s['off'], s['nc'])
		elif (s['init'] == '-en'):
			mat_blockSetCol(tbl['M'], 0, s['off'], n, g_num(-1))	
	return tbl		

def lcp_tbl_off(tbl, lbl):
	return tbl['structs'][lbl]['off']

def lcp_tbl_col(tbl, lbl):
	 return mat_col(tbl['M'], lcp_tbl_off(tbl, lbl))

def lcp_tbl_lbl(tbl, i):
	xi = tbl['L'][i]; structs = tbl['structs']; sv = lcp_tbl_sorted_structs(tbl);
	for si in range(len(sv)):
		if (xi < sv[si]['off']):
			if (sv[si]['nc'] > 1):
				return '{}{}'.format(sv[si-1]['lbl'], 1+xi-sv[si-1]['off'])
			else:
				return sv[si-1]['lbl']
	return ''		
		
def lcp_tbl_lbls(tbl):
	return [lcp_tbl_lbl(tbl, i) for i in range(tbl['n'])]

def lcp_tbl_lbls2(tbl):
	return  '({})'.format(', '.join(lcp_tbl_lbls(tbl)))

def lcp_tbl_pp_struct(n):
	return {'w': lcp_tbl_struct(n, 0, 'w', 'id'), 
			'z': lcp_tbl_struct(n, n, 'z', 0), 
			'q': lcp_tbl_struct(1, (2*n), 'q', 0)}
	
def lcp_tbl_pp_create(n):
	return lcp_tbl_create(n, lcp_tbl_pp_struct(n))

def lcp_tbl_pp_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), m)
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q

def lcp_tbl_pp_mqinit(tbl, M):
	for r in range(len(M)): lcp_tbl_pp_rinit(tbl, r, M[r][:-1], M[r][-1])

def lcp_tbl_pp_solution(tbl):
	M = tbl['M']; L = tbl['L']; q = lcp_tbl_col(tbl, 'q'); wz = [g_num(0)]*(tbl['nc']-1);
	for i in range(len(L)): wz[L[i]] = q[i]
	return wz

def lcp_tbl_pp_compl(tbl, xi):
	return (xi +  tbl['n']) % (2*tbl['n'])

def lcp_solve_ppm1_tableau(tbl, opts = {}):
	# Principal Pivot Method I, Murty p.255 (opt. p.259)
	# Finite for P, may cycle otherwise.
	#
	maxit = opts.get('maxit', 0); it = 0; status = 1;
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		M = tbl['M']; q = lcp_tbl_col(tbl, 'q');
		# Test for termination
		if all(x >= 0 for x in q):
			status = 2; break;
		cands = [x for x in range(len(q)) if q[x] < 0]
		r,c = cands[-1], lcp_tbl_pp_compl(tbl, tbl['L'][cands[-1]]) 
		if (M[r][c] == 0):
			status = 0; break;
		lcp_tbl_pivot(tbl, r, c)
		opt_print('{}. pvt: {}-{}, {}'.format(it, r,c, lcp_tbl_lbls2(tbl)), opts)
	if (status == 2):
		tbl['wz'] = lcp_tbl_pp_solution(tbl)
	else:	
		tbl['wz'] = []
	return False

def lcp_tbl_cpa_struct(n):
	return {'w': lcp_tbl_struct(n, 0, 'w', 'id'), 
			'z': lcp_tbl_struct(n, n, 'z', 0), 
			'z0': lcp_tbl_struct(1, 2*n, 'z0', '-en'), 
			'q': lcp_tbl_struct(1, (2*n)+1, 'q', 0)}

def lcp_tbl_cpa_create(n):
	return lcp_tbl_create(n, lcp_tbl_cpa_struct(n))

def lcp_tbl_cpa_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), m)
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q

def lcp_tbl_cpa_mqinit(tbl, M):
	for r in range(len(M)): lcp_tbl_cpa_rinit(tbl, r, M[r][:-1], M[r][-1])

def lcp_solve_cpa_tableau(tbl, opts = {}):
	# Complementary Pivot Algorithm, Murty p.66
	#
	#Initialization, p.71
	t,qt = vec_argmin2(lcp_tbl_col(tbl, 'q'))
	if (qt >= 0):
		#tbl['wz'] = lcp_tbl_solution(tbl)
		return True
	else:
		lcp_tbl_pivot(tbl, t, lcp_tbl_off(tbl, 'z0'))
	opt_print('Init. {}'.format(lcp_tbl_lbls2(tbl)), opts)
	
	return False	
	maxit = opts.get('maxit', 0); it = 0; status = 1;
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		M = tbl['M']; q = lcp_tbl_col(tbl, 'q');
		# Test for termination, p.74
		if all(x >= 0 for x in q):
			status = 2; break;
		cands = [x for x in range(len(q)) if q[x] < 0]
		r,c = cands[-1], lcp_tbl_pp_compl(tbl, tbl['L'][cands[-1]]) 
		if (M[r][c] == 0):
			status = 0; break;
		lcp_tbl_pivot(tbl, r, c)
		opt_print('{}. pvt: {}-{}, {}'.format(it, r,c, lcp_tbl_lbls2(tbl)), opts)
	if (status == 2):
		tbl['wz'] = lcp_tbl_solution(tbl)
	else:	
		tbl['wz'] = []
	return False

if 0:
	tbl = lcp_tbl_pp_create(3)
	lcp_tbl_pp_mqinit(tbl, mat_rational([
			[-1, 1, -1, 3],
			[-1, 1, -1, 5],
			[-1, -1, -1, 9]
			]) )
	print tbl['M']
	lcp_tbl_pivot(tbl, 5, 2)
	print tbl['M']

if 0:
	#Murty p.255 
	tbl = lcp_tbl_pp_create(3)
	lcp_tbl_pp_mqinit(tbl, mat_rational([
			[-1, 0, 0, -1],
			[-2, -1, 0, -1],
			[-2, -2, -1, -1]
			]) )
	lcp_solve_ppm1_tableau(tbl, {'maxit':20, 'log':True})
	#print tbl['M']
	vec_print(tbl['wz'])

if 0:
	#Murty p.261
	tbl = lcp_tbl_pp_create(3)
	lcp_tbl_pp_mqinit(tbl, mat_float([
			[10, 0, -2, 10],
			[2, 0.1, -0.4, 1],
			[0, 0.2, 0.1, -1]
			]) )
	lcp_solve_ppm1_tableau(tbl, {'maxit':10, 'log':False})
	vec_print(tbl['wz'])

if 0:
	#Murty p.262
	tbl = lcp_tbl_pp_create(3)
	lcp_tbl_pp_mqinit(tbl, mat_rational([
			[1, 0, -2, 1],
			[-2, 1, 4, -1],
			[-4, 2, 9, -3]
			]) )
	lcp_solve_ppm1_tableau(tbl, {'maxit':10, 'log':True})
	vec_print(tbl['wz'])

if 0:
	#Murty p.265
	tbl = lcp_tbl_pp_create(4)
	lcp_tbl_pp_mqinit(tbl, mat_float([
			[-1, 2, -1, 1, -4],
			[-2, 0, 2, -1, -4],
			[1, -2, 0, 3, 2],
			[-2, 1, -3, -3, 1]
			]) )
	lcp_solve_ppm1_tableau(tbl, {'maxit':10, 'log':True})
	vec_print(tbl['wz'])

if 1:
	#Murty p.255 
	tbl = lcp_tbl_cpa_create(3)
	lcp_tbl_cpa_mqinit(tbl, mat_float([
			[-1, 0, 0, -1],
			[-2, -1, 0, -1],
			[-2, -2, -1, -1]
			]) )
	lcp_solve_cpa_tableau(tbl, {'maxit':20, 'log':True})
	#print tbl['M']
	#vec_print(tbl['wz'])