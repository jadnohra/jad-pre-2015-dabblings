import sys
import math
import itertools
import copy
import fractions
import operator
import ctypes

def g_num_default(x):
	return float(x)
g_num = g_num_default


def print_tab(list, pref='', sep=' ', post=''):
    col_width = [max(len(str(x)) for x in col) for col in itertools.izip_longest(*list, fillvalue='')]
    for line in list:
        print pref + sep.join("{:>{}}".format(x, col_width[i]) for i,x in enumerate(line)) + post
def mat_print(M):
	print_tab([[str(x) for x in M[r]] for r in range(len(M))])
def log_print(str, log = True):
	if (log):
		print str	
def opt_print(str, opt = {}):
 	log_print(str, opt.get('log', False))
def vec_str(v):
		return '({})'.format(', '.join(str(x) for x in v))	
def vec_print(v, log = True):
	print vec_str(v) if log else 0

def m_isgn(v):
	return int(math.copysign(1, v))
def m_ineg(v):
	return 1 if v < 0 else 0
def vec_create(n,v):
	return [v]*n
def vec_dim(v):
	return len(v)
def vec_is_empty(v):
	return vec_dim(v) == 0
def vec_copy(a):
	return [x for x in a]
def vec_transfer(a, b):
	for i in range(len(a)): b[i] = a[i] 
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
	return min(enumerate(v), key=operator.itemgetter(1)); 
def vec_argmax2(v):
	return max(enumerate(v), key=operator.itemgetter(1)); 
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
def mat_is_empty(M):
	return mat_rows(M) == 0 or mat_cols(M) == 0
def mat_create(r,c,v):
	return [[v]*c for x in xrange(r)]
def mat_zero(r,c):
	return mat_create(r,c, g_num(0))
def mat_identity(r,c):
	M = mat_create(r,c, g_num(0))
	for i in range(r):
		M[i][i] = g_num(1)
	return M
def mat_to_vec(M):
	v = []
	for r in M:
		v.extend(r)
	return v	
def mat_block_identity(M, r,c, n):
	for i in range(n): 
		for j in range(n):
			M[r+i][c+j] = g_num(0)
		M[r+i][c+i] = g_num(1)		
def mat_block_set_col(M, r,c, n, v):
	for j in range(n): M[r+j][c] = v
def mat_col(M, c):
	v = [None]*len(M);
	for i in range(len(M)): v[i] = M[i][c]
	return v
def mat_rput(M, r, coff, v):
	for i in range(len(v)): M[r][coff+i] = v[i]
def mat_swapr(M, i, j):
	r=M[i]; M[i]=M[j]; M[j]=r
def mat_swapc(M, i, j):
	for ri in range(len(M)):
		r = M[ri][i]; M[ri][i] = M[ri][j]; M[ri][j] = r;
def mat_copy(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]; rn = N[i]
		for j in range(c1):
			rn[j] = rm[j]
	return N		
def mat_copy_to(M, N):
	r1 = len(M);c1 = len(M[0]);
	for i in range(r1):
		rm = M[i]; rn = N[i];
		for j in range(c1):
			rn[j] = rm[j]
	return N	
def mat_extract_block(M, roff, rows, coff, cols):
	B = mat_create(rows, cols, None)
	for ri in range(rows):
		for ci in range(cols):
			B[ri][ci]=M[roff+ri][coff+ci]
	return B		
def mat_put_block(M, roff, rows, coff, cols, B):
	for ri in range(rows):
		for ci in range(cols):
			M[roff+ri][coff+ci]=B[ri][ci]
def mat_extract_block2(M, rows, cols):
	B = mat_create(len(rows), len(cols), None)
	for ri in range(len(rows)):
		for ci in range(len(cols)):
			B[ri][ci]=M[rows[ri]][cols[ci]]
	return B			
def mat_put_block2(M, rows, cols, B):
	for ri in range(len(rows)):
		for ci in range(len(cols)):
			M[rows[ri]][cols[ci]]=B[ri][ci]	
def mat_block_implode(M, rblocks, cblocks):
	BM = mat_create(len(rblocks), len(cblocks), None)
	roff = 0; bri = 0;
	for b_rows in rblocks:
		coff = 0; cri = 0;
		for b_cols in cblocks:
			BM[bri][cri] = mat_extract_block(M, roff, b_rows, coff, b_cols)
			coff += b_cols; cri += 1;
		roff += b_rows; bri += 1;
	return BM	
def mat_block_explode(BM, rblocks, cblocks):
	M = mat_create(sum(rblocks), sum(cblocks), None)
	roff = 0; bri = 0;
	for b_rows in rblocks:
		coff = 0; bci = 0;
		for b_cols in cblocks:
			mat_put_block(M, roff, b_rows, coff, b_cols, BM[bri][bci])
			coff += b_cols; bci += 1;
		roff += b_rows; bri += 1;
	return M	
def mat_block_implode2(M, rblocks, cblocks):
	BM = mat_create(len(rblocks), len(cblocks), None)
	for bri in range(len(rblocks)):
		for bci in range(len(cblocks)):
			BM[bri][bci] = mat_extract_block2(M, rblocks[bri], cblocks[bci])
	return BM	
def mat_block_explode2(BM, rblocks, cblocks):
	M = mat_create(sum([len(rows) for rows in rblocks]), sum([len(cols) for cols in cblocks]), None)
	for bri in range(len(rblocks)):
		for bci in range(len(cblocks)):
			mat_put_block2(M, rblocks[bri], cblocks[bci], BM[bri][bci])
	return M
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
	if (len(V2) == 0): return vec_copy(V2)
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

def g_num_rational(x):
	return fractions.Fraction(str(x))

def vec_rational(V):
	n = len(V);
	W = vec_create(n, None)
	for i in range(n):
		W[i] = g_num_rational(V[i])
	return W
def mat_rational(M):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]
		rn = N[i]
		for j in range(c1):
			rn[j] = g_num_rational(rm[j])
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

def lcp_lex(v):
	for e in v:
		if e > 0:
			return 1
		if e < 0:
			return -1
	return 0		

def lcp_tbl_leaving_topmost(tbl, r_cands, col):
	M = tbl['M']; qoff = lcp_tbl_off(tbl, 'q');
	cmp_lambda = lambda x,y: m_isgn(x[0]-y[0]) if (x[1] == y[1]) else m_isgn(x[1]-y[1])
	ratios = sorted([ [ri, M[ri][qoff]/M[ri][col]] for ri in r_cands ], cmp=cmp_lambda)
	#print 'ratios', [[x[0], str(x[1])] for x in ratios]
	return ratios[0][0]

def lcp_tbl_leaving_lexi(tbl, r_cands, col):
	M = tbl['M']; qoff = lcp_tbl_off(tbl, 'q');
	ratios = sorted([ [ri, M[ri][qoff]/M[ri][col]] for ri in r_cands ], key=lambda x: x[1])
	if (len(ratios) <= 1 or ratios[0][1] != ratios[1][1]):
		return ratios[0][0]
	#print 'ratios', [[x[0], str(x[1])] for x in ratios]	
	lex_cands = [el[0] for el in filter(lambda el: el[1]==ratios[0][1], ratios)] 
	lex_ratios = [ [ri, vec_divs(M[ri], M[ri][col])] for ri in lex_cands ]
	lex_ratios = sorted(lex_ratios, cmp=lambda x,y: lcp_lex(vec_sub(y[1], x[1])))
	#print 'lex_ratios', [[x[0], vec_str(x[1])] for x in lex_ratios]
	return lex_ratios[0][0]

def lcp_tbl_leaving(tbl, r_cands, col, opts):
	if opts.get('no-lex', False):
		return lcp_tbl_leaving_topmost(tbl, r_cands, col)
	else:	
		 return lcp_tbl_leaving_lexi(tbl, r_cands, col)

def lcp_tbl_struct(nc, off, end, lbl, init):
	return { 'nc':nc, 'off':off, 'end':end, 'lbl':lbl, 'init':init }

def lcp_tbl_sorted_structs(tbl):
	return sorted(tbl['structs'].values(), key=lambda x: x['off'])

def lcp_tbl_create(n, structs, nrows):
	nc = sum([s['nc'] for s in structs.values()])
	tbl = { 'nrows':nrows, 'n':n, 'nc':nc, 'L':[i for i in range(nrows)], 'M':mat_create(nrows, nc, g_num(0)), 'structs':structs }	
	for s in structs.values():
		if (s['init'] == 'id'):
			mat_block_identity(tbl['M'], 0, s['off'], s['nc'])
		elif (s['init'] == '-en'):
			mat_block_set_col(tbl['M'], 0, s['off'], nrows, g_num(-1))	
	return tbl		

def lcp_tbl_off(tbl, lbl):
	return tbl['structs'][lbl]['off']

def lcp_tbl_col(tbl, lbl):
	 return mat_col(tbl['M'], lcp_tbl_off(tbl, lbl))

def lcp_tbl_lbl(tbl, xi):
	structs = tbl['structs']; sv = lcp_tbl_sorted_structs(tbl);
	for si in range(len(sv)):
		if (xi >= sv[si]['off'] and xi <= sv[si]['end']):
			if (sv[si]['nc'] > 1):
				return '{}{}'.format(sv[si]['lbl'], 1+xi-sv[si]['off'])
			else:
				return sv[si]['lbl']
	return ''
		
def lcp_tbl_lbls(tbl, xis):
	return [lcp_tbl_lbl(tbl, xi) for xi in xis]

def lcp_tbl_lbls_str(tbl, xis):
	return  '({})'.format(', '.join(lcp_tbl_lbls(tbl, xis)))

def lcp_tbl_solution(tbl, lbls, str=False):
	L = tbl['L']; q = lcp_tbl_col(tbl, 'q'); 
	sol = []
	for lbl in lbls:
		s = tbl['structs'][lbl]
		lblsol = [g_num(0)]*s['nc']
		for i in range(len(L)): 
			if (L[i] >= s['off'] and L[i] <= s['end']):
				xi = L[i]-s['off']
				lblsol[xi] = q[i]
		if (str):		
			if (s['nc'] == 1):
				lblsol[i] = '{}={}'.format(s['lbl'], lblsol[i])
			else:	
				for i in range(s['nc']):
					lblsol[i] = '{}{}={}'.format(s['lbl'], i+1, lblsol[i])
		sol.extend(lblsol)
	return sol

def lcp_tbl_print_M(tbl):
	M = tbl['M']; L = tbl['L'];
	head = [['']+lcp_tbl_lbls(tbl, range(len(M[0])))]
	list = [[lcp_tbl_lbl(tbl, L[r])]+[str(x) for x in M[r]] for r in range(len(M))]
	head.extend(list)
	print_tab(head)

def lcp_tbl_pp_struct(n):
	return {'w': lcp_tbl_struct(n, 0, n-1, 'w', 'id'), 
			'z': lcp_tbl_struct(n, n, (2*n)-1, 'z', 0), 
			'q': lcp_tbl_struct(1, (2*n), (2*n), 'q', 0)}
def lcp_tbl_pp_create(n,nrows):
	return lcp_tbl_create(n, lcp_tbl_pp_struct(n), nrows)
def lcp_tbl_pp_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), vec_neg(m))
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q
# solve w-Mz=q
def lcp_tbl_pp_init_Mq(tbl, Mq):
	for r in range(len(Mq)): lcp_tbl_pp_rinit(tbl, r, Mq[r][:-1], Mq[r][-1])
def lcp_tbl_pp_create_Mq(Mq): 
	tbl = lcp_tbl_pp_create(len(Mq[0])-1, len(Mq)); lcp_tbl_pp_init_Mq(tbl, Mq); return tbl;
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
		if all(x >= 0 for x in q): # Success 
			status = 2; break;
		r_cands = [x for x in range(len(q)) if q[x] < 0]
		r,c = r_cands[-1], lcp_tbl_pp_compl(tbl, tbl['L'][r_cands[-1]]) 
		if (M[r][c] == 0): # Failure
			status = 0; break;
		# Pivot
		lcp_tbl_pivot(tbl, r, c)
		opt_print('{}. pvt: {}-{}, {}'.format(it, r,c, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
	if (status == 2):
		tbl['sol'] = lcp_tbl_solution(tbl,['z'])
	else:	
		tbl['sol'] = []
	return (status == 2)

def lcp_solve_ppcd1_tableau(tbl, opts = {}):
	# Dantzig-Cottle Principal Pivoting Method, Murty p.273
	# Processes P matrices.
	#
	M = tbl['M']; qoff = lcp_tbl_off(tbl, 'q'); n = tbl['n']; nrows = tbl['nrows'];
	maxit = opts.get('maxit', 0); it = 0; status = 1;
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		if all(M[x][qoff] >= 0 for x in range(n)): # Success 
			status = 2; break;
		q_cands = [x for x in range(n) if M[x][qoff] < 0]
		r_disting = q_cands[-1]
		c_disting = lcp_tbl_pp_compl(tbl, tbl['L'][r_disting]) 
		r_cands = [r_disting] + [x for x in range(nrows) if M[x][c_disting] > 0 and M[x][qoff] >= 0]
		#mat_print(tbl['M'], '')
		r_block = lcp_tbl_leaving(tbl, r_cands, c_disting, opts); xi_block = tbl['L'][r_block];
		lcp_tbl_pivot(tbl, r_block, c_disting)
		opt_print('{}. M-pvt: {}-{}, {}'.format(it, r_block,c_disting, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
		while ( (lcp_tbl_pp_compl(tbl, xi_block) != c_disting) 
				and (status == 1 and (maxit == 0 or it < maxit)) ):
			it = it + 1
			c_driv = lcp_tbl_pp_compl(tbl, xi_block)
			r_cands = [r_disting] + [x for x in range(nrows) if M[x][c_driv] > 0 and M[x][qoff] >= 0]
			r_block = lcp_tbl_leaving(tbl, r_cands, c_driv, opts); xi_block = tbl['L'][r_block];
			lcp_tbl_pivot(tbl, r_block, c_driv)
			opt_print('{}. m-pvt: {}-{}, {}'.format(it, r_block,c_driv, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
	if (status == 2):
		tbl['sol'] = lcp_tbl_solution(tbl,['z'])
	else:	
		tbl['sol'] = []
	return (status == 2)


def lcp_tbl_ppcd2_struct(n, nrows):
	return {'w': lcp_tbl_struct(nrows, 0, nrows-1, 'w', 'id'), 
			'z': lcp_tbl_struct(n, nrows, nrows+(n-1), 'z', 0), 
			'q': lcp_tbl_struct(1, (nrows+n), (nrows+n), 'q', 0),
			'b': lcp_tbl_struct(1, (nrows+n)+1, (nrows+n)+1, 'b', 0)}
def lcp_tbl_ppcd2_create(n,nrows):
	return lcp_tbl_create(n, lcp_tbl_ppcd2_struct(n, nrows), nrows)
def lcp_tbl_ppcd2_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), vec_neg(m))
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q
	tbl['M'][r][lcp_tbl_off(tbl, 'b')] = q
# solve w-Mz=q
def lcp_tbl_ppcd2_init_Mq(tbl, Mq):
	for r in range(len(Mq)): lcp_tbl_ppcd2_rinit(tbl, r, Mq[r][:-1], Mq[r][-1])
def lcp_tbl_ppcd2_create_Mq(Mq): 
	tbl = lcp_tbl_ppcd2_create(len(Mq[0])-1, len(Mq)); lcp_tbl_ppcd2_init_Mq(tbl, Mq); return tbl;
def lcp_tbl_ppcd2_compl(tbl, xi):
	return (xi +  tbl['n']) % (2*tbl['n'])

def lcp_tbl_ppcd2_pivot(tbl, plr, pli, pec, plv, pev):
	n = tbl['n']; M = tbl['M']; boff = lcp_tbl_off(tbl, 'b');
	lcp_tbl_pivot(tbl, plr, pec)
	#lcp_tbl_print_M(tbl)
	for ri in range(n): M[ri][boff] = M[ri][boff] - (M[ri][pli] * plv)
	#print '>>', plr, pev
	M[plr][boff] = pev
	tbl['lb'][pli] = plv
		
def lcp_tbl_ppcd2_leaving_topmost(tbl, r_cands, col, lbda, r_disting, i_disting):
	M = tbl['M']; L = tbl['L']; boff = lcp_tbl_off(tbl, 'b');
	cmp_lambda = lambda x,y: m_isgn(x[0]-y[0]) if (x[1] == y[1]) else m_isgn(x[1]-y[1])
	ratios = sorted([[ri, (M[ri][boff]-m_ineg(M[ri][boff])*lbda)/M[ri][col]] for ri in r_cands], cmp=cmp_lambda)
	for el in filter(lambda el: el[1]==ratios[0][1] and L[el[0]] == i_disting, ratios):
		return el
	return ratios[0]

def lcp_tbl_ppcd2_is_block(tbl, r, c): return tbl['M'][r][c] != 0
def lcp_tbl_ppcd2_is_upper_block(tbl, r, c): return tbl['M'][r][c] > 0
def lcp_tbl_ppcd2_is_lower_block(tbl, r, c): return tbl['M'][r][c] < 0
def lcp_tbl_ppcd2_is_no_block(tbl, r, c): return tbl['M'][r][c] == 0

def lcp_tbl_ppcd2_block_change(tbl, r, c):
	M = tbl['M']; L=tbl['L']; boff = lcp_tbl_off(tbl, 'b'); lbda=tbl['lbda']; mrc = M[r][c];
	if (mrc == 0): 
		return [None, None, M[r][boff]]
	if (mrc > 0):
		low_block = m_ineg(M[r][boff])*lbda
		change = (M[r][boff] - low_block) / mrc
		return [None, change, low_block]
	if (mrc < 0):
		zero_block = g_num(0)
		change = (M[r][boff] - zero_block) / mrc
		return [change, None, zero_block]	

def lcp_solve_ppcd2_tableau(tbl, opts = {}):
	# Asymmetric PPM, Cottle p.260, 336
	# Dantzig-Cottle Principal Pivoting Method, Murty p.276.
	# Processes P, PSD matrices.
	#
	# TODO: lexi
	#
	M = tbl['M']; n = tbl['n']; nrows = tbl['nrows']; qoff = lcp_tbl_off(tbl, 'q'); boff = lcp_tbl_off(tbl, 'b');
	lb = [g_num(0)]*(nrows+n); tbl['lb'] = lb; 
	maxit = opts.get('maxit', 0); it = 0; status = 1;
	lbda = g_num(int(math.ceil(min(M[x][qoff] for x in range(nrows)) - 1)))
	tbl['lbda'] = lbda
	if opts.get('log', False):
		lcp_tbl_print_M(tbl); 
	opt_print('lbda {}'.format(lbda), opts)	
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		if all(M[x][qoff] >= 0 for x in range(nrows)): # Success 
			status = 2; break;
		# Determine distinguished
		r_disting = -1; i_disting = -1;
		r_disting = next((x for x in range(nrows) if M[x][boff] < 0), -1)
		if (r_disting != -1):
			i_disting = tbl['L'][r_disting]; i_driv = lcp_tbl_pp_compl(tbl, i_disting);
		else:
			first_lbda = next((x for x in range(len(lb)) if lb[x] == lbda), -1)
			if (first_lbda == -1) or (all(M[x][qoff] >= 0 for x in range(nrows))):
				status = 2; break;
			i_disting = first_lbda; i_driv = i_disting;
			# Pivot-less one-step major cycle
			if 0:
				print 'disting', lcp_tbl_lbl(tbl, i_disting)
				is_blocked = False
				for ri in range(nrows):
					change = lcp_tbl_ppcd2_block_change(tbl, ri, i_driv)
					if (change[1] and (change[1] + lbda) < 0):
						is_blocked = True
						break
				if (not is_blocked):
					for ri in range(nrows):
						M[ri][boff] += M[ri][i_driv]*lbda # or is it -=
					lb[i_driv] = g_num(0)
					r_disting = -1
		if opts.get('log', False):			
			print 'disting', lcp_tbl_lbl(tbl, i_disting), 'driving', lcp_tbl_lbl(tbl, i_driv)
		while (status == 1 and (maxit == 0 or it < maxit)):
			it = it + 1
			# Determine blocking
			r_block = -1; driv_change = None; block_val = None; # TODO: lexi
			for ri in range(nrows):
				change = lcp_tbl_ppcd2_block_change(tbl, ri, i_driv)
				if change[1] and ((r_block == -1) or (change[1] < driv_change)):
					r_block = ri; driv_change = change[1]; block_val = change[2];
			if (r_disting != -1):		
				disting_change = lcp_tbl_ppcd2_block_change(tbl, r_disting, i_driv)
				if disting_change[0] and ((r_block == -1) or (disting_change[0] < driv_change)):
					r_block = r_disting; driv_change = disting_change[0]; block_val = disting_change[2];
			if opts.get('log', False):
				print 'drive:', lcp_tbl_lbl(tbl, i_driv), driv_change, block_val, driv_change+lb[i_driv] if (not driv_change is None) else None
			if (r_block == -1):
				status = 0; break;
			# Pivot
			i_block = tbl['L'][r_block]	
			lcp_tbl_ppcd2_pivot(tbl, r_block, i_block, i_driv, block_val, driv_change+lb[i_driv])
			if (r_disting == -1):
				r_disting = r_block
			opt_print('{}. pvt: {}-{}, {}'.format(it, r_block,i_driv, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
			if opts.get('log', False):
				lcp_tbl_print_M(tbl); vec_print(tbl['lb']);
			# Decide next operation
			if (i_block != i_disting):
				i_driv = lcp_tbl_pp_compl(tbl, i_block) 
			else:	
				break
			# print r_block, driv_bound, lcp_tbl_lbl(tbl, tbl['L'][r_block])

	if (status == 2):
		tbl['sol'] = lcp_tbl_solution(tbl,['z'])
	else:	
		tbl['sol'] = []
	return (status == 2)

def lcp_tbl_cpa_struct(n):
	return {'w': lcp_tbl_struct(n, 0, n-1, 'w', 'id'), 
			'z': lcp_tbl_struct(n, n, (2*n)-1, 'z', 0), 
			'z0': lcp_tbl_struct(1, (2*n), (2*n), 'z0', '-en'), 
			'q': lcp_tbl_struct(1, (2*n)+1, (2*n)+1, 'q', 0)}

def lcp_tbl_cpa_create(n,nrows): 
	return lcp_tbl_create(n, lcp_tbl_cpa_struct(n), nrows)
# solve w-Mz=q
def lcp_tbl_cpa_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), vec_neg(m))
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q
# solve w-Mz=q
def lcp_tbl_cpa_init_Mq(tbl, Mq):
	for r in range(len(Mq)): lcp_tbl_cpa_rinit(tbl, r, Mq[r][:-1], Mq[r][-1])
def lcp_tbl_cpa_create_Mq(Mq): 
	tbl = lcp_tbl_cpa_create(len(Mq[0])-1, len(Mq)); lcp_tbl_cpa_init_Mq(tbl, Mq); return tbl;

def lcp_solve_cpa_tableau(tbl, opts = {}):
	# Complementary Pivot Algorithm, Murty p.66, opt. p.81
	#
	#Initialization, p.71
	if opts.get('log', False):
		lcp_tbl_print_M(tbl)
	status = 1
	r,qr = vec_argmin2(lcp_tbl_col(tbl, 'q'))
	if (qr >= 0):
		lcp_tbl_solution(tbl, ['z'])
		status = 2
	else:
		c = lcp_tbl_off(tbl, 'z0')
		dropped = tbl['L'][r];
		lcp_tbl_pivot(tbl, r, c); 
		opt_print('0. pvt: {}-{}, {}'.format(r,c, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
		if opts.get('log', False):
			lcp_tbl_print_M(tbl)
	maxit = opts.get('maxit', 0); it = 0; 
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		L = tbl['L']; q = lcp_tbl_col(tbl, 'q'); z0i = lcp_tbl_off(tbl, 'z0');
		z0r = L.index(z0i) if z0i in L else None
		if (z0r is None or q[z0r] == 0): # Success, p.74
			status = 2; break;
		M = tbl['M']
		c = lcp_tbl_pp_compl(tbl, dropped)
		r_cands = [x for x in range(tbl['nrows']) if M[x][c] > 0]
		if (len(r_cands) == 0): # Failure, p.68
			status = 0; break;
		r = lcp_tbl_leaving(tbl, r_cands, c, opts)
		dropped = tbl['L'][r]; 
		lcp_tbl_pivot(tbl, r, c); 
		opt_print('{}. pvt: {}-{}, {}'.format(it, r,c, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
		if opts.get('log', False):
			lcp_tbl_print_M(tbl)
	if (status == 2):
		tbl['sol'] = lcp_tbl_solution(tbl, ['z'])
	else:	
		tbl['sol'] = []
	return (status == 2)

def mlcp_to_lcp_Mq(Mq, bounds, subst):
	# TODO: handle unbounded variables using a phase 1 algorithm that drives them 
	# to basic variables, trying both directions, ending with an almost-compl basis.
	# (splitting each unbounded into 2, then choosing the one that can be a successful driver)
	N = []
	for bi in xrange(len(bounds)):
		b = bounds[bi]
		if b[0] is not None:
			subst[bi] = [g_num(1), b[0]]
			for ri in xrange(len(Mq)):
				Mq[ri][-1] -= Mq[ri][bi]*b[0]
			if b[1] is not None:
				N.append(bi)
		elif b[1] is not None:
			subst[bi] = [g_num(-1), b[1]]
			for ri in xrange(len(Mq)):
				Mq[ri][-1] -= Mq[ri][bi]*b[1]
				Mq[ri][bi] = -Mq[ri][bi]
	cMq = mat_create(len(Mq)+len(N), len(bounds)+len(N)+1, g_num(0))
	mat_copy_to(Mq, cMq)
	mat_swapc(cMq, len(bounds), len(cMq[0])-1)
	off = [len(Mq), len(bounds)]
	for i in range(len(N)):
		bi = N[i]
		#cMq[off[0]+i][bi] = g_num(1)
		#cMq[off[0]+i][off[1]+i] = g_num(1)
		cMq[off[0]+i][bi] = g_num(-1)
		cMq[i][off[1]+i] = g_num(1)
		cMq[off[0]+i][-1] = bounds[bi][1]-bounds[bi][0]
	return cMq	

def mlcp_subst_sol(Mq, lcp_sol, subst):
	n = len(Mq[0])-1
	mlcp_sol = []
	for i in range(n):
		mlcp_sol.append(lcp_sol[i] * subst[i][0] + subst[i][1])
	return mlcp_sol	

def mlcp_to_lcp_Mq2(Mq, bounds, subst):
	# Extension to Lemke, Judice. With the sign fix of the erratum: -[qj3 + Mj3j3 * bj3]
	def convertJ(Mq, J1, J2, J3, bj1, bj3):
		def mat_copy2(M):
			return mat_copy(M) if not mat_is_empty(M) else [[]]	
		def mat_add2(M, N):
			return mat_add(M, N) if not mat_is_empty(N) and not mat_is_empty(M) else mat_copy2(M)
		def mat_neg2(M):
			return mat_neg(M) if not mat_is_empty(M) else mat_copy2(M)
		def mat_mulv2(M, v):
			return mat_mulv(M, v) if not mat_is_empty(M) and not vec_is_empty(v) else []
		j1 = len(J1); j2 = len(J2); j3 = len(J3);
		irblocks = [J1, J2, J3]; icblocks = [J1, J2, J3, [len(Mq[0])-1]];
		BMq = mat_block_implode2(Mq, irblocks, icblocks)
		#print BMq[0][0], BMq[0][1], BMq[0][2]
		#print BMq[1][0], BMq[1][1], BMq[1][2]
		#print BMq[2][0], BMq[2][1], BMq[2][2]
		cBMq = mat_create(4, 5, None)
		cBMq[0][0] = mat_copy2(BMq[0][0]); cBMq[0][1] = mat_copy2(BMq[0][1]); cBMq[0][2] = mat_neg2(BMq[0][2]);
		cBMq[0][3] = mat_identity(j1, j1)
		cBMq[0][-1] = mat_add2(BMq[0][-1], [mat_mulv2(BMq[0][2], bj3)])
		cBMq[1][0] = mat_copy2(BMq[1][0]); cBMq[1][1] = mat_copy2(BMq[1][1]); cBMq[1][2] = mat_neg2(BMq[1][2]);
		cBMq[1][3] = mat_zero(j2, j1)
		cBMq[1][-1] = mat_add2(BMq[1][-1], [mat_mulv2(BMq[1][2], bj3)])
		cBMq[2][0] = mat_neg2(BMq[2][0]); cBMq[2][1] = mat_neg2(BMq[2][1]); cBMq[2][2] = mat_copy2(BMq[2][2]);
		cBMq[2][3] = mat_zero(j3, j1)
		cBMq[2][-1] = mat_neg2(mat_add2(BMq[2][-1], [mat_mulv2(BMq[2][2], bj3)]))
		cBMq[3][0] = mat_neg2(mat_identity(j1, j1))
		cBMq[3][1] = mat_zero(j1, j2); cBMq[3][2] = mat_zero(j1, j3); cBMq[3][3] = mat_zero(j1, j1)
		cBMq[3][-1] = [vec_copy(bj1)]
		erblocks = [j1, j2, j3, j1]; ecblocks = [j1, j2, j3, j1, 1];
		return mat_block_explode(cBMq, erblocks, ecblocks)
	# working copies
	bounds = vec_copy(bounds)
	Mq = mat_copy(Mq)
	# preprocess
	id_subst = [g_num(1), g_num(0)]
	for i in range(len(subst)):
		subst[i] = id_subst
	for bi in xrange(len(bounds)):
		b = bounds[bi]
		if b[0] is not None and b[1] is not None:
			if b[0] != 0:
				a = bounds[bi][0]
				subst[bi] = [g_num(1), a]
				for ri in range(len(Mq)):
					Mq[ri][-1] += Mq[ri][bi]*a
				bounds[bi][1] -= a; bounds[bi][0] -= a;
		elif b[1] is not None:
			subst[bi] = [g_num(-1), bounds[bi][1]]
	# map		
	G = []; J1 = []; J2 = []; J3 = []; bj1 = []; bj3 = [];
	for bi in xrange(len(bounds)):
		b = bounds[bi]
		if b[0] is not None and b[1] is not None:
			J1.append(bi); bj1.append(b[1]);
		elif b[0] is not None:
			J2.append(bi)
		elif b[1] is not None:
			J3.append(bi); bj3.append(b[1]);
		else:
			G.append(bi)
	# handle unbounded
	if (len(G) > 0):
		return None
	# convert	
	return convertJ(Mq, J1, J2, J3, bj1, bj3)

def get_test_Mq(test):
	Mq_tbl = {
		'Murty p.255':	# P
		[
			[1, 0, 0, -1],
			[2, 1, 0, -1],
			[2, 2, 1, -1] 
		],
		'Murty p.261':	# P
		[
			[10, 0, -2, 10],
			[2, 0.1, -0.4, 1],
			[0, 0.2, 0.1, -1]
		],
		'Murty p.262': # P
		[
			[1, 0, -2, 1],
			[-2, 1, 4, -1],
			[-4, 2, 9, -3]
		],
		'Murty p.265':	# PSD
		[
			[1, -2, 1, -1, -4],
			[2, 0, -2, 1, -4],
			[-1, 2, 0, -3, 2],
			[2, -1, 3, 3, 1]
		],
		'Murty p.268':	# PSD, no-sol
		[
			[1, -1, 1, 1, 2],
			[1, 1, 0, 2, 0],
			[-1, 0, 1, 0, -2],
			[-1, -2, 0, 0, -1]
		],
		'Murty p.77':
		[
			[1, -1, -1, -1, 3],
			[-1, 1, -1, -1, 5],
			[1, 1, 2, 0, -9],
			[1, 1, 0, 2, -5]
		],
		'Murty p.79':
		[
			[-1, 0, -3, -3],
			[1, -2, -5, -2],
			[-2, -1, -2, -1],
		],
		'Murty p.79 (mod)':
		[
			[1, 0, -3, -3],
			[-1, -2, -5, -2],
			[2, -1, -2, -1],
		],
		'Murty p.81':
		[
			[1, 0, 0, -8],
			[2, 1, 0, -12],
			[2, 2, 1, -14],
		],
		'Murty p.83':
		[
			[1, 2, 0, -1],
			[0, 1, 2, -1],
			[2, 0, 1, -1],
		],
		'Murty p.97':	# pca-ray-termin
		[
			[-1.5, 2, -5],
			[-4, 4, 17]
		],
		'Murty p.107':
		[
			[2, 1, 1, -4],
			[1, 2, 1, -5],
			[1, 1, 2, -1],
		],
		'Cottle p.258':
		[
			[0, -1, 2, -3],
			[2, 0, -2, 6],
			[-1, 1, 0, -1],
		],
		'test 1':
		[
			[-1, 0, 4],
			[0, 1, 16],
			#[4, 1, 0],
			#[-4, -1, 0],
		]
	}
	return Mq_tbl[test]

def test_ppm1(tests, opts = {}):
	for test in tests:
		tbl = lcp_tbl_pp_create_Mq( mat_rational(get_test_Mq(test)) )
		lcp_solve_ppm1_tableau(tbl, opts)
		vec_print(tbl['sol'])

def test_cpa(tests, opts = {}):
	for test in tests:
		tbl = lcp_tbl_cpa_create_Mq( mat_rational(get_test_Mq(test)) )
		lcp_solve_cpa_tableau(tbl, opts)
		vec_print(tbl['sol'])	

def test_ppcd1(tests, opts = {}):
	for test in tests:
		tbl = lcp_tbl_pp_create_Mq( mat_rational(get_test_Mq(test)) )
		lcp_solve_ppcd1_tableau(tbl, opts)
		vec_print(tbl['sol'])		

def test_ppcd2(tests, opts = {}):
	for test in tests:
		tbl = lcp_tbl_ppcd2_create_Mq( mat_rational(get_test_Mq(test)) )
		lcp_solve_ppcd2_tableau(tbl, opts)
		vec_print(tbl['sol'])

def run_tests():
	if 0:
		tbl = lcp_tbl_pp_create_Mq(mat_rational([ [-1, 1, -1, 3], [-1, 1, -1, 5], [-1, -1, -1, 9] ]) )
		mat_print(tbl['M']); lcp_tbl_pivot(tbl, 2, 5); mat_print(tbl['M']);
	if 0: test_ppm1(['Murty p.255'], {'maxit':20, 'log':True})
	if 0: test_ppm1(['Murty p.261'], {'maxit':10, 'log':False})
	if 0: test_ppm1(['Murty p.262'], {'maxit':10, 'log':True})
	if 0: test_ppm1(['Murty p.265'], {'maxit':10, 'log':True})
	if 0: test_cpa(['Murty p.77'], {'maxit':20, 'log':True})
	if 0: test_cpa(['Murty p.79'], {'maxit':4, 'log':True})
	if 0: test_cpa(['Murty p.79 (mod)'], {'maxit':4, 'log':True})
	if 0: test_cpa(['Murty p.81'], {'maxit':20, 'log':True})
	if 0: test_cpa(['Murty p.83'], {'maxit':10, 'log':True, 'no-lex':True}); test_cpa('Murty p.83', {'maxit':10, 'log':True}); 
	if 0: test_cpa(['Murty p.97'], {'maxit':20, 'log':True})
	if 0: test_cpa(['Murty p.107'], {'maxit':20, 'log':True})
	if 0: test_ppcd1(['Murty p.255'], {'maxit':20, 'log':True})
	if 0: test_ppcd1(['Murty p.261'], {'maxit':20, 'log':True})
	if 0: test_ppcd1(['Murty p.262'], {'maxit':20, 'log':True})
	if 0: test_ppcd2(['Murty p.255', 'Murty p.261', 'Murty p.262'], {'maxit':20, 'log':False})
	if 0: test_ppcd2(['Cottle p.258'], {'maxit':20, 'log':True})
	if 0: test_ppcd2(['Murty p.265'], {'maxit':20, 'log':True})
	if 0: 
		bounds = [[-4, -2], [0, None]]; subst = [None]*len(bounds)
		Mq = get_test_Mq('test 1')
		cMq = mlcp_to_lcp_Mq(mat_rational(Mq), bounds, subst)
		mat_print(cMq)
		tbl = lcp_tbl_ppcd2_create_Mq( mat_rational(cMq) )
		lcp_solve_ppcd2_tableau(tbl, {})
		#tbl = lcp_tbl_cpa_create_Mq( mat_rational(cMq) )
		#lcp_solve_cpa_tableau(tbl, {})
		vec_print(tbl['sol'])
		vec_print(mlcp_subst_sol(Mq, tbl['sol'], subst))
	if 1: 
		bounds = [[-4, -2], [0, None]]; subst = [None]*len(bounds)
		Mq = get_test_Mq('test 1')
		cMq = mlcp_to_lcp_Mq2(mat_rational(Mq), bounds, subst)
		mat_print(cMq)
		

def solve_mlcp_cdll_mprog(Mq, bounds, opts = {}):
	def ctypes_flist(l): cltype = ctypes.c_float*len(l); return cltype(*l);
	def conv_bound_lo(b): return float('-inf') if b is None else float(b)
	def conv_bound_hi(b): return float('inf') if b is None else float(b)
	lib = ctypes.cdll.LoadLibrary('D:/jad/depots/sandy1/depot/Other/Personal/Jad.Nohra/Lab/mprog/out/mprog_dll.dll')
	#print lib.multiply(2, 2)
	n = mat_rows(Mq)
	A = mat_to_vec(mat_block_implode(Mq, [n], [n, 1])[0][0])
	b = vec_neg(mat_col(Mq, n))
	lo = [conv_bound_lo(x[0]) for x in bounds]
	hi = [conv_bound_hi(x[1]) for x in bounds]
	csol = ctypes_flist([0]*2)
	if opts.get('log', False):
		print A,b,lo,hi
	#solved = lib.solveOdeDantzigLCP(2, ctypes_flist([1.0, 2.0, 3.0, 4.0]), ctypes_flist([1,-1]), ctypes_flist([1, float('inf')]), ctypes_flist([1, float('inf')]), csol)
	solved = lib.solveOdeDantzigLCP(n, ctypes_flist(A), ctypes_flist(b), ctypes_flist(lo), ctypes_flist(hi), csol)
	sol = [float(x) for x in csol]
	if opts.get('log', False):
		print solved, sol
	if not solved:
		sol = []
	return sol

def solve_mlcp(Mq, bounds, opts = {}):
	algo = opts.get('algo', 'cpa')
	if algo == 'ode':
		return solve_mlcp_cdll_mprog(Mq, bounds, opts)
	subst = [None]*len(bounds)
	subst2 = [None]*len(bounds)
	cMq2 = mlcp_to_lcp_Mq2(mat_copy(Mq), vec_copy(bounds), subst2)
	cMq = mlcp_to_lcp_Mq(Mq, bounds, subst)
	if opts.get('log', False):
		print 'Mq'; mat_print(Mq)
		print 'cMq'; mat_print(cMq); print subst;
		print 'cMq2'; mat_print(cMq2); print subst2;
	if algo == 'cpa':
		tbl = lcp_tbl_cpa_create_Mq(cMq)
		lcp_solve_cpa_tableau(tbl, opts)
	elif algo == 'ppm1':
		tbl = lcp_tbl_pp_create_Mq(cMq)
		lcp_solve_ppm1_tableau(tbl, opts)	
	elif algo == 'ppcd1':
		tbl = lcp_tbl_pp_create_Mq(cMq)
		lcp_solve_ppcd1_tableau(tbl, opts)		
	else:
		tbl = lcp_tbl_ppcd2_create_Mq(cMq)
		lcp_solve_ppcd2_tableau(tbl, opts)
	sol = []
	if len(tbl['sol']) > 0: 
		sol = mlcp_subst_sol(Mq, tbl['sol'], subst)
	if opts.get('log', False):
		vec_print(tbl['sol'])
		vec_print(sol)
	return sol	

def solve_mlcp_lists(n, list_M, list_q, list_lo, list_hi, mul_q, opts = {}):
	def conv_bound(b): return None if (float('inf') == b or float('-inf') == b) else float(b)
	Mq = mat_create(n, n+1, None)
	bounds = [None]*n
	li = 0
	for ri in range(n):
		for ci in range(n):
			Mq[ri][ci] = list_M[ri*n+ci]
		Mq[ri][n] = mul_q*list_q[ri]
	li = 0
	for ri in range(n):
		bounds[ri] = [conv_bound(list_lo[ri]), conv_bound(list_hi[ri])]
	return solve_mlcp(Mq, bounds, opts)	
	

def solve_mlcp_file(fin, fout, opts = {}):
	with open(fin, 'r') as fi: 
		def read_bound(b): return None if 'inf' in b else float(b)
		def read_bounds(b): return [read_bound(b[0]), read_bound(b[1])]
		mode = 'pref'; Mq = []; bounds = [];
		for line in fi:
			line = "".join(line.split())
			if mode == 'pref':
				mode = 'n'
			elif mode == 'n':
				n = int(line)
				mode = 'Mq'
			elif mode == 'Mq':
				Mq.append([float(x) for x in line.split(',')])
				if (len(Mq) == n):
					mode = 'bds'
			elif mode == 'bds':
				bd = line.split(',')
				bounds.append(read_bounds(bd))
	sol = solve_mlcp(Mq, bounds, opts)
	if (fout is not None):	
		with open(fout, 'w') as fo: fo.write(','.join([str(x) for x in sol]))
	else:
		vec_print(sol)

if 0:
	M = [
		[1, -1, -1, -1, 3],
		[-1, 1, -1, -1, 5],
		[1, 1, 2, 0, -9],
		[1, 1, 0, 2, -5]
	]
	BM = mat_block_implode(M, [2, 2], [2, 3])
	print BM
	MM = mat_block_explode(BM, [2, 2], [2, 3])
	print MM

	BM = mat_block_implode2(M, [[0,3], [1,2]], [[0,1], [2,4,3]])
	print BM
	MM = mat_block_explode2(BM, [[0,3], [1,2]], [[0,1], [2,4,3]])
	print MM
	MM = mat_block_explode(BM, [2, 2], [2, 3])
	print MM
elif  hasattr(sys, 'argv'):
	if '-tests' in sys.argv:
		g_num = g_num_rational
		run_tests()
	elif '-in' in sys.argv:
		g_num = g_num_default
		fip = 1 + (sys.argv.index('-in') if '-in' in sys.argv else -2)
		fop = 1 + (sys.argv.index('-out') if '-out' in sys.argv else -2)
		algo = 1 + (sys.argv.index('-algo') if '-algo' in sys.argv else -2)
		log = '-log' in sys.argv
		if fip >= 0:
			fip = sys.argv[fip] if fip >= 0 else None
			fop = sys.argv[fop] if fop >= 0 else None
			algo = sys.argv[algo] if algo >= 0 else 'cpa'
			opts = {'log':log, 'algo':algo}
			solve_mlcp_file(fip, fop, opts)
