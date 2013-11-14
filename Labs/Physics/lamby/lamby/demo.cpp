#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../glfw/include/GLFW/glfw3.h"
#pragma comment( lib, "../../glfw/lib-msvc100/glfw3.lib")
#pragma comment( lib, "opengl32.lib")
#pragma comment( lib, "glu32.lib")

#include "stdio.h"

#include "nics.h"
#include "lamby.h"
#include "painty.h"
#include "thingy.h"
#include "touchy.h"
#include "rbody.h"


struct TimerGlfw
{
	TimerGlfw()
	{
		init(1.0f/30.0f);
	}

	void init(float deltaT)
	{
		 timeOrigin = -1.0;
		 totalFrames = 0;
		 dt = deltaT;
	}

	void update()
	{
		double clockTime = glfwGetTime();

		if (timeOrigin < 0.0)
		{
			timeOrigin = clockTime;
			lastTime = 0.0;
			time = 0.0;
			elapsedFrames = 0;
			elapsedTime = 0.0f;
		}
		else
		{
			clockTime = clockTime - timeOrigin;
			float elaps = (float)(clockTime - time);
			elapsedFrames = (int)((elaps / dt)+0.5f);
			lastTime = time;
			elapsedTime = float(elapsedFrames) * dt;
			time += elapsedTime;
		}
	}

	float dt;
	double timeOrigin;
	double lastTime;
	double time;
	unsigned long totalFrames;
	int elapsedFrames;
	float elapsedTime;
};

struct Scene
{
	Thingies thingies;
	PhysWorld physWorld;
	TimerGlfw clock;
	Painty painty;
};


struct WindowData
{
	float scale;
	bool translating;
	float transl_ref[2];
	float transl_moving[2];
	float transl[2];

	Scene scene;
	double lastTime;

	WindowData() : scale(1.0f), lastTime(0.0) 
	{ 
		translating = false; transl_moving[0]=0.0f; transl_moving[1]=0.0f; transl[0]=0.0f; transl[1]=0.0f; 
		initPhysWorld(); 
	}
	
	void initPhysWorld()
	{
		scale = 1.0f/10.0f;
		if (0)
		{
			scene.physWorld.addRBody( v_z(), u_k(), 2 );
			scene.physWorld.addRBody( muls(u_ij(), 2.0f) , muls(u_i(), 0.2f), 5 );
		}
		if (1)
		{
			scene.physWorld.addRBody( v_z(), v_z(), 2 );
			scene.physWorld.addRBody( v_z(), v_z(), 1*6 );
		}
	}
};

void drawScene(Scene& scene)
{
	glfwSwapInterval(1);	//vsync on.
	scene.clock.update();
	float dt = scene.clock.elapsedTime;
	Painty& painty = scene.painty;
	
	if (dt)
		painty.begin();

	if (dt)
	{
		PhysWorld& w = scene.physWorld;
		
		//if (dt)
		{
			//w.applyGravity();
			w.step(1.0f*dt);
		}

		PhysWorld::RBodyIter it(w.rbodies); RBody* el; 
		while(el = it.next()) 
		{
			const RShape& shape = w.rshapes[el->shape];
			draw_convex(painty, rigid(asV2(el->q), el->q(2)), shape.v, shape.count, shape.r, u_ijk());
		}

		//if (dt)
		{
			if (w.rbodies.size >= 2)
			{
				RBody* b1 = w.rbodies.at(0);
				RBody* b2 = w.rbodies.at(1);

				M3 m1 = rigid(asV2(b1->q), b1->q(2));
				M3 m2 = rigid(asV2(b2->q), b2->q(2));

				const RShape& shape1 = w.rshapes[b1->shape];
				const RShape& shape2 = w.rshapes[b2->shape];

				if (0)
				{
					gjk::Out_gjk_distance out = gjk::gjk_distance(w.gjk, m1, shape1.v, shape1.count, shape1.r, m2, shape2.v, shape2.count, shape2.r, 1.e-5f);

					if (out.success)
					{
						V2 v[] = { out.v1, out.v2 };
						draw_convex(painty, m3_id(), v, 2, 0.0f, u_j());
					}
					else
						printf("failed\n");
				}
				else
				{
					gjk::Out_epa_distance out; 
					gjk::epa_distance(out, w.epa, m1, shape1.v, shape1.count, shape1.r, m2, shape2.v, shape2.count, shape2.r, 1.e-3f, 1.e-5f);

					if (out.success)
					{
						V2 v[] = { out.v1, out.v2 };
						draw_convex(painty, m3_id(), v, 2, 0.0f, out.dist > out.gjk.eps ? u_j() : u_i());
					}
					else
						printf("failed\n");
				}
			}
		}
	}

	if (0)
	{
		Thingies thingies;

		V2 v[] = { V2(-0.2f,-0.2f), V2(-0.2f,0.2f), V2(0.2f,0.2f), V2(0.2f,-0.2f) };
		M3 m = rigid(V2(-0.1f, -0.3f), m_rad(20.0f));
		thingies.addConvex(m, v, 4, Rl(0.1));

		Thingies::ConvexIter it(thingies.convexes); Convex* el; 
		while(el = it.next()) 
			draw_convex(painty, el->transf, el->v, el->count, el->r, u_ijk());
	}

	if (0)
	{
		if (1)
		{
			V2 v[] = { V2(-0.2f,-0.2f), V2(-0.2f,0.2f), V2(0.2f,0.2f), V2(0.2f,-0.2f) };
			M3 m = rigid(V2(0.1f, 0.3f), m_rad(20.0f));
			draw_convex(painty, m, v, 4, u_ijk());
		}

		if (1)
		{
			V2 v[] = { V2(-0.2f,-0.2f), V2(-0.23f,0.2f), V2(0.2f,0.25f), V2(0.2f,-0.2f) };
			M3 m = rigid(V2(0.1f, 0.3f), m_rad(-30.0f));
			draw_convex(painty, m, v, 4, Rl(0.2), u_ijk());
		}
	}

	if (dt)
		painty.end();

	painty.draw();
}

void display(GLFWwindow* window)
{
	WindowData& wd = *(WindowData*) glfwGetWindowUserPointer(window);

	int width,height; 
	glfwGetWindowSize(window, &width, &height);
	glViewport( 0, 0, width, height );
	
	float aspect = (float)width/(float)height;
	if (0)
	{
		glMatrixMode( GL_PROJECTION ); 
		glLoadIdentity(); 
		gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 1.0f, 100.0f );
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glMatrixMode(GL_PROJECTION);      
		glLoadIdentity();                 
		gluOrtho2D(-1.0*aspect, 1.0*aspect, -1.0, 1.0); // Set clipping area's left, right, bottom, top
	}

    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); 
    glClear( GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); 
    glLoadIdentity(); 

	if (1)
	{
		glTranslatef(aspect*(wd.transl[0]+wd.transl_moving[0])*2.0f/(float)width, (wd.transl[1]+wd.transl_moving[1])*2.0f/(float)height, 0.0f);
		glScalef(wd.scale, wd.scale, 1.0f);
	}

	double t = glfwGetTime();
	drawScene(wd.scene);

	glfwSwapBuffers(window);
}

namespace glfwHandler
{

	void key(GLFWwindow* window, int key,int press, int, int) 
	{
		switch (key) 
		{
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, 1); break;
		}
	}
	 
	void resize(GLFWwindow* window, int width,int height)
	{
	}

	void scroll(GLFWwindow* window, double ox,double oy)
	{
		WindowData& wd = *(WindowData*) glfwGetWindowUserPointer(window);
		float off = (float) (oy * 0.05);
		if (wd.scale + off > 0.0f) 
		{
			wd.scale += off;
		}
	}

	void mouseButton(GLFWwindow* window,int button, int action,int mods)
	{
		WindowData& wd = *(WindowData*) glfwGetWindowUserPointer(window);

		if (button == GLFW_MOUSE_BUTTON_2)
		{
			if (action == GLFW_PRESS)
			{
				double x, y;
				glfwGetCursorPos(window, &x, &y);
				wd.translating = true;
				wd.transl_ref[0] = (float)(x);
				wd.transl_ref[1] = (float)(y);
			}
			else if (action == GLFW_RELEASE)
			{
				wd.translating = false;
				wd.transl[0] += wd.transl_moving[0];
				wd.transl[1] += wd.transl_moving[1];
				wd.transl_moving[0] = 0.0f;
				wd.transl_moving[1] = 0.0f;
			}
		}
	}

	void cursorPos(GLFWwindow* window, double x,double y)
	{
		WindowData& wd = *(WindowData*) glfwGetWindowUserPointer(window);
		
		if (wd.translating)
		{
			wd.transl_moving[0] = (float)(x)-wd.transl_ref[0];
			wd.transl_moving[1] = -((float)(y)-wd.transl_ref[1]);
		}
		else
		{
		}
	}
}

int main(void)
{
	if (0)
	{
		gjk::gjk_tests();
		printf("gjk: %f us.\n", double(nics::getGjkRunTime())*(1000.0*1000.0));
		nics::Rrt_Rn<float, 4> rrt;
		nics::flann_test1();
		nics::flann_test2();
		//printf("rrt: %f us.\n", double(nics::rtt_test1())*(1000.0*1000.0));
		printf("rrt: %f us.\n", double(nics::rtt_test2())*(1000.0*1000.0));
	
		//return 0;
	}
	
    if (!glfwInit()) return -1;
	
    GLFWwindow* window = glfwCreateWindow(800, 600, "Lamby", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);

	WindowData wd;
	glfwSetWindowUserPointer(window, &wd);

	int w,h; glfwGetWindowSize(window, &w, &h); glfwHandler::resize(window, w, h);
	glfwSetWindowSizeCallback(window, glfwHandler::resize); 
	glfwSetKeyCallback(window, glfwHandler::key); 
	glfwSetScrollCallback(window, glfwHandler::scroll);
	glfwSetMouseButtonCallback(window, glfwHandler::mouseButton);
	glfwSetCursorPosCallback(window, glfwHandler::cursorPos);
	
	while (!glfwWindowShouldClose(window))
	{
		display(window); 
		glfwPollEvents();
	}

    return 0;
}