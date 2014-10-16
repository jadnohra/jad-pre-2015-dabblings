import time
import copy
import sys

def base_sentences(nvars):
	base_vars = [['p'], ['q'], ['r']]
	for i in range(nvars-len(base_vars)):
		base_vars.append(['p'+str(i+1)])
	return base_vars[:nvars]	

def s_disj(s1, s2):
	return [s1, 'v', s2]

def s_neg(s):
	return ['^', s]

def subs(src, old, new):
	def subs_inplace(dest, old, new):
		for i in range(len(dest)):
			if (dest[i] == old):
					dest[i] = new
			elif (isinstance(dest[i], list)):
				subs_inplace(dest[i], old, new)	
	dest = copy.deepcopy(src)
	subs_inplace(dest, old, new)
	return dest

def s_impl(s1, s2):
	return [s1, '=>', s2]

schema_infos = [
	{ 'schema':s_impl(s_disj(['x'],['x']), ['x']), 'symbols':[['x']] },
	{ 'schema':s_impl(['x'], s_disj(['x'],['y'])), 'symbols':[['x'], ['y']] },
	{ 'schema':s_impl(s_disj(['x'],['y']), s_disj(['y'],['x'])), 'symbols':[['x'], ['y']] },
	#{ schema:s_impl(s_impl(['x'], ['y']),s_impl(s_disj(['z'],['x']), s_disj(['z'],['y']))), 'symbols':[['x'], ['y'], ['z']] },
	{ 'schema':s_impl(s_impl(['x'], ['y']),s_impl(s_impl(['z'],['x']), s_impl(['z'],['y']))), 'symbols':[['x'], ['y'], ['z']] },
	]

def try_mponens(a, b):
	if (len(b) == 3 and b[1] == '=>' and a == b[0]):
		return b[2]
	if (len(a) == 3 and a[1] == '=>' and b == a[0]):
		return a[2]
	return None	

def s_len(lst):
	if (isinstance(lst, list)):
		return sum([s_len(x) for x in lst])
	else:
		return 1

def s_nice(lst):
	if (isinstance(lst, list)):
		#print lst
		return '(' + ''.join([s_nice(x) for x in lst]) + ')' if len(lst) > 1 else lst[0]
	else:
		return lst

def gen_axioms(schema_info, base_subs):
	def gen_axioms_rec(info, si, axiom, base_subs, axioms):
		if (si < len(info['symbols'])):
			symb = info['symbols'][si]
			for i in range(len(base_subs)):
				naxiom = subs(axiom, symb, base_subs[i])
				gen_axioms_rec(info, si+1, naxiom, base_subs, axioms)
		else:
			axioms.append(axiom)
	axioms = []
	gen_axioms_rec(schema_info, 0, list(schema_info['schema']), base_subs, axioms)
	#print axioms; raw_input("....");	
	return axioms				

def gen_proofs(depth, thm_len, base_subs, thms, proof = [], cnt=[0]):
	def bleep():
		if (cnt[0] == 1 or cnt[0] % 10000 == 0):
			sys.stdout.flush()
			print '<{0}>\r'.format(cnt[0]),
	#print proof; raw_input("...");
	n = len(proof)
	if (n >= depth):
		return	
	for si in schema_infos:
		#print s_nice(si['schema']); raw_input("...");	
		axioms = gen_axioms(si, base_subs)
		for axm in axioms:
			#print s_nice(axm); raw_input("..");	
			if (axm not in proof):	
				nproof = list(proof); nproof.append(axm); cnt[0] = cnt[0]+1; bleep();
				gen_proofs(depth, thm_len, base_subs, thms, nproof, cnt)
	for ti in range(len(proof)):
		for tj in range(ti+1,len(proof)):
			#print s_nice(proof[ti]), s_nice(proof[tj])
			thm = try_mponens(proof[ti], proof[tj])
			if (thm is not None):
				if (s_len(thm) <= thm_len):
					if (thm not in thms):
						print s_nice(thm)
						thms.append(thm)
				if (thm not in proof):
					nproof = list(proof); nproof.append(thm); cnt[0] = cnt[0]+1; bleep();
					gen_proofs(depth, thm_len, base_subs, thms, nproof, cnt)	

#print try_mponens(['x','=>','y'], ['x'])


if 0:
	T1 = s_impl(s_disj(['p'], ['p']), ['p'])
	T2_1 = s_impl(s_disj(['p'], ['p']), ['p'])
	T2_2 = s_impl(['p'], s_disj(['p'], ['p']))
	T2_3 = s_impl(['p'], ['p'])
	T2 = s_impl(T2_1, s_impl(T2_2, T2_3))
	print s_nice(T1)
	print s_nice(T2)
	print s_len(T1), s_len(T2)
	print try_mponens(T1, T2)

if 1:
	arg_base = int(sys.argv[sys.argv.index('-base')+1]) if ('-base' in sys.argv) else 1
	base_subs = base_sentences(arg_base)
	base_subs.append(s_disj(['p'], ['p'])) 
	base_subs.append(s_disj(['q'], ['q'])) 
	
	arg_len = int(sys.argv[sys.argv.index('-len')+1]) if ('-len' in sys.argv) else 3
	arg_depth = int(sys.argv[sys.argv.index('-depth')+1]) if ('-depth' in sys.argv) else 3
	ts = time.time()
	thms = []; gen_proofs(arg_depth, arg_len, base_subs, thms);
	print time.time() - ts, 'sec.'
	for thm in thms:
		print s_nice(thm)


