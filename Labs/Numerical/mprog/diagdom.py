import sys
import math
import itertools
import copy
import fractions
import operator
import ctypes
import os
import traceback
import time
import struct
import json

def sys_argv_has(keys):
	if hasattr(sys, 'argv'):
		keys = keys if isinstance(keys, list) else [keys]
		if (hasattr(sys, 'argv')):
			for i in range(len(keys)):
				 if (keys[i] in sys.argv):
					return True
	return False

def sys_argv_has_key(keys):
	if hasattr(sys, 'argv'):
		keys = keys if isinstance(keys, list) else [keys]
		if ( hasattr(sys, 'argv')):
			for key in keys:
				ki = sys.argv.index(key) if key in sys.argv else -1
				if (ki >= 0 and ki+1 < len(sys.argv)):
					return True
	return False

def sys_argv_get(keys, dflt):
	if hasattr(sys, 'argv'):
		keys = keys if isinstance(keys, list) else [keys]
		if ( hasattr(sys, 'argv')):
			for key in keys:
				ki = sys.argv.index(key) if key in sys.argv else -1
				if (ki >= 0 and ki+1 < len(sys.argv)):
					return sys.argv[ki+1]
	return dflt

def g_num_default(x):
	return float(x)
g_num = g_num_default


def print_tab(list, pref='', sep=' ', post=''):
	col_width = [max(len(str(x)) for x in col) for col in itertools.izip_longest(*list, fillvalue='')]
	for line in list:
		print pref + sep.join("{:>{}}".format(x, col_width[i]) for i,x in enumerate(line)) + post
def mat_print(M):
	print_tab([[str(x) for x in M[r]] for r in range(len(M))])

LogBlip = 1; LogDbg = 2;
def log_print(str, log = True):
	if (log):
		print str
def opt_print(str, opt = {}):
	log_print(str, opt.get('log', 0))
def vec_str(v):
		return '({})'.format(', '.join(str(x) for x in v))
def vec_print(v, log = True):
	print vec_str(v) if log else 0

def m_min(a, b):
	return a if a <= b else b
def m_max(a, b):
	return a if a >= b else b
def m_abs(v):
	return v if v >= 0 else -v
def m_isgn(v):
	return int(math.copysign(1, v))
def m_ineg(v):
	return 1 if v < 0 else 0
def m_is_between(x, a, b):
	return x >= a and x <= b
def m_is_between_strict(x, a, b):
	return x > a and x < b
def m_len2(v1, v2):
	return math.sqrt(v1*v1+v2*v2)
def vec_create(n,v):
	return [v]*n
def vec_dim(v):
	return len(v)
def vec_is_empty(v):
	return vec_dim(v) == 0
def vec_copy(a):
	return [copy.deepcopy(x) for x in a]
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
def vec_dot(a,b):
	d = g_num(0)
	for i in range(len(a)):
		d += a[i]*b[i]
	return d
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
			rn[j] = copy.deepcopy(rm[j])
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
		coff = 0; bci = 0;
		for b_cols in cblocks:
			BM[bri][bci] = mat_extract_block(M, roff, b_rows, coff, b_cols)
			coff += b_cols; bci += 1;
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
def mat_divs(M, s):
	r1 = len(M);c1 = len(M[0]);
	N = mat_create(r1, c1, None)
	for i in range(r1):
		for j in range(c1):
			N[i][j] = M[i][j]/s
	return N
def mat_add(M, N):
	r1 = len(M);c1 = len(M[0]);
	S = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]; rn = N[i]; rs = S[i];
		for j in range(c1):
			rs[j] = rm[j]+rn[j]
	return S
def mat_add_transp(M, Nt):
	r1 = len(M);c1 = len(M[0]);
	S = mat_create(r1, c1, None)
	for i in range(r1):
		rm = M[i]; rs = S[i];
		for j in range(c1):
			rs[j] = rm[j]+Nt[j][i]
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
def mat_det(M, r = 0):
	n = len(M); det = 0;
	if (n == 1):
		return M[0][0]
	rblk = [x for x in range(n) if x != 0]
	for ci in range(n):
		cblk = [x for x in range(n) if x != ci]
		B = mat_block_implode2(M, [rblk], [cblk])[0][0]
		det += M[0][ci] * mat_det(B, r+1) * math.pow(-1, ci)
	return det
def mat_adj(M):
	n = len(M);
	A = mat_create(n, n, 0)
	for ri in range(n):
		rblk = [x for x in range(n) if x != ri]
		for ci in range(n):
			cblk = [x for x in range(n) if x != ci]
			B = mat_block_implode2(M, [rblk], [cblk])[0][0]
			A[ri][ci] = mat_det(B) * math.pow(-1, ri+ci)
	return A

def mat_adj_inv(M):
	det = mat_det(M)
	if (det == 0):
		return None
	A = mat_adj(M)
	return mat_divs(mat_transp(A), det)

def mat_condition(M):
	iM = mat_adj_inv(M)
	return sum([m_abs(x) for x in M[0]]) * sum([m_abs(x) for x in iM[0]])

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

def data_conv_hex_int(x):
	return int(x, 16)

def data_conv_hex_cflt(x):
	x = x[2:] if x.startswith('0x') else x
	struct.unpack('!f', x.zfill(8).decode('hex'))[0]

def data_conv_csv_mat(x, r, c, dconv):
	

def def_data_conv():
	 return { 'h-i':data_conv_hex_int, 'hex-int':data_conv_hex_int, 'h-cf':data_conv_hex_cflt, 'hex-cflt':data_conv_hex_cflt }

def load_json_matrix(dict, dconv):
	r = dconv['h-i'](dict['m']); c = dconv['h-i'](dict['n']); M = dict['M'];


def load_json_matrices(path, dconv):

	with open(path, 'r') as fi: 
		str = fi.read()
		str = str.replace('\n', '').replace('\r', '').replace('\t', ' ')
		dict = json.loads(str)
		for k in dict.keys():
			if (k == 'Mlcp_A'):
				load_matrix()


def main():
	if sys_argv_has('-in'):
		load_json(sys_argv_get('-in', ''), def_data_conv())

if hasattr(sys, 'argv'):
	main()
