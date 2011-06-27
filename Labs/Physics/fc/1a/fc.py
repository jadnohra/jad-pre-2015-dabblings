import pyglet
import math

window = pyglet.window.Window()

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

class Particle:
	pos = [0,0]
	vel = [0,0]
	
	def __init__(self, p, v):
		self.pos = p
		self.vel = v
	
particles = []
particles.append(Particle([200,100], [12, 5]))	
particles.append(Particle([200,200], [5, 12]))	

xm = 100.0

@window.event
def on_draw():
	global xm
	window.clear()
	#pyglet.clock.ClockDisplay().draw()
	#640X480
	#label.draw()
	#pyglet.graphics.draw(2, pyglet.gl.GL_POINTS, ('v2i', (10, 15, 30, 35)))
	#pyglet.graphics.draw(2, pyglet.gl.GL_POINTS, ('v2i', (630, 15, 30, 35)))
	draw_box(630, 15, 30, 35)
	draw_line(630, 215, 30, 235)
	draw_particle(xm, 100, 10)
	
	for p in particles:
		draw_particle(p.pos[0], p.pos[1], 5)
	

def update(dt):
	global xm
	global particles
	xm += 10.0 * dt
	for p in particles:
		for i in range(2):
			p.pos[i] = p.pos[i] + p.vel[i] * dt

	
pyglet.clock.schedule_interval(update, 1.0/60)	
pyglet.app.run()