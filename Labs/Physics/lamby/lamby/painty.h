#ifndef LAMBY_PAINTY_H
#define LAMBY_PAINTY_H

#pragma comment( lib, "opengl32.lib")
#pragma comment( lib, "glu32.lib")
#include <GL/gl.h>

#include "stdlib.h"
#include "gaussy.h"

struct Painty
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

	Painty() : vs(0), count(0), subdiv(6), angle_subdiv(0)
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

	~Painty() 
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

void draw_circle(Painty& ctx, V2 v, Rl r, V3p col)
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

void draw_convex(Painty& ctx, M3p m, const V2* v, int count, V3p col)
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

void draw_convex(Painty& ctx, M3p m, const V2* v, int count, Rl r, V3p col)
{
	if (r == Rl(0))
		draw_convex(ctx, m, v, count, col);

	if (count == 1)
	{
		draw_circle(ctx, mulv(m, v[0]), r, col);
		return;
	}

	struct Local
	{
		static void append_vertex(float* vs, int& vsi, M3p m, const V2* v, int lv, Rl r, int i, int div)
		{
			V2 d1 = sub(v[i], v[(i+lv-1)%lv]);
			V2 d2 = sub(v[(i+1)%lv], v[i]);
			V2 n1 = orth(normalize(d1));
			V2 n2 = orth(normalize(d2));
						
			if (dot(n1, d2) > Rl(0))
			{
				n1 = neg(n1); n2 = neg(n2);
			}

			Rl l = 0;
			Rl dl = Rl(1)/Rl(div);
			V2 n[] = {n1, n2};
			Rl lmb[] = {0, 0};
			for (int j=0; j<div+1; j++)
			{
				lmb[0]=l; lmb[1]=Rl(1)-l;
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

#endif // LAMBY_PAINTY_H