#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../glfw/include/GLFW/glfw3.h"
#pragma comment( lib, "../../glfw/lib-msvc100/glfw3.lib")
#pragma comment( lib, "opengl32.lib")
#pragma comment( lib, "glu32.lib")

#include "lamby.h"
#include "painty.h"


struct WindowData
{
	float scale;
	bool translating;
	float transl_ref[2];
	float transl_moving[2];
	float transl[2];

	WindowData() : scale(1.0f) { translating = false; transl_moving[0]=0.0f; transl_moving[1]=0.0f; transl[0]=0.0f; transl[1]=0.0f; }
};

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

		if (1)
		{
			Painty painty;
			V2 v[] = { V2(-0.2f,-0.2f), V2(-0.2f,0.2f), V2(0.2f,0.2f), V2(0.2f,-0.2f) };
			M3 m = rigid(V2(0.1f, 0.3f), m_rad(20.0f));
			draw_convex(painty, m, v, 4, u_ijk());
		}

		if (1)
		{
			Painty painty;
			V2 v[] = { V2(-0.2f,-0.2f), V2(-0.23f,0.2f), V2(0.2f,0.25f), V2(0.2f,-0.2f) };
			M3 m = rigid(V2(0.1f, 0.3f), m_rad(-30.0f));
			draw_convex(painty, m, v, 4, Rl(0.2), u_ijk());
		}

		if (0)
		{
			glBegin(GL_LINE_LOOP);         
			  glColor3f(1.0f, 1.0f, 1.0f); 
			  glVertex2f(-0.5f, -0.5f);     
			  glVertex2f(-0.5f, 0.5f);     
			  glVertex2f(0.5f, 0.5f);
			  glVertex2f(0.5f, -0.5f);
		   glEnd();

		   // CCW
			glBegin(GL_QUADS);              
			  glColor3f(1.0f, 0.0f, 0.0f); 
			  glVertex2f(-0.8f, 0.1f);     
			  glVertex2f(-0.2f, 0.1f);     
			  glVertex2f(-0.2f, 0.7f);
			  glVertex2f(-0.8f, 0.7f);

			  glColor3f(0.0f, 1.0f, 0.0f); 
			  glVertex2f(-0.7f, -0.6f);
			  glVertex2f(-0.1f, -0.6f);
			  glVertex2f(-0.1f,  0.0f);
			  glVertex2f(-0.7f,  0.0f);

			  glColor3f(0.2f, 0.2f, 0.2f); 
			  glVertex2f(-0.9f, -0.7f);
			  glColor3f(1.0f, 1.0f, 1.0f);
			  glVertex2f(-0.5f, -0.7f);
			  glColor3f(0.2f, 0.2f, 0.2f); 
			  glVertex2f(-0.5f, -0.3f);
			  glColor3f(1.0f, 1.0f, 1.0f); 
			  glVertex2f(-0.9f, -0.3f);
			glEnd();
			glBegin(GL_TRIANGLES);         
			  glColor3f(0.0f, 0.0f, 1.0f); 
			  glVertex2f(0.1f, -0.6f);
			  glVertex2f(0.7f, -0.6f);
			  glVertex2f(0.4f, -0.1f);

			  glColor3f(1.0f, 0.0f, 0.0f);
			  glVertex2f(0.3f, -0.4f);
			  glColor3f(0.0f, 1.0f, 0.0f); 
			  glVertex2f(0.9f, -0.4f);
			  glColor3f(0.0f, 0.0f, 1.0f); 
			  glVertex2f(0.6f, -0.9f);
			glEnd();
			glBegin(GL_POLYGON);           
			  glColor3f(1.0f, 1.0f, 0.0f); 
			  glVertex2f(0.4f, 0.2f);
			  glVertex2f(0.6f, 0.2f);
			  glVertex2f(0.7f, 0.4f);
			  glVertex2f(0.6f, 0.6f);
			  glVertex2f(0.4f, 0.6f);
			  glVertex2f(0.3f, 0.4f);
			glEnd();
		}
	}

	if (0)
	{
		glBegin(GL_QUADS); //Begin quadrilateral coordinates
		//Trapezoid
		glVertex3f(-0.7f, -1.5f, -5.0f);
		glVertex3f(0.7f, -1.5f, -5.0f);
		glVertex3f(0.4f, -0.5f, -5.0f);
		glVertex3f(-0.4f, -0.5f, -5.0f);
		glEnd(); //End quadrilateral coordinates

		glBegin(GL_TRIANGLES); //Begin triangle coordinates
		//Pentagon
		glVertex3f(0.5f, 0.5f, -5.0f);
		glVertex3f(1.5f, 0.5f, -5.0f);
		glVertex3f(0.5f, 1.0f, -5.0f);
		glVertex3f(0.5f, 1.0f, -5.0f);
		glVertex3f(1.5f, 0.5f, -5.0f);
		glVertex3f(1.5f, 1.0f, -5.0f);
		glVertex3f(0.5f, 1.0f, -5.0f);
		glVertex3f(1.5f, 1.0f, -5.0f);
		glVertex3f(1.0f, 1.5f, -5.0f);

		//Triangle
		glVertex3f(-0.5f, 0.5f, -5.0f);
		glVertex3f(-1.0f, 1.5f, -5.0f);
		glVertex3f(-1.5f, 0.5f, -5.0f);
		glEnd(); //End triangle coordinates
	}
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
	}
}

int main(void)
{
    if (!glfwInit()) return -1;
	
    GLFWwindow* window = glfwCreateWindow(640, 480, "Lamby", NULL, NULL);
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