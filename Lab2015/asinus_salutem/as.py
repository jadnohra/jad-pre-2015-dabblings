from sympy import *
import numpy as numpy
import re
import sys
import copy
from sets import Set

execfile('helper_arg.py')
execfile('helper_math1.py')

_ctx = {
	'parse_func_re' : re.compile(ur'((?:\d+\.*(?:0?e-\d+)?\d*(?!\w)|\w+)\b)(?!\s*\()'),
	'parse_number_re' : re.compile(ur'\d+\.*(?:0?e-\d+)?\d*(?!\w)'),
	'parse_symb_re' : re.compile(ur'\w+'),
	'parse_arithm_re' : re.compile(ur'(\+|\-|\*|\/|\\|\^|\.)'),
	'parse_constants' : 'pi'.split(','),
}

def to_std_delims(frag):
	return frag.replace('[','(').replace(']',')')
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
def func_str_to_sym(_func_str, case_swap=False):
	fctx = { 'eval_constants':Set(), 'eval_vars':Set(), 'values':{} }; fctx['_func_str'] = _func_str; fctx['case_swap'] = case_swap;
	func_str = to_sym_str(_func_str); fctx['func_str'] = func_str;
	(parse_symbols, parse_numbers, parse_constants) = parse_func_str(func_str)
	fctx['parse_symbols'] = parse_symbols; fctx['parse_numbers'] = parse_numbers; fctx['parse_constants'] = parse_constants;
	parse_symbols = sorted(parse_symbols)
	eval_vars = []; eval_constants = [];
	for symb in parse_symbols:
		is_var = (symb.upper() != symb)
		if ( is_var if (not case_swap) else (not is_var) ):
			print 'Registered variable', symb
			def_var(fctx, symb)
			#	if (not has_range(symb)):
			#		def_range(symb, [-1.0,1.0,0.1])
			if (symb not in eval_vars):
				eval_vars.append(symb)
		else:
			print 'Registered constant', symb
			def_const(fctx, symb)
			if (symb not in eval_constants):
				eval_constants.append(symb)
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
def func_dist(fvars, lbdf1, lbdf2, (rlo, rhi, h)):
	err = []; ts = [0.0 for x in range(len(fvars))]
	could_step = True
	while could_step:
		coords = [rlo+t*(rhi-rlo) for t in ts]
		err.append(m_sq(lbdf1(*coords) - lbdf2(*coords)))
		could_step = multi_step(ts, h)
	return vec_norm(err)
if not sys.flags.interactive:
	if arg_has('-pyscript'):
		execfile(arg_get('-pyscript', ''))
	else:
		print func_str_to_sym('A*cos(x)+B*sin(y)+[1^2*cos(x)]')
