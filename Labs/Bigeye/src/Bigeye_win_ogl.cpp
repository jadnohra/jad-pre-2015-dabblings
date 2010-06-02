#ifdef WIN32
#include "../include/Bigeye/Bigeye_win_ogl.h"
#include "ARB_Multisample.h"
#include "ShapeUtil.h"
#include "MagickWand.h"

namespace BE
{


WideString::WideString()
:	mBuffer(NULL)
,	mSize(0)
{
}

WideString::WideString(const char* inString)
:	mBuffer(NULL)
,	mSize(0)
{
	Set(inString);
}


void WideString::Set(const char* inString)
{
	int required_size = MultiByteToWideChar(CP_ACP, 0, inString, -1, NULL, 0);

	if (required_size == 0)
		return;

	++required_size;

	if (required_size > mSize)
	{
		mSize = required_size;
		mBuffer = (LPWSTR) (std::realloc(mBuffer, mSize * sizeof(WCHAR)));
	}

	if (!MultiByteToWideChar(CP_ACP, 0, inString, -1, mBuffer, mSize))
	{
		if (mSize > 0)
			mBuffer[0] = 0;
	}
}

ChildWidgetContainer::ChildWidgetContainer(bool inDeleteWidgets)
:	mDeleteWidgets(inDeleteWidgets)
{
}


ChildWidgetContainer::~ChildWidgetContainer()
{
	if (mDeleteWidgets)
		Delete();
}


void ChildWidgetContainer::SetDelectWidgets(bool inDeleteWidgets)
{
	mDeleteWidgets = inDeleteWidgets;
}


void ChildWidgetContainer::Delete()
{
	for (size_t i=0; i<mChildWidgets.size(); ++i)
	{
		delete mChildWidgets[i];
		mChildWidgets[i] = NULL;
	}
}


void ChildWidgetContainer::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty)
{
	SceneTransform parent_world_tfm = inParentTransform * inParentLocalTransform;

	for (size_t i=0; i<mChildWidgets.size(); ++i)
		mChildWidgets[i]->Update(inApp, inTimeSecs, parent_world_tfm, inParentTransformDirty);
}


void ChildWidgetContainer::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty)
{
	SceneTransform parent_world_tfm = inParentTransform * inParentLocalTransform;

	for (size_t i=0; i<mChildWidgets.size(); ++i)
		mChildWidgets[i]->Render(inApp, inTimeSecs, parent_world_tfm, inParentTransformDirty);
}


bool SimpleSliderWidget::Create(const glm::vec2& inPos, const glm::vec2& inSize)
{
	mPos = to3d_point(inPos);
	mSize = inSize;

	mSliderPos = 0.2f;

	return true;
}


void SimpleSliderWidget::UpdateGeometry(const glm::vec2& inWorldPos)
{
	{
		//static const glm::vec4 color(236.0f/255.0f, 115.0f/255.0f, 0.0f, 1.0f);
		static const glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
		static const glm::vec4 colors[4] = { color, color, color, color};
		
		float rect_height = 4.0f;
		rect_height = 80.0f;
		
		glm::vec2 start_pos = inWorldPos;
		vert2d(start_pos) += up2d(0.5f * rect_height);

		mRangeRect.SetPosSize(start_pos, glm::vec2(mSize.x, rect_height), 20.0f, 5, colors);
	}

	{
		//static const glm::vec4 color(236.0f/255.0f, 115.0f/255.0f, 0.0f, 1.0f);
		static const glm::vec4 color(0.0f, 200.0f/255.0f, 240.0f/255.0f, 1.0f);
		static const glm::vec4 colors[4] = { color, color, color, color};
		
		glm::vec2 start_pos = inWorldPos;
		glm::vec2 end_pos = start_pos + mSize;
		glm::vec2 slider_pos = start_pos * (1.0f-mSliderPos) + end_pos * mSliderPos;
		
		float slider_height = 8.0f;
		
		vert2d(slider_pos) += up2d(0.5f * slider_height);
		
		mSliderRect.SetPosSize(slider_pos, glm::vec2(20.0f, slider_height), 2.0f, 1, colors);
	}

	{
		mRectangleOutline.SetToShape(mRangeRect.GetPositions(), 1.0f, 1.0f);
	}
}


void SimpleSliderWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (inParentTransformDirty || !mSliderRect.IsSet())
	{
		glm::vec3 world_pos = inParentTransform * mPos;
		UpdateGeometry(to2d_point(world_pos));
	}
}


void SimpleSliderWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec2 start_pos = to2d_point(inParentTransform * mPos);
	glm::vec2 end_pos = start_pos + mSize;

	int point_count = ((int) (glm::length(mSize) * 0.05f)) + 2;
	float inc = 1.0f / (float) point_count;


	inApp.GetOGLStateManager().Enable(EOGLState_NormalWidget);
	{
		//mRangeRect.RenderGL();

		static const glm::vec4 outline_color(0.0f, 0.0f, 0.0f, 1.0f);
		mRectangleOutline.RenderGL(outline_color);

		/*

		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glLineWidth(1.0f);
		
		glBegin(GL_LINE_STRIP);

		float factor = 0.0f;
		for (int i=0; i<point_count; ++i)
		{
			glm::vec2 point = start_pos * (1.0f-factor) + end_pos * factor;
			
			glVertex2f(horiz2d(point), vert2d(point));

			factor += inc;
		}

		glEnd();
		*/
	}

	{
		mSliderRect.RenderGL();

		static const glm::vec4 outline_color(0.0f, 0.0f, 0.0f, 1.0f);
		glLineWidth(1.5f);
		mSliderRect.RenderOutlineGL(outline_color);
	}
}


bool SimpleTextWidget::Create(const glm::vec2& inPos, const char* inText)
{
	mPos = to3d_point(inPos);
	mText = inText;

	return true;
}


void SimpleTextWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = inParentTransform * mPos;

	{
		const OGLState_FontRender* font_render = (const OGLState_FontRender*) inApp.GetOGLStateManager().Enable(EOGLState_FontRender);
		//glColor4f(1.0f, 1.0f, 1.0f,1.0f);
		font_render->Render(mText.c_str(), world_pos.x, world_pos.y);
	}
}


SimpleTextureWidget::SimpleTextureWidget()
:	mTexture(-1)
{
}

SimpleTextureWidget::~SimpleTextureWidget()
{
	//TODO free texture
}

bool SimpleTextureWidget::Create(const glm::vec2& inPos, const char* inTexturePath)
{
	mPos = to3d_point(inPos);
	mTexture = 0;

	GLsizei dims[2];

	if (!MagicWand::ReadImageToGLTexture(inTexturePath, mTexture, dims[0], dims[1]))
		return false;

	mSize.x = (float) dims[0];
	mSize.y = (float) dims[1];

	//glGenTextures(1, &mTexture);
	//glBindTexture(GL_TEXTURE_2D, mTexture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, 512,512, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);

	return true;
}


void SimpleTextureWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = inParentTransform * mPos;

	inApp.GetOGLStateManager().Enable(EOGLState_TextureWidget);
	{
		glBindTexture(GL_TEXTURE_2D, mTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex2f(world_pos.x,world_pos.y);
			 glTexCoord2f(1.0f,0.0f); glVertex2f(world_pos.x+mSize.x,world_pos.y);
			 glTexCoord2f(1.0f,1.0f); glVertex2f(world_pos.x+mSize.x,world_pos.y+mSize.y);
			 glTexCoord2f(0.0f,1.0f); glVertex2f(world_pos.x,world_pos.y+mSize.y);
		glEnd();
	}
}


SimplePanelWidget::SimplePanelWidget()
:	mChildren(true)
{
}


bool SimplePanelWidget::Create(const glm::vec2& inPos, const glm::vec2& inSize)
{
	mPos = to3d_point(inPos);
	mSize = inSize;

	return true;
}


void SimplePanelWidget::UpdateGeometry(const glm::vec2& inWorldPos)
{
	// TODO try colors like in : http://www.gameanim.com/2009/08/27/street-fighter-iv-facial-controls/
	// also see the rounded rects when using right panel + shadow placement.
	{
		//static float shades[4] = {84.0f/255.0f, 82.0f/255.0f, 37.0f/255.0f, 34.0f/255.0f};
		//static const glm::vec4 color(0.0f, 200.0f/255.0f, 240.0f/255.0f, 1.0f);
		static const glm::vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
		static const glm::vec4 colors[4] = { color, color, color, color};
		mRectangle.SetPosSize(inWorldPos, mSize, 8.0f, 5, colors, 0.1f, true);
	}

	/*
	{
		glm::vec2 shadow_pos = inWorldPos;
		shadow_pos.x += -3.0f;
		shadow_pos.y += 3.0f;

		glm::vec2 shadow_size = mSize;
		shadow_size.x += 2.0f;

		//static glm::vec4 shades[4] = { glm::vec4(0.85f), glm::vec4(1.0f), glm::vec4(0.85f), glm::vec4(0.8f)};
		//shadow_size.x += 3.0f;
		static glm::vec4 shades[4] = { glm::vec4(0.9f), glm::vec4(1.0f), glm::vec4(0.8f), glm::vec4(0.85f)};
		mShadowRectangle.SetPosSize(shadow_pos, shadow_size, 10.0f, 5, shades);
	}
	*/
}


void SimplePanelWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (inParentTransformDirty || !mRectangle.IsSet())
	{
		glm::vec3 world_pos = inParentTransform * mPos;
		UpdateGeometry(to2d_point(world_pos));
	}

	SceneTransform local_transform;
	local_transform[2] = mPos;

	mChildren.Update(inApp, inTimeSecs, inParentTransform, local_transform, inParentTransformDirty || false);
}


void SimplePanelWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = inParentTransform * mPos;

	/*
	{
		glm::vec2 shadow_pos = todim<glm::vec3, glm::vec2>::point(world_pos);
		shadow_pos.x += -4.0f;
		shadow_pos.y += 5.0f;

		glm::vec2 shadow_size = mSize;
		shadow_size.x += 2.0f;
		
		inApp.GetOGLStateManager().Enable(EOGLState_ShadowWidget);
		mShadowRectangle.RenderGL();

		//glBegin(GL_QUADS);
		//
		//glColor4f(0.85f, 0.85f, 0.85f, 1.0f);
		//glVertex2f(shadow_pos.x, shadow_pos.y);
		//glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		//glVertex2f(shadow_pos.x+shadow_size.x, shadow_pos.y);
		//glColor4f(0.85f, 0.85f, 0.85f, 1.0f);
		//glVertex2f(shadow_pos.x+shadow_size.x, shadow_pos.y+shadow_size.y);
		//glColor4f(0.7f, 0.7f, 0.7f, 1.0f);
		//glVertex2f(shadow_pos.x, shadow_pos.y+shadow_size.y);

		//glEnd();
	}
	*/

	{
		//inApp.GetOGLStateManager().Enable(EOGLState_NormalWidget);
		const glm::vec4 outline_color(0.0f, 0.0f, 0.0f, 1.0f );
		mRectangle.RenderGL(inApp.GetOGLStateManager(), EOGLState_ShadowWidget, EOGLState_NormalWidget);
	}

	/*
	{
		const OGLState_FontRender* font_render = (const OGLState_FontRender*) inApp.GetOGLStateManager().Enable(EOGLState_FontRender);
		glColor4f(0.0f,0.0f,0.0f,1.0f);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);
		font_render->Render("ABCD", world_pos.x, world_pos.y);
	}
	*/

	{
		SceneTransform local_transform;
		local_transform[2] = mPos;
		mChildren.Render(inApp, inTimeSecs, inParentTransform, local_transform, inParentTransformDirty || false);
	}
}


void OGLState_NormalWidget::Set()
{
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}


void OGLState_ShadowWidget::Set()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_DST_COLOR,GL_ZERO);
	glDisable(GL_TEXTURE_2D);
}


void OGLState_TextureWidget::Set()
{
	//glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}


NativeWindowWidget::NativeWindowWidget()
:	mHWND(NULL)
,	mHDC(NULL)
,	mHRC(NULL)
,	mIsWNDCLASSRegistered(false)
,	mChildren(true)
{
}


NativeWindowWidget::~NativeWindowWidget()
{
	Destroy();
}


void NativeWindowWidget::Destroy()
{
	if (mHWND != 0)
	{	
		if (mHDC != 0)
		{
			wglMakeCurrent (mHDC, 0);
			if (mHRC != 0)			
			{
				wglDeleteContext (mHRC);
				mHRC = 0;			

			}
			ReleaseDC (mHWND, mHDC);
			mHDC = 0;					
		}
		DestroyWindow (mHWND);			
		mHWND = 0;						
	}
}


LRESULT CALLBACK NativeWindowWidget::WindowProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Get The Window Context
	NativeWindowWidget* pTHIS = (NativeWindowWidget*) (GetWindowLong(hWnd, GWL_USERDATA));

	return pTHIS->WindowProc(hWnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK NativeWindowWidget::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)	
	{
		case WM_CLOSE:
		{
			PostMessage (hWnd, WM_QUIT, 0, 0);
			Destroy();
		}
		break;

		/*
		case WM_SIZE:												
			switch (wParam)												
			{
				//case SIZE_MINIMIZED:								
				//	window->isVisible = FALSE;						
				//return 0;											

				case SIZE_MAXIMIZED:									
					//window->isVisible = TRUE;							
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		
				return 0;												

				case SIZE_RESTORED:										
					//window->isVisible = TRUE;							
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		
				return 0;												
			}
		break;			
		*/
	}
		

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


bool NativeWindowWidget::Create(App& inApp, const WideString& inWindowName, int inWidth, int inHeight)
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;					

	int bits_per_pixel = 32;

	PIXELFORMATDESCRIPTOR pfd =											// pfd Tells Windows How We Want Things To Be
	{
		sizeof (PIXELFORMATDESCRIPTOR),									// Size Of This Pixel Format Descriptor
		1,																// Version Number
		PFD_DRAW_TO_WINDOW |											// Format Must Support Window
		PFD_SUPPORT_OPENGL |											// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,												// Must Support Double Buffering
		PFD_TYPE_RGBA,													// Request An RGBA Format
		bits_per_pixel,													// Select Our Color Depth
		0, 0, 0, 0, 0, 0,												// Color Bits Ignored
		1,																// Alpha Buffer
		0,																// Shift Bit Ignored
		0,																// No Accumulation Buffer
		0, 0, 0, 0,														// Accumulation Bits Ignored
		16,																// 16Bit Z-Buffer (Depth Buffer)  
		0,																// No Stencil Buffer
		0,																// No Auxiliary Buffer
		PFD_MAIN_PLANE,													// Main Drawing Layer
		0,																// Reserved
		0, 0, 0															// Layer Masks Ignored
	};

	RECT windowRect = {0, 0, inWidth, inHeight};	

	//GLuint PixelFormat;											

	// Adjust Window, Account For Window Borders
	AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	
	// Register A Window Class
	if (!mIsWNDCLASSRegistered)
	{
		WNDCLASSEX windowClass;											// Window Class
		ZeroMemory (&windowClass, sizeof (WNDCLASSEXA));					// Make Sure Memory Is Cleared
		windowClass.cbSize			= sizeof (WNDCLASSEXA);					// Size Of The windowClass Structure
		windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
		windowClass.lpfnWndProc		= NativeWindowWidget::WindowProcProxy;	// WindowProc Handles Messages
		windowClass.hInstance		= inApp.GetHINSTANCE();					// Set The Instance
		windowClass.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);			// Class Background Brush Color
		windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
		windowClass.lpszClassName	= inWindowName;							// Sets The Applications Classname
		
		if (RegisterClassEx (&windowClass) == 0)							
		{
			return false;													
		}

		mIsWNDCLASSRegistered = true;
	}
	
	
	mHWND = CreateWindowEx (windowExtendedStyle,					// Extended Style
								   inWindowName,					// Class Name
								   inWindowName,					// Window Title
								   windowStyle,							// Window Style
								   0, 0,								// Window X,Y Position
								   windowRect.right - windowRect.left,	// Window Width
								   windowRect.bottom - windowRect.top,	// Window Height
								   HWND_DESKTOP,						// Desktop Is Window's Parent
								   0,									// No Menu
								   inApp.GetHINSTANCE(), // Pass The Window Instance
								   this);

	if (mHWND == 0)
		return false;
	
	mHDC = GetDC (mHWND);									
	if (mHDC == 0)												
	{
		Destroy();
		return false;													
	}

	GLuint PixelFormat;	

	if(!arbMultisampleSupported)
	{
		PixelFormat = ChoosePixelFormat (mHDC, &pfd);				// Find A Compatible Pixel Format
		if (PixelFormat == 0)										
		{
			Destroy();
			return false;		
		}

	}
	else
	{
		PixelFormat = arbMultisampleFormat;
	}

	if (SetPixelFormat (mHDC, PixelFormat, &pfd) == FALSE)		
	{
		Destroy();
		return false;		
	}

	mHRC = wglCreateContext (mHDC);						
	if (mHRC == 0)												
	{
		Destroy();
		return false;										
	}

	if (wglMakeCurrent (mHDC, mHRC) == FALSE)
	{
		Destroy();
		return false;	
	}
	
	if(!arbMultisampleSupported && CHECK_FOR_MULTISAMPLE)
	{
		if(InitMultisample(inApp.GetHINSTANCE(), mHWND, pfd))
		{
			
			Destroy();
			return Create(inApp, inWindowName, inWidth, inHeight);
		}
	}
		

	ShowWindow (mHWND, SW_NORMAL);	

	//ReshapeGL (window->init.width, window->init.height);				// Reshape Our GL Window

	{
		mDefaultFont.Create("media/DroidSans.ttf", 16.0f);

		inApp.GetOGLStateManager().StartBuild(EOGLState_Count);
		inApp.GetOGLStateManager().BuildSetState(new OGLState(), EOGLState_Reset);
		inApp.GetOGLStateManager().BuildSetState(new OGLState_NativeWindowWidget(*this), EOGLState_NativeWindowWidget);
		inApp.GetOGLStateManager().BuildSetState(new OGLState_NormalWidget(), EOGLState_NormalWidget, EOGLState_NativeWindowWidget);
		inApp.GetOGLStateManager().BuildSetState(new OGLState_ShadowWidget(), EOGLState_ShadowWidget, EOGLState_NativeWindowWidget);
		inApp.GetOGLStateManager().BuildSetState(new OGLState_FontRender(mDefaultFont), EOGLState_FontRender, EOGLState_NativeWindowWidget);
		inApp.GetOGLStateManager().BuildSetState(new OGLState_TextureWidget(), EOGLState_TextureWidget, EOGLState_NativeWindowWidget);
		inApp.GetOGLStateManager().EndBuild();
	}

	Test(inApp);
	
	return true;
}


void NativeWindowWidget::Test(App& inApp)
{
	{
		SimpleTextureWidget* widget = new SimpleTextureWidget();
		//widget->Create(glm::vec2(200.0f, 100.0f), "media/tiny_test.bmp");
		widget->Create(glm::vec2(200.0f, 100.0f), "media/imagick_button.png");
		mChildren.mChildWidgets.push_back(widget);
	}

	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(glm::vec2(10.0f, 100.0f), glm::vec2(50.0f, 200.0f));
		mChildren.mChildWidgets.push_back(widget);
	}

	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(glm::vec2(130.0f, 240.0f), glm::vec2(200.0f, 200.0f));
		mChildren.mChildWidgets.push_back(widget);

		ChildWidgetContainer& children = widget->GetChildren();

		float pos_vert = 2.0f;
		pos_vert += mDefaultFont.GetPixelHeight();

		/*
		{
			SimpleTextWidget* text_widget = new SimpleTextWidget();
			text_widget->Create(glm::vec2(2.0f, pos_vert), "Hey you");
			pos_vert += mDefaultFont.GetPixelHeight();

			children.mChildWidgets.push_back(text_widget);
		}

		{
			SimpleTextWidget* text_widget = new SimpleTextWidget();
			text_widget->Create(glm::vec2(2.0f, pos_vert), "Hey you ...");
			pos_vert += mDefaultFont.GetPixelHeight();

			children.mChildWidgets.push_back(text_widget);
		}
		*/


		{
			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(glm::vec2(2.0f, pos_vert), glm::vec2(150.0f, 0.0f));
			pos_vert += mDefaultFont.GetPixelHeight();

			children.mChildWidgets.push_back(slider_widget);
		}


		
	}
	
	/*
	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(glm::vec2(10.0f, 100.0f), glm::vec2(500.0f, 600.0f));
		mChildren.mChildWidgets.push_back(widget);
	}
	*/
}


OGLState_NativeWindowWidget::OGLState_NativeWindowWidget(NativeWindowWidget& inParent)
:	mParent(inParent)
{
}

void OGLState_NativeWindowWidget::Set()
{
	RECT rect;
	GetClientRect(mParent.GetHWND(), &rect);							

	int window_width=rect.right-rect.left;							
	int window_height=rect.bottom-rect.top;		

	glViewport(0, 0, (GLsizei)(window_width), (GLsizei)(window_height));
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0.0f, window_width, window_height, 0.0f, -1.0f, 1.0f);
	glOrtho(0.0f, window_width, window_height, 0.0f, -1.0f, 1.0f);


	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	// For 2D pixel precise mode
	//glTranslatef (0.375f, 0.375f, 0.0f);
	//glTranslatef (0.5f, 0.5f, 0.0f);

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}


void NativeWindowWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) 
{
	mChildren.Update(inApp, inTimeSecs, inParentTransform, kIdentitySceneTransform, inParentTransformDirty || false);
}


void NativeWindowWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) 
{
	inApp.GetOGLStateManager().Enable(EOGLState_NativeWindowWidget);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f);
	glClearColor(176.0f/255.0f, 176.0f/255.0f, 176.0f/255.0f, 1.0f);
	//glClearColor(49.0f/255.0f, 140.0f/255.0f, 231.0f / 255.0f, 1.0f);
	
	//glClearColor(100.0f/255.0f, 149.0f/255.0f, 237.0f / 255.0f, 1.0f);
	//glClearColor(75.0f/255.0f, 146.0f/255.0f, 219.0f / 255.0f, 1.0f);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	mChildren.Render(inApp, inTimeSecs, inParentTransform, kIdentitySceneTransform, inParentTransformDirty || false);

	{
		//const OGLState_FontRender* font_render = (const OGLState_FontRender*) inApp.GetOGLStateManager().Enable(EOGLState_FontRender);
		//font_render->Render("AbcdefGhIJK", 300.0f, 100.0f);
	}

	SwapBuffers(mHDC);
	inApp.GetOGLStateManager().Enable(EOGLState_Reset);
}




App::App()
:	mWindow(NULL)
{
}


App::~App()
{
	delete mWindow;
}


bool App::Create(const char* inWindowName, int inWidth, int inHeight)
{
	if (mWindow != NULL)
		return false;

	mHINSTANCE = GetModuleHandle(NULL);	
	mWindow = new NativeWindowWidget();

	if (!mWindow->Create(*this, WideString(inWindowName), inWidth, inHeight))
	{
		delete mWindow;
		mWindow = NULL;
		return false;
	}

	return true;
}

float App::GetInputState(int inInputID)
{
	return 0.0f;
}

int App::GetInputEventCount()
{
	return 0;
}

const InputEventInfo& App::GetInputEvent(int inIndex)
{
	static InputEventInfo info;

	return info;
}


void App::ConsumeInputEvents()
{
}

bool App::Update(float inTimeSecs)
{
	if (mWindow)
	{
		if (mWindow->GetHWND() == NULL)
			return false;

		MSG msg;

		while (PeekMessage(&msg, mWindow->GetHWND(), 0, 0, PM_REMOVE))
		{
			if (msg.message != WM_QUIT)						
			{
				DispatchMessage (&msg);					
			}
			else										
			{
				return false;
			}
		}

		SceneTransform identity;

		mWindow->Update(*this, inTimeSecs, identity, false);
		mWindow->Render(*this, inTimeSecs, identity, false);

		//else												
		{
			/*
			{
				// Process Application Loop
				tickCount = GetTickCount ();				// Get The Tick Count
				app.Update (tickCount - window.lastTickCount);	// Update The Counter
				window.lastTickCount = tickCount;			// Set Last Count To Current Count
				app.Draw();									// Draw Our Scene

				SwapBuffers (window.hDC);					// Swap Buffers (Double Buffering)
			}
			*/
		}
	}

	return true;
}

}

#endif
