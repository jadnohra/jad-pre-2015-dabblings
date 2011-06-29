import pyglet
import math

#--------------------------------
#------------ PHYSICS -----------
#--------------------------------

class World:
	particles = []


class Particle:
	pos = [0,0]
	vel = [0,0]
	radius = 0.2

	def __init__(self, p, v, r):
		self.pos = p
		self.vel = v
		self.radius = r

class ContactPair:
	obj = []


def findContactPairs(world):
	return 0


#--------------------------------
#------------ RENDERING	---------
#--------------------------------

ppm = 40.0      # pixels per meter
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
world.particles.append(Particle([10.0,10.0], [1, 0.5], 0.2))	
world.particles.append(Particle([12.0,10.0], [0.3, -1.2], 0.2))	

config = pyglet.gl.Config(double_buffer=True)
window = pyglet.window.Window(800,600, config=config)


@window.event
def on_draw():
	window.clear()
	pyglet.clock.ClockDisplay().draw()
	
	for p in world.particles:
		draw_particle(p.pos[0] * ppm, p.pos[1] * ppm, p.radius * ppm)
	

def update(dt):
	for p in world.particles:
		for i in range(2):
			p.pos[i] = p.pos[i] + p.vel[i] * dt


pyglet.clock.schedule_interval(update, 1.0/30.0)	
pyglet.app.run()

