#
# idea: if a closed form solution is not possible, what about some closed form expression for some 
# other variables than positins, e.g kineric energy, speed, work , no clue ... that can be used to 
# improve the solver results
#
#
#

import sys
import math
import time
import random
from gaussy import *
import pyglet

dbgPositions = False
dbgSolver = False

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
	constraints = None
	forces = None
	clientUpdate = None
	momentum = 0.0
	corout = False
	solveLSFunc = None
	solveFunc = None
	solveIters = 1 # only used for iterative solvers
	ERP = 0.05
	
	def __init__(self):
		self.statics = []
		self.forces = []
		self.particles = []
		self.constraints = []
		self.staticContactPairs = []
		self.contactPairs = []
		self.clientUpdate = None
		self.solveFunc = solveConstraintsLS
		self.solveLSFunc = GaussSolve2

	

class Particle:
	acc = [0,0]
	ppos = [0,0]
	vel = [0,0]
	#todo use vel so that the impulse forumaltion works
	pos = [0,0]
	radius = 0.2
	m = 1.0
	
	def __init__(self, p, r):
		self.acc = [0,0]
		self.vel = [0,0]
		self.ppos = [p[0], p[1]]
		self.pos = [p[0], p[1]]
		self.radius = r
		self.m = r

	def applyImpulse_(self, imp, dt):
		if self.m > 0.0:
			dv = v2_muls(imp, 1.0/self.m)
			dp = v2_muls(dv, dt)
			self.pos = v2_add(self.pos, dp)


	def applyImpulse(self, imp, dt):
		if self.m > 0.0:
			dv = v2_muls(imp, 1.0/self.m)
			self.vel = v2_add(self.vel, dv)


def createMinvDiag(particles):
	invMDiag = [0.0]*(len(particles)*2)
	for i in range(len(particles)):
		if particles[i].m > 0.0: 
			invMDiag[2*i+0] = 1.0/particles[i].m
		else:
			invMDiag[2*i+0] = 0.0
		invMDiag[2*i+1] = invMDiag[2*i+0]
	return invMDiag	


def createExtVel(particles, dt):
	extVel = Matrix( len(particles)*2, 1, 0.0 )
	for i in range(len(particles)):
		vel = v2_muls( particles[i].acc, particles[i].m * dt )
		extVel[2*i+0][0] = vel[0]
		extVel[2*i+1][0] = vel[1]
	return extVel	


def createIntVel(particles, dt):
	invDt = 1.0/dt
	intVel = Matrix( len(particles)*2, 1, 0.0 )
	for i in range(len(particles)):
		#vel = v2_muls(  v2_sub(particles[i].pos, particles[i].ppos), invDt )
		vel = particles[i].vel
		intVel[2*i+0][0] = vel[0]
		intVel[2*i+1][0] = vel[1]
	return intVel	


def createJacobianAndBias(constraints, particles, erp, dt):
		
	invDt = 1.0 / dt	
	J = Matrix( len(constraints), len(particles) * 2, 0.0 )
	Bias = Matrix( len(constraints), 1, 0.0 )

	for index in range(len(constraints)):	
		ct = constraints[index]
		Jrow = J[index]	
		Jb1 = ct.p[0] * 2
		Jb2 = ct.p[1] * 2
		
		if (particles[ct.p[0]].m > 0.0 or particles[ct.p[1]].m > 0.0):
			
			vdist = v2_sub(particles[ct.p[0]].pos, particles[ct.p[1]].pos)
			err = v2_len(vdist) - ct.rl
			vdir = v2_normalize(vdist)

			if (particles[ct.p[0]].m > 0.0):
				Jrow[Jb1] = vdir[0]
				Jrow[Jb1+1] = vdir[1]

			if (particles[ct.p[1]].m > 0.0):
				Jrow[Jb2] = -vdir[0]
				Jrow[Jb2+1] = -vdir[1]
			
			Bias[index][0] = -err * invDt * 1.0 * erp

	return [J, Bias]


def solveLS_G(JB, RHS, iters):
	# Gaussian
	return GaussSolve2(JB, RHS)


def solveLS_GS(JB, RHS, iters):
	# Gauss-Seidel Solver
	#print 'Will converge:', GaussSeidelWillConverge(JB, True), 'Might:', GaussSeidelMightConverge(JB, True)
	return GaussSeidelSolve2ZeroGuess(JB, RHS, IterObj(iters, -1.0)) 


def solveLS_GSC01(JB, RHS, iters):
	iter = IterObj(50, 0.01)
	ret = GaussSeidelSolve2ZeroGuess(JB, RHS, iter) # 0.01 maxerr, 50 iter
	#print iter.it
	#if iter.it >= 50:
	#	print iter.it, 'iters,', iter.err, 'err'
	return ret


def solveConstraintsLS(w, constraints, particles, erp, dt):

	if (len(constraints) == 0):
		return

	dbg = dbgSolver	

	J_Bias = createJacobianAndBias(constraints, particles, erp, dt)
	J = J_Bias[0]
	Bias = J_Bias[1]

	if dbg:
		PrintM(J, 'J')
		PrintM(Bias, 'Bias')
	Jt = Transp(J)
	MinvDiag = createMinvDiag(particles)
	extVel = createExtVel(particles, dt)
	intVel = createIntVel(particles, dt)

	#PrintV(MinvDiag)
	#PrintM(Jt)
	B = MulMDiag1(MinvDiag, Jt)
	#PrintM(B)
	JB = MulM(J, B)

	if dbg:
		PrintM(JB, 'JB')

	if dbg:
		PrintM(intVel, 'intVel')
		PrintM(extVel, 'extVel')
	RHS_V = AddM(intVel, extVel)
	#PrintM(J)
	#PrintM(RHS_V)
	RHS_JV = MulM(J, RHS_V)
	#RHS = NegM(RHS_JV)
	RHS = SubM(Bias, RHS_JV)

	#PrintM(JB)
	#PrintM(RHS)
		
	lbda = w.solveLSFunc(JB, RHS, w.solveIters)
	
	# Gaussian solver
	#lbda = GaussSolve2(JB, RHS)

	if dbg:
		PrintV(lbda)

	if (lbda[0] == None):
		PrintM(JB, 'Failed JB')
		return
	
	impulses = MulMV(Jt, lbda)
	if dbg:
		PrintM(impulses, 'Impulses')

	for i in range(len(particles)):
		imp = [ impulses[i*2+0][0], impulses[i*2+1][0] ]
		#PrintV(imp)
		particles[i].applyImpulse(imp, dt)


def solveConstraintsSI_iter(constraints, particles, erp, dt):
		
	invDt = 1.0 / dt	

	for index in range(len(constraints)):	
		ct = constraints[index]

		p0 = particles[ct.p[0]]
		p1 = particles[ct.p[1]]
		
		if (p0.m > 0.0 or p1.m > 0.0):
			
			vdist = v2_sub(p0.pos, p1.pos)
			err = v2_len(vdist) - ct.rl
			vdir = v2_normalize(vdist)

			dvel = v2_sub(p0.vel, p1.vel)
			dvelProj = v2_dot(dvel, vdir)
			bias = -err * invDt * 1.0 * erp

			corrVel = v2_muls(vdir, bias - dvelProj)

			invSumMass = 1.0 / (p0.m + p1.m)

			if (p0.m > 0.0):
				fac = p1.m * invSumMass
				if fac == 0.0: 
					fac = 1.0
				imp0 = v2_muls(corrVel, fac * p0.m )
				p0.applyImpulse(imp0, dt)	
			if (p1.m > 0.0):
				fac = p0.m * invSumMass
				if fac == 0.0: 
					fac = 1.0
				imp1 = v2_muls(corrVel, -fac * p1.m )
				p1.applyImpulse(imp1, dt)


def solveConstraintsSI(w, constraints, particles, erp, dt):
	for i in range(w.solveIters):
		solveConstraintsSI_iter(constraints, particles, erp, dt)


class DistConstraint:
	p = None
	rl = 0.0

	def __init__(self, p1, p2, rl):
		self.p = [ p1, p2 ]
		self.rl = rl
					


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
		solve(w)
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



def solve(w):
	#solveConstraints(w.constraints, w.particles, w.solveLSFunc, w.ERP, w.dt)
	w.solveFunc(w, w.constraints, w.particles, w.ERP, w.dt)
	

def applyExternalForces(w):
	
	for p in w.particles:
		if (p.m > 0.0):
			p.acc = [0.0, w.g]
		else:
			p.acc = [0.0, 0.0]


def stepMotion(w):
	
	w.momentum = 0.0
	dt = w.dt

	dbg = dbgPositions

	if dbg:
		print 'positions'

	for p in w.particles:
		p.vel = v2_add( p.vel, v2_muls(p.acc, dt))
		p.pos = v2_add( p.pos, v2_muls(p.vel, dt))
		if dbg:
			PrintV(p.pos)
		#p.vel = v2_add( p.vel, v2_muls(p.acc, dt))
		v2_zero(p.acc)

		speed = v2_len(p.vel)
		w.momentum = w.momentum + speed * p.m


def stepMotion_(w):
	
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


def shuffleConstraints(w):
	random.seed(0)
	random.shuffle(w.constraints)	

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

	if 0:
		p1 = len(w.particles)
		w.particles.append(Particle([5.0,20.0], 0.1))
		w.particles[-1].m = 0.0
		p2 = len(w.particles)
		w.particles.append(Particle([6.5,20.0], 0.1))
		w.constraints.append(DistConstraint(p1, p2, 1.0))
		#w.g=0


	if 1:
		fac = 3.0
		r = 0.1
		l = fac * 2.0
		ct = int(fac * 8)
		
		dl = l / ct	
		p1 = len(w.particles)
		w.particles.append(Particle([10.0,20.0], r))
		w.particles[-1].m = 0.0
	
		for pi in range(1, ct):
			p2 = len(w.particles)
			w.particles.append(Particle([10.0+pi*dl,20.0], r))
			w.constraints.append(DistConstraint(p1, p2, dl))
			p1 = p2



	if 0:	
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
	
argSolveFuncName = ''
argIters = 1
argERP = 1.0
argShuffle = False

def nextWorld():
	global world
	global worldFillers
	global worldFillerIndex
	global argSolveFuncName
	global argRand

	world = World()
	worldFillers[worldFillerIndex](world)
	worldFillerIndex = (worldFillerIndex+1) % len(worldFillers)
	#pyglet.clock.schedule_interval(update, world.updateDt)	
	if (argSolveFuncName.startswith('solveLS')):
		world.solveFunc = solveConstraintsLS
		world.solveLSFunc = globals()[argSolveFuncName]
	elif (argSolveFuncName.startswith('solveSI')):
		world.solveFunc = solveConstraintsSI
	
	world.solveIters = argIters
	world.ERP = argERP
	
	if (argShuffle):
		shuffleConstraints(world)
		


def repeatWorld():
	global worldFillerIndex
	worldFillerIndex = (len(worldFillers) + worldFillerIndex-1) % len(worldFillers)
	nextWorld()


config = pyglet.gl.Config(double_buffer=True)
window = pyglet.window.Window(800,600, config=config)

fps_display = pyglet.clock.ClockDisplay(pyglet.font.load('Arial', 10), interval=0.1, format='%(fps).0f', color=(1.0, 1.0, 1.0, 0.5))
singleStep = False
doSingleStep = False
microStep = False
doMicroStep = False

for arg in sys.argv:
	if (arg == '-step'):
		singleStep = True
	if (arg == '-shuffle'):
		argShuffle = True
	elif (arg.startswith('solve')):
		argSolveFuncName = arg
	elif (arg.startswith('solve')):
		argSolveFuncName = arg
	elif (arg.startswith('-iters=')):
		spl = arg.split('=')
		argIters = int(spl[1])
	elif (arg.startswith('-erp')):
		spl = arg.split('=')
		argERP = float(spl[1])

nextWorld()

@window.event
def on_draw():
	window.clear()
	
	fps_display.update_text()
	fps = pyglet.text.Label(fps_display.label.text, font_name='Arial', font_size=10, x=window.width, y=window.height,anchor_x='right', anchor_y='top')
	fps.draw()
	
	mom = pyglet.text.Label('{0:.2f} - {1:.2f}'.format(world.momentum, world.momentum/max(1,len(world.particles))), font_name='Arial', font_size=10, x=0, y=window.height,anchor_x='left', anchor_y='top')
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
		singleStep = not singleStep
		doSingleStep = False

	if symbol == pyglet.window.key.SPACE:
		doSingleStep = True	

	if symbol == pyglet.window.key.M:
		microStep = not microStep
		doMicroStep = False

	if symbol == pyglet.window.key.SPACE:
		doMicroStep = True	


pyglet.clock.schedule_interval(update, 1.0/60.0)	
pyglet.app.run()

# arch -i386 python2.6 turtly1.py 
