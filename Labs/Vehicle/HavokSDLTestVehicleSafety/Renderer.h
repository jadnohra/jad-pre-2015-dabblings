#ifndef RENDERER_H
#define RENDERER_H

#include <algorithm>
#include <vector>
#include "SDL.h"
#include "SDL_opengl.h" 
#include "MathInc.h"

struct Color
{
	union
	{
		struct 
		{
			float v[4];
		};

		struct 
		{
			float r;
			float g;
			float b;
			float a;
		};
	};

	Color(float r_ = 1.0f, float g_ = 1.0f, float b_ = 1.0f, float a_ = 1.0f)
	:	r(r_), g(g_), b(b_), a(a_)
	{
	}

	Color(int r_, int g_, int b_, int a_ = 255)
		:	r((float)r_ / 255.0f), g((float)g_ / 255.0f), b((float)b_ / 255.0f), a((float)a_ / 255.0f)
	{
	}

	static const Color kBlack;
	static const Color kWhite;
	static const Color kRed;
	static const Color kBlue;
	static const Color kGreen;
};


class Renderer
{
public:

	int mScreenWidth;
	int mScreenHeight;
	float mGlobalAlphaMul;
	float mWorldScale;
	Vector2D mWorldTranslation;
	bool mIsRendering;

	struct RenderCircle
	{
		Vector2D v;
		float radius;
		Color color;
		float alpha; 
		bool thin;

		RenderCircle()
		{
		}

		RenderCircle(const Vector2D& v_, float radius_, const Color& color_, float alpha_ = -1.0f, bool thin_ = false)
		:	v(v_)
		,	radius(radius_)
		,	color(color_)
		,	alpha(alpha_)
		,	thin(thin_)
		{
		}
	};

	struct RenderLine
	{
		Vector2D p1;
		Vector2D p2;
		Color color;
		float alpha;
		float width;

		RenderLine()
		{
		}

		RenderLine(const Vector2D& p1_, const Vector2D& p2_, const Color& color_, float alpha_ = -1.0f, float width_ = 1.0f)
			:	p1(p1_)
			,	p2(p2_)
			,	color(color_)
			,	alpha(alpha_)
			,	width(width_)
		{
		}
		
	};

	
	typedef std::vector<RenderCircle> RenderCircles;
	typedef std::vector<RenderLine> RenderLines;


	RenderCircles mRenderCircles;
	RenderLines mRenderLines;

	Renderer()
	{
		mScreenWidth = 1024;
		mScreenHeight = 768;

		mGlobalAlphaMul = 0.75f;
		mWorldScale = 5.0f;
		mIsRendering = false;
	}

	void BeginRender() 
	{ 
		mIsRendering = true; 
	}

	void EndRender() 
	{ 
		for (size_t i = 0; i <  mRenderCircles.size(); ++i)
		{
			RenderCircle& circle = mRenderCircles[i];

			DrawCircle(circle.v, circle.radius, circle.color, circle.alpha, circle.thin);
		}
		mRenderCircles.clear();

		for (size_t i = 0; i <  mRenderLines.size(); ++i)
		{
			RenderLine& line = mRenderLines[i];

			DrawLine(line.p1, line.p2, line.color, line.alpha, line.width);
		}
		mRenderLines.clear();

		mIsRendering = false; 
	}

	int GetScreenWidth() { return mScreenWidth; }
	int GetScreenHeight() { return mScreenHeight; }

	bool InitVideo(unsigned int flags = SDL_ANYFORMAT | SDL_OPENGL | SDL_DOUBLEBUF) 
	{
		int width = mScreenWidth; 
		int height = mScreenHeight;

		// Load SDL
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
			return false;
		}
		atexit(SDL_Quit); // Clean it up nicely :)

		SDL_Surface* screen = SDL_SetVideoMode(width, height, 32, flags);
		if (screen == NULL) {
			fprintf(stderr, "Unable to set video mode: %s\n", SDL_GetError());
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		
		
		SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 0); 
		// This does not seem to work, only the driver settings are usually used...
		int check2;
		int check3 = SDL_GL_GetAttribute( SDL_GL_SWAP_CONTROL, &check2 );
		
		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 4 );
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		
		glEnable(GL_POINT_SMOOTH);
		glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_LINE_SMOOTH);
		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_POLYGON_SMOOTH);
		glHint (GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable( GL_TEXTURE_2D );

		glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

		glViewport( 0, 0, width, height );

		glClear( GL_COLOR_BUFFER_BIT );

		glMatrixMode( GL_PROJECTION );
		glLoadIdentity();

		glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		// For 2D pixel precise mode
		glTranslatef (0.375f, 0.375f, 0.0f);

		return true;
	}


	void DrawCircle(const Vector2D& v, float radius, const Color& color, float alpha = -1.0f, bool thin = false)
	{
		DrawArc(v, radius, 0.0f, 2.0f * MATH_PIf, color, alpha, thin);
	}

	void DrawArc(const Vector2D& v, float radius, float from, float to, const Color& color, float alpha = -1.0f, bool thin = false)
	{
		if (!mIsRendering)
		{
			mRenderCircles.push_back(RenderCircle(v, radius, color, alpha, thin));
			return;
		}

		if (alpha < 0.0f)
			alpha = color.a;

		glColor4f(color.r, color.g, color.b, mGlobalAlphaMul * alpha);
		glLineWidth(thin ? 0.7f : std::min(3.0f, std::max(1.25f, mWorldScale / 6)));
		glBegin(GL_LINE_LOOP);

		//glBegin(GL_TRIANGLE_FAN);
		//glVertex2f(v.x, v.y);

		for (float angle = from; angle <= to; angle += (to-from) / 32.0f)
		{
			glVertex2f(v.x + sinf(angle) * radius, v.y + cosf(angle) * radius);
		}

		glEnd();
	}

	void DrawArc(const Vector2D& v, const Vector2D& from, float arc_angle, const Color& color, float alpha = -1.0f, bool thin = false)
	{
		if (!mIsRendering)
		{
			//mRenderCircles.push_back(RenderCircle(v, radius, color, alpha, thin));
			return;
		}

		if (alpha < 0.0f)
			alpha = color.a;

		glColor4f(color.r, color.g, color.b, mGlobalAlphaMul * alpha);
		glLineWidth(thin ? 0.7f : std::min(3.0f, std::max(1.25f, mWorldScale / 6)));
		glBegin(GL_LINE_STRIP);

		//glBegin(GL_TRIANGLE_FAN);
		//glVertex2f(v.x, v.y);

		int i=0;
		for (float angle = 0.0f; i<=32; ++i, angle += (arc_angle) / 32.0f)
		{
			Vector2D offset = rotate(from, angle);
			glVertex2f(v.x + offset.x, v.y + offset.y );
		}

		glEnd();
	}


	void DrawLine(const Vector2D& p1, const Vector2D& p2, const Color& color, float alpha = -1.0f, float width = 1.0f)
	{
		if (!mIsRendering)
		{
			mRenderLines.push_back(RenderLine(p1, p2, color, alpha, width));
			return;
		}

		if (alpha < 0.0f)
			alpha = color.a;

		glColor4f(color.r, color.g, color.b, mGlobalAlphaMul * alpha);
		glLineWidth(width);
		glBegin(GL_LINE_LOOP);

		glVertex2f(p1.x, p1.y);
		glVertex2f(p2.x, p2.y);

		glEnd();
	}

	void DrawQuad(const Vector2D& p1, const Vector2D& p2, 
				  const Vector2D& p3, const Vector2D& p4, 
				  const Color& color, float alpha = -1.0f, float width = 1.0f, bool cross = false)
	{
		DrawLine(p1, p2, color, alpha, width);
		DrawLine(p2, p3, color, alpha, width);
		DrawLine(p3, p4, color, alpha, width);
		DrawLine(p4, p1, color, alpha, width);

		if (cross)
		{
			DrawLine(p1, p3, color, alpha, 0.5f * width);
			DrawLine(p2, p4, color, alpha, 0.5f * width);
		}
	}

	void DrawPoly2DWorldToScreen(const Poly2D& poly,
								const Color& color, float alpha = -1.0f, float width = 1.0f)
	{
		if (poly.points.size() >= 2)
		{
			for (int i=0; i+1< poly.points.size(); ++i)		
			{
				DrawLine(WorldToScreen(poly.points[i]), WorldToScreen(poly.points[i+1]), color, alpha, width);
			}

			DrawLine(WorldToScreen(poly.points[poly.points.size()-1]),WorldToScreen(poly.points[0]), color, alpha, width);
		}
	}

	void DrawPolyPath2DWorldToScreen(const PolyPath2D& polyPath2D,
									 const Color& color, float alpha = -1.0f, float width = 1.0f)
	{
		for (int i=0; i<polyPath2D.polys.size(); ++i)
		{
			DrawPoly2DWorldToScreen(polyPath2D.polys[i], color, alpha, width);
		}
	}

	void DrawPolyPointPath2DWorldToScreen(const PolyPointPath2D& polyPointPath2D,
									 const Color& normal_color, const Color& error_color, float alpha = -1.0f, float width = 1.0f)
	{
		for (int i=0; i+1< polyPointPath2D.mPoints.size(); ++i)		
		{
			Vector2D from;
			Vector2D to;

			Color color = normal_color;

			if (i == 0 || polyPointPath2D.mPoints[i].mCurveRadius <= 0.0f)
			{
				from = WorldToScreen(polyPointPath2D.mPoints[i].mPos);
			}
			else
			{
				from = WorldToScreen(polyPointPath2D.mPoints[i].mCurveEndPoint);
			}

			if (polyPointPath2D.mPoints[i+1].mCurveRadius <= 0.0f)
			{
				if (polyPointPath2D.mPoints[i+1].mCurveRadius < 0.0f)
					color = error_color;

				to = WorldToScreen(polyPointPath2D.mPoints[i+1].mPos);
			}
			else
			{				
				to = WorldToScreen(polyPointPath2D.mPoints[i+1].mCurveStartPoint);
			}

			DrawLine(from, to, color, alpha, width);

			if (polyPointPath2D.mPoints[i].mCurveRadius > 0.0f)
			{
				const PolyPointPath2D::PathPolyPoint& pt = polyPointPath2D.mPoints[i];

				//float start_angle = SignedAngle((pt.mCurveStartPoint - pt.mCurveCenter).Normalized(), Vector2D(0.0f, -1.0f));
				//float end_angle = SignedAngle((pt.mCurveEndPoint - pt.mCurveCenter).Normalized(), Vector2D(0.0f, -1.0f));

				//start_angle = 0.0f;
				//end_angle = MATH_PIf*0.2f;
				
				//DrawCircle(WorldToScreen(pt.mCurveCenter), 
				//	WorldToScreen(pt.mCurveRadius), color, alpha, true);

				float arc_angle = SignedAngle(pt.mCurveStartPoint-pt.mCurveCenter, pt.mCurveEndPoint-pt.mCurveCenter);

				DrawArc(WorldToScreen(pt.mCurveCenter), WorldToScreenDir(pt.mCurveStartPoint-pt.mCurveCenter), arc_angle, color, alpha, true);

				//DrawLine(WorldToScreen(pt.mCurveCenter), WorldToScreen(pt.mCurveStartPoint), color, alpha, width);
				//DrawLine(h u WorldToScreen(pt.mCurveCenter), WorldToScreen(pt.mCurveEndPoint), color, alpha, width);

				//DrawLine(WorldToScreen(pt.mCurveCenter), WorldToScreen(pt.mDebugPt1), color, alpha, width);
				//DrawLine(WorldToScreen(pt.mCurveCenter), WorldToScreen(pt.mDebugPt2), color, alpha, width);
			}
		}
	}

	void DrawArrow(const Vector2D& p1, const Vector2D& p2, const Color& color, float alpha = -1.0f)
	{
		DrawLine(p1, p2, color, alpha);
		
		float lineLength = (p2 - p1).Length();
		float headSize = 0.03f * lineLength;

		Vector2D dir1 = (p2 - p1).Normalized();
		Vector2D dir2 = rotate(dir1, 0.5f * MATH_PIf);

		Vector2D offset1 = dir1 * -(2.0f * headSize) + dir2 * headSize;
		Vector2D offset2 = dir1 * -(2.0f * headSize) + dir2 * -headSize;

		DrawLine(p2, p2 + offset1, color, alpha);
		DrawLine(p2, p2 + offset2, color, alpha);
	}


	Vector2D WorldToScreen(const Vector2D& v)
	{
		return ((v + mWorldTranslation) * mWorldScale) + Vector2D(0.5f * (float) mScreenWidth, 0.5f * (float) mScreenHeight);
	}

	Vector2D WorldToScreenDir(const Vector2D& v)
	{
		return (v*mWorldScale);
	}

	float WorldToScreen(float v)
	{
		return v * mWorldScale;
	}

	Vector2D ScreenToWorld(const Vector2D& v)
	{
		return (((v) - Vector2D(0.5f * (float) mScreenWidth, 0.5f * (float) mScreenHeight)) * (1.0f / mWorldScale))  - mWorldTranslation ;
	}

	Vector2D ScreenToWorldDir(const Vector2D& v)
	{
		return (v) * (1.0f / mWorldScale);
	}

	float ScreenToWorld(float v)
	{
		return v / mWorldScale;
	}

};

#endif