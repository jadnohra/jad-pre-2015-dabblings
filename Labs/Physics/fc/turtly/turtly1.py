import pyglet
import math
import time
from gaussy import *


class World:
	updateDt = 1.0/60.0
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
	acc = [0,0]
	ppos = [0,0]
	pos = [0,0]
	radius = 0.2
	m = 1.0
	invM = 1.0 / m
	
	def __init__(self, p, r):
		self.acc = [0,0]
		self.ppos = [p[0], p[1]]
		self.pos = [p[0], p[1]]
		self.radius = r
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
		
		applyExternalForces(w)
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



def applyExternalForces(w):
	
	for p in w.particles:
		if (p.m > 0.0):
			p.acc = [0.0, w.g]
		else:
			p.acc = [0.0, 0.0]


def stepMotion(w):
	
	w.momentum = 0.0
	dt2 = w.dt * w.dt
	
	for p in w.particles:
		ppos = p.pos
		dacc = v2_muls(p.acc, dt2)
		p.pos = v2_add( v2_sub( v2_add(p.pos, p.pos), p.ppos), dacc)
		p.ppos = ppos
		v2_zero(p.acc)

		speed = v2_len(v2_sub(p.pos, p.ppos))
		w.momentum = w.momentum + speed * p.m



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
#------------ WORLDS ------------
#--------------------------------
	
worldFillers = []
worldFillerIndex = 0


def fillWorldLongCable1(w):
	if 1:	
		for j in range(1, 5):
			tl = 5.0
			rf = 0.5
			sx = 5.0+j*5
			sy = 20.0
			ox = 2.0
			num=j*5
			w.particles.append(Particle([sx,sy], 0.01))	
			for i in range(1, num):
				w.particles.append(Particle([sx+(ox*i)/num,sy+(rf*tl*i)/num], 0.01))


worldFillerIndex = len(worldFillers)
worldFillers.append(fillWorldLongCable1)


#--------------------------------
#------------ MAIN	-------------
#--------------------------------
	

def nextWorld():
	global world
	global worldFillers
	global worldFillerIndex
	
	world = World()
	worldFillers[worldFillerIndex](world)
	worldFillerIndex = (worldFillerIndex+1) % len(worldFillers)
	#pyglet.clock.schedule_interval(update, world.updateDt)	

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
		
	
	for p in world.particles:
		draw_particle(p.pos[0] * ppm, p.pos[1] * ppm, p.radius * ppm)
		

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


