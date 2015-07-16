# coding=utf-8
from __future__ import division # prevent integer division (http://docs.sympy.org/0.7.1/gotchas.html)
from sympy import *
import numpy as numpy
import re
import sys
import copy
from sets import Set
import traceback

execfile('helper_arg.py')
execfile('helper_math1.py')

g_dbg = False

_ctx = {
	'parse_func_re' : re.compile(ur'((?:\d+\.*(?:0?e-\d+)?\d*(?!\w)|\w+)\b)(?!\s*\()'),
	'parse_number_re' : re.compile(ur'\d+\.*(?:0?e-\d+)?\d*(?!\w)'),
	'parse_symb_re' : re.compile(ur'\w+'),
	'parse_arithm_re' : re.compile(ur'(\+|\-|\*|\/|\\|\^|\.)'),
	'parse_constants' : 'pi'.split(','),
	'parse_func_func_re' : re.compile(ur'\w+(?=\()'),
}
def tok_print_group(group, tl, depth=0):
	for p in group['parts']:
		if (type(p) is not list):
			tok_print_group(p, tl, depth+1)
		else:
			content = ' '.join(["[{}]".format(tl[tli][1]) for tli in p])
			print '{}{} {}'.format(''.join([' ']*(depth)), '{}|_'.format(' ' if depth-1 else '') if depth else '', content)
def tok_group_to_str(group, tl):
	els = []
	for p in group['parts']:
		if (type(p) is not list):
			els.append('({})'.format(tok_group_to_str(p, tl)))
		else:
			els.extend([tl[x][1] for x in p])
	return ''.join(els)
def parse_func_tokenize(func_str, parse_strict):
	def has_ws(token):
		return ''.join(token.split()) != token
	def is_number(token, final = True, leaf = None):
		return not has_ws(token) and (not (token.startswith('+') or token.startswith('-'))) and (is_float(token) if final else is_float(token+'0'))
	def is_symbol(token, final = True, leaf = None):
		tl = token[-1]
		return not has_ws(token) and not (is_operator(tl) or is_group_start(tl) or is_group_end(tl) or (is_number(token.strip(), True) if final else False) )
	def is_symbol_restricted(token, final = True, leaf = None):
		return token[:1].isalpha() and is_symbol(token, final, leaf)
	def is_operator(token, final = True, leaf = None):
		if token in ['+','-','/','*','^']:
			return True
	def is_ws(token, final = True, leaf = None):
		if len(token.strip()) == 0:
			return True
	def is_group_start(token, final = True, leaf = None):
		return token in ['[','(']
	def is_group_end(token, final = True, leaf = None):
		return token in [']',')']
	def parse_group(tl, tli, tlo, depth):
		group = { 'parts':[], 'started':False }
		while tli < len(tl):
			#print depth, group, tli
			tok = tl[tli]
			if (tok[0] == ']'):
				tlo[0] = tli+1; return group if (group['started']) else None;
			if (tok[0] == '['):
				if group['started']:
					ngtlo = [0]; ng = parse_group(tl, tli, ngtlo, depth+1);
					if ng:
						tli = ngtlo[0]-1; group['parts'].append(ng);
					else:
						return None
				else:
					group['started'] = True
			else:
				if group['started']:
					if len(group['parts']) == 0 or (type(group['parts'][-1]) is not list):
						group['parts'].append([])
					group['parts'][-1].append(tli)
				else:
					if (tok[0] != 'ws'):
						return None
			tli = tli+1
		return None
	def parse_root_group(tl):
		tlo = [0]; root = parse_group(tl, 0, tlo, 0);
		rest_is_ws = all( tl[x][0]=='w' for x in range(tlo[0], len(tl)) )
		return root if (root and rest_is_ws) else None
	def print_group(group, tl, depth=0):
		return tok_print_group(group, tl, depth)
	def group_to_str(group, tl):
		return tok_group_to_str(group, tl)
	token_tests = { 'n':is_number, 's':is_symbol_restricted if parse_strict else is_symbol, 'o':is_operator, 'w':is_ws, '[':is_group_start, ']':is_group_end }
	root = {'parent':None, 'children':[], 'token':['?',''] }
	pre_leafs = [root]
	for ch in func_str:
		post_leafs = []
		for leaf in pre_leafs:
			tp,to = leaf['token']
			nto = to+ch
			if (tp != '?'):
				if token_tests[tp](nto, False, leaf):
					leaf['token'][1] = nto
					post_leafs.append(leaf)
				else:
					if token_tests[tp](to, True, leaf):
						for ttp,tt in token_tests.items():
							if tt(ch, False, leaf):
								nl = {'parent':leaf, 'children':[], 'token':[ttp, ch] }
								leaf['children'].append(nl); post_leafs.append(nl);
			else:
				cands = []
				for ttp,tt in token_tests.items():
					if tt(nto, False, leaf):
						cands.append(ttp)
				if len(cands) == 1:
					leaf['token'][0] = cands[0]; leaf['token'][1] = nto;
					post_leafs.append(leaf)
				elif len(cands) > 1:
					for c in cands:
						nl = {'parent':leaf if len(leaf['token'][1]) else None, 'children':[], 'token':[c, nto] }
						leaf['children'].append(nl); post_leafs.append(nl);
		pre_leafs = post_leafs
	if g_dbg:
		print 'Dbg: parsing [{}]'.format(func_str)
	valid_token_lists = []
	for leaf in pre_leafs:
		tp,to = leaf['token']
		if (tp != '?' and token_tests[tp](to, True, leaf)):
			token_list = []
			while leaf:
				token_list.append(leaf['token']); leaf = leaf['parent'];
			valid_token_lists.append(list(reversed(token_list)))
	no_ws_token_lists = []
	for tl in valid_token_lists:
		ptl = []
		for tli in range(len(tl)):
			if tl[tli][0] != 'w':
				ptl.append(tl[tli])
		no_ws_token_lists.append(ptl)
	auto_op_token_lists = []
	for tl in no_ws_token_lists:
		ptl = []
		for tli in range(len(tl)):
			if tl[tli][0] in ['n', 's']:
				if tli+1<len(tl) and tl[tli+1][0] in ['n', 's']:
					ptl.append(tl[tli]); ptl.append(('o','*'));
				elif tli-1>0 and tl[tli-1][0] in [']']:
					ptl.append(('o','*')); ptl.append(tl[tli]);
				else:
					ptl.append(tl[tli])
			else:
				ptl.append(tl[tli])
		auto_op_token_lists.append(ptl)
	processed_token_lists = auto_op_token_lists
	valid_group_trees = []
	for tl in processed_token_lists:
		gtl = [('[','')] + tl + [(']','')]
		tl_group = parse_root_group(gtl);
		if (tl_group):
			#print_group(tl_group, gtl); print '';
			valid_group_trees.append((gtl, tl_group))
	if len(valid_group_trees) == 0:
		if g_dbg:
			print 'Note: invalid expression'
		return ('', None, None)
	lbd_less_tokens = lambda x,y: len(x[0])-len(y[0])
	sorted_group_trees = sorted(valid_group_trees, cmp=lbd_less_tokens)
	if len(sorted_group_trees)>1 and lbd_less_tokens(sorted_group_trees[0], sorted_group_trees[1]) == 0:
		if g_dbg:
			print 'Note: ambiguous expression'
		return ('', None, None)
	gt = sorted_group_trees[0]
	if g_dbg:
		print_group(gt[1], gt[0])
		print group_to_str(gt[1], gt[0])
	return (group_to_str(gt[1], gt[0]), gt[1], gt[0])
g_sympy_constants = ('j','inf','nan','pi','degree','e','phi','euler','catalan','apery','khinchin','glaisher','mertens','twinprime')
g_as_constants = {'PI':'pi', 'E':'e', 'J':'j'}
def name_to_sympy(name, is_func):
	pre_repl = name.replace("'", '_p_' if name.upper() != name else '_P_')
	if (not is_func) or pre_repl != name:
		return '_{}'.format(pre_repl)
	return name
def fname_to_sympy(name):
	return name_to_sympy(name, True)
def parse_func_str(_func_str, parse_strict = arg_has('-parse_strict')):
	func_str, group_tree, toks = parse_func_tokenize(_func_str, parse_strict)
	sympy_toks = copy.copy(toks)
	sympy_transl = {}; f_vars = []; f_constants = []; sympy_vars = []; sympy_constants = [];
	for ti in range(len(sympy_toks)):
		tok = sympy_toks[ti]; pre_tok = copy.copy(tok);
		if tok[0] == '[' and len(tok[1]):
			tok[1] = '('
		elif tok[0] == ']' and len(tok[1]):
			tok[1] = ')'
		elif tok[0] == 'o' and tok[1] == '^':
			tok[1] = '**'
		elif tok[0] == 's' and tok[1] in g_as_constants.keys():
			tok[1] = g_as_constants[tok[1]]
		elif tok[0] == 's':
			is_func = ti+1<len(sympy_toks) and sympy_toks[ti+1][0] == '['
			repl = name_to_sympy(tok[1], is_func)
			if repl != tok[1]:
				sympy_transl[tok[1]] = repl; tok[1] = repl;
			if not is_func:
				if tok[1].upper() == tok[1]:
					f_constants.append(pre_tok[1]); sympy_constants.append(tok[1])
				else:
					f_vars.append(pre_tok[1]); sympy_vars.append(tok[1])
	sympy_func_str = tok_group_to_str(group_tree, sympy_toks)
	return (group_tree, func_str, toks, sorted(f_vars), sorted(f_constants), sympy_func_str, sympy_toks, sympy_transl, sorted(sympy_vars), sorted(sympy_constants))
def eval_sym_str(str, symbs, do_expand, transl={}):
	for symb in symbs:
		exec('{} = Symbol("{}")'.format(transl.get(symb, symb), transl.get(symb, symb)))
	ev = eval(str); return expand(ev) if do_expand else ev;
def subs_sym_str(str, func_lambds):
	for (name,lambd) in func_lambds:
		exec('{} = lambd;'.format(name))
	return float( eval(str) )
def as_list(names):
	return names.split(',') if (type(names) is not list) else names
def has_const(fctx, name):
	return name in fctx['eval_constants'];
def def_const(fctx, names, val = 0.5):
	names = as_list(names)
	for name in names:
		for d in (fctx['eval_constants'],fctx['eval_vars']):
			d.discard(name)
		fctx['eval_constants'].add(name)
		fctx['values'][name] = val
def val_of_const(fctx, name, def_val = 0.5):
	if not has_const(fctx, name):
		def_const(fctx, name, def_val)
	return fctx['values'][name]
def set_const(fctx, name, val = 0.5):
	if not has_const(fctx, name):
		def_const(fctx, name, val)
	else:
		fctx['values'][name] = val
def def_var(fctx, names):
	names = as_list(names)
	for name in names:
		for d in (fctx['eval_constants'],fctx['eval_vars']):
			d.discard(name)
		fctx['eval_vars'].add(name)
def find_composition_map(func_str, fctx_map):
	matches = re.findall(_ctx['parse_func_func_re'], func_str)
	comp_map = {}
	for m in matches:
		print m
		if (m in fctx_map) and (m not in comp_map):
			comp_map[m] = '_F{}_'.format(len(comp_map))
	return comp_map
def inject_composition_map(func_str, comp_map, forward):
	inj_func_str = func_str
	for (k,v) in comp_map.items():
		(k,v) = (k,v) if forward else (v,k)
		sub_re = re.compile(ur'{}\s*(?=\()'.format(k))
		inj_func_str = sub_re.sub(v, inj_func_str)
	return inj_func_str
def subs_functional_vars(func_name, sympy_func_str, eval_vars, funcs, sympy_funcs, comps = Set()):
	subs_func_str = sympy_func_str
	for fv in eval_vars:
		if (fv in sympy_funcs):
			comps.add(fv); sympy_funcs[fv]['is_comp'].add(func_name);
			sub_re = re.compile(ur'{}'.format(fv))
			#print '{} -> {}'.format(fv, sympy_funcs[fv]['sympy_func_str'])
			subs_func_str = sub_re.sub('({})'.format(sympy_funcs[fv]['sympy_func_str']), subs_func_str)
			#print subs_func_str
	if (subs_func_str != sympy_func_str):
		return subs_functional_vars(func_name, subs_func_str, eval_vars, funcs, sympy_funcs, comps)
	else:
		return subs_func_str
def func_update_sym_func(fctx, sym_func, f_vars, f_constants, eval_vars, eval_constants):
	fctx['sym_func'] = sym_func
	eval_func = sym_func
	for cnt in eval_constants:
		eval_func = eval_func.subs(cnt, val_of_const(fctx, cnt))
	fctx['f_vars'] = copy.copy(f_vars); fctx['f_constants'] = copy.copy(f_constants);
	fctx['eval_vars'] = copy.copy(eval_vars); fctx['eval_constants'] = copy.copy(eval_constants);
	fctx['eval_func'] = eval_func
	fctx['eval_func_exp'] = expand(eval_func)
	fctx['lambd_f'] = lambdify(eval_vars, expand(eval_func), "numpy")
def func_str_to_sym(_func_str, func_name='', funcs={}, sympy_funcs={}):

	fctx = {

	'lvl_raw': { 'name':'', 'func_str':''  },
	'lvl_parsed':  { 'group_tree':None, 'func_str':None, 'toks':None, 'f_vars':None, 'f_constants':None, 'sympy_func_str':None, 'sympy_toks':None, 'sympy_transl':None, 'sympy_vars':None, 'sympy_constants':None },
	'lvl_comp': { 'comp_func_str':'', 'comp_map':{}, 'comps':Set(), 'is_comp':Set(), }
	'lvl_comp_parsed': {},
	'lvl_eval': { 'eval_vars':[], 'eval_constants':[], 'const_values':{}, },
	};
	fctx['_func_str'] = _func_str
	(group_tree, func_str, toks, f_vars, f_constants, sympy_func_str, sympy_toks, sympy_transl, eval_vars, eval_constants) = parse_func_str(_func_str)
	fctx['name'] = func_name; fctx['func_str'] = func_str; fctx['sympy_func_str'] = sympy_func_str; fctx['sympy_transl'] = sympy_transl;
	sympy_func_str = subs_functional_vars(func_name, sympy_func_str, eval_vars, funcs, sympy_funcs, fctx['comps'])
	for ev in eval_vars:
		def_var(fctx, ev)
	for ec in eval_constants:
		def_const(fctx, ec)
	print sympy_func_str, eval_vars
	sym_func = eval_sym_str(sympy_func_str, eval_vars + eval_constants, False, fctx['sympy_transl'])
	func_update_sym_func(fctx, sym_func, f_vars, f_constants, eval_vars, eval_constants)
	return fctx
def func_sym_to_df(fctx, var):
	dfctx = copy.copy(fctx)
	dsym_func = diff(dfctx['sym_func'], dfctx['sympy_transl'].get(var, var))
	func_update_sym_func(dfctx, dsym_func, dfctx['f_vars'], dfctx['f_constants'], dfctx['eval_vars'], dfctx['eval_constants'])
	for nk in ['_func_str', 'func_str']:
		funcs[nk] = 'd({},{})'.format(funcs[nk], var)
	return dfctx
def multi_step(ts, h):
	could_step = False
	for ti in range(len(ts)):
		ts[ti] = ts[ti] + h
		if (ts[ti] <= 1.0):
			could_step = True; break;
		else:
			ts[ti] = 0.0
	return could_step
def func_gridtest(vlen1, vlen2, lbdf1, lbdf2, (rlo, rhi, h), excpt = False, seed = None, quiet = False):
	vlen = max(vlen1, vlen2)
	err = 0.0; ts = [0.0 for x in range(vlen)]
	could_step = True
	while could_step:
		coords = [rlo+t*(rhi-rlo) for t in ts]
		err = max(err, abs(lbdf1(*(coords[:vlen1])) - lbdf2(*(coords[:vlen2]))) )
		could_step = multi_step(ts, h)
	return err
def func_randtest(vlen1, vlen2, lbdf1, lbdf2, (rlo, rhi, n), excpt = False, seed = None, quiet = False):
	if seed is not None:
		numpy.random.seed(seed)
	vlen = max(vlen1, vlen2); coords = [rlo+x*(rhi-rlo) for x in numpy.random.random(int(n)+vlen-1)];
	err = 0.0; i = 0; except_count = 0;
	with numpy.errstate(invalid='raise'):
		while i < len(coords)-vlen+1:
			has_except = False
			if excpt:
				try:
					err = max(err, abs(lbdf1(*(coords[i:i+vlen1])) - lbdf2(*(coords[i:i+vlen2]))) )
				except:
					has_except = True; except_count = except_count + 1
			else:
				err = max(err, abs(lbdf1(*(coords[i:i+vlen1])) - lbdf2(*(coords[i:i+vlen2]))) )
			if has_except:
				coords[i:i+vlen] = [rlo+x*(rhi-rlo) for x in numpy.random.random(vlen)]
			else:
				i = i+1
	if (except_count  and not quiet):
		print 'Note: Bypassed {} exceptions.'.format(except_count)
	return err
def func_ztest(fctx, subs, k_int, (rlo, rhi, n), excpt = False, seed = None, quiet = False):
	if seed is not None:
		numpy.random.seed(seed)
	subs_keys = subs.keys(); subs_fctx = [subs[k] for k in subs_keys]; subs_lbds = [x['lambd_f'] for x in subs_fctx];
	in_vars = Set()
	for sub in subs_fctx + [fctx]:
		for ev in sub['eval_vars']:
			in_vars.add(ev)
	in_vars = list(in_vars)
	lbd_inds = [ [in_vars.index(x) for x in sub['eval_vars']] for sub in subs_fctx ]
	f_lbd_inds = [in_vars.index(x) for x in fctx['eval_vars']]
	subs_inds = [ fctx['eval_vars'].index(x) for x in fctx['eval_vars'] if x in subs_keys ]
	k_inds = [ in_vars.index(x) for x in in_vars if x.startswith('k') ] if k_int else []
	vlen = len(in_vars); coords = [rlo+x*(rhi-rlo) for x in numpy.random.random(int(n)+vlen-1)];
	k_coords = numpy.random.randint(-10, 11, int(n)+vlen-1)
	subs_vals = [0.0]*len(subs);
	lbd_f = fctx['lambd_f']; fvlen = len(fctx['eval_vars']);
	err = 0.0; except_count = 0;
	with numpy.errstate(invalid='raise'):
		i = 0
		while i < (len(coords)-vlen+1):
			has_except = False
			coords_i = coords[i:i+vlen]; coords_k = k_coords[i:i+vlen];
			for k,j in zip(k_inds, range(len(k_inds))):
				coords_i[k] = coords_k[j]
			for lbd,sub_lbd_inds,si in zip(subs_lbds, lbd_inds, range(len(subs_keys))):
				sub_coords = [coords_i[x] for x in sub_lbd_inds]
				if excpt:
					try:
						subs_vals[si] = lbd(*sub_coords)
					except:
						if g_dbg:
							print 'except'
						except_count = except_count+1; has_except = True; break;
				else:
					subs_vals[si] = lbd(*sub_coords)
				if g_dbg:
					print '{} ({}) = {}'.format(subs_fctx[si]['name'], zip(subs_fctx[si]['eval_vars'], sub_coords), subs_vals[si])
			if has_except:
				coords[i:i+vlen] = [rlo+x*(rhi-rlo) for x in numpy.random.random(vlen)]
				k_coords[i:i+vlen] = numpy.random.randint(-10, 11, vlen)
			else:
				coords_f = [coords_i[x] for x in f_lbd_inds]
				for j in range(len(subs_inds)):
					coords_f[subs_inds[j]] = subs_vals[j]
				f_val = lbd_f(*coords_f)
				if g_dbg:
					print '{} ({}) = {}'.format(fctx['name'], zip(fctx['eval_vars'], coords_f), f_val)
				err = max(err, abs(f_val))
				i = i+1
	if (except_count  and not quiet):
		print 'Note: Bypassed {} exceptions.'.format(except_count)
	return err
def pp_func_args(fctx, use_sympy=False):
	return '{}({})'.format(fctx['name'], ', '.join(fctx['eval_vars' if use_sympy else 'f_vars']))
def pp_func_args_val(fctx, use_sympy=False):
	eval_str = str(fctx['eval_func']) if use_sympy else fctx['_func_str']
	return '{} = {}'.format(pp_func_args(fctx, use_sympy), eval_str)
def pp_func_args_orig(fctx, empty_name = False):
	return '{} = {}'.format(''.join([' ']*len(pp_func_args(fctx))) if empty_name else pp_func_args(fctx, ltex), fctx['_func_str'])
def resubs_functional_vars(name, funcs, sympy_funcs, updated = Set()):
	for fn in funcs[name]['is_comp']:
		updated.add(fn);
		funcs[fn] = func_str_to_sym(funcs[fn]['_func_str'], fn, funcs); funcs[fn]['name'] = fn;
	for fn in funcs[name]['is_comp']:
		resubs_functional_vars(funcs[fn]['name'], funcs, sympy_funcs, updated)
def create_dsl():
	print ' Asinus Salutem'
	return { 'funcs':{}, 'sympy_funcs':{}, 'dbg':False, 'g_dbg':arg_has('-dbg'),
			'sections':[x for x in arg_get('-sections', '').split(',') if len(x)],
			'cur_sec':None }
def dsl_add_fctx(dsl, name, fctx, allow_clobber, quiet):
	old_func = dsl['funcs'].get(name, None); old_is_comp = old_func['is_comp'] if old_func else None;
	if (not allow_clobber and old_func is not None):
		raise Exception()
	dsl['funcs'][name] = fctx; dsl['sympy_funcs'][name_to_sympy(name, False)] = fctx;
	if (not quiet):
		print ' ', pp_func_args_val(fctx, use_sympy=False)
		print ' ', pp_func_args_val(fctx, use_sympy=True)
	if (old_is_comp is not None) and len(old_is_comp):
		fctx['is_comp'] = old_is_comp
		updated = Set()
		resubs_functional_vars(name, dsl['funcs'], dsl['sympy_funcs'], updated)
		if (not quiet):
			print '   [{}]'.format(', '.join(updated))
def process_dsl_command(dsl, inp, quiet=False):
	global g_dbg
	funcs = dsl['funcs'];
	input_splt = inp.split(' ')
	cmd = input_splt[0]
	dsl['dbg'] = True if ('-dbg' in input_splt) else dsl['g_dbg']; g_dbg = dsl['dbg'];
	run_sec = len(dsl['sections']) == 0 or dsl['cur_sec'] in dsl['sections']
	if (cmd in ['let']):
		name = input_splt[1]
		func_str = ' '.join(input_splt[3:])
		if '[' in name or '(' in name:
			name_splt = name.split('(' if '(' in name else '[')
			name = name_splt[0]; fvars = name_splt[1].split(',');
		fctx = func_str_to_sym(func_str, name, dsl['funcs'], dsl['sympy_funcs'])
		old_func = funcs.get(name, None); old_is_comp = old_func['is_comp'] if old_func else None;
		dsl_add_fctx(dsl, name, fctx, True, quiet or not run_sec)
	elif (cmd == 'bake'):
		fn,bn = input_splt[1], input_splt[2]
		bfunc = copy.copy(funcs[fn]); bfunc['name'] = bn; bfunc['comps'] = Set();
		dsl_add_fctx(dsl, bn, bfunc, False, quiet or not run_sec)
	elif (cmd == 'd'):
		fn,varn = input_splt[1], input_splt[2]
		name = 'd({},{})'.format(fn, varn)
		fctx = func_sym_to_df(funcs[fn], varn)
		dsl_add_fctx(dsl, name, fctx, False, quiet or not run_sec)
	elif (cmd in ['ftest', 'frandtest', 'fgridtest'] and run_sec):
		n1,n2 = input_splt[1], input_splt[2]
		lo_hi_n = [-1.0,1.0,1000]; seed = None;
		state = ''; state_lhn = 0; excpt = False
		for part in input_splt[3:]:
			if state == '':
				if part == '-k':
					k_int = True
				elif part == '-except':
					excpt = True
				elif part == '-seed':
					state = 'seed'
				elif state_lhn < 3 and is_float(part):
					lo_hi_n[state_lhn] = float(part); state_lhn = state_lhn+1;
			elif state == 'seed':
				seed = int(part); state = '';
		func_test = func_randtest if (cmd != 'fgridtest') else func_gridtest
		print func_test(len(funcs[n1]['eval_vars']), len(funcs[n2]['eval_vars']), funcs[n1]['lambd_f'], funcs[n2]['lambd_f'], lo_hi_n, excpt, seed = seed, quiet = quiet)
	elif (cmd == 'ztest' and run_sec):
		fn = input_splt[1]; fctx = funcs[fn];
		k_int = False; subs = {}; lo_hi_n = [-1.0, 1.0, 1000]; seed = None;
		state = ''; state_ev = ''; state_lhn = 0; excpt = False
		for part in input_splt[2:]:
			if state == '':
				if part in fctx['eval_vars']:
					state_ev = part; state = 'ev';
				elif part == '-k':
					k_int = True
				elif part == '-except':
					excpt = True
				elif part == '-seed':
					state = 'seed'
				elif state_lhn < 3 and is_float(part):
					lo_hi_n[state_lhn] = float(part); state_lhn = state_lhn+1;
			elif state == 'seed':
				seed = int(part); state = '';
			elif state == 'ev':
				subs[state_ev] = funcs[part]; state = '';
		print func_ztest(fctx, subs, k_int, lo_hi_n, excpt, seed = seed, quiet = quiet)
	elif (cmd in ['print', 'p'] and run_sec):
		name = input_splt[1]
		print ' ', pp_func_args_val(funcs[name], use_sympy = False)
		print ' ', pp_func_args_val(funcs[name], use_sympy = True)
		if len(funcs[name]['comps']):
			print ' ', pp_func_args_orig(funcs[name], empty_name = True)
	elif (cmd in ['echo'] and run_sec):
		print ' '.join(input_splt[1:])
	elif (cmd in ['section']):
		sec = input_splt[1]; dsl['cur_sec'] = sec;
		run_sec = len(dsl['sections']) == 0 or dsl['cur_sec'] in dsl['sections']
		if len(input_splt) >= 2 and run_sec:
			print ' '.join(input_splt[2:])
	elif (cmd in ['?', 'calc', 'eval'] and run_sec):
		print subs_sym_str(to_sym_str(' '.join(input_splt[1:])), [ ( name_to_sympy(x['name'], True),x['lambd_f']) for x in funcs.values()])
def enter_dsl(dsl):
	go_dsl = True; sys_exit = False;
	while go_dsl:
		try:
			print ' >',
			inp = raw_input()
			if (inp == 'e'):
				go_dsl = False
			elif (inp == 'q'):
				go_dsl = False; sys_exit = True;
			else:
				process_dsl_command(dsl, inp)
		except:
			traceback.print_exc()
		#e = sys.exc_info()[0]; raise e;
	if (sys_exit):
		sys.exit()
if not sys.flags.interactive:
	g_dbg = arg_has('-dbg')
	if arg_has('-pyexec'):
		execfile(arg_get('-pyexec', ''))
	elif arg_has('-pyscript'):
		with open(arg_get('-pyscript', ''), "r") as ifile:
			for line in [x.rstrip() for x in ifile.readlines()]:
				print line; exec(line);
	elif arg_has('-script'):
		dsl = create_dsl()
		with open(arg_get('-script', ''), "r") as ifile:
			quiet = arg_has('-quiet'); echo = arg_has('-echo');
			for line in [x.rstrip().strip() for x in ifile.readlines()]:
				if echo:
					print ' >', line;
				process_dsl_command(dsl, line, quiet=quiet);
		if arg_has('-continue'):
			enter_dsl(dsl)
	elif arg_has('-test'):
		print func_str_to_sym('A*cos(x)+B*sin(y)+[1^2*cos(x)]')
	else:
		enter_dsl(create_dsl())
