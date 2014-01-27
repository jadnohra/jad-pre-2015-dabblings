#ifndef LAB_WINDOW2D_H
#define LAB_WINDOW2D_H

#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "../../../glfw/include/GLFW/glfw3.h"
#pragma comment( lib, "../../../glfw/lib-msvc100/glfw3.lib")
#pragma comment( lib, "opengl32.lib")
#pragma comment( lib, "glu32.lib")

#include "math_ops.h"

namespace window2d
{
	struct DrawContext
	{
		float* vs;
		float* angles;
		V2* sincos;
		int count;
		int subdiv;
		int angle_subdiv;
		GLuint listName;

		float* getBuffer(int count_)
		{
			if (count_ > count)
			{
				free(vs);
				count = count_;
				vs = (float*) malloc(count*sizeof(float));
			}
			return vs;
		}

		DrawContext() : vs(0), count(0), subdiv(6), angle_subdiv(0)
		{ 
			getBuffer(64); 
			glEnableClientState(GL_VERTEX_ARRAY); 
			listName = glGenLists(1);

			angle_subdiv = subdiv * 4;
			angles = (float*) malloc( subdiv * 4 * sizeof(float));
			sincos = (V2*) malloc( subdiv * 4 * sizeof(V2));

			for (int i=0; i<angle_subdiv; ++i)
			{
				angles[i] = 2.0f*rl_pi()*float(i)/float(angle_subdiv);
				sincos[i].x[0] = m_sin(angles[i]);
				sincos[i].x[1] = m_cos(angles[i]);
			}
		}

		~DrawContext() 
		{ 
			free(vs); 
			free(angles);
			free(sincos);
			vs = 0; 
			glDeleteLists(listName, 1);
		}

		void begin()
		{
			glNewList(listName, GL_COMPILE);
		}

		void end()
		{
			glEndList();
		}

		void draw()
		{
			glCallList(listName);
		}
	};

	void draw_circle(DrawContext& ctx, V2 v, Sc r, V3p col)
	{
		float* vs = ctx.getBuffer(2*ctx.angle_subdiv);
		for (int i=0,j=0;i<ctx.angle_subdiv;i++,j+=2)
		{
			V2 cv = add(v, muls(ctx.sincos[i], r));
			vs[j]=cv(0); vs[j+1]=cv(1);
		}
		glColor3f(col(0), col(1), col(2)); 
		glVertexPointer( 2, GL_FLOAT, 0, vs ); 
		glDrawArrays( GL_LINE_LOOP, 0, ctx.angle_subdiv );    
	}

	void draw_circle(DrawContext& ctx, M3p m, V2 v, Sc r, V3p col)
	{
		return draw_circle(ctx, m * v, r, col);
	}

	void draw_convex(DrawContext& ctx, M3p m, const V2* v, int count, V3p col)
	{
		float* vs = ctx.getBuffer(2*count);
		for (int i=0,j=0;i<count;i++,j+=2)
		{
			V2 vi = mulp(m, v[i]);
			vs[j]=vi(0); vs[j+1]=vi(1);
		}
		glColor3f(col(0), col(1), col(2)); 
		glVertexPointer( 2, GL_FLOAT, 0, vs ); 
		glDrawArrays( GL_LINE_LOOP, 0, count );    
	}

	void draw_line(DrawContext& ctx, M3p m, V2p v1, V2p v2, V3p col)
	{
		V2 v[] = {v1, v2};
		draw_convex(ctx, m, v, 2, col);
	}

	void draw_convex(DrawContext& ctx, M3p m, const V2* v, int count, Sc r, V3p col)
	{
		if (r == Sc(0))
			draw_convex(ctx, m, v, count, col);

		if (count == 1)
		{
			draw_circle(ctx, mulv(m, v[0]), r, col);
			return;
		}

		struct Local
		{
			static void append_vertex(float* vs, int& vsi, M3p m, const V2* v, int lv, Sc r, int i, int div)
			{
				V2 d1 = sub(v[i], v[(i+lv-1)%lv]);
				V2 d2 = sub(v[(i+1)%lv], v[i]);
				V2 n1 = orth(normalize(d1));
				V2 n2 = orth(normalize(d2));

				if (dot(n1, d2) > Sc(0))
				{
					n1 = neg(n1); n2 = neg(n2);
				}

				Sc l = 0;
				Sc dl = Sc(1)/Sc(div);
				V2 n[] = {n1, n2};
				Sc lmb[] = {0, 0};
				for (int j=0; j<div+1; j++)
				{
					lmb[0]=l; lmb[1]=Sc(1)-l;
					V2 dv = add(v[i], mul(normalize(lin_comb(n, lmb, 2)), r));
					dv = mulp(m, dv);
					l = l + dl;
					vs[vsi++] = dv(0); vs[vsi++] = dv(1);
				}
			}
		};

		int div = ctx.subdiv;
		float* vs = ctx.getBuffer(2* (count*(div+1)));

		int vsi=0;
		for (int i=0; i<count; ++i)
			Local::append_vertex(vs, vsi, m, v, count, r, (count-1-i), div);

		glColor3f(col(0), col(1), col(2)); 
		glVertexPointer( 2, GL_FLOAT, 0, vs ); 
		glDrawArrays( GL_LINE_LOOP, 0, vsi/2 );   
	}
}

namespace window2d
{

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
}


namespace window2d
{
	struct WindowData;

	class WindowClient
	{
	public:

		virtual void init(WindowData& wd);
		virtual bool shouldRender(WindowData& wd);
		virtual bool shouldWaitVSync(WindowData& wd);
		virtual void render(WindowData& wd);
	};

	struct WindowData
	{
		int width,height; 
		float scale;
		float init_scale;
		bool translating;
		float transl_ref[2];
		float transl_moving[2];
		float transl[2];
		V2 cursor;
		bool cursor_pressed;
		bool cursor_released;
		mutable DrawContext dc;
	
		WindowClient sClient;
		WindowClient* client;

		WindowData() : width(1), height(1), scale(1.0f), init_scale(1.0f), cursor_pressed(false), cursor_released(false)
		{ 
			client = &sClient;
			translating = false; transl_moving[0]=0.0f; transl_moving[1]=0.0f; transl[0]=0.0f; transl[1]=0.0f; 
		}
	};
	
	template<typename T>
	Sc from_pixels(const WindowData& wd, T v)
	{
		return Sc(v)/(Sc(wd.width)*wd.scale);
	}
	

	
	void WindowClient::init(WindowData& wd) { }
	bool WindowClient::shouldWaitVSync(WindowData& wd) { return true; }
	bool WindowClient::shouldRender(WindowData& wd) { return true; }
	void WindowClient::render(WindowData& wd) 
	{ 
		draw_circle( wd.dc, wd.cursor, from_pixels(wd, Sc(8)*(wd.scale/wd.init_scale)), u_ijk() ); 
		draw_circle( wd.dc, wd.cursor, from_pixels(wd, 8), u_ijk() ); 
	}

	
	void window_unproject(int inX, int inY, float& x, float& y)
	{
		GLint viewport[4];
		GLdouble modelview[16];
		GLdouble projection[16];
		GLfloat winX, winY, winZ;
		GLdouble posX, posY, posZ;

		glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
		glGetDoublev( GL_PROJECTION_MATRIX, projection );
		glGetIntegerv( GL_VIEWPORT, viewport );

		winX = (float)inX;
		winY = (float)viewport[3] - (float)inY;
		glReadPixels( inX, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );

		gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);

		x = (float)posX;
		y = (float)posY;
	}

	void display(GLFWwindow* window)
	{
		WindowData& wd = *(WindowData*) glfwGetWindowUserPointer(window);
	
		if (!wd.client->shouldRender(wd)) return;
		if (wd.client->shouldWaitVSync(wd)) 
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
		
		glfwGetWindowSize(window, &wd.width, &wd.height);
		glViewport( 0, 0, wd.width, wd.height );

		float aspect = (float)wd.width/(float)wd.height;
		if (0)
		{
			glMatrixMode( GL_PROJECTION ); 
			glLoadIdentity(); 
			gluPerspective(45.0f, (GLfloat)wd.width/(GLfloat)wd.height, 1.0f, 100.0f );
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
			glTranslatef(aspect*(wd.transl[0]+wd.transl_moving[0])*2.0f/(float)wd.width, (wd.transl[1]+wd.transl_moving[1])*2.0f/(float)wd.height, 0.0f);
			glScalef(wd.scale, wd.scale, 1.0f);
		}

		{
			double xd,yd;
			glfwGetCursorPos(window, &xd, &yd);
			window_unproject((int) xd, (int) yd, wd.cursor.x[0], wd.cursor.x[1]);
		}

		wd.client->render(wd);

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
			if (wd.scale + off > 1.e-3f) 
			{
				wd.scale += off;
			}
		}

		void mouseButton(GLFWwindow* window,int button, int action,int mods)
		{
			WindowData& wd = *(WindowData*) glfwGetWindowUserPointer(window);

			if (button == GLFW_MOUSE_BUTTON_1)
			{
				if (action == GLFW_PRESS)
				{
					wd.cursor_pressed = true;
					wd.cursor_released = false;
				}
				else if (action == GLFW_RELEASE)
				{
					wd.cursor_pressed = false;
					wd.cursor_released = true;
				}
			}
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

	int loop(WindowClient* client = 0)
	{
		if (!glfwInit()) return -1;

		GLFWwindow* window = glfwCreateWindow(800, 600, "p0_contacts2d", NULL, NULL);
		if (!window) { glfwTerminate(); return -1; }
		glfwMakeContextCurrent(window);

		WindowData wd;
		if (client) { wd.client = client; wd.client->init(wd); wd.init_scale = wd.scale; }
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

}

#endif