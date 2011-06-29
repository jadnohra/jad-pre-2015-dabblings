import pyglet
import math

#--------------------------------
#------------ PHYSICS -----------
#--------------------------------

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
	g = -9.8
	timeScale = 0.2
	dt = 0.0
	statics = []
	particles = []
	contactPais = []
	
	
class Convex:
	v1 = [0,0]
	v2 = [0,0]

	def __init__(self, v1, v2):
		self.v1 = v1
		self.v2 = v2
	

class Particle:
	pos = [0,0]
	vel = [0,0]
	radius = 0.2
	acc = [0,0]

	def __init__(self, p, v, r):
		self.pos = p
		self.vel = v
		self.radius = r


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
	info = None
	
	def __init__(self, o1, o2, inf):
		self.obj[0] = o1
		self.obj[1] = o2
		self.info = inf

def stepWorld(w, dt):
	w.dt = dt * w.timeScale
	applyExternalForces(w)
	stepMotion(w)
	w.contactPais = findContactPairs(w)


def applyExternalForces(w):
	for p in w.particles:
		p.acc = [0.0, w.g]

def stepMotion(w):
	for p in w.particles:
		p.vel = v2_add(p.vel, v2_muls(p.acc, w.dt))
		p.pos = v2_add(p.pos, v2_muls(p.vel, w.dt))


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
	
	for i1 in range(len(w.particles)):
		p1 = w.particles[i1]
		
		for s in w.statics:
			info = contactSegmentCircle(s.v1, s.v2, p1.pos, p1.radius)
			if (info != None):
				pairs.append(ContactPair(s, p1, info))
		
		for i2 in range(i1+1, len(w.particles)):
			p2 = w.particles[i2]
			info = contactCircleCircle(p1.pos, p1.radius, p2.pos, p2.radius)
			if (info != None):
				pairs.append(ContactPair(p1, p2, info))
	
	return pairs


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
	
world = World()
#random
world.particles.append(Particle([20.0,20.0], [1, 0.5], 0.4))	
world.particles.append(Particle([24.0,20.0], [0.3, -1.2], 0.4))	
#intersect
world.particles.append(Particle([20.0,10.0], [0.8, 0.0], 0.4))	
world.particles.append(Particle([24.0,10.0], [-0.6, 0.0], 0.4))	

world.particles.append(Particle([2.0,1.4], [0.0, 0.0], 0.4))	
world.particles.append(Particle([3.0,1.5], [0.0, 0.0], 0.4))	
world.particles.append(Particle([4.0,1.6], [0.0, 0.0], 0.4))	

#floor
world.statics.append(Convex([1.0,6.0], [39.0, 6.0]))	
world.statics.append(Convex([1.0,1.0], [39.0, 1.0]))	


config = pyglet.gl.Config(double_buffer=True)
window = pyglet.window.Window(800,600, config=config)


@window.event
def on_draw():
	window.clear()
	pyglet.clock.ClockDisplay().draw()
	
	for s in world.statics:
		draw_line(s.v1[0] * ppm, s.v1[1] * ppm, s.v2[0] * ppm, s.v2[1] * ppm)
	
	for p in world.particles:
		draw_particle(p.pos[0] * ppm, p.pos[1] * ppm, p.radius * ppm)
		
	for c in world.contactPais:
		pt2 = v2_add(c.info.p, v2_muls(v2_normalize(c.info.n), c.info.d))
		draw_line(c.info.p[0]*ppm, c.info.p[1]*ppm, pt2[0]*ppm, pt2[1]*ppm)
	

def update(dt):
	stepWorld(world, dt)
	

pyglet.clock.schedule_interval(update, 1.0/30.0)	
pyglet.app.run()

