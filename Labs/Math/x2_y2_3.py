
def is_div(a,d):
	return 1.0*a/d == float(int(1.0*a/d))

def have_common(a,b):
	for f in range(2,min(a,b)+1):
		if is_div(a,f) and is_div(b,f):
			return True
	return False


rr=1000
for c in range(1,rr):
	for d in range(1,rr):
		for f in range(2,min(c,d)+1):
			if (not have_common(c,d)):
				for a in range (1,rr):
					num=c*c+a*a*d*d
					if (is_div(num, 3)):
						#print 'found: ' + `c` + '^2' + '+' + `a` + '^2 x ' + `d` + '^2'
						if (not is_div(a, 3)):
							print 'found: ' + `c` + '^2' + '+' + `a` + '^2 x ' + `d` + '^2'
							sys.exit()
