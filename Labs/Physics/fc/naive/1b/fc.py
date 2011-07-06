import pyglet
import math
import time

#--------------------------------
#------------ PHYSICS -----------
#--------------------------------
#
# Defects:
#	1. Smaller frame rate directly causes tunneling.
#	2. Need a separate position update after collisions, otherwize impulse is lost.
#	3. Cr less than 1 direcly causes penetration.	
#		- use hard penetration resolution before processing the impulse 
#			(of course not precise, but fixing that goes under 'tunneling' at the moment.
#		- Defect
#			Hard penetration resolution works 'perfectly' if all we ever have is 2 objects.
#			As soon as we have fixed time steps, we will always end up with penetrations and therefore 'lost info'
#			But we can never have zero time steps, Use TOI? as an alternative to give us time steps up to precision of floating point.
#			But even with that we have problems:
#			Once we have more, e.g: particle squashed between 2 walls that are closer to each other than radius,
#			Fixing one penetration can produce another ad. infinitum, the one object update per frame leaves us
#			with rest penetrations depending on the order.
#			with one update per object per frame a iterative method will always fail, instead we need a condition of all
#			contacts having been resolved, can take forever .. fp .. not possible ..
#			Can we solve it globally? no because this depends on the 'future' (except if all are resting contacts).
#			Can we build a graph for ordering going from largest mass and contacts? e.g: particles colliding with static
#			(inf. mass) first? how would this help, have to think more...
#			What happens in the real world: everything takes time, it is sequential in the sense that it goes from atom
#			to atom, but in parallel and at the speed of light (think rod with size 10 light seconds twisted from both sides
#			in opposite directions), if done with a 'physics engine' we would be breaking the speed of light
#			So we have to this speed of light parallel compu on our crappy computers, and as we already knew, the best approx. wins.
#			It is good to think of nothing as hard, e.g in V test, imagine bottom is elastic, it would take time for a high speed 
#			collision from above to be seen in a buldge on the bottom, and then the reaction, and this is what happens
#			except on atomic level and at speed of light ... In effect the idea of a physics engine is crazy but we have to do it!
#			also this means a very close look at FEM is crucial!!
#			Try to V experiment in box2d, havok, bullet, what makes it work?  watch out we need to compare to a micro-collision based
#			engine.
#			
#
# Incomplete:
#	1. no circle segment endpoint intersections.
#

def v2_add(v1, v2):
	return [v1[0]+v2[0], v1[1]+v2[1]]
	
def v2_sub(v1, v2):
	return [v1[0]-v2[0], v1[1]-v2[1]]

def v2_dot(v1, v2):
	return v1[0]*v2[0]+v1[1]*v2[1]

def v2_lenSq(v1):
	return v2_dot(v1, v1)

def v2_len(v1):
	return math.sqrt(v2_lenSq(v1))
	
def v2_muls(v1, s):
	return [v1[0]*s, v1[1]*s]	

def v2_normalize(v1):
	return v2_muls(v1, 1.0/v2_len(v1))


class World:
	g = -9.8 * 1.0
	timeScale = 1.0
	timeStep = 1.0/60.0
	lastTime = -1.0
	perfTime = -1.0
	frame = -1
	dt = 0.0
	statics = None
	particles = None
	staticContactPairs = None
	contactPairs = None
	clientUpdate = None
	momentum = 0.0
	
	def __init__(self):
		self.statics = []
		self.particles = []
		self.staticContactPairs = []
		self.contactPairs = []
		self.clientUpdate = None
	
class Material:
	cr = 1.0
	
	def __init__(self, cr):
		self.cr = cr


class Convex:
	v1 = [0,0]
	v2 = [0,0]
	mat = None

	def __init__(self, v1, v2, mat):
		self.v1 = v1
		self.v2 = v2
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
		self.invM = 1.0/self.m


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


def stepWorld(w, dt):
	
	w.perfTime = time.time()
	
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
		
		applyExternalForces(w)
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


def applyExternalForces(w):
	for p in w.particles:
		p.acc = [0.0, w.g]

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
		
		for s in w.statics:
			info = contactSegmentCircle(s.v1, s.v2, p1.pos, p1.radius)
			if (info != None):
				info.n = v2_normalize(info.n)
				staticPairs.append(ContactPair(s, p1, info))
				
		
		for i2 in range(i1+1, len(w.particles)):
			p2 = w.particles[i2]
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

def resolveStaticContacts(w):
	
	for pair in w.staticContactPairs:
		o1 = pair.obj[0]
		o2 = pair.obj[1]
		
		n = pair.info.n
		nDot = v2_dot(o2.vel, n)
		
		removeContactPenetration(w, pair.info, o1, 0.0, o2, 1.0)
		
		if (nDot < 0):
			nVel = v2_muls(n, nDot)
			tVel = v2_sub(o2.vel, nVel)
			cr = o1.mat.cr * o2.mat.cr
			rnVel = nDot * -cr
			o2.vel = v2_add(v2_muls(n, rnVel), tVel)
			o2.collided = True
						
def resolveNonStaticContacts(w):			
	for pair in w.contactPairs:
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
			
			cr = o1.mat.cr * o2.mat.cr
			
			p = o1.m * n1Dot + o2.m * n2Dot
			q = cr * (n2Dot - n1Dot)
									
			rnVel1 = (p + o2.m * q) / tm
			rnVel2 = (p - o1.m * q) / tm
			
			o1.vel = v2_add(v2_muls(n, rnVel1), tVel1)
			o1.collided = True		
			
			o2.vel = v2_add(v2_muls(n, rnVel2), tVel2)
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
	#pyglet.graphics.draw(4, pyglet.gl.GL_LINE_LOOP, ('v2f', (x1, y1, x2, y1, x2, y2, x1, y2)))

def draw_line(x1, y1, x2, y2):
	pyglet.graphics.draw(2, pyglet.gl.GL_LINES, ('v2f', (x1, y1, x2, y2)))


def draw_particle(x, y, r):
	vertices = []
	for sc in particle_sincos:
		vertices.append(x + r*sc[0])
		vertices.append(y + r*sc[1])
	pyglet.graphics.draw(particle_vcount, pyglet.gl.GL_LINE_LOOP, ('v2f', vertices))


#--------------------------------
#------------ MAIN	-------------
#--------------------------------
	
def fillWorldBox(w):	
	#floor
	floorMat = Material(1.0)
	w.statics.append(Convex([0.5,1.0], [39.5, 1.0], floorMat))	

	#walls
	wallMat = Material(1.0)
	w.statics.append(Convex([1.0,0.5], [1.0, 29.5], wallMat))	
	w.statics.append(Convex([39.5,29.0], [0.5, 29.0], wallMat))	
	w.statics.append(Convex([39.0,29.5], [39.0, 0.5], wallMat))	
	
	
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
	w.statics.append(Convex([25.0,8.0], [35.0, 8.0], floorMat))	


def fillWorld2(w):
	sharedMat = Material(1.0)

	fillWorldBox(w)

	w.particles.append(Particle([10.0,10.0], [7.8, 0.0], 0.4, sharedMat))	
	w.particles.append(Particle([30.0,10.0], [-7.8, 0.0], 0.4, sharedMat))	

	#floor
	floorMat = Material(1.0)
	w.statics.append(Convex([1.0,1.0], [39.0, 1.0], floorMat))	

	#walls
	wallMat = Material(1.0)
	w.statics.append(Convex([1.0,1.0], [1.0, 29.0], wallMat))	
	w.statics.append(Convex([39.0,29.0], [1.0, 29.0], wallMat))	
	w.statics.append(Convex([39.0,29.0], [39.0, 1.0], wallMat))	


lastClientTime = -1.0
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
	w.statics.append(Convex([20.0,7.0], [15.0, 20.0], funnelMat))	
	w.statics.append(Convex([20.0,7.0], [25.0, 20.0], funnelMat))	

	lastClientTime = -1.0
	w.clientUpdate = updateVWorld1


def nextWorld():
	global world
	global worldFillers
	global worldFillerIndex
	
	world = World()
	worldFillers[worldFillerIndex](world)
	worldFillerIndex = (worldFillerIndex+1) % len(worldFillers)

worldFillers = [fillWorld1, fillWorld2, fillVWorld1]

world = World()
#fillWorld1(world)
#fillWorld2(world)
#fillVWorld1(world)
worldFillerIndex = 0
nextWorld()

config = pyglet.gl.Config(double_buffer=True)
window = pyglet.window.Window(800,600, config=config)

fps_display = pyglet.clock.ClockDisplay(pyglet.font.load('Arial', 10), interval=0.1, format='%(fps).0f', color=(1.0, 1.0, 1.0, 0.5))

@window.event
def on_draw():
	window.clear()
	
	fps_display.update_text()
	fps = pyglet.text.Label(fps_display.label.text, font_name='Arial', font_size=6, x=window.width, y=window.height,anchor_x='right', anchor_y='top')
	fps.draw()
	
	mom = pyglet.text.Label('{0:.2f} - {1:.2f}'.format(world.momentum, world.momentum/len(world.particles)), font_name='Arial', font_size=6, x=0, y=window.height,anchor_x='left', anchor_y='top')
	mom.draw()
		
	
	for s in world.statics:
		draw_line(s.v1[0] * ppm, s.v1[1] * ppm, s.v2[0] * ppm, s.v2[1] * ppm)
	
	for p in world.particles:
		draw_particle(p.pos[0] * ppm, p.pos[1] * ppm, p.radius * ppm)
		
	for c in world.contactPairs:
		pt2 = v2_add(c.info.p, v2_muls(v2_normalize(c.info.n), c.info.d))
		draw_line(c.info.p[0]*ppm, c.info.p[1]*ppm, pt2[0]*ppm, pt2[1]*ppm)
	

def update(dt):
	stepWorld(world, dt)



@window.event	
def on_key_press(symbol, modifiers):
	if symbol == pyglet.window.key.N:
		nextWorld()
		

pyglet.clock.schedule_interval(update, 1.0/60.0)	
pyglet.app.run()

