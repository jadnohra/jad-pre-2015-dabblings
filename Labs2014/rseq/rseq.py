import sys
import math
import fractions
from fractions import Fraction as frac
import urllib

def argv_get(k, dft):
	return sys.argv[sys.argv.index(k)+1] if k in sys.argv else dft

def lcm(a,b):
	return (a*b)/fractions.gcd(a, b)

def alcm(s):
	v = s[0]
	for i in range(1,len(s)):
		v = lcm(v, s[i])
	return v	

def query_oeis(str_seq):
	resp = urllib.urlopen('http://oeis.org/search?fmt=text&q='+str_seq).read()
	#print resp
	if ('No results.' in resp):
		return False
	else:
		return True

xstep0 = frac(argv_get('xstep', '1/50'))
ystep0 = frac(argv_get('ystep', '1/100'))
n = int(argv_get('n', 20))
dbg = '-dbg' in sys.argv

tested = {}
xstep = xstep0
ystep = ystep0
for xit in range(12):

	for yit in range(7):
		rseq = []

		for i in range(n):
			fx = frac(i,1)*xstep
			y = math.sin(float(fx))
			fy = frac(0,1)
			while (float(fy+ystep) < y):
				fy = fy + ystep
			while (float(fy-ystep) > y):
				fy = fy - ystep
			rseq.append(fy)

		mul_num = alcm([x.denominator for x in rseq])
		seq = [x*mul_num for x in rseq]
		seq = [x.numerator if x.denominator == 1 else 'n/a' for x in seq]
		str_seq = str(seq).strip('[]').replace(' ','')
		if (str_seq not in tested):
			tested[str_seq] = True
			if dbg:
				print str_seq
			if query_oeis(str_seq):
				print ' >>> FOUND:', str_seq	
		
		ystep = ystep*frac('1/2')
	
	
	xstep = xstep*frac('19/20')
	ystep = ystep0



