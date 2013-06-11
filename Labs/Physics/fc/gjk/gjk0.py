import math
import time
import pprint
import os
import random
import pyglet

#execfile('../../../gaussy/gaussy.py')
execfile('Z:/Personal/Lab/gaussy/gaussy.py')

gTestEpa = True

def test_gjk_distance(p1, cvx1, p2, cvx2):
	global gTestEpa
	if (gTestEpa):
		return gjk_epa_distance(p1, cvx1, p2, cvx2, 0.0001)
	else:
		return gjk_distance(p1, cvx1, p2, cvx2)


class World:
	g = -9.8 * 1.0
	timeScale = 1.0
	timeStep = 1.0/60.0
#	timeStep = 1.0/160.0
	lastTime = -1.0
	perfTime = -1.0
	frame = -1
	dt = 0.0
	statics = None
	kinetics = None
	particles = None
	forces = None
	staticContactPairs = None
	contactPairs = None
	clientUpdate = None
	momentum = 0.0
	corout = False
	
	def __init__(self):
		self.statics = []
		self.kinetics = []
		self.forces = []
		self.particles = []
		self.staticContactPairs = []
		self.contactPairs = []
		self.clientUpdate = None

	
class Material:
	cr = 1.0
	
	def __init__(self, cr):
		self.cr = cr


class Convex:
	v = []
	p = [0.0, 0.0]
	mat = None

	def __init__(self, v, mat):
		self.v = v
		self.mat = mat
	

class Particle:
	pos = [0,0]
	vel = [0,0]
	radius = 0.2
	acc = [0,0]
	m = 1.0
	invM = 1.0 / m
	mat = None
	collided = False
	
	def __init__(self, p, v, r, mat):
		self.pos = p
		self.vel = v
		self.radius = r
		self.mat = mat
		self.m = r
		if (self.m != 0.0):
			self.invM = 1.0/self.m
		else:
			self.invM = 0.0


class ContactInfo:
	n = [0.0, 0.0]
	p = [0.0, 0.0]
	d = 0.0

	def __init__(self, n, d, p):
		self.n = n
		self.d = d
		self.p = p


class ContactPair:
	obj = [None, None]
	info = None	#info is relative to 1st obj (e.g: normal from 1st to 2nd obj)
	
	def __init__(self, o1, o2, inf):
		self.obj = [o1, o2]
		self.info = inf


class SpringForce:
	obj = None
	k = 1.0
	r = 1.0
	dz = 0.001
	
	def __init__(self, o1, o2, k, r):
		self.obj = [o1, o2]
		self.k = k
		self.r = r


	def apply(self, w):
		o1 = self.obj[0]
		o2 = self.obj[1]
		vec = v2_sub(o2.pos, o1.pos)
		d = v2_len(vec)
		err = d-self.r
		
		if (math.fabs(err) > self.dz):
			dir = v2_muls(vec, 1.0/d)
			f = self.k * err
			fvec = v2_muls(dir, f)
			if (o1.m > 0.0):
				o1.acc = v2_add(o1.acc, v2_muls(fvec, o1.invM))
				#print o1.acc
			if (o2.m > 0.0):
				o2.acc = v2_add(o2.acc, v2_muls(fvec, -o2.invM))
				#print f
				#print vec
				#print o2.acc
				#print v2_muls(fvec, o2.invM)
	
sCableForceMat = Material(1.0)

class CableForce:
	obj = None
	l = 1.0
	mat = None

	def __init__(self, o1, o2, r, l):
		self.obj = [o1, o2]
		self.l = l
		self.mat = Material(math.sqrt(r))

	def apply(self, w):
		o1 = self.obj[0]
		o2 = self.obj[1]
		posVec = v2_sub(o2.pos, o1.pos)
		dist = v2_len(posVec)

		if (dist > self.l):
			n = v2_normalize(v2_sub(o1.pos, o2.pos))
			
			err = self.l - dist
			info = ContactInfo(n, err, o1.pos)
			pair = ContactPair(o1, o2, info)

			if (o1.m > 0.0):
				newVels = processNonStaticContact(w, pair, self.mat, sCableForceMat)

				if (newVels != None):
					pair.obj[0].vel = newVels[0]
					#pair.obj[0].collided = True
					pair.obj[1].vel = newVels[1]
					#pair.obj[1].collided = True

			else:
				newVel = processStaticContact(w, pair, self.mat, sCableForceMat)

				if (newVel != None):
					pair.obj[1].vel = newVel
					#pair.obj[1].collided = True



def stepWorld(w, dt, corout):
	
	w.perfTime = time.time()
	w.corout = corout
	
	if (w.frame < 0):
		w.frame = 0
		w.lastTime = 0.0
		
	newTime = w.lastTime + dt*w.timeScale
	stepTime = 1.0 * w.frame * w.timeStep

	#print newTime, stepTime
	
	frameCount = 0
	w.dt = w.timeStep
	while (stepTime + w.timeStep <= newTime):
		
		if (w.clientUpdate != None):
			w.clientUpdate(w, w.timeStep)
		
		applyForces(w)
		stepMotion(w)
		allPairs = findContactPairs(w)
		w.staticContactPairs = allPairs[0]
		w.contactPairs = allPairs[1]
		resolveContacts(w)
		stepCollidedMotion(w)
		w.frame = w.frame + 1
		frameCount = frameCount + 1
		stepTime = 1.0 * w.frame * w.timeStep
		#print "+", stepTime
	
	#print "*", newTime, stepTime
	
	w.lastTime = newTime
	
	if (frameCount == 0):
		w.perfTime = -1.0
	else:
		w.perfTime = (time.time() - w.perfTime) / (1.0 * frameCount)
	#print '{0:.3f}'.format(perfTime)



def applyForces(w):
	
	for p in w.particles:
		if (p.m > 0.0):
			p.acc = [0.0, w.g]
		else:
			p.acc = [0.0, 0.0]

	for f in w.forces:
		f.apply(w)



def stepMotion(w):
	
	w.momentum = 0.0
	
	for p in w.particles:
		p.vel = v2_add(p.vel, v2_muls(p.acc, w.dt))
		p.pos = v2_add(p.pos, v2_muls(p.vel, w.dt))
		
		w.momentum = w.momentum + v2_len(p.vel) * p.m



def stepCollidedMotion(w):
	for p in w.particles:
		if p.collided:
			p.pos = v2_add(p.pos, v2_muls(p.vel, w.dt))
			p.collided = False



def contactCircleCircle(p1, r1, p2, r2):
	n = v2_sub(p2, p1)
	dist = v2_len(n) - (r1 + r2)
	if dist <= 0:
		return ContactInfo(n, dist, v2_add(p1, v2_muls(v2_normalize(n), r1)))
	return None	



def contactSegmentCircle(s1, s2, p, r):
	d = v2_sub(s2, s1)
	t = (v2_dot(p, d) - v2_dot(s1, d)) / v2_dot(d, d)
	
	if t >= 0 and t <= 1:
		cp = v2_add(s1, v2_muls(d, t))
		dist = -r+v2_len(v2_sub(cp, p))
		if dist <= 0:
			n = v2_sub(p, cp)
			return ContactInfo(n, dist, cp)
		
	return None	


def findContactPairs(w):
	
	pairs = []
	staticPairs = []
	
	for i1 in range(len(w.particles)):
		p1 = w.particles[i1]
		
		if (p1.m > 0.0):

			for s in w.statics:
				info = contactSegmentCircle(s.v1, s.v2, p1.pos, p1.radius)
				if (info != None):
					info.n = v2_normalize(info.n)
					staticPairs.append(ContactPair(s, p1, info))
				
		
			for i2 in range(i1+1, len(w.particles)):
				p2 = w.particles[i2]
				if (p2.m > 0.0):
					info = contactCircleCircle(p1.pos, p1.radius, p2.pos, p2.radius)
					if (info != None):
						info.n = v2_normalize(info.n)
						pairs.append(ContactPair(p1, p2, info))
	
	return [staticPairs, pairs]



def removeContactPenetration(w, cInfo, o1, f1, o2, f2):
	if (f1 != 0.0):
		o1.pos = v2_add(o1.pos, v2_muls(cInfo.n, cInfo.d * f1))
		
	if (f2 != 0.0):
		o2.pos = v2_add(o2.pos, v2_muls(cInfo.n, -cInfo.d * f2))



def processStaticContact(w, pair, mat1, mat2):
		o1 = pair.obj[0]
		o2 = pair.obj[1]
		
		n = pair.info.n
		nDot = v2_dot(o2.vel, n)
		
		removeContactPenetration(w, pair.info, o1, 0.0, o2, 1.0)
		
		if (nDot < 0):
			nVel = v2_muls(n, nDot)
			tVel = v2_sub(o2.vel, nVel)
			cr = mat1.cr * mat2.cr
			rnVel = nDot * -cr
			return v2_add(v2_muls(n, rnVel), tVel)
			
		return None	


def resolveStaticContacts(w):
	
	for pair in w.staticContactPairs:
		newVel = processStaticContact(w, pair, pair.obj[0].mat, pair.obj[1].mat)
		
		if (newVel != None):
			o2 = pair.obj[1]
			o2.vel = newVel
			o2.collided = True


def processNonStaticContact(w, pair, mat1, mat2):			
		o1 = pair.obj[0]
		o2 = pair.obj[1]

		n = pair.info.n
		n1Dot = v2_dot(o1.vel, n)
		n2Dot = v2_dot(o2.vel, n)
		
		tm = o1.m + o2.m
		removeContactPenetration(w, pair.info, o1, o2.m/tm, o2, o1.m/tm)
		
		if (n2Dot-n1Dot < 0):
			
			nVel1 = v2_muls(n, n1Dot)
			tVel1 = v2_sub(o1.vel, nVel1)
			
			nVel2 = v2_muls(n, n2Dot)
			tVel2 = v2_sub(o2.vel, nVel2)
			
			cr = mat1.cr * mat2.cr
			
			p = o1.m * n1Dot + o2.m * n2Dot
			q = cr * (n2Dot - n1Dot)
									
			rnVel1 = (p + o2.m * q) / tm
			rnVel2 = (p - o1.m * q) / tm
			
			o1NewVel = v2_add(v2_muls(n, rnVel1), tVel1)		
			o2NewVel = v2_add(v2_muls(n, rnVel2), tVel2)
			return [o1NewVel, o2NewVel]
		return None	
	

def resolveNonStaticContacts(w):			
	for pair in w.contactPairs:
		newVels = processNonStaticContact(w, pair, pair.obj[0].mat, pair.obj[1].mat)

		if (newVels != None):
			o1 = pair.obj[0]
			o1.vel = newVels[0]
			o1.collided = True
			o2 = pair.obj[1]
			o2.vel = newVels[1]
			o2.collided = True
			
def resolveContacts(w):
	resolveNonStaticContacts(w)
	resolveStaticContacts(w)
			

#--------------------------------
#------------ RENDERING	---------
#--------------------------------

ppm = 20.0      # pixels per meter
mpp = 1.0/ppm   # meters per pixel
particle_vcount = 16
particle_angles = [ 2*math.pi*float(i)/particle_vcount for i in range( particle_vcount ) ]
particle_sincos = [ [math.cos(a), math.sin(a)] for a in particle_angles ]

def draw_box(x1, y1, x2, y2):
	pyglet.graphics.draw(4, pyglet.gl.GL_QUADS, ('v2f', (x1, y1, x2, y1, x2, y2, x1, y2)))


def draw_convex(p, v, ppm):
	flatv = [0.0] * (len(v)*2)
	for i in range(len(v)):
		flatv[2*i] = (p[0]+v[i][0]) * ppm
		flatv[2*i+1] = (p[1]+v[i][1]) * ppm
	pyglet.graphics.draw(len(v), pyglet.gl.GL_LINE_LOOP, ('v2f', flatv))

def draw_convex_col(p, v, ppm, col):
	flatv = [0.0] * (len(v)*2)
	for i in range(len(v)):
		flatv[2*i] = (p[0]+v[i][0]) * ppm
		flatv[2*i+1] = (p[1]+v[i][1]) * ppm
	pyglet.graphics.draw(len(v), pyglet.gl.GL_LINE_LOOP, ('v2f', flatv), ('c3f', (col)*len(v)) )


def draw_line(x1, y1, x2, y2):
	pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2f', (x1, y1, x2, y2)))


def draw_thick_line_col(x1, y1, x2, y2, ppm, w, col):
	if (w <= 1):
		draw_line(x1, y1, x2, y2)
	n = v2_sub([x2, y2], [x1, y1])	
	n = [-n[1], n[0]]
	n = v2_normalize(n)
	sc = w/(2.0*ppm)
	v0 = v2_add([x1, y1], v2_muls(n, sc))
	v1 = v2_add([x2, y2], v2_muls(n, sc))
	v2 = v2_add([x2, y2], v2_muls(n, -sc))
	v3 = v2_add([x1, y1], v2_muls(n, -sc))

	draw_convex_col([0.0, 0.0], [v0, v1, v2, v3], ppm, col)


def draw_line_col(x1, y1, x2, y2, col):
	pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2f', (x1, y1, x2, y2)), ('c3f', (col)*2) )


def draw_arrow_col(x1, y1, x2, y2, ppm, w, col):
	if (w <= 1):
		draw_line(x1, y1, x2, y2)
	n = v2_sub([x2, y2], [x1, y1])	
	n = [-n[1], n[0]]
	n = v2_normalize(n)
	sc = w/(2.0*ppm)
	v0 = v2_add([x1, y1], v2_muls(n, sc))
	v1 = v2_add([x1, y1], v2_muls(n, -sc))
	v2 = [x2, y2]

	draw_convex_col([0.0, 0.0], [v0, v1, v2], ppm, col)


def draw_cross(x, y, r):
	pyglet.graphics.draw(4, pyglet.gl.GL_LINES, ('v2f', (x-r, y, x+r, y, x, y-r, x, y+r)))


def draw_particle(x, y, r):
	vertices = []
	for sc in particle_sincos:
		vertices.append(x + r*sc[0])
		vertices.append(y + r*sc[1])
	pyglet.graphics.draw(particle_vcount, pyglet.gl.GL_LINE_LOOP, ('v2f', vertices))



#--------------------------------
#------------ MAIN	-------------
#--------------------------------
	
worldFillers = []
worldFillerIndex = 0

def fillWorldBox(w):	
	#floor
	floorMat = Material(1.0)
	w.statics.append(Convex([[0.5,1.0], [39.5, 1.0]], floorMat))	

	#walls
	wallMat = Material(1.0)
	w.statics.append(Convex([[1.0,0.5], [1.0, 29.5]], wallMat))	
	w.statics.append(Convex([[39.5,29.0], [0.5, 29.0]], wallMat))	
	w.statics.append(Convex([[39.0,29.5], [39.0, 0.5]], wallMat))	
worldFillers.append(fillWorldBox)	
worldFillerIndex = len(worldFillers)-1

	
def fillWorldGJK1(w):
	sharedMat = Material(1.0)
	
	fillWorldBox(w)

	w.kinetics.append(Convex([[20.0,20.0], [24.0,20.0], [24.0,24.0], [20.0,24.0]], sharedMat))	
	#w.kinetics.append(Convex([[0.0,0.0], [5.0,0.0], [5.0,5.0], [0.0,5.0]], sharedMat))	
	#w.kinetics.append(Convex([[0.0,0.0], [2.0,0.0], [2.0,2.0], [0.0,2.0]], sharedMat))	
	#w.kinetics.append(Convex([[-1.0,-1.0], [1.0,-1.0], [1.0,1.0], [-1.0,1.0]], sharedMat))	

	cvx1 = Convex([[0.0,-5.0], [5.0,-5.0], [5.0,5.0], [0.0,5.0]], sharedMat)
	cvx1.p = [9.0, 10.0]
	w.kinetics.append(cvx1)	

	cvx2 = Convex([[0.0,0.0], [-3.0,3.0], [-3.0,-3.0]], sharedMat)
	cvx2.p = [10.0, 10.0]
	w.kinetics.append(cvx2)	

	random.seed(66)

	for i in range(3):

		num_v = random.randrange(3, 8)
		r = random.uniform(1.0, 4.0)
		cvx = Convex(randConvex(r, num_v), sharedMat)
		cvx.p =  [random.uniform(10.0, 35.0), random.uniform(10.0, 25.0)]

		w.kinetics.append(cvx)

worldFillers.append(fillWorldGJK1)	
worldFillerIndex = len(worldFillers)-1


def fillWorld1(w):
	sharedMat = Material(1.0 * 0.8)
	
	fillWorldBox(w)
	
	if 1:
		w.particles.append(Particle([22.0,10.0], [0.0, 0.0], 0.4, sharedMat))	

	if 1:
		#random
		w.particles.append(Particle([20.0,20.0], [1, 0.5], 0.4, sharedMat))	
		w.particles.append(Particle([24.0,20.0], [0.3, -1.2], 0.4, sharedMat))	
		w.particles.append(Particle([16.0,16.0], [-1.2, -1.2], 0.7, sharedMat))	
		#intersect
		w.particles.append(Particle([20.0,10.0], [0.8, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([24.0,10.0], [-0.6, 0.0], 0.4, sharedMat))	
		
	if 1:
		#floor bounce
		w.particles.append(Particle([2.0,1.4], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([3.0,1.5], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([4.0,1.6], [0.0, 0.0], 0.4, sharedMat))	
		
	if 0:
		#separating
		w.particles.append(Particle([6.0,1.4], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([6.6,1.4], [0.0, 0.0], 0.4, sharedMat))	

	#platform
	floorMat = Material(1.0 * 0.9)
	w.statics.append(Convex([[25.0,8.0], [35.0, 8.0]], floorMat))	
worldFillers.append(fillWorld1)


def fillWorld2(w):
	sharedMat = Material(1.0)

	fillWorldBox(w)

	w.particles.append(Particle([10.0,10.0], [7.8, 0.0], 0.4, sharedMat))	
	w.particles.append(Particle([30.0,10.0], [-7.8, 0.0], 0.4, sharedMat))	

	#floor
	floorMat = Material(1.0)
	w.statics.append(Convex([[1.0,1.0], [39.0, 1.0]], floorMat))	

	#walls
	wallMat = Material(1.0)
	w.statics.append(Convex([[1.0,1.0], [1.0, 29.0]], wallMat))	
	w.statics.append(Convex([[39.0,29.0], [1.0, 29.0]], wallMat))	
	w.statics.append(Convex([[39.0,29.0], [39.0, 1.0]], wallMat))	
worldFillers.append(fillWorld2)


def fillWorldSpring1(w):
	sharedMat = Material(0.9)

	fillWorldBox(w)
	#w.g = 0.0

	if 1:	
		w.particles.append(Particle([20.0,10.0], [1.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([20.0,10.5], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 4.0, 1.0))

	if 1: 
		w.particles.append(Particle([10.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([10.0,14.8], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 1.5, 5.0))

	if 1:
		w.particles.append(Particle([12.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([12.0,10.5], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 1.5, 1.0))

	if 1:
		w.particles.append(Particle([14.0,3.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([14.0,3.5], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 1.0, 4.0))


	if 1:	
		w.particles.append(Particle([25.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([25.0,10.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 1.5, 1.0))

	if 1:	
		w.particles.append(Particle([34.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([35.0,10.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 0.7, 1.0))
		wallMat = Material(1.0)
		w.statics.append(Convex([[30.0,0.5], [30.0, 29.0]], wallMat))	

	if 1:	
		w.particles.append(Particle([34.0,25.0], [0.5, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([35.0,25.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 0.7, 1.0))
		w.particles.append(Particle([37.0,23.0], [0.5, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([38.0,23.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], 0.7, 1.0))
		wallMat = Material(0.9)
		w.statics.append(Convex([[29.5,18.0], [40.5, 18.0]], wallMat))	


worldFillers.append(fillWorldSpring1)


def fillWorldSpring2(w):
	sharedMat = Material(0.6)

	fillWorldBox(w)
	#w.g = 0.0

	if 1:
		s=500.0
		w.particles.append(Particle([20.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([24.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([24.0,6.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([20.0,6.0], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(SpringForce(w.particles[-1], w.particles[-2], s, 4.0))
		w.forces.append(SpringForce(w.particles[-1], w.particles[-3], s, 4.0*1.4142))
		w.forces.append(SpringForce(w.particles[-1], w.particles[-4], s, 4.0))
		w.forces.append(SpringForce(w.particles[-2], w.particles[-3], s, 4.0))
		w.forces.append(SpringForce(w.particles[-2], w.particles[-4], s, 4.0*1.4142))
		w.forces.append(SpringForce(w.particles[-3], w.particles[-4], s, 4.0))

worldFillers.append(fillWorldSpring2)


def fillWorldCable1(w):
	sharedMat = Material(0.9)
	defR = 0.3

	fillWorldBox(w)
	#w.g = 0.0

	if 1:	
		w.particles.append(Particle([20.0,10.0], [1.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([20.0,10.5], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 1.0))

	if 1: 
		w.particles.append(Particle([10.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([10.0,11.8], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 5.0))

	if 1:
		w.particles.append(Particle([12.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([12.0,10.5], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 1.0))

	if 1:
		w.particles.append(Particle([14.0,3.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([14.0,3.5], [0.0, 0.0], 0.0, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 4.0))


	if 1:	
		w.particles.append(Particle([25.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([25.0,10.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 1.0))

	if 1:	
		w.particles.append(Particle([34.0,10.0], [0.0, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([35.0,10.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 1.5))
		wallMat = Material(1.0)
		w.statics.append(Convex([[30.0,0.5], [30.0, 29.0]], wallMat))	

	if 1:	
		w.particles.append(Particle([34.0,25.0], [0.5, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([35.0,25.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 2.0))
		w.particles.append(Particle([37.0,23.0], [0.5, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([38.0,23.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 2.0))
		wallMat = Material(0.9)
		w.statics.append(Convex([[29.5,18.0], [40.5, 18.0]], wallMat))	

worldFillers.append(fillWorldCable1)

def fillWorldLongCable1(w):
	sharedMat = Material(0.9)
	defR = 0.3

	fillWorldBox(w)
	#w.g = 0.0

	if 1:	
		for j in range(1, 5):
			tl = 5.0
			rf = 0.5
			sx = 5.0+j*5
			sy = 20.0
			ox = 2.0
			num=j*5
			w.particles.append(Particle([sx,sy], [0.0, 0.0], 0.0, sharedMat))	
			for i in range(1, num):
				w.particles.append(Particle([sx+(ox*i)/num,sy+(rf*tl*i)/num], [0.0, 0.0], 0.01, sharedMat))
				w.forces.append(CableForce(w.particles[-2], w.particles[-1], defR , tl/(num-1)))

	wallMat = Material(0.9)
	w.statics.append(Convex([[2.0,20.0], [2.0, 15.0]], wallMat))	


worldFillers.append(fillWorldLongCable1)


lastClientTime = -1.0
def updateWorldSupportCable1(w, dt):
	global lastClientTime
	sharedMat = Material(0.5)
	
	if (lastClientTime == -2.0):
		return
		
	if (lastClientTime == -1.0):
		lastClientTime = w.lastTime
		
	if (w.lastTime - lastClientTime > 5.0):
		lastClientTime = -2.0
		for k in range(2):
			for j in range(1,4):
				w.particles.append(Particle([5.0+((j-1)*10)+2.5, 11.0+k*10.0], [0.0, 0.0], 1.5, sharedMat))


def fillWorldSupportCable1(w):
	global lastClientTime
	sharedMat = Material(0.9)
	defR = 0.3

	fillWorldBox(w)
	#w.g = 0.0

	if 1:	
		for k in range (2):
			for j in range(1, 4):
				tl = 5.0
				rf = 0.5
				sx = 5.0+(j-1)*10
				sy = 10.0+k*10
				ox = tl
				num=j*5
				w.particles.append(Particle([sx,sy], [0.0, 0.0], 0.0, sharedMat))	
				for i in range(1, num):
					w.particles.append(Particle([sx+ox*i/num,sy], [0.0, 0.0], 0.05 + k*0.1, sharedMat))
					w.forces.append(CableForce(w.particles[-2], w.particles[-1], defR , tl/(num-1)))
				w.particles.append(Particle([sx+tl,sy], [0.0, 0.0], 0.0, sharedMat))		
				w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , tl/(num-1)))

	lastClientTime = -1.0
	w.clientUpdate = updateWorldSupportCable1

worldFillers.append(fillWorldSupportCable1)



def updateVWorld1(w, dt):
	global lastClientTime
	sharedMat = Material(0.5)
		
	if (lastClientTime == -1.0 or w.lastTime - lastClientTime > 3.0):
		lastClientTime = w.lastTime
		w.particles.append(Particle([18.0, 20.0], [0.0, 0.0], 0.4, sharedMat))
		

def fillVWorld1(w):
	global lastClientTime

	fillWorldBox(w)

	#funnel
	funnelMat = Material(1.0)
	w.statics.append(Convex([[20.0,7.0], [15.0, 20.0]], funnelMat))	
	w.statics.append(Convex([[20.0,7.0], [25.0, 20.0]], funnelMat))	

	lastClientTime = -1.0
	w.clientUpdate = updateVWorld1
worldFillers.append(fillVWorld1)


def nextWorld():
	global gWorld
	global worldFillers
	global worldFillerIndex
	
	gWorld = World()
	worldFillers[worldFillerIndex](gWorld)
	worldFillerIndex = (worldFillerIndex+1) % len(worldFillers)

def repeatWorld():
	global worldFillerIndex
	worldFillerIndex = (len(worldFillers) + worldFillerIndex-1) % len(worldFillers)
	nextWorld()

gWorld = World()
nextWorld()


config = pyglet.gl.Config(double_buffer=True)
window = pyglet.window.Window(800,600, config=config)

fps_display = pyglet.clock.ClockDisplay(pyglet.font.load('Arial', 10), interval=0.1, format='%(fps).0f', color=(1.0, 1.0, 1.0, 0.5))
gSingleStep = False
gDoSingleStep = False
gMicroStep = False
gDoMicroStep = False
gMousePos = None
gMouseHoverObj = None
gMousePick = 0
gPrevMousePick = 0
gMousePickStart = None
gMousePickObj = None
gMousePickObjVec = None
gDoTest = True


@window.event
def on_draw():
	window.clear()
	
	fps_display.update_text()
	fps = pyglet.text.Label(fps_display.label.text, font_name='Arial', font_size=6, x=window.width, y=window.height,anchor_x='right', anchor_y='top')
	fps.draw()
	
	mom = pyglet.text.Label('{0:.2f} - {1:.2f}'.format(gWorld.momentum, gWorld.momentum/max(1,len(gWorld.particles))), font_name='Arial', font_size=6, x=0, y=window.height,anchor_x='left', anchor_y='top')
	mom.draw()
		
	
	for s in gWorld.statics:
		draw_convex(s.p, s.v, ppm)

	for k in gWorld.kinetics:
		draw_convex(k.p, k.v, ppm)
	
	for p in gWorld.particles:
		draw_particle(p.pos[0] * ppm, p.pos[1] * ppm, p.radius * ppm)
		
	for c in gWorld.contactPairs:
		pt2 = v2_add(c.info.p, v2_muls(v2_normalize(c.info.n), c.info.d))
		draw_line(c.info.p[0]*ppm, c.info.p[1]*ppm, pt2[0]*ppm, pt2[1]*ppm)

	for f in gWorld.forces:
		draw_line(f.obj[0].pos[0] * ppm, f.obj[0].pos[1] * ppm, f.obj[1].pos[0] * ppm, f.obj[1].pos[1] * ppm)

	if 0:
		mat = Material(0.9)
		p1 = Particle([10.0,10.0], [0.0, 0.0], 5.0, mat)
		p2 = Particle([15.2,10.0], [0.0, 0.0], 0.5, mat)

		draw_particle(p1.pos[0] * ppm, p1.pos[1] * ppm, p1.radius * ppm)
		draw_particle(p2.pos[0] * ppm, p2.pos[1] * ppm, p2.radius * ppm)

		info = contactCircleCircle(p1.pos, p1.radius, p2.pos, p2.radius)
		if (info != None):
			info.n = v2_normalize(info.n)
			pt2 = v2_add(info.p, v2_muls(v2_normalize(info.n), info.d))
			draw_line(info.p[0]*ppm, info.p[1]*ppm, pt2[0]*ppm, pt2[1]*ppm)

	if gMouseHoverObj != None:
		draw_convex_col(gMouseHoverObj.p, gMouseHoverObj.v, ppm, [1.0, 0.0, 0.0])
	if gMousePickObj != None:
		draw_convex_col(gMousePickObj.p, gMousePickObj.v, ppm, [1.0, 0.0, 0.0])	

	handleMouse()
	handleKeyboard()
	


def handleMouse():
	global gPrevMousePick
	global gMousePickStart 
	global gMouseHoverObj
	global gMousePickObj
	
	gMouseHoverObj = None
	if (gMousePick == 0):
		gMousePickObj = None

	if (gMousePos == None):
		return

	if (gMousePick == 0):
		draw_particle(gMousePos[0], gMousePos[1], 2.0)
		doHover(v2_muls(gMousePos, mpp))
	else:	
		draw_cross(gMousePos[0], gMousePos[1], 8.0)
		doPick(v2_muls(gMousePos, mpp), v2_muls(gMousePickStart, mpp), gPrevMousePick == 0)

	gPrevMousePick = gMousePick			



def doHover(pos):
	global gMouseHoverObj

	gMouseHoverObj = None
	for k in gWorld.kinetics:
		dist = test_gjk_distance(k.p, k.v, [0.0,0.0], [pos])
		#print pos
		#print dist[0]
		if (dist[0] <= dist[1]):
			if (dist[0] < 0.0):
				#draw_line_col(dist[2][0]*ppm, dist[2][1]*ppm, dist[3][0]*ppm, dist[3][1]*ppm, [0.3, 0.3, 0.2])
				draw_arrow_col(dist[3][0], dist[3][1], dist[2][0], dist[2][1], ppm, 20.0, [0.3, 0.3, 0.2])
			gMouseHoverObj = k
			#draw_convex_col(k.p, k.v, ppm, [1.0, 0.0, 0.0])
		else:
			draw_line_col(dist[2][0]*ppm, dist[2][1]*ppm, dist[3][0]*ppm, dist[3][1]*ppm, [0.2, 0.2, 0.2])
			


def doPick(pos, startPos, init):
	global gMousePickObj
	global gMousePickObjVec

	if init:
		gMousePickObj = None
		for k in gWorld.kinetics:
			dist = test_gjk_distance(k.p, k.v, [0.0, 0.0], [pos])
			if (dist[0] <= dist[1]):
				gMousePickObj = k
				gMousePickObjVec = k.p

	
	if gMousePickObj != None:
		move = v2_sub(pos, startPos)
		gMousePickObj.p = v2_add(gMousePickObjVec, move)
		#draw_convex_col(gMousePickObj.p, gMousePickObj.v, ppm, [1.0, 0.0, 1.0])


def handleKeyboard():
	global gDoTest

	if gDoTest:
		ks = gWorld.kinetics
		for i in range(len(ks)):
			for j in range(i+1, len(ks)):
				dist = test_gjk_distance(ks[i].p, ks[i].v, ks[j].p, ks[j].v)
				#print dist[0]
				if (dist[0] <= dist[1]):
					draw_convex_col(ks[i].p, ks[i].v, ppm, [1.0, 1.0, 0.0])
					draw_convex_col(ks[j].p, ks[j].v, ppm, [1.0, 1.0, 0.0])
					
					if (dist[0] < 0.0):
						#draw_line_col(dist[2][0]*ppm, dist[2][1]*ppm, dist[3][0]*ppm, dist[3][1]*ppm, [0.4, 0.4, 0.0])	
						draw_arrow_col(dist[2][0], dist[2][1], dist[3][0], dist[3][1], ppm, 12.0, [0.4, 0.4, 0.0])	
				else:	
					draw_line_col(dist[2][0]*ppm, dist[2][1]*ppm, dist[3][0]*ppm, dist[3][1]*ppm, [0.3, 0.3, 0.0])	
		


def update(dt):
	global gMicroStep
	global gDoMicroStep
	global gSingleStep
	global gDoSingleStep

	if gSingleStep:
		if (gDoSingleStep):
			stepWorld(gWorld, gWorld.timeStep, False)
			gDoSingleStep = False
#	elif gMicroStep:
#		if (gDoMicroStep):
#			stepWorld(gWorld, gWorld.timeStep, True)
#			gDoMicroStep = False
	else:
		stepWorld(gWorld, dt, False)



@window.event	
def on_key_press(symbol, modifiers):
	global gMicroStep
	global gDoMicroStep
	global gSingleStep
	global gDoSingleStep
	global gDoTest
	global gTestEpa

	if symbol == pyglet.window.key.N:
		nextWorld()
		
	if symbol == pyglet.window.key.R:
		repeatWorld()

	if symbol == pyglet.window.key.S:
		gSingleStep = not gSingleStep
		gDoSingleStep = False

	if symbol == pyglet.window.key.SPACE:
		gDoSingleStep = True	

	if symbol == pyglet.window.key.M:
		gMicroStep = not gMicroStep
		gDoMicroStep = False

	if symbol == pyglet.window.key.SPACE:
		gDoMicroStep = True	

	if symbol == pyglet.window.key.T:
		gDoTest = not gDoTest

	if symbol == pyglet.window.key.E:
		gTestEpa = not gTestEpa


@window.event
def on_mouse_motion(x, y, dx, dy):
	global gMousePos
	global gMousePick
	gMousePos = [x, y]
	gMousePick = 0

@window.event	
def on_mouse_press(x, y, button, modifiers):
	global gMousePick
	global gMousePickStart
	gMousePick = 1
	gMousePickStart = [x, y]

@window.event
def on_mouse_release(x, y, button, modifiers):
	global gMousePick
	gMousePick = 0

@window.event
def on_mouse_drag(x, y, dx, dy, buttons, modifiers):
	global gMousePos
	global gMousePick
	gMousePos = [x, y]
	gMousePick = 1


pyglet.clock.schedule_interval(update, 1.0/60.0)	
pyglet.app.run()

