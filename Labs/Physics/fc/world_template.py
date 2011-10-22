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
	forces = None
	clientUpdate = None
	momentum = 0.0
	corout = False
	
	def __init__(self):
		self.statics = []
		self.forces = []
		self.particles = []
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
		stepMotion(w)
		allPairs = findContactPairs(w)
		w.staticContactPairs = allPairs[0]
		w.contactPairs = allPairs[1]
		
		for i in range(16):
			applyForces(w)

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
		w.statics.append(Convex([30.0,0.5], [30.0, 29.0], wallMat))	

	if 1:	
		w.particles.append(Particle([34.0,25.0], [0.5, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([35.0,25.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 2.0))
		w.particles.append(Particle([37.0,23.0], [0.5, 0.0], 0.4, sharedMat))	
		w.particles.append(Particle([38.0,23.5], [0.0, 0.0], 0.4, sharedMat))	
		w.forces.append(CableForce(w.particles[-1], w.particles[-2], defR , 2.0))
		wallMat = Material(0.9)
		w.statics.append(Convex([29.5,18.0], [40.5, 18.0], wallMat))	

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
	w.statics.append(Convex([2.0,20.0], [2.0, 15.0], wallMat))	


worldFillerIndex = len(worldFillers)
worldFillers.append(fillWorldLongCable1)

worldFillerIndex = len(worldFillers)
worldFillers.append(fillWorldSupportCable1)


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


