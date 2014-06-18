import math
import itertools
import copy
import fractions
import operator

def g_num_default(x):
	return x
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

def lcp_tbl_create(n, structs):
	nc = sum([s['nc'] for s in structs.values()])
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
def lcp_tbl_pp_create(n):
	return lcp_tbl_create(n, lcp_tbl_pp_struct(n))
def lcp_tbl_pp_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), vec_neg(m))
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q
# solve w-Mz=q
def lcp_tbl_pp_init_Mq(tbl, Mq):
	for r in range(len(Mq)): lcp_tbl_pp_rinit(tbl, r, Mq[r][:-1], Mq[r][-1])
def lcp_tbl_pp_create_Mq(Mq): 
	tbl = lcp_tbl_pp_create(len(Mq[0])-1); lcp_tbl_pp_init_Mq(tbl, Mq); return tbl;
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
	M = tbl['M']; qoff = lcp_tbl_off(tbl, 'q'); n = tbl['n'];
	maxit = opts.get('maxit', 0); it = 0; status = 1;
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		if all(M[x][qoff] >= 0 for x in range(n)): # Success 
			status = 2; break;
		q_cands = [x for x in range(n) if M[x][qoff] < 0]
		r_disting = q_cands[-1]
		c_disting = lcp_tbl_pp_compl(tbl, tbl['L'][r_disting]) 
		r_cands = [r_disting] + [x for x in range(n) if M[x][c_disting] > 0 and M[x][qoff] >= 0]
		#mat_print(tbl['M'], '')
		r_block = lcp_tbl_leaving(tbl, r_cands, c_disting, opts); xi_block = tbl['L'][r_block];
		lcp_tbl_pivot(tbl, r_block, c_disting)
		opt_print('{}. M-pvt: {}-{}, {}'.format(it, r_block,c_disting, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
		while ( (lcp_tbl_pp_compl(tbl, xi_block) != c_disting) 
				and (status == 1 and (maxit == 0 or it < maxit)) ):
			it = it + 1
			c_driv = lcp_tbl_pp_compl(tbl, xi_block)
			r_cands = [r_disting] + [x for x in range(n) if M[x][c_driv] > 0 and M[x][qoff] >= 0]
			r_block = lcp_tbl_leaving(tbl, r_cands, c_driv, opts); xi_block = tbl['L'][r_block];
			lcp_tbl_pivot(tbl, r_block, c_driv)
			opt_print('{}. m-pvt: {}-{}, {}'.format(it, r_block,c_driv, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
	if (status == 2):
		tbl['sol'] = lcp_tbl_solution(tbl,['z'])
	else:	
		tbl['sol'] = []
	return (status == 2)


def lcp_tbl_ppcd2_struct(n):
	return {'w': lcp_tbl_struct(n, 0, n-1, 'w', 'id'), 
			'z': lcp_tbl_struct(n, n, (2*n)-1, 'z', 0), 
			'q': lcp_tbl_struct(1, (2*n), (2*n), 'q', 0),
			'b': lcp_tbl_struct(1, (2*n)+1, (2*n)+1, 'b', 0)}
def lcp_tbl_ppcd2_create(n):
	return lcp_tbl_create(n, lcp_tbl_ppcd2_struct(n))
def lcp_tbl_ppcd2_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), vec_neg(m))
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q
	tbl['M'][r][lcp_tbl_off(tbl, 'b')] = q
# solve w-Mz=q
def lcp_tbl_ppcd2_init_Mq(tbl, Mq):
	for r in range(len(Mq)): lcp_tbl_ppcd2_rinit(tbl, r, Mq[r][:-1], Mq[r][-1])
def lcp_tbl_ppcd2_create_Mq(Mq): 
	tbl = lcp_tbl_ppcd2_create(len(Mq[0])-1); lcp_tbl_ppcd2_init_Mq(tbl, Mq); return tbl;
def lcp_tbl_ppcd2_compl(tbl, xi):
	return (xi +  tbl['n']) % (2*tbl['n'])

def lcp_tbl_ppcd2_pivot(tbl, plr, pli, pec, plv):
	n = tbl['n']; M = tbl['M']; boff = lcp_tbl_off(tbl, 'b');
	lcp_tbl_pivot(tbl, plr, pec)
	tbl['lb'][pli] = plv
	for ri in range(n): M[ri][boff] = M[ri][boff] - (M[ri][pli] * plv)
		
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
		change = (M[r][boff] - low_block ) / mrc
		return [None, change, low_block]
	if (mrc < 0):
		zero_block = g_num(0)
		change = (M[r][boff] - zero_block ) / mrc
		return [change, None, zero_block]	

def lcp_solve_ppcd2_tableau(tbl, opts = {}):
	# Asymmetric PPM, Cottle p.260, 336
	# Dantzig-Cottle Principal Pivoting Method, Murty p.276.
	# Processes P, PSD matrices.
	#
	#TODO: lexi
	#
	M = tbl['M']; n = tbl['n']; qoff = lcp_tbl_off(tbl, 'q'); boff = lcp_tbl_off(tbl, 'b');
	lb = [g_num(0)]*(2*n); tbl['lb'] = lb; 
	maxit = opts.get('maxit', 0); it = 0; status = 1;
	lbda = g_num(int(math.ceil(min(M[x][qoff] for x in range(n)) - 1)))
	tbl['lbda'] = lbda
	print 'lbda', lbda
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		if all(M[x][qoff] >= 0 for x in range(n)): # Success 
			status = 2; break;
		# Determine distinguished
		#r_disting = -1
		#while (r_disting == -1) and (status == 1) and (maxit == 0 or it < maxit):
		if True:
			r_disting = next((x for x in range(n) if M[x][boff] < 0), -1)
			if (r_disting != -1):
				i_disting = tbl['L'][r_disting]; i_driv = lcp_tbl_pp_compl(tbl, i_disting);
			else:
				first_lbda = next((x for x in range(len(lb)) if lb[x] == lbda), -1)
				print 'first_lbda', first_lbda
				if (first_lbda == -1) or (all(M[x][qoff] >= 0 for x in range(n))):
					status = 2; break;
				i_disting = first_lbda; i_driv = i_disting;
				# Pivot-less one-step major cycle
				if 0:
					print 'disting', lcp_tbl_lbl(tbl, i_disting)
					is_blocked = False
					for ri in range(n):
						change = lcp_tbl_ppcd2_block_change(tbl, ri, i_driv)
						if (change[1] and (change[1] + lbda) < 0):
							is_blocked = True
							break
					if (not is_blocked):
						for ri in range(n):
							M[ri][boff] += M[ri][i_driv]*lbda # or is it -=
						lb[i_driv] = g_num(0)
						r_disting = -1
		print 'disting', lcp_tbl_lbl(tbl, i_disting), 'driving', lcp_tbl_lbl(tbl, i_driv)
		while (status == 1 and (maxit == 0 or it < maxit)):
			it = it + 1
			# Determine blocking
			r_block = -1; driv_change = None; block_val = None; # TODO: lexi
			for ri in range(n):
				change = lcp_tbl_ppcd2_block_change(tbl, ri, i_driv)
				if change[1] and ((r_block == -1) or (change[1] < driv_change)):
					r_block = ri; driv_change = change[1]; block_val = change[2];
			if (r_disting != -1):		
				disting_change = lcp_tbl_ppcd2_block_change(tbl, r_disting, i_driv)
				if disting_change[0] and ((r_block == -1) or (disting_change[0] < driv_change)):
					r_block = r_disting; driv_change = disting_change[0]; block_val = disting_change[2];
			print 'drive:', driv_change, block_val
			if (r_block == -1):
				status = 0; break;
			# Pivot
			i_block = tbl['L'][r_block]	
			lcp_tbl_ppcd2_pivot(tbl, r_block, i_block, i_driv, block_val)
			opt_print('{}. pvt: {}-{}, {}'.format(it, r_block,i_driv, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
			lcp_tbl_print_M(tbl)
			vec_print(tbl['lb'])
			# Decide next operation
			if (i_block != i_disting):
				i_driv = lcp_tbl_pp_compl(tbl, i_block) 
			else:	
				print 'NEXT'
				break



			# print r_block, driv_bound, lcp_tbl_lbl(tbl, tbl['L'][r_block])
			# return 0	
					

			# r_cands = [x for x in range(n) if M[x][i_driv] > 0]
			# if (len(r_cands) == 0):	# Unblocked, no solution
			# 	status = 0; break;
			# print 'r_cands', r_cands	
			# r_block, driv_theta = lcp_tbl_ppcd2_leaving_topmost(tbl, r_cands, i_driv, lbda, r_disting, i_disting)
			# if (i_driv == i_disting and driv_theta >= 0):
			# 	print 'status 3'
			# 	status = 3; break; #TODO, no pivot, todo leaving should return disting if that is involved in a tie
			# # Pivot
			# i_block = tbl['L'][r_block]
			# print 'driv', lcp_tbl_lbl(tbl, i_driv), 'theta', driv_theta, 'r_block', r_block, 'block', lcp_tbl_lbl(tbl, i_block)
			# lcp_tbl_ppcd2_pivot(tbl, r_block, i_driv, lbda, i_disting)
			# vec_print(tbl['lb'])
			# mat_print(tbl['M'])
			# opt_print('{}. pvt: {}-{}, {}'.format(it, r_block,i_driv, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
			# # Decide next step
			# if (i_block != i_disting):
			# 	i_driv = lcp_tbl_pp_compl(tbl, i_block) 
			# else:
			# 	break;	

	if (status == 2):
		tbl['sol'] = lcp_tbl_solution(tbl,['z'])
	else:	
		tbl['sol'] = []
	return (status == 2)

#----------------------------------------------------------
# def lcp_solve_ppcd2_tableau(tbl, opts = {}):
# 	# Asymmetric PPM, Cottle p.260, 336
# 	# Dantzig-Cottle Principal Pivoting Method, Murty p.276.
# 	# Processes P, PSD matrices.
# 	#
# 	M = tbl['M']; qoff = lcp_tbl_off(tbl, 'q'); n = tbl['n'];
# 	nb = ([0]*n)+([1]*n); tbl['nb'] = nb;
# 	nbv = [g_num(0)]*(2*n); tbl['nbv'] = nbv;
# 	maxit = opts.get('maxit', 0); it = 0; status = 1;
# 	lbda = g_num(int(min(M[x][qoff] for x in range(n)) - 2))
# 	while (status == 1 and (maxit == 0 or it < maxit)):
# 		it = it + 1
# 		if all(M[x][qoff] >= 0 for x in range(n)): # Success 
# 			status = 2; break;
# 		# Determine distinguished
# 		lbda_nb = next((x for x in range(2*n) if nb[x] == 1 and nbv[x]=lbda), -1)
# 		if (lbda_nb != -1):
# 			i_disting = lbda_nb; i_driv = lbda_nb;
# 		else:
# 			i_disting = next(x for x in range(n) if M[x][qoff] < 0)
# 			i_driv = lcp_tbl_pp_compl(tbl, i_disting) 
# 		if (nb[i_driv] == 0) raise ValueError('')
# 		# Determine blocking


# 		q_cands = [x for x in range(n) if M[x][qoff] < 0]
# 		r_disting = q_cands[-1]
# 		c_disting = lcp_tbl_pp_compl(tbl, tbl['L'][r_disting]) 
# 		if (M[r_disting][c_disting] == 0) and all(M[x][c_disting] <= 0 for x in range(n)): # no solution
# 			status = 0; break;
# 		r_cands = [r_disting] + [x for x in range(n) if M[x][c_disting] > 0 and M[x][qoff] >= 0]
# 		#mat_print(tbl['M'], '')
# 		r_block = lcp_tbl_leaving(tbl, r_cands, c_disting, opts); xi_block = tbl['L'][r_block];
# 		lcp_tbl_pivot(tbl, r_block, c_disting)
# 		opt_print('{}. M-pvt: {}-{}, {}'.format(it, r_block,c_disting, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
# 		while ( (lcp_tbl_pp_compl(tbl, xi_block) != c_disting) 
# 				and (status == 1 and (maxit == 0 or it < maxit)) ):
# 			it = it + 1
# 			c_driv = lcp_tbl_pp_compl(tbl, xi_block)
# 			r_cands = [r_disting] + [x for x in range(n) if M[x][c_driv] > 0 and M[x][qoff] >= 0]
# 			r_block = lcp_tbl_leaving(tbl, r_cands, c_driv, opts); xi_block = tbl['L'][r_block];
# 			lcp_tbl_pivot(tbl, r_block, c_driv)
# 			opt_print('{}. m-pvt: {}-{}, {}'.format(it, r_block,c_driv, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
# 	if (status == 2):
# 		tbl['sol'] = lcp_tbl_solution(tbl,['z'])
# 	else:	
# 		tbl['sol'] = []
# 	return (status == 2)

def lcp_tbl_cpa_struct(n):
	return {'w': lcp_tbl_struct(n, 0, n-1, 'w', 'id'), 
			'z': lcp_tbl_struct(n, n, (2*n)-1, 'z', 0), 
			'z0': lcp_tbl_struct(1, (2*n), (2*n), 'z0', '-en'), 
			'q': lcp_tbl_struct(1, (2*n)+1, (2*n)+1, 'q', 0)}

def lcp_tbl_cpa_create(n): 
	return lcp_tbl_create(n, lcp_tbl_cpa_struct(n))

# solve w-Mz=q
def lcp_tbl_cpa_rinit(tbl, r, m, q):
	mat_rput(tbl['M'], r, lcp_tbl_off(tbl, 'z'), vec_neg(m))
	tbl['M'][r][lcp_tbl_off(tbl, 'q')] = q

# solve w-Mz=q
def lcp_tbl_cpa_init_Mq(tbl, Mq):
	for r in range(len(Mq)): lcp_tbl_cpa_rinit(tbl, r, Mq[r][:-1], Mq[r][-1])

def lcp_tbl_cpa_create_Mq(Mq): 
	tbl = lcp_tbl_cpa_create(len(Mq[0])-1); lcp_tbl_cpa_init_Mq(tbl, Mq); return tbl;

def lcp_solve_cpa_tableau(tbl, opts = {}):
	# Complementary Pivot Algorithm, Murty p.66, opt. p.81
	#
	#Initialization, p.71
	#mat_print(tbl['M'], '')
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
		#mat_print(tbl['M'], '')	
	maxit = opts.get('maxit', 0); it = 0; 
	while (status == 1 and (maxit == 0 or it < maxit)):
		it = it + 1
		L = tbl['L']; q = lcp_tbl_col(tbl, 'q'); z0i = lcp_tbl_off(tbl, 'z0');
		z0r = L.index(z0i) if z0i in L else None
		if (z0r is None or q[z0r] == 0): # Success, p.74
			status = 2; break;
		M = tbl['M']
		c = lcp_tbl_pp_compl(tbl, dropped)
		r_cands = [x for x in range(tbl['n']) if M[x][c] > 0]
		if (len(r_cands) == 0): # Failure, p.68
			status = 0; break;
		r = lcp_tbl_leaving(tbl, r_cands, c, opts)
		dropped = tbl['L'][r]; 
		lcp_tbl_pivot(tbl, r, c); 
		opt_print('{}. pvt: {}-{}, {}'.format(it, r,c, lcp_tbl_lbls_str(tbl, tbl['L'])), opts)
		#mat_print(tbl['M'], '');
	if (status == 2):
		tbl['sol'] = lcp_tbl_solution(tbl, ['z'])
	else:	
		tbl['sol'] = []
	return (status == 2)

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
		]
	}
	return Mq_tbl[test]

if 1:
	g_num = g_num_rational

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
if 1: test_ppcd2(['Cottle p.258'], {'maxit':20, 'log':True})
if 0: test_ppcd2(['Murty p.265'], {'maxit':20, 'log':True})
