from sympy import *
import numpy as numpy
import re
import sys
import copy
from sets import Set
import traceback

execfile('helper_arg.py')
execfile('helper_math1.py')

_ctx = {
	'parse_func_re' : re.compile(ur'((?:\d+\.*(?:0?e-\d+)?\d*(?!\w)|\w+)\b)(?!\s*\()'),
	'parse_number_re' : re.compile(ur'\d+\.*(?:0?e-\d+)?\d*(?!\w)'),
	'parse_symb_re' : re.compile(ur'\w+'),
	'parse_arithm_re' : re.compile(ur'(\+|\-|\*|\/|\\|\^|\.)'),
	'parse_constants' : 'pi'.split(','),
	'parse_func_func_re' : re.compile(ur'\w+(?=\()'),
}

def to_std_delims(frag):
	return frag.replace('[','(').replace(']',')').replace('((', '(').replace('))', ')')
def to_sym_exp(frag):
	return frag.replace('^', '**')
def to_std_exp(frag):
	return frag.replace('**', '^')
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
	sym_func = eval_sym_str(func_str, eval_vars + eval_constants, False); fctx['sym_func'] = sym_func;
	eval_func = sym_func
	for cnt in eval_constants:
		eval_func = eval_func.subs(cnt, val_of_const(fctx, cnt))
	fctx['lambd_vars'] = copy.copy(eval_vars)
	fctx['eval_func'] = eval_func;
	fctx['eval_func_exp'] = expand(eval_func);
	fctx['lambd_f'] = lambdify(eval_vars, expand(eval_func), "numpy")
	fctx['eval_df'] = []; fctx['lambd_df'] = [];
	for ev in eval_vars:
		df = diff(eval_func, ev)
		fctx['eval_df'].append(df)
		fctx['lambd_df'].append(lambdify(eval_vars, df, "numpy"))
	return fctx
def multi_step(ts, h):
	could_step = False
	for ti in range(len(ts)):
		ts[ti] = ts[ti] + h
		if (ts[ti] <= 1.0):
			could_step = True; break;
		else:
			ts[ti] = 0.0
	return could_step
def func_dist(vlen1, vlen2, lbdf1, lbdf2, (rlo, rhi, h)):
	vlen = max(vlen1, vlen2)
	err = []; ts = [0.0 for x in range(vlen)]
	could_step = True
	while could_step:
		coords = [rlo+t*(rhi-rlo) for t in ts]
		err.append( m_sq(lbdf1(*(coords[:vlen1])) - lbdf2(*(coords[:vlen2]))) )
		could_step = multi_step(ts, h)
	try:
		return vec_norm(err)
	except:
		print err
		return float('inf')
def pp_func_args(fctx):
	return '{}({})'.format(fctx['name'], ', '.join(fctx['lambd_vars']))
def pp_func_args_val(fctx):
	return '{} = {}'.format(pp_func_args(fctx), to_std_exp(str(fctx['eval_func'])))
def pp_func_args_orig(fctx, empty_name = False):
	return '{} = {}'.format(''.join([' ']*len(pp_func_args(fctx))) if empty_name else pp_func_args(fctx), fctx['_func_str'])
def resubs_functional_vars(name, funcs, updated = Set()):
	for fn in funcs[name]['is_comp']:
		updated.add(fn)
		funcs[fn] = func_str_to_sym(funcs[fn]['_func_str'], functional_var_map=funcs, func_name=fn); funcs[fn]['name'] = fn;
	for fn in funcs[name]['is_comp']:
		resubs_functional_vars(funcs[fn]['name'], funcs, updated)
def create_dsl():
	print ' Asinus Salutem'
	return { 'funcs':{} }
def process_dsl_command(dsl, inp, quiet=False):
	funcs = dsl['funcs']
	input_splt = inp.split(' ')
	cmd = input_splt[0]
	if (cmd == 'e'):
		go_dsl = False
	elif (cmd == 'q'):
		go_dsl = False; sys_exit = True;
	elif (cmd in ['func', 'fun', 'fn', 'f']):
		name = input_splt[1]
		func_str = ' '.join(input_splt[2:])
		fctx = func_str_to_sym(func_str, func_name=name, functional_var_map=funcs)
		old_func = funcs.get(name, None); old_is_comp = old_func['is_comp'] if old_func else None;
		funcs[name] = fctx; fctx['name'] = name;
		if (not quiet):
			print ' ', pp_func_args(fctx)
		if (old_is_comp is not None) and len(old_is_comp):
			fctx['is_comp'] = old_is_comp
			updated = Set()
			resubs_functional_vars(name, funcs, updated)
			if (not quiet):
				print '   [{}]'.format(', '.join(updated))
	elif (cmd == 'dist'):
		n1,n2 = input_splt[1], input_splt[2]
		rng = (-1.0,1.0,0.05)
		rng = [input_splt[3:][i] if (len(input_splt[3:]) > i) else rng[i] for i in range(len(rng))]
		print func_dist(len(funcs[n1]['lambd_vars']), len(funcs[n2]['lambd_vars']), funcs[n1]['lambd_f'], funcs[n2]['lambd_f'], rng)
	elif (cmd in ['print', 'p']):
		name = input_splt[1]
		print ' ', pp_func_args_val(funcs[name])
		if len(funcs[name]['comps']):
			print ' ', pp_func_args_orig(funcs[name], empty_name = True)
	elif (cmd in ['?', 'calc', 'eval']):
		#coords = inp.split('(')[1].split(')')[0].split(',')
		#coords = [float(x) for x in coords if is_float(x)]
		#print funcs[fn]['lambd_f'](*coords)
		print '.'
def enter_dsl():
	go_dsl = True; sys_exit = False;
	dsl = create_dsl()
	while go_dsl:
		try:
			print ' >',
			inp = raw_input()
			process_dsl_command(inp, dsl)
		except:
			traceback.print_exc()
		#e = sys.exc_info()[0]; raise e;
	if (sys_exit):
		sys.exit()
if not sys.flags.interactive:
	if arg_has('-pyexec'):
		execfile(arg_get('-pyexec', ''))
	elif arg_has('-pyscript'):
		with open(arg_get('-pyscript', ''), "r") as ifile:
			for line in [x.rstrip() for x in ifile.readlines()]:
				print line; exec(line);
	elif arg_has('-script'):
		with open(arg_get('-script', ''), "r") as ifile:
			dsl = create_dsl(); quiet = arg_has('-quiet');
			for line in [x.rstrip() for x in ifile.readlines()]:
				if not quiet:
					print ' >', line;
				process_dsl_command(dsl, line, quiet=quiet);
	elif arg_has('-test'):
		print func_str_to_sym('A*cos(x)+B*sin(y)+[1^2*cos(x)]')
	else:
		enter_dsl()
