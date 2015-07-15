# coding=utf-8
# Allow function names like f' (replace when doing exe with func names)
# Fix double parenthesis, sometimes they are legal cos(f1(3))

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

def to_std_delims(frag):
	return frag.replace('[','(').replace(']',')')#.replace('((', '(').replace('))', ')')
def to_sym_exp(frag):
	return frag.replace('^', '**').replace('√', 'sqrt')
def to_std_exp(frag):
	return frag.replace('**', '^').replace('sqrt', '√')
def to_sym_str(func_str):
	return to_sym_exp(to_std_delims(func_str))
def parse_func_str(func_str):
	symbols = []; numbers = []; constants = [];
	matches = re.findall(_ctx['parse_func_re'], func_str)
	for m in matches:
		subm = re.findall(_ctx['parse_number_re'], m)
		if len(subm) == 1 and subm[0] == m:
			numbers.append(m)
			continue
		subm = re.findall(_ctx['parse_symb_re'], m)
		if len(subm) == 1 and subm[0] == m:
			if (m.lower() in _ctx['parse_constants']):
				constants.append(m)
			else:
				symbols.append(m)
			continue
	return (symbols, numbers, constants)
def form_func_str(func_str, cprefix = 'C'):
	(symbols, numbers, constants) = parse_func_str(func_str)
	form_func = func_str; form_consnts = [];
	for num in numbers:
		if ( floor(float(num)) != float(num)):
			form_c = '{}{}'.format(cprefix, len(form_consnts)); form_consnts.append(form_c);
			form_func = form_func.replace(num, form_c)
	return form_func, form_consnts
def eval_sym_str(str, symbs, do_expand):
	for symb in symbs:
		exec('{} = Symbol("{}")'.format(symb, symb))
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
def parse_vars_func_str(func_str, case_swap=False):
	(parse_symbols, parse_numbers, parse_constants) = parse_func_str(func_str)
	parse_symbols = sorted(parse_symbols)
	eval_vars = []; eval_constants = [];
	for symb in parse_symbols:
		is_var = (symb.upper() != symb)
		if ( is_var if (not case_swap) else (not is_var) ):
			if (symb not in eval_vars):
				eval_vars.append(symb)
		else:
			if (symb not in eval_constants):
				eval_constants.append(symb)
	return (parse_symbols, parse_numbers, parse_constants, eval_vars, eval_constants)
def subs_functional_vars(func_name, func_str, func_var_map, case_swap, comps = Set()):
	(parse_symbols, parse_numbers, parse_constants, eval_vars, eval_constants) = parse_vars_func_str(func_str, case_swap)
	subs_func_str = func_str
	for fv in eval_vars:
		if (fv in func_var_map):
			comps.add(fv); func_var_map[fv]['is_comp'].add(func_name);
			sub_re = re.compile(ur'{}'.format(fv))
			subs_func_str = sub_re.sub('({})'.format(func_var_map[fv]['func_str']), subs_func_str)
	if (subs_func_str != func_str):
		return subs_functional_vars(func_name, subs_func_str, func_var_map, case_swap, comps)
	else:
		return subs_func_str
def func_update_sym_func(fctx, sym_func, eval_vars, eval_constants):
	fctx['sym_func'] = sym_func
	eval_func = sym_func
	for cnt in eval_constants:
		eval_func = eval_func.subs(cnt, val_of_const(fctx, cnt))
	fctx['eval_vars'] = copy.copy(eval_vars); fctx['eval_constants'] = copy.copy(eval_constants);
	fctx['eval_func'] = eval_func
	fctx['eval_func_exp'] = expand(eval_func)
	fctx['lambd_f'] = lambdify(eval_vars, expand(eval_func), "numpy")
def func_str_to_sym(_func_str, case_swap=False, functional_var_map={}, func_name=''):
	fctx = { 'eval_constants':Set(), 'eval_vars':Set(), 'values':{}, 'comp_map':{}, 'comps':Set(), 'is_comp':Set() };
	fctx['_func_str'] = _func_str; fctx['case_swap'] = case_swap;
	func_str = to_sym_str(_func_str); fctx['func_str'] = func_str;
	func_str = subs_functional_vars(func_name, func_str, functional_var_map, case_swap, fctx['comps'])
	(parse_symbols, parse_numbers, parse_constants, eval_vars, eval_constants) = parse_vars_func_str(func_str, case_swap)
	for ev in eval_vars:
		def_var(fctx, ev)
	for ec in eval_constants:
		def_const(fctx, ec)
	fctx['parse_symbols'] = parse_symbols; fctx['parse_numbers'] = parse_numbers; fctx['parse_constants'] = parse_constants;
	sym_func = eval_sym_str(func_str, eval_vars + eval_constants, False);
	func_update_sym_func(fctx, sym_func, eval_vars, eval_constants)
	return fctx
def func_sym_to_df(fctx, var):
	dfctx = copy.copy(fctx)
	dsym_func = diff(dfctx['sym_func'], var)
	func_update_sym_func(dfctx, dsym_func, dfctx['eval_vars'], dfctx['eval_constants'])
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
def func_gridtest(vlen1, vlen2, lbdf1, lbdf2, (rlo, rhi, h)):
	vlen = max(vlen1, vlen2)
	err = 0.0; ts = [0.0 for x in range(vlen)]
	could_step = True
	while could_step:
		coords = [rlo+t*(rhi-rlo) for t in ts]
		err = max(err, abs(lbdf1(*(coords[:vlen1])) - lbdf2(*(coords[:vlen2]))) )
		could_step = multi_step(ts, h)
	return err
def func_randtest(vlen1, vlen2, lbdf1, lbdf2, (rlo, rhi, n)):
	vlen = max(vlen1, vlen2); coords = [rlo+x*(rhi-rlo) for x in numpy.random.random(int(n)+vlen-1)];
	err = 0.0;
	for i in range(len(coords)-vlen+1):
		err = max(err, abs(lbdf1(*(coords[i:i+vlen1])) - lbdf2(*(coords[i:i+vlen2]))) )
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
def pp_func_args(fctx):
	return '{}({})'.format(fctx['name'], ', '.join(fctx['eval_vars']))
def pp_func_args_val(fctx, ltex = False):
	eval_str = latex(fctx['eval_func']) if ltex else to_std_exp(str(fctx['eval_func']))
	return '{} = {}'.format(pp_func_args(fctx), eval_str)
def pp_func_args_orig(fctx, empty_name = False, ltex = False):
	return '{} = {}'.format(''.join([' ']*len(pp_func_args(fctx))) if empty_name else pp_func_args(fctx, ltex), fctx['_func_str'])
def resubs_functional_vars(name, funcs, updated = Set()):
	for fn in funcs[name]['is_comp']:
		updated.add(fn)
		funcs[fn] = func_str_to_sym(funcs[fn]['_func_str'], functional_var_map=funcs, func_name=fn); funcs[fn]['name'] = fn;
	for fn in funcs[name]['is_comp']:
		resubs_functional_vars(funcs[fn]['name'], funcs, updated)
def create_dsl():
	print ' Asinus Salutem'
	return { 'funcs':{}, 'dbg':False, 'g_dbg':arg_has('-dbg'),
			'sections':[x for x in arg_get('-sections', '').split(',') if len(x)],
			'cur_sec':None }
def process_dsl_command(dsl, inp, quiet=False):
	global g_dbg
	funcs = dsl['funcs']
	input_splt = inp.split(' ')
	cmd = input_splt[0]
	dsl['dbg'] = True if ('-dbg' in input_splt) else dsl['g_dbg']; g_dbg = dsl['dbg'];
	run_sec = len(dsl['sections']) == 0 or dsl['cur_sec'] in dsl['sections']
	if (cmd in ['func', 'fun', 'fn', 'f']):
		name = input_splt[1]
		func_str = ' '.join(input_splt[2:])
		fctx = func_str_to_sym(func_str, func_name=name, functional_var_map=funcs)
		old_func = funcs.get(name, None); old_is_comp = old_func['is_comp'] if old_func else None;
		funcs[name] = fctx; fctx['name'] = name;
		if (not quiet and run_sec):
			print ' ', pp_func_args_val(fctx)
		if (old_is_comp is not None) and len(old_is_comp):
			fctx['is_comp'] = old_is_comp
			updated = Set()
			resubs_functional_vars(name, funcs, updated)
			if (not quiet and run_sec):
				print '   [{}]'.format(', '.join(updated))
	elif (cmd == 'bake'):
		n1,n2 = input_splt[1], input_splt[2]
		funcs[n2] = copy.copy(funcs[n1]); funcs[n2]['name'] = n2; funcs[n2]['comps'] = Set();
	elif (cmd == 'd'):
		fn,varn = input_splt[1], input_splt[2]
		name = 'd({},{})'.format(fn, varn)
		funcs[name] = func_sym_to_df(funcs[fn], varn); funcs[name]['name'] = name; funcs[name]['_func_str'] = 'd({},{})'.format(funcs[fn]['_func_str'], varn);
	elif (cmd in ['ftest', 'frandtest', 'fgridtest'] and run_sec):
		n1,n2 = input_splt[1], input_splt[2]
		rng = (-1.0,1.0,1000)
		rng = [float(input_splt[3:][i]) if (len(input_splt[3:]) > i) else rng[i] for i in range(len(rng))]
		func_test = func_randtest if (cmd != 'fgridtest') else func_gridtest
		print func_test(len(funcs[n1]['eval_vars']), len(funcs[n2]['eval_vars']), funcs[n1]['lambd_f'], funcs[n2]['lambd_f'], rng)
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
		name = input_splt[1]; ltex = '-latex' in input_splt;
		print ' ', pp_func_args_val(funcs[name], ltex = ltex)
		if len(funcs[name]['comps']):
			print ' ', pp_func_args_orig(funcs[name], empty_name = True, ltex = ltex)
	elif (cmd in ['echo'] and run_sec):
		print ' '.join(input_splt[1:])
	elif (cmd in ['section']):
		sec = input_splt[1]; dsl['cur_sec'] = sec;
		run_sec = len(dsl['sections']) == 0 or dsl['cur_sec'] in dsl['sections']
		if len(input_splt) >= 2 and run_sec:
			print ' '.join(input_splt[2:])
	elif (cmd in ['?', 'calc', 'eval'] and run_sec):
		print subs_sym_str(to_sym_str(' '.join(input_splt[1:])), [ (x['name'],x['lambd_f']) for x in funcs.values()])
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
