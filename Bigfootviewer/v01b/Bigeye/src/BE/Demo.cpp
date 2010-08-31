#include "MainWindow.h"


class Test3DScene : public BE::SimpleRenderToTextureWidget::Scene
{
public:

	float mRenderTime;
	glm::vec2 mSize;
	BE::OGLRenderToTexture* mTexture;

	virtual void Update(const BE::WidgetContext& context, BE::SimpleRenderToTextureWidget& inParent, BE::OGLRenderToTexture& inTexture);
	virtual void Render(BE::Renderer& inRenderer);
};


void CreateWidgets(BE::MainWindow& inWindow, Test3DScene& inScene);


int main()
{
	BE::MainWindow main_window;

	if (main_window.Create("Bigeye demo", 1024, 768))
	{
		Test3DScene scene;

		CreateWidgets(main_window, scene);

		TimerMillis timer;

		while (main_window.Update(timer.GetTimeSecs()))
		{
			timer.CacheTime();
		}
	}

	return 0;
}





void CreateWidgets(BE::MainWindow& inWindow, Test3DScene& inScene)
{
	BE::NativeWindowWidget* root =  inWindow.GetRootWidget();
	BE::WidgetContext context(inWindow, 0.0f);

	using namespace BE;

#if 1
	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(context, glm::vec2(10.0f, 30.0f), glm::vec2(780.0f, 720.0f), MagicWand::FRAME_NORMAL_CUT_UPPER, SimplePanelWidget::NoOverflowSlider, false);
		root->GetChildren().mChildWidgets.push_back(widget);

		SimplePanelWidget* parent_widget = widget;
		ChildWidgetContainer& children = widget->GetChildren();

#if 1
		{
			SimpleRenderToTextureWidget* widget = new SimpleRenderToTextureWidget();
			widget->Create(context, glm::vec2(0.0f, 0.0f), parent_widget->GetInternalSize());
			widget->SetScene(&inScene);
			children.mChildWidgets.push_back(widget);
		}
#endif
	}
#endif


	{
		SimpleTextWidget* text_widget = new SimpleTextWidget();
		text_widget->Create(context, glm::vec2(800.0f, 25.0f), MagicWand::TextInfo("Tools", 0, 14.0f, true, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());

		root->GetChildren().mChildWidgets.push_back(text_widget);
	}

	
	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(context, glm::vec2(800.0f, 55.0f), glm::vec2(200.0f, 250.0f), MagicWand::FRAME_NORMAL, SimplePanelWidget::AutoOverflowSlider, true);
		root->GetChildren().mChildWidgets.push_back(widget);

		ChildWidgetContainer& children = widget->GetChildren();

		MagicWand::SizeConstraints sizeConstraints;
		horiz2d(sizeConstraints.mMinSize) = 150.0f;
		horiz2d(sizeConstraints.mMaxSize) = 150.0f;

		float pos_vert = 6.0f;
		float height_offset = 6.0f;

		{
			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(context, glm::vec2(8.0f, pos_vert), glm::vec2(150.0f, 0.0f), MagicWand::TextInfo("Slide", 0, 12.0f, false, glm::vec2(2.0f, 2.0f)), sizeConstraints);
			slider_widget->SetSliderPos(0.3f);
			pos_vert += vert2d(slider_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(slider_widget);
		}

#if 1
		{
			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(context, glm::vec2(8.0f, pos_vert), glm::vec2(150.0f, 0.0f), MagicWand::TextInfo("Bold Slide", 0, 12.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);
			slider_widget->SetSliderPos(0.6f);

			pos_vert += vert2d(slider_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(slider_widget);
		}

		pos_vert += 4.0f;

		{
			MagicWand::SizeConstraints halfSizeConstraints = sizeConstraints;

			horiz2d(halfSizeConstraints.mMaxSize) = (horiz2d(halfSizeConstraints.mMaxSize) / 2.0f) - 4.0f;
			horiz2d(halfSizeConstraints.mMinSize) = horiz2d(halfSizeConstraints.mMaxSize);

			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(8.0f, pos_vert), false, MagicWand::TextInfo("Big eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), halfSizeConstraints);

			children.mChildWidgets.push_back(button_widget);

			{
				SimpleButtonWidget* button_widget = new SimpleButtonWidget();
				button_widget->Create(context, glm::vec2(horiz2d(halfSizeConstraints.mMinSize) + 6.0f + 8.0f, pos_vert), false, MagicWand::TextInfo("Bold eye ;)", 0, 12.0f, true, glm::vec2(10.0f, 2.0f)), halfSizeConstraints);

				children.mChildWidgets.push_back(button_widget);
			}

			pos_vert += vert2d(button_widget->GetSize()) + height_offset;
		}



		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(8.0f, pos_vert), false, MagicWand::TextInfo("Bigger eye ;)", 0, 16.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);

			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}


		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Toggled eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			button_widget->SetIsToggled(true);

			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Untoggled eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);

			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
#endif

#if 0
		for (int i=0; i<10; ++i)
		{
			std::string filler_name;
			strtk::construct(filler_name, "", "Filler ", i);

			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo(filler_name.c_str(), 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);

			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
#endif

#if 1
		{
			pos_vert += 100.0f;

			std::string filler_name = "Filler 66";

			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo(filler_name.c_str(), 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);

			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
#endif

	}
}



void Test3DScene::Update(const BE::WidgetContext& context, BE::SimpleRenderToTextureWidget& inParent, BE::OGLRenderToTexture& inTexture)	
{
	mSize.x = inParent.GetSize().x;
	mSize.y = inParent.GetSize().y;
	mRenderTime = context.mTimeSecs;
	mTexture = &inTexture;
}

void Test3DScene::Render(BE::Renderer& inRenderer)	
{
	mTexture->BeginRender();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glClearColor(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f);
	//glClearColor(59.0f/255.0f, 59.0f/255.0f, 59.0f/255.0f, 1.0f);
	//glClearColor(176.0f/255.0f, 176.0f/255.0f, 176.0f/255.0f, 1.0f);
	//glClearColor(49.0f/255.0f, 140.0f/255.0f, 231.0f / 255.0f, 1.0f);
	//glClearColor(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f);

	//glClearColor(100.0f/255.0f, 149.0f/255.0f, 237.0f / 255.0f, 1.0f);
	glClearColor(75.0f/255.0f, 146.0f/255.0f, 219.0f / 255.0f, 1.0f);

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f,(GLfloat)mSize.x/(GLfloat)mSize.y,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();		

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	float rtri = 16.0f * mRenderTime;
	float rquad = 128.0f * -mRenderTime;

	glLoadIdentity();									// Reset The Current Modelview Matrix
	glTranslatef(-1.5f,0.0f,-6.0f);						// Move Left 1.5 Units And Into The Screen 6.0
	glRotatef(rtri,0.0f,1.0f,0.0f);						// Rotate The Triangle On The Y axis ( NEW )
	glBegin(GL_TRIANGLES);								// Start Drawing A Triangle
	glColor3f(1.0f,0.0f,0.0f);						// Red
	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Front)
	glColor3f(0.0f,1.0f,0.0f);						// Green
	glVertex3f(-1.0f,-1.0f, 1.0f);					// Left Of Triangle (Front)
	glColor3f(0.0f,0.0f,1.0f);						// Blue
	glVertex3f( 1.0f,-1.0f, 1.0f);					// Right Of Triangle (Front)
	glColor3f(1.0f,0.0f,0.0f);						// Red
	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Right)
	glColor3f(0.0f,0.0f,1.0f);						// Blue
	glVertex3f( 1.0f,-1.0f, 1.0f);					// Left Of Triangle (Right)
	glColor3f(0.0f,1.0f,0.0f);						// Green
	glVertex3f( 1.0f,-1.0f, -1.0f);					// Right Of Triangle (Right)
	glColor3f(1.0f,0.0f,0.0f);						// Red
	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Back)
	glColor3f(0.0f,1.0f,0.0f);						// Green
	glVertex3f( 1.0f,-1.0f, -1.0f);					// Left Of Triangle (Back)
	glColor3f(0.0f,0.0f,1.0f);						// Blue
	glVertex3f(-1.0f,-1.0f, -1.0f);					// Right Of Triangle (Back)
	glColor3f(1.0f,0.0f,0.0f);						// Red
	glVertex3f( 0.0f, 1.0f, 0.0f);					// Top Of Triangle (Left)
	glColor3f(0.0f,0.0f,1.0f);						// Blue
	glVertex3f(-1.0f,-1.0f,-1.0f);					// Left Of Triangle (Left)
	glColor3f(0.0f,1.0f,0.0f);						// Green
	glVertex3f(-1.0f,-1.0f, 1.0f);					// Right Of Triangle (Left)
	glEnd();											// Done Drawing The Pyramid

	glLoadIdentity();									// Reset The Current Modelview Matrix
	glTranslatef(1.5f,0.0f,-7.0f);						// Move Right 1.5 Units And Into The Screen 7.0
	glRotatef(rquad,1.0f,1.0f,1.0f);					// Rotate The Quad On The X axis ( NEW )
	glBegin(GL_QUADS);									// Draw A Quad
	glColor3f(0.0f,1.0f,0.0f);						// Set The Color To Green
	glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Right Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Left Of The Quad (Top)
	glVertex3f(-1.0f, 1.0f, 1.0f);					// Bottom Left Of The Quad (Top)
	glVertex3f( 1.0f, 1.0f, 1.0f);					// Bottom Right Of The Quad (Top)
	glColor3f(1.0f,0.5f,0.0f);						// Set The Color To Orange
	glVertex3f( 1.0f,-1.0f, 1.0f);					// Top Right Of The Quad (Bottom)
	glVertex3f(-1.0f,-1.0f, 1.0f);					// Top Left Of The Quad (Bottom)
	glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Left Of The Quad (Bottom)
	glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom Right Of The Quad (Bottom)
	glColor3f(1.0f,0.0f,0.0f);						// Set The Color To Red
	glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Front)
	glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Front)
	glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Left Of The Quad (Front)
	glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Right Of The Quad (Front)
	glColor3f(1.0f,1.0f,0.0f);						// Set The Color To Yellow
	glVertex3f( 1.0f,-1.0f,-1.0f);					// Top Right Of The Quad (Back)
	glVertex3f(-1.0f,-1.0f,-1.0f);					// Top Left Of The Quad (Back)
	glVertex3f(-1.0f, 1.0f,-1.0f);					// Bottom Left Of The Quad (Back)
	glVertex3f( 1.0f, 1.0f,-1.0f);					// Bottom Right Of The Quad (Back)
	glColor3f(0.0f,0.0f,1.0f);						// Set The Color To Blue
	glVertex3f(-1.0f, 1.0f, 1.0f);					// Top Right Of The Quad (Left)
	glVertex3f(-1.0f, 1.0f,-1.0f);					// Top Left Of The Quad (Left)
	glVertex3f(-1.0f,-1.0f,-1.0f);					// Bottom Left Of The Quad (Left)
	glVertex3f(-1.0f,-1.0f, 1.0f);					// Bottom Right Of The Quad (Left)
	glColor3f(1.0f,0.0f,1.0f);						// Set The Color To Violet
	glVertex3f( 1.0f, 1.0f,-1.0f);					// Top Right Of The Quad (Right)
	glVertex3f( 1.0f, 1.0f, 1.0f);					// Top Left Of The Quad (Right)
	glVertex3f( 1.0f,-1.0f, 1.0f);					// Bottom Left Of The Quad (Right)
	glVertex3f( 1.0f,-1.0f,-1.0f);					// Bottom Right Of The Quad (Right)
	glEnd();											// Done Drawing The Quad


	mTexture->EndRender();
}
