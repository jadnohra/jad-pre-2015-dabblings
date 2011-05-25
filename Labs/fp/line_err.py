import array
from PIL import Image


def test_1(a, b, c, min, max, inc, prt = True):
	x = float(min)
	hit = 0.0
	miss = 0.0
	eps = 0.0
	
	while (x <= max):
		y=-(a*x+c)/b
		d = a*x+b*y+c
		if (d == 0.0):
			hit += 1.0
		else:
			miss += 1.0
		if (abs(d) > eps):
			eps = abs(d)
		x += inc	
	ratio = hit/(hit+miss)	
	if (prt):
		print (`hit` + "/" + `miss` + " = " + `(100.0*ratio)` + "%, eps = " + `eps`)
	return ratio

def testimg_1(size, diva, divc, fi):
	#im = Image.new("L", (100, 100))
	buff = array.array('B', [128] * size*size)

	for a in range(1, size+1):
		for c in range(1, size+1):	
			ratio = test(a/diva, 1.0, c/divc, -10, 10, 0.1, False)
			#print ratio
			#if (c < 5): ratio = 0
			buff[(a-1)+size*((size-c))] = (int) (ratio * 255)
			


	im = Image.frombuffer("L", (size,size), buff, "raw", "L", 0, 1)
	path="d:/lerr/i_" + `fi` + ".jpg"
	im.save(path)
	print(path)
	#im.show()


#1_test(0.1, 1, 0.1, -10, 10, 0.1)
#1_testimg(200, 10000.0)

def testimgs_1():
	c = 1
	for i in range(0, 100):
		testimg_1(200, 1000.0, float(1000.0 - float(i)/10.0), c)
		c += 1
		
def test_2(a, b, c, refx, wnd, res, fi, der=0.0):
	refy=-(a*refx+c)/b
	minx=refx-wnd*0.5
	inc=wnd/res
	miny=refy-wnd*0.5

	buff_t = array.array('f', [0.5] * res*res)
	buff = array.array('B', [128] * res*res)
	buff_b = array.array('B', [128] * res*res)
	x=minx
	y=miny
	mine=10000.0
	maxe=-10000.0

	print "ref "+ `refx` +","+`refy`

	for xi in range(0, res):
		for yi in range(0, res):
			#err = a*x+b*y+c
			#print `x` +","+`y`+"->"+`err`
			err = abs(a*x+b*y+c)
			
			#if (yi < 5):
			#	err = 0.0

			buff_t[xi+res*((res-yi)-1)] = err
			if (err >= maxe):
				maxe = err
			if (err <= mine):
				mine = err
				
			y+=inc
		x+=inc
		y=miny
	
	erange=maxe-mine
	norm=(1.0/erange)*255.0
	if (der != 0.0):
		norm=(1.0/der)*255.0

	
	for i in range(0,res*res):
		rn = ((buff_t[i]-mine)*norm)
		if (rn > 255.0):
			rn = 255.0
		buff[i] = int(rn)
		if (buff_t[i]==0.0):
			buff_b[i] = 255
		else:
			buff_b[i] = 0

	im = Image.frombuffer("L", (res,res), buff, "raw", "L", 0, 1)
	path="d:/lerr/z_" + `fi` + ".jpg"
	im.save(path)
	print(path)
	#im.show()
	
	im_b = Image.frombuffer("L", (res,res), buff_b, "raw", "L", 0, 1)
	path="d:/lerr/zb_" + `fi` + ".jpg"
	im_b.save(path)
	print(path)
	#im_b.show()

for t in range(0,500):
	test_2(0.1 + t/100.0, -1.0, 1.0, 1.0, 0.0000001, 256, 1000+t, 0.0000001);	
