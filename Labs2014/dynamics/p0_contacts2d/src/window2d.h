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

		virtual bool shouldRender(const WindowData& wd);
		virtual bool shouldWaitVSync(const WindowData& wd);
		virtual void render(const WindowData& wd);
	};

	struct WindowData
	{
		int width,height; 
		float scale;
		bool translating;
		float transl_ref[2];
		float transl_moving[2];
		float transl[2];
		V2 cursor;
		mutable DrawContext dc;
	
		WindowClient sClient;
		WindowClient* client;

		WindowData() : width(1), height(1), scale(1.0f)
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
	

	
	bool WindowClient::shouldWaitVSync(const WindowData& wd) { return true; }
	bool WindowClient::shouldRender(const WindowData& wd) { return true; }
	void WindowClient::render(const WindowData& wd) 
	{ 
		draw_circle( wd.dc, wd.cursor, from_pixels(wd, Sc(8)*wd.scale), u_ijk() ); 
		draw_circle( wd.dc, wd.cursor, from_pixels(wd, 8), u_ijk() ); 
	}

	/*
	void drawScene(WindowData& wd, Scene& scene)
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
				if (true)
				{
					RBody* b1 = w.rbodies.at(0);
					//RBody* b2 = w.rbodies.at(1);

					M3 m1 = rigid(asV2(b1->q), b1->q(2));
					//M3 m2 = rigid(asV2(b2->q), b2->q(2));
					M3 m2 = rigid(wd.cursor, Sc(0));
					//m2 = rigid(V2(h2f(0x3f333333), h2f(0xbebbbbbc)), Sc(0));
					//m2 = rigid(V2(h2f(0x40244444), h2f(0x3f955555)), Sc(0));
					//m2 = rigid(V2(h2f(0x3f3bbbbc), h2f(0x3e6eeeef)), Sc(0));

					//printf("0x%x,0x%x\n", f2h(wd.cursor(0)), f2h(wd.cursor(1)));

					const RShape& shape1 = w.rshapes[b1->shape];
					const RShape& shape2 = w.rshapes[11];
					//const RShape& shape2 = w.rshapes[b2->shape];

					//draw_circle(painty, wd.cursor, Sc(0.2), u_ijk());
					draw_convex(painty, m2, shape2.v, shape2.count, shape2.r, u_ijk());

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

				if (false && w.rbodies.size >= 2)
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
			thingies.addConvex(m, v, 4, Sc(0.1));

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
				draw_convex(painty, m, v, 4, Sc(0.2), u_ijk());
			}
		}

		if (dt)
			painty.end();

		painty.draw();
	}
	*/

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
		if (client) wd.client = client;
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