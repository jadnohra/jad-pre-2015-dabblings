import pyglet
import math
import time
from gaussy import *


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
	particles = None
	staticParticles = None
	pendulums = None
	forces = None
	clientUpdate = None
	momentum = 0.0
	corout = False
	
	def __init__(self):
		self.statics = []
		self.forces = []
		self.particles = []
		self.staticParticles = []
		self.pendulums = []
		self.staticContactPairs = []
		self.contactPairs = []
		self.clientUpdate = None

	

class Particle:
	pos = [0,0]
	vel = [0,0]
	radius = 0.2
	acc = [0,0]
	m = 1.0
	invM = 1.0 / m
	collided = False
	
	def __init__(self, p, r):
		self.pos = p
		self.vel = [0.0,0.0]
		self.radius = r
		self.m = r
		if (self.m != 0.0):
			self.invM = 1.0/self.m
		else:
			self.invM = 0.0



class DoublePendulum:
	fixed = None
	p = None
	l = None
	w = None
	restSteps = 0
	dt = -1.0

	def __init__(self, particles, p0, p1, p2):
		self.fixed = p0
		self.p = [p1,p2]
		self.l = [ v2_dist(particles[p0].pos, particles[p1].pos), v2_dist(particles[p1].pos, particles[p2].pos) ]
		self.w = [0.0, 0.0]


class DoublePendulumEuler(DoublePendulum):		

	def __init__(self, particles, p0, p1, p2):
		DoublePendulum.__init__(self, particles, p0,p1,p2)
	
	def step(self, g, particles, dt):

		step_dt = self.dt
		if self.dt == -1.0:
			step_dt = dt

		steps = dt / step_dt
		steps = steps + self.restSteps
		
		
		for i in range(steps):

			p1 = particles[self.p[0]]
			p2 = particles[self.p[1]]
			m1 = p1.m
			m2 = p2.m
			L1 = self.l[0]
			L2 = self.l[1]
			halfpi = 0.5*math.pi
			th1 = v2_angle(particles[self.fixed].pos, p1.pos)+halfpi
			th2 = v2_angle(p1.pos, p2.pos)+halfpi
			w1 = self.w[0]
			w1sq = w1*w1
			w2 = self.w[1]
			w2sq = w2*w2
			g=-g
			
			
			w1p_n = ( ( -g*(2.0*m1+m2)*math.sin(th1) ) - ( m2*g*math.sin(th1-2.0*th2) ) - ( 2.0*math.sin(th1-th2) * m2 * (w2sq*L2 + w1sq*L1*math.cos(th1-th2)  ) ) )
			w1p_d = L1* ( 2.0*m1 + m2 - m2*math.cos(2.0*th1 - 2.0*th2) )
			w1p = w1p_n / w1p_d
			
			w2p_n = ( 2.0*math.sin(th1-th2) * (w1sq * L1 * (m1+m2) + g*(m1+m2)*math.cos(th1) + w2sq*L2*m2*math.cos(th1-th2 ) ) )
			w2p_d = L2* ( 2.0*m1 + m2 - m2*math.cos(2.0*th1-2.0*th2) )
			w2p = w2p_n / w2p_d
	
	
			new_w1 = w1 + w1p*step_dt
			new_w2 = w2 + w2p*step_dt
	
			new_th1 = th1 + w1*step_dt
			new_th2 = th2 + w2*step_dt
	
			self.w[0] = new_w1
			self.w[1] = new_w2
	
			ref = [1.0, 0.0]
			dp1 = v2_rot( ref, new_th1-halfpi )
			dp2 = v2_rot( ref, new_th2-halfpi )
	
			p0 = particles[self.fixed]
			
			invDt = 1.0/step_dt
			
			old_p1 = v2_copy(p1.pos)
			p1.pos = v2_add(p0.pos, v2_muls(dp1, L1))
			p1.vel = v2_muls(v2_sub(p1.pos, old_p1), invDt)
			
			old_p2 = v2_copy(p2.pos)
			p2.pos = v2_add(p1.pos, v2_muls(dp2, L2))
			p2.vel = v2_muls(v2_sub(p2.pos, old_p2), invDt)


		if ( float(int(steps)) != steps):
			self.restSteps = 1




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
		
		#applyForces(w)
		stepPendulums(w, w.dt)
		stepMotion(w)

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



def stepPendulums(w, dt):

	for p in w.pendulums:
		p.step(w.g, w.staticParticles, dt)


def stepMotion(w):
	
	w.momentum = 0.0
	
	for p in w.particles:
		p.vel = v2_add(p.vel, v2_muls(p.acc, w.dt))
		p.pos = v2_add(p.pos, v2_muls(p.vel, w.dt))
		
		w.momentum = w.momentum + v2_len(p.vel) * p.m



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
	
worldFillers = []
worldFillerIndex = 0


def setupPendulum(w, offset):

	particles = w.staticParticles

	if 1:
		p0 = len(particles)
		particles.append(Particle(v2_add(offset, [0.0,0.0]), 0.1))
		particles[-1].m = 0.0
		
		p1 = len(particles)
		particles.append(Particle(v2_add(offset, [1.0,0.0]), 0.1))
		particles[-1].m = 1.0
		
		p2 = len(particles)
		particles.append(Particle(v2_add(offset, [2.0,0.0]), 0.1))
		particles[-1].m = 1.0
		
		w.pendulums.append(DoublePendulumEuler(particles, p0, p1, p2))

		return [p0, p1, p2]	


def fillWorldLongCable1(w):

	particles = w.staticParticles

	off = [5.0, 20.0]
	ox = 0.0

	if 1:
		ps = setupPendulum(w, [off[0]+ox, off[1]])
		ox = ox + 4.0
		w.pendulums.append(DoublePendulumEuler(particles, ps[0], ps[1], ps[2]))
		w.pendulums[-1].dt = 1.0/(60.0)

	if 1:
		ps = setupPendulum(w, [off[0]+ox, off[1]])
		ox = ox + 4.0
		w.pendulums.append(DoublePendulumEuler(particles, ps[0], ps[1], ps[2]))
		w.pendulums[-1].dt = 1.0/(60.0)

	if 1:
		ps = setupPendulum(w, [off[0]+ox, off[1]])
		ox = ox + 4.0
		w.pendulums.append(DoublePendulumEuler(particles, ps[0], ps[1], ps[2]))
		w.pendulums[-1].dt = 1.0/(60.0*4.0)

	if 1:
		ps = setupPendulum(w, [off[0]+ox, off[1]])
		ox = ox + 4.0
		w.pendulums.append(DoublePendulumEuler(particles, ps[0], ps[1], ps[2]))
		w.pendulums[-1].dt = 1.0/(60.0*8.0)

	if 1:
		ps = setupPendulum(w, [off[0]+ox, off[1]])
		ox = ox + 4.0
		w.pendulums.append(DoublePendulumEuler(particles, ps[0], ps[1], ps[2]))
		w.pendulums[-1].dt = 1.0/(60.0*16.0)



worldFillerIndex = len(worldFillers)
worldFillers.append(fillWorldLongCable1)


def nextWorld():
	global world
	global worldFillers
	global worldFillerIndex
	
	world = World()
	worldFillers[worldFillerIndex](world)
	worldFillerIndex = (worldFillerIndex+1) % len(worldFillers)

def repeatWorld():
	global worldFillerIndex
	worldFillerIndex = (len(worldFillers) + worldFillerIndex-1) % len(worldFillers)
	nextWorld()

nextWorld()


config = pyglet.gl.Config(double_buffer=True)
window = pyglet.window.Window(800,600, config=config)

fps_display = pyglet.clock.ClockDisplay(pyglet.font.load('Arial', 10), interval=0.1, format='%(fps).0f', color=(1.0, 1.0, 1.0, 0.5))
singleStep = False
doSingleStep = False
microStep = False
doMicroStep = False

@window.event
def on_draw():
	window.clear()
	
	fps_display.update_text()
	fps = pyglet.text.Label(fps_display.label.text, font_name='Arial', font_size=6, x=window.width, y=window.height,anchor_x='right', anchor_y='top')
	fps.draw()
	
	mom = pyglet.text.Label('{0:.2f} - {1:.2f}'.format(world.momentum, world.momentum/max(1,len(world.particles))), font_name='Arial', font_size=6, x=0, y=window.height,anchor_x='left', anchor_y='top')
	mom.draw()
		
	
	for s in world.statics:
		draw_line(s.v1[0] * ppm, s.v1[1] * ppm, s.v2[0] * ppm, s.v2[1] * ppm)
	
	for p in world.particles:
		draw_particle(p.pos[0] * ppm, p.pos[1] * ppm, p.radius * ppm)
		
	for p in world.staticParticles:
		draw_particle(p.pos[0] * ppm, p.pos[1] * ppm, p.radius * ppm)
		
	for p in world.pendulums:
		p0 = world.staticParticles[p.fixed].pos
		p1 = world.staticParticles[p.p[0]].pos
		p2 = world.staticParticles[p.p[1]].pos
		draw_line(p0[0]*ppm, p0[1]*ppm, p1[0]*ppm, p1[1]*ppm)
		draw_line(p1[0]*ppm, p1[1]*ppm, p2[0]*ppm, p2[1]*ppm)

	for c in world.contactPairs:
		pt2 = v2_add(c.info.p, v2_muls(v2_normalize(c.info.n), c.info.d))
		draw_line(c.info.p[0]*ppm, c.info.p[1]*ppm, pt2[0]*ppm, pt2[1]*ppm)

	for f in world.forces:
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

def update(dt):
	global microStep
	global doMicroStep
	global singleStep
	global doSingleStep

	if singleStep:
		if (doSingleStep):
			stepWorld(world, world.timeStep, False)
			doSingleStep = False
#	elif microStep:
#		if (doMicroStep):
#			stepWorld(world, world.timeStep, True)
#			doMicroStep = False
	else:
		stepWorld(world, dt, False)



@window.event	
def on_key_press(symbol, modifiers):
	global microStep
	global doMicroStep
	global singleStep
	global doSingleStep

	if symbol == pyglet.window.key.N:
		nextWorld()
		
	if symbol == pyglet.window.key.R:
		repeatWorld()

	if symbol == pyglet.window.key.S:
		singleStep = ~singleStep
		doSingleStep = False

	if symbol == pyglet.window.key.SPACE:
		doSingleStep = True	

	if symbol == pyglet.window.key.M:
		microStep = ~microStep
		doMicroStep = False

	if symbol == pyglet.window.key.SPACE:
		doMicroStep = True	


pyglet.clock.schedule_interval(update, 1.0/60.0)	
pyglet.app.run()

#arch -i386 python2.6 doublependulum.py
