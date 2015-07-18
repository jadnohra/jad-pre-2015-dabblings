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
k_as_const_map = {'@pi':'pi', '@e':'e', '@i':'j'}
k_sympy_constants = ('j','inf','nan','pi','degree','e','phi','euler','catalan','apery','khinchin','glaisher','mertens','twinprime')
k_format_sympy_const_map = {'pi':'pi', 'e':'e', 'j':'j'}

def format_sympy_prepare_func_str(funct_str):
	return funct_str.replace('**', '^').replace('sqrt', '√')
def pp_toks(toks):
	return ' '.join(["{}".format(tok[1]) for tok in toks if len(tok[1])])
def pp_indexed_toks(toks, toksi):
	return ' '.join(["{}".format(toks[i][1]) for i in toksi if len(toks[i][1])])
def tok_group_toks(group, tl):
	els = []
	for p in group['parts']:
		if (type(p) is not list):
			els.extend(tok_group_toks(p, tl))
		else:
			els.extend([tl[x] for x in p])
	return els
def tok_print_group(group, tl, depth=0):
	for p in group['parts']:
		if (type(p) is not list):
			tok_print_group(p, tl, depth+1)
		else:
			print '{}{} {}'.format(''.join([' ']*(depth)), '{}|_'.format(' ' if depth-1 else '') if depth else '', pp_indexed_toks(tl,p))
def tok_group_to_str(group, tl):
	return ''.join([x[1] for x in tok_group_toks(group, tl)])
def parse_group_is_trivial(group, pre_is_func):
	return not pre_is_func and (len(group['parts']) == 3 and (type(group['parts'][1]) is list and len(group['parts'][1]) == 1))
def parse_group_is_simple(group, pre_is_func):
	return parse_group_is_trivial(group, pre_is_func) or len(group['parts']) == 3 and type(group['parts'][1]) is not list
def parse_group_reduce(group, tl, parent_is_simple, pre_is_func):
	is_simple, is_trivial = parse_group_is_simple(group, pre_is_func), parse_group_is_trivial(group, pre_is_func)
	parts = group['parts']; ngroup = { 'parts':[] }; pre_is_func = False;
	for p,pi in zip(parts, range(len(parts))):
		if (type(p) is not list):
			ngroup['parts'].append(parse_group_reduce(p, tl, is_simple, pre_is_func))
			pre_is_func = False
		else:
			ngroup['parts'].append(copy.copy(p))
			pre_is_func = len(p) and (tl[p[-1]][0],tl[p[-1]][2]) == ('s','func')
	if (parent_is_simple and is_simple) or is_trivial:
		return { 'parts':[ngroup['parts'][1]] }
	else:
		return ngroup
def root_parse_group_reduce(root, tl):
	return parse_group_reduce(root, tl, parse_group_is_simple(root, False), False)
def parse_group(tl, tli, tlo, depth):
	group = { 'parts':[] }; started = False;
	while tli < len(tl):
		tok = tl[tli]
		if (tok[0] == ']'):
			group['parts'].append([tli]); tlo[0] = tli+1; return group if (started) else None;
		elif (tok[0] == '['):
			if started:
				ngtlo = [0]; ng = parse_group(tl, tli, ngtlo, depth+1);
				if ng:
					tli = ngtlo[0]-1; group['parts'].append(ng);
				else:
					return None
			else:
				group['parts'].append([tli]); started = True;
		else:
			if started:
				if len(group['parts']) <= 1 or (type(group['parts'][-1]) is not list):
					group['parts'].append([])
				group['parts'][-1].append(tli)
			else:
				if (tok[0] != 'ws'):
					return None
		#print depth, started, group, tli, tl
		tli = tli+1
	return None
def parse_root_group(tl):
	gtl = [('[','(','')] + tl + [(']',')','')]
	tlo = [0]; root = parse_group(gtl, 0, tlo, 0);
	rest_is_ws = all( tl[x][0]=='w' for x in range(tlo[0], len(tl)) )
	return gtl, root_parse_group_reduce(root, gtl) if (root and rest_is_ws) else None
def parse_refine_toks(toks, const_map):
	ntoks = copy.deepcopy(toks)
	for ti in range(len(ntoks)):
		tok = ntoks[ti]
		if tok[0] == 's' and tok[2] == '':
			if ti+1<len(ntoks) and ntoks[ti+1][0] == '[':
				tok[2] = 'func'
			elif tok[1] in const_map:
				tok[2] = 'const'
			else:
				tok[2] = 'var' if (not tok[1].upper() == tok[1]) else 'cvar'
	return ntoks
def parse_func_tokenize(func_str, parse_strict, const_map):
	def has_ws(token):
		return ''.join(token.split()) != token
	def is_number(token, final = True, leaf = None):
		return not has_ws(token) and (not (token.startswith('+') or token.startswith('-'))) and (is_float(token) if final else is_float(token+'0'))
	def is_symbol(token, final = True, leaf = None):
		tl = token[-1]
		return not has_ws(token) and not (is_separator(tl) or is_operator(tl) or is_group_start(tl) or is_group_end(tl) or (is_number(token.strip(), True) if final else False) )
	def is_symbol_restricted(token, final = True, leaf = None):
		return token[:1].isalpha() and is_symbol(token, final, leaf)
	def is_operator(token, final = True, leaf = None):
		if token in ['+','-','/','*','^']:
			return True
	def is_separator(token, final = True, leaf = None):
		if token in [',']:
			return True
	def is_ws(token, final = True, leaf = None):
		if len(token.strip()) == 0:
			return True
	def is_group_start(token, final = True, leaf = None):
		return token in ['[','(']
	def is_group_end(token, final = True, leaf = None):
		return token in [']',')']
	def print_group(group, tl, depth=0):
		return tok_print_group(group, tl, depth)
	def group_to_str(group, tl):
		return tok_group_to_str(group, tl)
	token_tests = { 'n':is_number, 's':is_symbol_restricted if parse_strict else is_symbol, 'o':is_operator, 'w':is_ws, ',':is_separator, '[':is_group_start, ']':is_group_end }
	root = {'parent':None, 'children':[], 'token':['?','',''] }
	pre_leafs = [root]
	for ch in func_str:
		post_leafs = []
		for leaf in pre_leafs:
			tp,to,tr = leaf['token']
			nto = to+ch
			if (tp != '?'):
				if token_tests[tp](nto, False, leaf):
					leaf['token'][1] = nto
					post_leafs.append(leaf)
				else:
					if token_tests[tp](to, True, leaf):
						for ttp,tt in token_tests.items():
							if tt(ch, False, leaf):
								nl = {'parent':leaf, 'children':[], 'token':[ttp, ch, ''] }
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
						nl = {'parent':leaf if len(leaf['token'][1]) else None, 'children':[], 'token':[c, nto, ''] }
						leaf['children'].append(nl); post_leafs.append(nl);
		pre_leafs = post_leafs
	if g_dbg:
		print 'Dbg: parsing [{}]'.format(func_str)
	valid_token_lists = []
	for leaf in pre_leafs:
		tp,to,tr = leaf['token']
		if (tp != '?' and token_tests[tp](to, True, leaf)):
			token_list = []
			while leaf:
				token_list.append(leaf['token']); leaf = leaf['parent'];
			valid_token_lists.append(parse_refine_toks(list(reversed(token_list)), const_map))
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
					ptl.append(tl[tli]); ptl.append(('o','*',''));
				elif tli-1>0 and tl[tli-1][0] in [']']:
					ptl.append(('o','*','')); ptl.append(tl[tli]);
				elif tl[tli][2] != 'func' and tli+1<len(tl) and tl[tli+1][0] == '[':
					ptl.append(tl[tli]); ptl.append(('o','*',''));
				else:
					ptl.append(tl[tli])
			elif tl[tli][0] == ']' and tli+1<len(tl) and tl[tli+1][0] == '[':
				ptl.append(tl[tli]); ptl.append(('o','*',''));
			else:
				ptl.append(tl[tli])
		auto_op_token_lists.append(ptl)
	processed_token_lists = auto_op_token_lists
	valid_group_trees = []
	for tl in processed_token_lists:
		gtl, tl_group = parse_root_group(tl)
		if (tl_group):
			#print_group(tl_group, gtl); print '';
			valid_group_trees.append((gtl, tl_group))
	if len(valid_group_trees) == 0:
		if g_dbg:
			print 'Note: invalid expression'
		return (None, None)
	lbd_less_tokens = lambda x,y: len(x[0])-len(y[0])
	sorted_group_trees = sorted(valid_group_trees, cmp=lbd_less_tokens)
	if len(sorted_group_trees)>1 and lbd_less_tokens(sorted_group_trees[0], sorted_group_trees[1]) == 0:
		if g_dbg:
			print 'Note: ambiguous expression'
		return (None, None)
	gt = sorted_group_trees[0]
	if g_dbg:
		print_group(gt[1], gt[0])
		print pp_toks( tok_group_toks(gt[1], gt[0]))
		#print group_to_str(gt[1], gt[0])
	return gt[1], gt[0] # group_tree, toks
def fname_to_sympy(name):
	return name.replace("'", '_p_' if name.upper() != name else '_P_')
def vname_to_sympy(name, sympy_constants):
	pre_repl = name.replace("'", '_p_' if name.upper() != name else '_P_')
	return '_{}'.format(pre_repl) if (name in sympy_constants or not pre_repl[0].isalpha()) else pre_repl
def tok_translate_to_sympy(toks, const_map = k_as_const_map, sympy_constants = k_sympy_constants):
	t_toks = copy.deepcopy(toks); t_transl = {}; t_detransl = {};
	for ti in range(len(t_toks)):
		tok = t_toks[ti]; pre_tok = copy.copy(tok);
		if tok[0] == '[' and len(tok[1]) and tok[1] != '(':
			tok[1] = '('
		elif tok[0] == ']' and len(tok[1]) and tok[1] != ')':
			tok[1] = ')'
		elif tok[0] == 'o' and tok[1] == '^':
			tok[1] = '**'
		elif (tok[0],tok[2]) == ('s','func') and tok[1] == '√':
			tok[1] = 'sqrt'
		elif (tok[0],tok[2]) == ('s','const'):
			tok[1] = const_map[tok[1]]
		elif (tok[0],tok[2]) in [('s','cvar'), ('s','var')]:
			tok[1] = vname_to_sympy(tok[1], sympy_constants); t_transl[pre_tok[1]] = tok[1]; t_detransl[tok[1]] = pre_tok[1];
	return t_toks, t_transl, t_detransl
def def_sym_func(str, symbs):
	for symb in symbs:
		exec('{} = Symbol("{}")'.format(symb, symb))
	ev = eval(str); return ev;
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
		for d in (fctx['eval_constants'],fctx['lvl_composed']['vars']):
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
		for d in (fctx['eval_constants'],fctx['lvl_composed']['vars']):
			d.discard(name)
		fctx['lvl_composed']['vars'].add(name)
def fctx_compose(fctx, toks, f_cand_map, depth=0):
	cis = [x for x in range(len(toks)) if (toks[x][0],toks[x][2]) == ('s','var') and toks[x][1] in f_cand_map]
	if len(cis) == 0:
		if g_dbg and depth != 0:
			print 'Dbg: composed {}'.format(pp_toks(toks))
		return toks
	c_toks = []; pci = -1; name = fctx['lvl_raw']['name'];
	for ci in cis:
		comp_fname = toks[ci][1]; comp_fctx = f_cand_map[comp_fname];
		comp_fctx['lvl_composed']['dependants'].add(name); fctx['lvl_composed']['dependencies'].add(comp_fname);
		c_toks.extend(toks[pci+1:ci]); pci = ci;
		c_toks.extend( comp_fctx['lvl_parsed']['toks'] );
	c_toks.extend(toks[pci+1:])
	return fctx_compose(fctx, c_toks, f_cand_map, depth+1)
def fctx_relambda(fctx):
	fctx['lvl_sympy']['eval_lbd'] = lambdify(fctx['lvl_sympy']['eval_sympy_symbs'], fctx['lvl_sympy']['eval_func'], "numpy")
def fctx_resympify(fctx):
	t_toks, t_transl, t_detransl = tok_translate_to_sympy(fctx['lvl_composed']['toks'], k_as_const_map, k_sympy_constants)
	fctx['lvl_sympy']['toks'] = t_toks; fctx['lvl_sympy']['transl'] = t_transl; fctx['lvl_sympy']['detransl'] = t_detransl;
	cvar_values = fctx['lvl_raw']['cvar_values']
	fctx['lvl_sympy']['eval_toks'] = [x if (x[0],x[2]) != ('s','cvar') else [x[0],repr(cvar_values.get(x[1], 0.5)),x[2]] for x in fctx['lvl_sympy']['toks']]
	fctx['lvl_sympy']['eval_func_str'] = tok_group_to_str(fctx['lvl_composed']['group_tree'], fctx['lvl_sympy']['eval_toks'])
	fctx['lvl_sympy']['eval_sympy_symbs'] = [t_transl[x] for x in fctx['lvl_composed']['vars']]
	fctx['lvl_sympy']['eval_func'] = def_sym_func(fctx['lvl_sympy']['eval_func_str'], fctx['lvl_sympy']['eval_sympy_symbs'])
	fctx_relambda(fctx)
def fctx_recompose(fctx, fctx_map):
	fctx['dependencies'] = Set()
	fctx['lvl_composed']['toks'] = fctx_compose(fctx, fctx['lvl_parsed']['toks'], fctx_map)
	fctx['lvl_composed']['vars'] = sorted(list(set([x[1] for x in fctx['lvl_composed']['toks'] if (x[0],x[2]) == ('s','var')])))
	gtl, ggp = parse_root_group(fctx['lvl_composed']['toks'])
	fctx['lvl_composed']['toks'] = gtl; fctx['lvl_composed']['group_tree'] = ggp;
	#print 'recomp', pp_toks( tok_group_toks(fctx['lvl_composed']['group_tree'], fctx['lvl_composed']['toks'] ))
	fctx_resympify(fctx)
def fctx_reop(fctx, op, fctx_map):
	if op['type'] == 'df':
		sfctx = fctx_map[op['dependency_name']]; dvar = op['dvar'];
		func_str = format_sympy_prepare_func_str(str(diff(sfctx['lvl_sympy']['eval_func'], sfctx['lvl_sympy']['transl'][dvar])))
		fctx_reparse(fctx, func_str, fctx_map, k_format_sympy_const_map)
def fctx_create(func_name=''):
	return {
		'lvl_raw': { 'name':func_name, 'func_str':None, 'cvar_values':{}, 'op_dependencies':{}, 'op_dependants':{}  },
		'lvl_parsed':  { 'group_tree':None, 'toks':None, 'vars':None, 'cvars':None, },
		'lvl_composed': { 'toks':None, 'group_tree':None, 'vars':[], 'dependencies':Set(), 'dependants':Set(), },
		'lvl_sympy': { 'toks':[], 'eval_toks':[], 'transl':[], 'detransl':[],  },
		}
def fctx_create_by_op(op, fctx_map, func_name):
		fctx = fctx_create(func_name)
		fctx['lvl_raw']['op_dependencies'][op['dependency_name']] = op;
		fctx_map[op['dependency_name']]['lvl_raw']['op_dependants'][func_name] = op
		fctx_reop(fctx, op, fctx_map)
		return fctx
def fctx_reparse(fctx, func_str, fctx_map, parse_const_map):
	parse_gt, parse_toks = parse_func_tokenize(func_str, False, parse_const_map)
	parse_vars = sorted(list(set([x[1] for x in parse_toks if (x[0],x[2]) == ('s','var')])))
	parse_cvars = sorted(list(set([x[1] for x in parse_toks if (x[0],x[2]) == ('s','cvar')])))
	fctx['lvl_raw']['func_str'] = func_str; fctx['lvl_parsed']['group_tree'] = parse_gt; fctx['lvl_parsed']['toks'] = parse_toks;
	fctx['lvl_parsed']['vars'] = parse_vars; fctx['lvl_parsed']['cvars'] = parse_cvars;
	fctx_recompose(fctx, fctx_map)
def func_str_to_fctx(func_str, func_name='', fctx_map={}, parse_const_map = k_as_const_map):
	fctx = fctx_create(func_name); fctx_reparse(fctx, func_str, fctx_map, parse_const_map); return fctx;
def fctx_update_dependants(fctx, fctx_map, updated = Set()):
	loc_upd = Set()
	for fn in fctx['lvl_composed']['dependants']:
		loc_upd.add(fn); updated.add(fn); fctx_recompose(fctx_map[fn], fctx_map);
	for fn,op in fctx['lvl_raw']['op_dependants'].items():
		loc_upd.add(fn); updated.add(fn); fctx_reop(fctx_map[fn], op, fctx_map);
	for fn in loc_upd:
		fctx_update_dependants(fctx_map[fn], fctx_map, updated)
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
	return float(inf)
	vlen = max(vlen1, vlen2)
	err = 0.0; ts = [0.0 for x in range(vlen)]
	could_step = True
	while could_step:
		coords = [rlo+t*(rhi-rlo) for t in ts]
		err = max(err, abs(lbdf1(*(coords[:vlen1])) - lbdf2(*(coords[:vlen2]))) )
		could_step = multi_step(ts, h)
	return err
def func_randtest(vars1, vars2, lbdf1, lbdf2, (rlo, rhi, n), excpt = False, seed = None, quiet = False):
	if seed is not None:
		numpy.random.seed(seed)
	vlen1,vlen2 = len(vars1), len(vars2)
	all_vars = sorted(list(set(vars1+vars2)))
	vlen = len(all_vars); coords = [rlo+x*(rhi-rlo) for x in numpy.random.random(int(n)+vlen-1)];
	i_coords1 = [all_vars.index(x) for x in vars1]; i_coords2 = [all_vars.index(x) for x in vars2];
	err = 0.0; i = 0; except_count = 0;
	with numpy.errstate(invalid='raise'):
		while i < len(coords)-vlen+1:
			f_coords = [[coords[i+ci] for ci in i_coords] for i_coords in [i_coords1, i_coords2]]
			has_except = False
			if excpt:
				try:
					f_vals = lbdf1(*f_coords[0]), lbdf2(*f_coords[1])
					err = max(err, abs(f_vals[0] - f_vals[1]))
				except:
					has_except = True; except_count = except_count + 1
			else:
				f_vals = lbdf1(*f_coords[0]), lbdf2(*f_coords[1])
				err = max(err, abs(f_vals[0] - f_vals[1]))
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
	subs_keys = subs.keys(); subs_fctx = [subs[k] for k in subs_keys]; subs_lbds = [x['lvl_sympy']['eval_lbd'] for x in subs_fctx];
	in_vars = Set()
	for sub in subs_fctx + [fctx]:
		for ev in sub['lvl_composed']['vars']:
			in_vars.add(ev)
	in_vars = list(in_vars)
	lbd_inds = [ [in_vars.index(x) for x in sub['lvl_composed']['vars']] for sub in subs_fctx ]
	f_lbd_inds = [in_vars.index(x) for x in fctx['lvl_composed']['vars']]
	subs_inds = [ fctx['lvl_composed']['vars'].index(x) for x in fctx['lvl_composed']['vars'] if x in subs_keys ]
	k_inds = [ in_vars.index(x) for x in in_vars if x.startswith('k') ] if k_int else []
	vlen = len(in_vars); coords = [rlo+x*(rhi-rlo) for x in numpy.random.random(int(n)+vlen-1)];
	k_coords = numpy.random.randint(-10, 11, int(n)+vlen-1)
	subs_vals = [0.0]*len(subs);
	lbd_f = fctx['lvl_sympy']['eval_lbd']; fvlen = len(fctx['lvl_composed']['vars']);
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
					print '{} ({}) = {}'.format(subs_fctx[si]['lvl_raw']['name'], zip(subs_fctx[si]['lvl_composed']['vars'], sub_coords), subs_vals[si])
			if has_except:
				coords[i:i+vlen] = [rlo+x*(rhi-rlo) for x in numpy.random.random(vlen)]
				k_coords[i:i+vlen] = numpy.random.randint(-10, 11, vlen)
			else:
				coords_f = [coords_i[x] for x in f_lbd_inds]
				for j in range(len(subs_inds)):
					coords_f[subs_inds[j]] = subs_vals[j]
				f_val = lbd_f(*coords_f)
				if g_dbg:
					print '{} ({}) = {}'.format(fctx['lvl_raw']['name'], zip(fctx['lvl_composed']['vars'], coords_f), f_val)
				err = max(err, abs(f_val))
				i = i+1
	if (except_count  and not quiet):
		print 'Note: Bypassed {} exceptions.'.format(except_count)
	return err
def print_fctx(fctx, details):
	def print_decl(key, pre_key, decl_len):
		if decl_len == -1 or key != pre_key:
			content = '{}({})'.format(fctx['lvl_raw']['name'], ', '.join(fctx[key]['vars']))
			print content,
			return key,len(content)
		else:
			print ''.join([' ']*decl_len),
			return key,decl_len
	curr_decl = ''; decl_len = -1;
	for det,deti in zip(details, range(len(details))):
		if deti != 0:
			print ''
		if det in ['raw', 'r', '1']:
			curr_decl, decl_len = print_decl('lvl_parsed', curr_decl, decl_len)
			print ' = ', fctx['lvl_raw']['func_str'],
		elif det in ['comp', 'c', '2']:
			curr_decl, decl_len = print_decl('lvl_composed', curr_decl, decl_len)
			print ' = ', pp_toks(tok_group_toks(fctx['lvl_composed']['group_tree'], fctx['lvl_composed']['toks'])),
		elif det in ['sympy', 's', '3']:
			curr_decl, decl_len = print_decl('lvl_composed', curr_decl, decl_len)
			print ' = ', fctx['lvl_sympy']['eval_func_str']
	print '.'
def create_dsl():
	print ' Asinus Salutem'
	return { 'funcs':{}, 'sympy_funcs':{}, 'dbg':False, 'g_dbg':arg_has('-dbg'),
			'sections':[x for x in arg_get('-sections' if arg_has('-sections') else '-section', '').split(',') if len(x)],
			'cur_sec':None, 'is_focus_sec':True }
def dsl_is_focus_sec(dsl):
	return len(dsl['sections']) == 0 or dsl['cur_sec'] in dsl['sections']
def dsl_add_fctx(dsl, name, fctx, allow_update, quiet):
	old_func = None; dsl['funcs'].get(name, None); old_dep = None;
	if name in dsl['funcs']:
		old_func = dsl['funcs'][name]; old_dep = (old_func['lvl_raw']['op_dependants'], old_func['lvl_composed']['dependants'])
	if (not allow_update and old_func is not None):
		raise Exception("Function already exists, use the proper command (e.g 'relet') to update it.")
	dsl['funcs'][name] = fctx; fctx['lvl_raw']['name'] = name;
	if (not quiet):
		print_fctx(fctx, ['comp'])
	if (old_dep is not None) and (len(old_dep[0]) or len(old_dep[1])):
		fctx['lvl_raw']['op_dependants'] = copy.copy(old_dep[0])
		fctx['lvl_composed']['dependants'] = copy.copy(old_dep[1])
		updated = Set()
		fctx_update_dependants(fctx, dsl['funcs'], updated)
		if (not quiet):
			print '   [{}]'.format(', '.join(updated))
def process_dsl_command(dsl, inp, quiet=False):
	global g_dbg
	funcs = dsl['funcs'];
	input_splt = inp.split(' ')
	cmd = input_splt[0]
	dsl['dbg'] = True if ('-dbg' in input_splt) else dsl['g_dbg']; g_dbg = dsl['dbg'];
	is_focus_sec = dsl_is_focus_sec(dsl);
	if (cmd in ['let', 'relet']):
		name = input_splt[1]
		if '[' in name or '(' in name:
			name_splt = name.split('(' if '(' in name else '[')
			name = name_splt[0]; fvars = name_splt[1].split(',');
		func_str = ' '.join(input_splt[3:])
		if not func_str.startswith('PD('):
			fctx = func_str_to_fctx(func_str, name, dsl['funcs'])
			dsl_add_fctx(dsl, name, fctx, cmd == 'relet', quiet or not is_focus_sec)
		else: #TODO generalize this 2*PD(f1,x)+...
			gt, toks = parse_func_tokenize(func_str, False, {})
			fn,varn = toks[3][1], toks[5][1]
			fctx = fctx_create_by_op({'dependant_name':name, 'dependency_name':fn, 'dvar':varn, 'type':'df'} , funcs, name)
			dsl_add_fctx(dsl, name, fctx, '-u' in input_splt, quiet or not is_focus_sec)
	elif (cmd == 'bake'):
		fn,bn = input_splt[1], input_splt[2]
		bfunc = copy.deepcopy(funcs[fn]); bfunc['name'] = bn; bfunc['comps'] = Set();
		dsl_add_fctx(dsl, bn, bfunc, '-u' in input_splt, quiet or not is_focus_sec)
	elif (cmd in ['ftest', 'frandtest', 'fgridtest'] and is_focus_sec):
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
		print func_test(funcs[n1]['lvl_composed']['vars'], funcs[n2]['lvl_composed']['vars'], funcs[n1]['lvl_sympy']['eval_lbd'], funcs[n2]['lvl_sympy']['eval_lbd'], lo_hi_n, excpt, seed = seed, quiet = quiet)
	elif (cmd == 'ztest' and is_focus_sec):
		fn = input_splt[1]; fctx = funcs[fn];
		k_int = False; subs = {}; lo_hi_n = [-1.0, 1.0, 1000]; seed = None;
		state = ''; state_ev = ''; state_lhn = 0; excpt = False
		for part in input_splt[2:]:
			if state == '':
				if part in fctx['lvl_composed']['vars']:
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
	elif (cmd in ['print', 'p'] and is_focus_sec):
		name = input_splt[1]; details = input_splt[2].split(',') if len(input_splt) > 2 else ['comp'];
		print_fctx(funcs[name], details)
	elif (cmd in ['echo'] and is_focus_sec):
		print ' '.join(input_splt[1:])
	elif (cmd in ['section']):
		sec = input_splt[1]; dsl['cur_sec'] = sec;
		is_focus_sec = len(dsl['sections']) == 0 or dsl['cur_sec'] in dsl['sections']
		if len(input_splt) >= 2 and is_focus_sec:
			print ' '.join(input_splt[2:])
	elif (cmd in ['fnames']):
		print ' '.join(['[{}]'.format(k) for k in funcs.keys()])
	elif (cmd in ['?', 'calc', 'eval'] and is_focus_sec):
		print subs_sym_str(to_sym_str(' '.join(input_splt[1:])), [ ( name_to_sympy(x['name'], True),x['lvl_sympy']['eval_lbd']) for x in funcs.values()])
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
				if echo and dsl_is_focus_sec(dsl):
					print ' >', line;
				if line == 'quit':
					break
				process_dsl_command(dsl, line, quiet=quiet);
		if arg_has('-interact'):
			enter_dsl(dsl)
	elif arg_has('-test'):
		print func_str_to_fctx('A*cos(x)+B*sin(y)+[1^2*cos(x)]')
	else:
		enter_dsl(create_dsl())
