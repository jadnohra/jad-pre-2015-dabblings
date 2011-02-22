#include "BE/Bigeye.h"
#include "BE/ARB_Multisample.h"
#include "BF/Camera.h"
#include "BF/BFMath.h"
#include "BF/GridRenderer.h"
#include "BF/3rdParty/triangle/triangle.h"
#include "BF/DrawUtil.h"
#include "BF/Locomo.h"

class BigfootPlannerScene : public BE::SimpleRenderToTextureWidget::Scene, public BE::MainWindowClient
{
public:

	float mRenderTime;
	glm::vec2 mSize;
	BE::OGLRenderToTexture* mTexture;
	BF::Camera mCamera;
	BF::ViewportSetup mViewportSetup;
	BF::CameraSetup mCameraSetup;
	BF::GridRenderer mGrid;
	BF::CameraTurnTableRotationController mCameraController;
	bool mAutoSetupBasicScene;
	bool mIsValidPick;
	glm::vec3 mPick;
	
	BE::SimpleButtonWidget* mHelpButton;
	BE::SimplePanelWidget* mHelpPanel;
	BE::SimpleButtonWidget* mAutoLinkButton;
	BE::SimpleButtonWidget* mCreateNavmeshButton;
	BE::SimpleButtonWidget* mClearNavmeshButton;
	std::vector<glm::vec3> mNavmeshPoints;
	std::vector<glm::vec2> mNavmeshLinks;
	std::vector<glm::vec2> mNavmeshTrisVerts;
	std::vector<int> mNavmeshTris;
	std::vector<int> mNavmeshBorderSegs;
	int mPickNavmeshPoint;

	BigfootPlannerScene::BigfootPlannerScene() 
	:	mAutoSetupBasicScene(true)
	,	mRenderTime(-1.0f)
	,	mHelpButton(NULL)
	,	mHelpPanel(NULL)
	,	mAutoLinkButton(NULL)
	,	mCreateNavmeshButton(NULL)
	,	mClearNavmeshButton(NULL)
	,	mIsValidPick(false)
	,	mPickNavmeshPoint(-1)
	{
	}

	bool Create();

	virtual bool SupportsDragAndDrop() { return true; }
	virtual void OnFileDropped(BE::MainWindow* inWindow, const char* inFilePath);

	virtual void ProcessWidgetEvents(BE::MainWindow* inWindow, BE::WidgetEventManager& inManager);
	virtual void Update(const BE::WidgetContext& context, BE::SimpleRenderToTextureWidget& inParent, BE::OGLRenderToTexture& inTexture);
	virtual void Render(BE::Renderer& inRenderer);

	virtual void Unproject(const glm::vec2& inViewportPoint, glm::vec3& outSceneWorldPoint, glm::vec3* outSceneWorldRayDir);

	void RenderTestBasicScene(BE::Renderer& inRenderer);
	
	void OnFirstUpdate();

	void Triangulate();
};


void CreateWidgets(BE::MainWindow& inWindow, BigfootPlannerScene& inScene);


int PlannerMain()
{
	BigfootPlannerScene scene;
	BE::MainWindow main_window(&scene);
	
	if (main_window.Create("Bigfoot [Planner]", 1024, 768))
	{
		CreateWidgets(main_window, scene);

		if (scene.Create())
		{
			EnableVSync(true); // This seems not to be working
			TimerMillis timer;

			while (main_window.Update(timer.GetTimeSecs()))
			{
				timer.CacheTime();
			}
		}
	}
	
	return 0;
}


void CreateWidgets(BE::MainWindow& inWindow, BigfootPlannerScene& inScene)
{
	BE::NativeWindowWidget* root =  inWindow.GetRootWidget();
	BE::WidgetContext context(inWindow, 0.0f);

	using namespace BE;

	
	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(context, glm::vec2(10.0f, 10.0f), glm::vec2(1004.0f, 715.0f), MagicWand::FRAME_NORMAL_CUT_UPPER, SimplePanelWidget::NoOverflowSlider, false);
		root->GetChildren().mChildWidgets.push_back(widget);

		SimplePanelWidget* parent_widget = widget;
		ChildWidgetContainer& children = widget->GetChildren();

		{
			SimpleRenderToTextureWidget* widget = new SimpleRenderToTextureWidget();
			widget->Create(context, glm::vec2(0.0f, 0.0f), parent_widget->GetInternalSize(), 8);
			widget->SetScene(&inScene);
			children.mChildWidgets.push_back(widget);
		}
	}

	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(context, glm::vec2(10.0f, 730.0f), glm::vec2(1004.0f, 30.0f), MagicWand::FRAME_NORMAL, SimplePanelWidget::NoOverflowSlider, true);
		root->GetChildren().mChildWidgets.push_back(widget);

		SimplePanelWidget* parent_widget = widget;
		ChildWidgetContainer& children = widget->GetChildren();

		MagicWand::SizeConstraints sizeConstraints;
		horiz2d(sizeConstraints.mMinSize) = 25.0f;
		horiz2d(sizeConstraints.mMaxSize) = 25.0f;

		float pos_vert = 5.0f;
		float pos_horiz = 10.0f;

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(pos_horiz, pos_vert), true, MagicWand::TextInfo(" L ", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);

			pos_horiz += 30.0f;

			children.mChildWidgets.push_back(button_widget);
			inScene.mAutoLinkButton = button_widget;
			inScene.mAutoLinkButton->SetIsToggled(true);
		}


		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(pos_horiz, pos_vert), false, MagicWand::TextInfo(" T ", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);

			pos_horiz += 30.0f;

			children.mChildWidgets.push_back(button_widget);
			inScene.mCreateNavmeshButton = button_widget;
		}

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(pos_horiz, pos_vert), false, MagicWand::TextInfo(" X ", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);

			pos_horiz += 30.0f;

			children.mChildWidgets.push_back(button_widget);
			inScene.mClearNavmeshButton = button_widget;
		}


		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(975.0f, pos_vert), false, MagicWand::TextInfo(" ? ", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);

			children.mChildWidgets.push_back(button_widget);
			inScene.mHelpButton = button_widget;
		}

		{
			SimplePanelWidget* widget = new SimplePanelWidget();
			widget->Create(context, glm::vec2(680.0f, 575.0f), glm::vec2(300.0f, 165.0f), MagicWand::FRAME_NORMAL, SimplePanelWidget::NoOverflowSlider, true);
			root->GetChildren().mChildWidgets.push_back(widget);

			widget->RaiseLocalPosition(0.02f);

			inScene.mHelpPanel = widget;
			SimplePanelWidget* parent_widget = widget;
			ChildWidgetContainer& children = widget->GetChildren();

			float pos_horiz = 5.0f;
			float pos_vert = 5.0f;
			float height_offset = 2.0f;
				
			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo("BigFoot [Planner] ver. 0.1a", 0, 12.0f, true, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
					
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;

				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo("© 2011 Jad Nohra", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
					
				pos_vert += vert2d(text_widget->GetSize()) + 3.0f * height_offset;

				children.mChildWidgets.push_back(text_widget);
			}
		}
	}
}


void BigfootPlannerScene::RenderTestBasicScene(BE::Renderer& inRenderer)
{
	bool test_controller = true;

#ifdef BUILD_SUBMISSION
	test_controller = true;
#endif

	glm::mat4 view_matrix;

	float rtri = 16.0f * mRenderTime;
	float rquad = 128.0f * -mRenderTime;

	if (test_controller)
	{
		if (mAutoSetupBasicScene)
		{
			BF::CameraFollowSphereAutoSetup camera_auto_setup;
			BF::Sphere sphere; sphere.mPosition = glm::vec3(-1.5f,0.0f,-6.0f); sphere.mRadius = 5.0f;
#ifdef BUILD_SUBMISSION
			sphere.mPosition = glm::vec3(0.0f,0.0f,0.0f);
#endif
			glm::vec2 auto_depth_planes;
			camera_auto_setup.SetFollowParams(glm::vec3(1.0f, 1.0f, 1.0f));
			camera_auto_setup.SetupCamera(sphere, mCameraSetup.GetFOV(), glm::vec2(0.001f, 1000.0f), mCamera, auto_depth_planes);
			auto_depth_planes.y *= 100.0f;
			mCameraSetup.SetupDepthPlanes(auto_depth_planes);

			mAutoSetupBasicScene = false;
		}

		view_matrix = mCamera.GetViewMatrix();

		glm::mat4 triangle_world_matrix = glm::rotate(glm::translate(glm::mat4(), glm::vec3(-1.5f,0.0f,-6.0f)), rtri, 0.0f,1.0f,0.0f);
		glLoadMatrixf(glm::value_ptr(view_matrix * triangle_world_matrix));
	}
	else
	{
		glLoadIdentity();									// Reset The Current Modelview Matrix
		glTranslatef(-1.5f,0.0f,-6.0f);						// Move Left 1.5 Units And Into The Screen 6.0
		glRotatef(rtri,0.0f,1.0f,0.0f);						// Rotate The Triangle On The Y axis ( NEW )
	}
	
	return;

#ifdef BUILD_SUBMISSION
			return;
#endif

	glBegin(GL_TRIANGLES);							// Start Drawing A Triangle
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

	

	if (test_controller)
	{
		glm::mat4 quad_world_matrix = glm::rotate(glm::translate(glm::mat4(), glm::vec3(1.5f,0.0f,-7.0f)), rquad, 1.0f,1.0f,1.0f);
		glLoadMatrixf(glm::value_ptr(view_matrix * quad_world_matrix));
	}
	else
	{
		glLoadIdentity();									// Reset The Current Modelview Matrix
		glTranslatef(1.5f,0.0f,-7.0f);						// Move Right 1.5 Units And Into The Screen 7.0
		glRotatef(rquad,1.0f,1.0f,1.0f);					// Rotate The Quad On The X axis ( NEW )
	}

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
}


bool BigfootPlannerScene::Create()
{
	mCameraSetup.SetupDepthPlanes(glm::vec2(0.1f, 1000.0f));
	BF::AAB default_gid_aab;
	default_gid_aab.Include(glm::vec3(-50.0f, -50.0f, -50.0f));
	default_gid_aab.Include(glm::vec3(50.0f, 50.0f, 50.0f));
	mGrid.Setup(default_gid_aab, glm::vec2(100.0f, 100.0f));
	mCameraController.AttachCamera(mCamera);
	
	return true;
}


void BigfootPlannerScene::OnFileDropped(BE::MainWindow* inWindow, const char* inFilePath)
{
	{
		mAutoSetupBasicScene = true;

		mCameraSetup.SetupDepthPlanes(glm::vec2(0.1f, 100.0f));
		BF::AAB default_gid_aab;
		default_gid_aab.Include(glm::vec3(-50.0f, -50.0f, -50.0f));
		mGrid.Setup(default_gid_aab, glm::vec2(100.0f, 100.0f));
	}
}

void BigfootPlannerScene::Triangulate()
{
	if (mNavmeshPoints.size() < 3)
		return;

	int links_to_first_count = 0;
	for (size_t i = 0; i < mNavmeshLinks.size(); ++i)
	{
		if (mNavmeshLinks[i].x == 0 || mNavmeshLinks[i].y == 0)
		{
			++links_to_first_count;
		}
	}
	if (links_to_first_count == 1)
	{
		mNavmeshLinks.push_back(glm::vec2(0.0f, (float) (mNavmeshPoints.size() - 1)));
	}

	if (mNavmeshLinks.size() < 3)
		return;

	triangulateio in;
	triangulateio mid;
	triangulateio out;
	triangulateio vorout;

	memset(&in, 0, sizeof(triangulateio));
	memset(&out, 0, sizeof(triangulateio));
	memset(&mid, 0, sizeof(triangulateio));
	memset(&vorout, 0, sizeof(triangulateio));

	in.numberofpoints = mNavmeshPoints.size();
	in.numberofpointattributes = 0;
	in.pointlist = (REAL *) malloc(in.numberofpoints * 2 * sizeof(REAL));

	for (size_t i = 0; i < mNavmeshPoints.size(); ++i)
	{
		in.pointlist[i*2] = mNavmeshPoints[i].x;
		in.pointlist[i*2+1] = mNavmeshPoints[i].z;
	}

	in.numberofsegments = mNavmeshLinks.size();
	in.segmentlist = (int *) malloc(in.numberofsegments * 2 * sizeof(int));

	for (size_t i = 0; i < mNavmeshLinks.size(); ++i)
	{
		in.segmentlist[i*2] = 1+(int)mNavmeshLinks[i].x;
		in.segmentlist[i*2+1] = 1+(int)mNavmeshLinks[i].y;
	}

	triangulate("p", &in, &out, 0);

	mNavmeshTrisVerts.clear();
	mNavmeshTris.clear();
	

	if (out.numberoftriangles > 0)
	{
		mNavmeshPoints.clear();
		mNavmeshLinks.clear();
		mNavmeshBorderSegs.clear();

		mNavmeshTrisVerts.resize(out.numberofpoints);
		for (int i = 0; i < out.numberofpoints; ++i)
		{
			mNavmeshTrisVerts[i].x = out.pointlist[i*2];
			mNavmeshTrisVerts[i].y = out.pointlist[i*2+1];
		}

		mNavmeshTris.resize(out.numberoftriangles*3);
		for (size_t i = 0; i < out.numberoftriangles*3; ++i)
		{
			mNavmeshTris[i] = out.trianglelist[i]-1;
		}

		mNavmeshBorderSegs.resize(out.numberofsegments*2);
		for (int i = 0; i < out.numberofsegments; ++i)
		{
			mNavmeshBorderSegs[i*2] = out.segmentlist[i*2] - 1;
			mNavmeshBorderSegs[i*2+1] = out.segmentlist[i*2+1] - 1;
		}
	}
}

void BigfootPlannerScene::ProcessWidgetEvents(BE::MainWindow* inWindow, BE::WidgetEventManager& inManager)
{
	for (size_t i=0; i<inManager.GetEventCount(); ++i)
	{
		const BE::WidgetEvent& widget_event = inManager.GetEvent(i);
		if (widget_event.mWidget == mCreateNavmeshButton && mNavmeshLinks.size() >= 3)
		{
			Triangulate();	
		}
		else if (widget_event.mWidget == mClearNavmeshButton)
		{
			mNavmeshLinks.clear();
			mNavmeshPoints.clear();
			mNavmeshTrisVerts.clear();
			mNavmeshTris.clear();
			mNavmeshBorderSegs.clear();
		}
	}
}

void BigfootPlannerScene::Update(const BE::WidgetContext& context, BE::SimpleRenderToTextureWidget& inParent, BE::OGLRenderToTexture& inTexture)	
{
	mTexture = &inTexture;

	mSize.x = inParent.GetSize().x;
	mSize.y = inParent.GetSize().y;
	mViewportSetup.mWindowSize = mSize;
	mCameraSetup.SetupFOVY(mViewportSetup);

	if (mRenderTime < 0.0f)
		OnFirstUpdate();
	
	mRenderTime = context.mTimeSecs;
	
	if (mHelpButton != NULL && mHelpPanel != NULL)
		mHelpPanel->SetIsVisible(mHelpButton->IsPressed());

	mCameraController.Update(context, inParent, mViewportSetup, mCameraSetup);

	bool left_on = context.mMainWindow.GetInputState(BE::INPUT_MOUSE_LEFT) != 0.0f;
	bool right_up = (context.mMainWindow.GetInputState(BE::INPUT_MOUSE_RIGHT_CHANGED) != 0.0f) && (context.mMainWindow.GetInputState(BE::INPUT_MOUSE_RIGHT) == 0.0f);

	if (mPickNavmeshPoint+1 > mNavmeshPoints.size())
		mPickNavmeshPoint = -1;

	int link_count = mNavmeshLinks.size();

	if ((left_on || right_up))
	{
		bool valid_pick = false;
		glm::vec3 pick;

		glm::vec2 picked_2d;
		if (inParent.IsMainWindowPosInViewport(context, context.mMainWindow.GetMousePos(), picked_2d))
		{
			glm::vec3 origin;
			glm::vec3 dir;
			inParent.GetScene()->Unproject(picked_2d, origin, &dir);

			float t = -origin.y / dir.y;
			valid_pick = true;
			pick = origin + dir * t;
		}

		if (mNavmeshTris.empty())
		{
			int pick_index = -1;
			float pick_dist = -1.0f;

			if (valid_pick)
			{
				for (size_t i = 0; i < mNavmeshPoints.size(); ++i)
				{
					float dist = glm::distance(pick, mNavmeshPoints[i]);
					if (pick_dist == -1.0f || dist < pick_dist)
					{
						pick_dist = dist;
						pick_index = i;
					}
				}

				if (pick_dist != -1.0f && pick_dist <= 0.5f)
				{
					if (right_up)
					{
						if (mPickNavmeshPoint == -1)
							mPickNavmeshPoint = pick_index;
						else
						{
							if (mPickNavmeshPoint != pick_index)
							{
								bool exists = false;

								for (size_t i = 0; i < mNavmeshLinks.size(); ++i)
								{
									if ((mNavmeshLinks[i].x == mPickNavmeshPoint || mNavmeshLinks[i].y == mPickNavmeshPoint)
										&& (mNavmeshLinks[i].x == pick_index || mNavmeshLinks[i].y == pick_index))
									{
										exists = true;
										break;
									}
								}

								if (!exists)
								{
									mNavmeshLinks.push_back(glm::vec2());
									mNavmeshLinks.back().x = (int) mPickNavmeshPoint;
									mNavmeshLinks.back().y = (int) pick_index;
								}
							}
							mPickNavmeshPoint = -1;
						}
					}
				}
				else
				{
					if (left_on)
					{
						mNavmeshPoints.push_back(glm::vec3());
						mNavmeshPoints.back() = pick;
					}
				}
			}

			if (left_on)
			{
				mIsValidPick = valid_pick;
				mPick = pick;

				if (valid_pick && mAutoLinkButton->IsToggled())
				{
					int link_pick_index = -1;
					if (pick_index != -1 && pick_dist <= 0.5f)
					{
						link_pick_index = pick_index;
					}
					else
					{
						link_pick_index = ((int) mNavmeshPoints.size()) - 1;
					}

					if (mPickNavmeshPoint != -1 && link_pick_index != -1 && mPickNavmeshPoint != link_pick_index)
					{
						bool exists = false;

						for (size_t i = 0; i < mNavmeshLinks.size(); ++i)
						{
							if ((mNavmeshLinks[i].x == mPickNavmeshPoint || mNavmeshLinks[i].y == mPickNavmeshPoint)
								&& (mNavmeshLinks[i].x == link_pick_index || mNavmeshLinks[i].y == link_pick_index))
							{
								exists = true;
								break;
							}
						}

						if (!exists)
						{
							mNavmeshLinks.push_back(glm::vec2());
							mNavmeshLinks.back().x = (int) mPickNavmeshPoint;
							mNavmeshLinks.back().y = (int) link_pick_index;
						}
					}

					mPickNavmeshPoint = link_pick_index;
				}
			}
		}
		else
		{
			if (left_on)
			{
				mIsValidPick = valid_pick;
				mPick = pick;
			}
		}
	}

	if (mNavmeshLinks.size() > link_count)
	{
		if ((mNavmeshLinks.back().x == 0 || mNavmeshLinks.back().y == 0)
			&& (mNavmeshLinks.back().x == mNavmeshPoints.size()-1 || mNavmeshLinks.back().y == mNavmeshPoints.size()-1))
		{
			Triangulate();
		}
	}
}


void BigfootPlannerScene::Unproject(const glm::vec2& inViewportPoint, glm::vec3& outSceneWorldPoint, glm::vec3* outSceneWorldRayDir)
{
	GLint viewport[4] = { 0, 0, (int) mViewportSetup.mWindowSize.x, (int) mViewportSetup.mWindowSize.y };
	GLdouble modelview[16];
	const GLdouble* projection = mCameraSetup.GetGlPorjectionMatrix();
	GLdouble posX, posY, posZ;

	const glm::mat4& view_matrix = mCamera.GetViewMatrix();

	for (int i=0; i<16; ++i)
		modelview[i] = (GLdouble) (glm::value_ptr(view_matrix))[i];
	
	gluUnProject(inViewportPoint.x, mViewportSetup.mWindowSize.y - inViewportPoint.y, 0.0f, modelview, projection, viewport, &posX, &posY, &posZ);

	outSceneWorldPoint = glm::vec3(posX, posY, posZ);

	if (outSceneWorldRayDir)
	{
		glm::vec3 dir = outSceneWorldPoint - glm::vec3(mCamera.GetWorldMatrix()[3]);
		*outSceneWorldRayDir = glm::normalize(dir);
	}
}


void BigfootPlannerScene::OnFirstUpdate()
{
	//OnFileDropped(NULL, "../media/test.bvh");
}


void BigfootPlannerScene::Render(BE::Renderer& inRenderer)	
{
	if (mAutoSetupBasicScene)
	{
		BF::CameraFollowSphereAutoSetup camera_auto_setup;
		BF::Sphere sphere; sphere.mPosition = glm::vec3(-1.5f,0.0f,-6.0f); sphere.mRadius = 5.0f;
#ifdef BUILD_SUBMISSION
		sphere.mPosition = glm::vec3(0.0f,0.0f,0.0f);
#endif
		glm::vec2 auto_depth_planes;
		camera_auto_setup.SetFollowParams(glm::vec3(1.0f, 1.0f, 1.0f));
		camera_auto_setup.SetupCamera(sphere, mCameraSetup.GetFOV(), glm::vec2(0.001f, 1000.0f), mCamera, auto_depth_planes);
		auto_depth_planes.y *= 100.0f;
		mCameraSetup.SetupDepthPlanes(auto_depth_planes);

		mAutoSetupBasicScene = false;
	}

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
	glClearColor(115.0f/255.0f, 115.0f/255.0f, 115.0f/255.0f, 1.0f); // Blender

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mCameraSetup.SetGlProjectionMatrix();

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();		

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glEnable( GL_LINE_SMOOTH );
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glEnable( GL_POINT_SMOOTH );
    glHint(GL_POINT_SMOOTH, GL_NICEST);

	glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	float fogCol[4] = { 0.32f,0.25f,0.25f,1 };
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, 450.f*0.2f);
	glFogf(GL_FOG_END, 450.f*1.25f);
	glFogfv(GL_FOG_COLOR, fogCol);

	/*
	{
		glLoadMatrixf(glm::value_ptr(mCamera.GetViewMatrix()));
		mGrid.Render();
	}
	*/

	{
		float zbias = -1.0f*mCameraSetup.GetDepthPlanes().x;
		glLoadMatrixf(glm::value_ptr(mCamera.GetViewMatrix()));
		glBegin(GL_QUADS);								
		//glColor3f(0.9f,0.9f,0.9f);						
		glColor3f(0.95f,0.95f,0.95f);						
		glVertex3f(mGrid.mBox.mMin.x, zbias, mGrid.mBox.mMin.z);
		glVertex3f(mGrid.mBox.mMax.x, zbias, mGrid.mBox.mMin.z);
		glVertex3f(mGrid.mBox.mMax.x, zbias, mGrid.mBox.mMax.z);
		glVertex3f(mGrid.mBox.mMin.x, zbias, mGrid.mBox.mMax.z);
		glEnd();			

		glLineWidth(2.0f);
		glBegin(GL_LINE_LOOP);		
		glColor4ub(0,0,0,48);		
		//glColor4ub(0,48,64,32);		
		glVertex3f(mGrid.mBox.mMin.x-zbias, 0.0f, mGrid.mBox.mMin.z-zbias);
		glVertex3f(mGrid.mBox.mMax.x-zbias, 0.0f, mGrid.mBox.mMin.z-zbias);
		glVertex3f(mGrid.mBox.mMax.x-zbias, 0.0f, mGrid.mBox.mMax.z-zbias);
		glVertex3f(mGrid.mBox.mMin.x-zbias, 0.0f, mGrid.mBox.mMax.z-zbias);
		glEnd();
		glLineWidth(1.0f);


		glPointSize(2.5f);
		glBegin(GL_POINTS);		
		glColor4ub(0,0,0,48);	
		//glColor4f(0.9f,0.9f/4.0f,0.9f/16.0f, 0.85f);		
		glVertex3f(mGrid.mBox.mMin.x-zbias, 0.0f, mGrid.mBox.mMin.z-zbias);
		glVertex3f(mGrid.mBox.mMax.x-zbias, 0.0f, mGrid.mBox.mMin.z-zbias);
		glVertex3f(mGrid.mBox.mMax.x-zbias, 0.0f, mGrid.mBox.mMax.z-zbias);
		glVertex3f(mGrid.mBox.mMin.x-zbias, 0.0f, mGrid.mBox.mMax.z-zbias);
		glEnd();
		glPointSize(1.0f);
	}
	
	if (mIsValidPick)
	{
		glLoadMatrixf(glm::value_ptr(mCamera.GetViewMatrix()));

		glLineWidth(1.0f);
		glBegin(GL_LINES);								
		glColor4f(0.0f,0.0f,0.0f, 0.77f);		
		glVertex3f(mPick.x, mPick.y - 1.0f, mPick.z);
		glVertex3f(mPick.x, mPick.y + 2.0f, mPick.z);
		glVertex3f(mPick.x - 1.0f, mPick.y, mPick.z);
		glVertex3f(mPick.x + 1.0f, mPick.y, mPick.z);
		glVertex3f(mPick.x, mPick.y, mPick.z - 1.0f);
		glVertex3f(mPick.x, mPick.y, mPick.z + 1.0f);
		glEnd();
	}

	if (true)
	{
		glLoadMatrixf(glm::value_ptr(mCamera.GetViewMatrix()));

		glLineWidth(2.5f);
		glBegin(GL_LINES);		
		glColor4ub(0,48,64,220);
		for (size_t i = 0; i < mNavmeshLinks.size(); ++i)
		{
			int pa = (int) mNavmeshLinks[i].x;
			int pb = (int) mNavmeshLinks[i].y;
			glVertex3f(mNavmeshPoints[pa].x, mNavmeshPoints[pa].y, mNavmeshPoints[pa].z);
			glVertex3f(mNavmeshPoints[pb].x, mNavmeshPoints[pb].y, mNavmeshPoints[pb].z);
		}
		glEnd();	

		glPointSize(3.0f);
		glBegin(GL_POINTS);	
		glColor4ub(0,0,0,196);
		for (size_t i = 0; i < mNavmeshLinks.size(); ++i)
		{
			int pa = (int) mNavmeshLinks[i].x;
			int pb = (int) mNavmeshLinks[i].y;
			glVertex3f(mNavmeshPoints[pa].x, mNavmeshPoints[pa].y, mNavmeshPoints[pa].z);
			glVertex3f(mNavmeshPoints[pb].x, mNavmeshPoints[pb].y, mNavmeshPoints[pb].z);
		}
		glEnd();

		glLineWidth(1.0f);
		glBegin(GL_LINES);	
		for (size_t i = 0; i < mNavmeshPoints.size(); ++i)
		{
			glColor4ub(0,0,0,82);
			BF::DrawCircle(mNavmeshPoints[i].x, mNavmeshPoints[i].y, mNavmeshPoints[i].z, 0.5f);
		}
		glEnd();	
		if (mPickNavmeshPoint >= 0 && mPickNavmeshPoint < mNavmeshPoints.size())
		{
			glLineWidth(2.5f);
			glBegin(GL_LINES);	
			glColor4ub(0,48,64,220);
			BF::DrawCircle(mNavmeshPoints[mPickNavmeshPoint].x, mNavmeshPoints[mPickNavmeshPoint].y, mNavmeshPoints[mPickNavmeshPoint].z, 0.3f);
			glEnd();	
		}
		glLineWidth(1.0f);

		glLineWidth(1.5f);
		glBegin(GL_LINES);		
		glColor4ub(0,48,64,32);
		for (size_t i = 0; i < mNavmeshTris.size(); i+=3)
		{
			float ax = mNavmeshTrisVerts[mNavmeshTris[i]].x;
			float az = mNavmeshTrisVerts[mNavmeshTris[i]].y;
			float bx = mNavmeshTrisVerts[mNavmeshTris[i+1]].x;
			float bz = mNavmeshTrisVerts[mNavmeshTris[i+1]].y;
			float cx = mNavmeshTrisVerts[mNavmeshTris[i+2]].x;
			float cz = mNavmeshTrisVerts[mNavmeshTris[i+2]].y;

			glVertex3f(ax, 0.0f, az);
			glVertex3f(bx, 0.0f, bz);
			glVertex3f(ax, 0.0f, az);
			glVertex3f(cx, 0.0f, cz);
			glVertex3f(bx, 0.0f, bz);
			glVertex3f(cx, 0.0f, cz);
		}
		glEnd();	
		glLineWidth(1.0f);

		glLineWidth(2.5f);
		glBegin(GL_LINES);		
		glColor4ub(0,48,64,220);
		for (size_t i = 0; i < mNavmeshBorderSegs.size(); i+=2)
		{
			float ax = mNavmeshTrisVerts[mNavmeshBorderSegs[i]].x;
			float az = mNavmeshTrisVerts[mNavmeshBorderSegs[i]].y;
			float bx = mNavmeshTrisVerts[mNavmeshBorderSegs[i+1]].x;
			float bz = mNavmeshTrisVerts[mNavmeshBorderSegs[i+1]].y;

			glVertex3f(ax, 0.0f, az);
			glVertex3f(bx, 0.0f, bz);
		}
		glEnd();	
		glLineWidth(1.0f);

		glPointSize(4.0f);
		glBegin(GL_POINTS);		
		glColor4ub(0,0,0,196);
		for (size_t i = 0; i < mNavmeshTris.size(); i+=3)
		{
			float ax = mNavmeshTrisVerts[mNavmeshTris[i]].x;
			float az = mNavmeshTrisVerts[mNavmeshTris[i]].y;
			float bx = mNavmeshTrisVerts[mNavmeshTris[i+1]].x;
			float bz = mNavmeshTrisVerts[mNavmeshTris[i+1]].y;
			float cx = mNavmeshTrisVerts[mNavmeshTris[i+2]].x;
			float cz = mNavmeshTrisVerts[mNavmeshTris[i+2]].y;

			glVertex3f(ax, 0.0f, az);
			glVertex3f(bx, 0.0f, bz);
			glVertex3f(cx, 0.0f, cz);
		}
		glEnd();	


		//glDisable(GL_CULL_FACE); 
		glBegin(GL_TRIANGLES);		
		glColor4ub(0,192,255,64);
		for (size_t i = 0; i < mNavmeshTris.size(); i+=3)
		{
			float ax = mNavmeshTrisVerts[mNavmeshTris[i]].x;
			float az = mNavmeshTrisVerts[mNavmeshTris[i]].y;
			float bx = mNavmeshTrisVerts[mNavmeshTris[i+1]].x;
			float bz = mNavmeshTrisVerts[mNavmeshTris[i+1]].y;
			float cx = mNavmeshTrisVerts[mNavmeshTris[i+2]].x;
			float cz = mNavmeshTrisVerts[mNavmeshTris[i+2]].y;

			glVertex3f(ax, 0.0f, az);
			glVertex3f(bx, 0.0f, bz);
			glVertex3f(cx, 0.0f, cz);
		}
		glEnd();	
	}

	static bool did_once = false;
	std::vector<glm::vec2> steps;

	if (!did_once)
	{
		using namespace BF;

		LocomoWorld world;
		world.fwd = glm::vec2(0.0f, 1.0f);
		world.right = glm::vec2(1.0f, 0.0f);

		LocomoBody body;
		body.footDist = 0.4f;

		LocomoState start_state;

		start_state.l.pos = glm::vec2();
		start_state.l.dir = 0.0f;

		start_state.r.pos = glm::vec2();
		start_state.r.dir = 0.0f;

		start_state.support = (EFootFlag) (ELFlag | ERFlag);

		WalkParams walk;

		walk.stride = 0.75f;
		walk.side = 0.0f;
		walk.speed = 1.5f;
		walk.turn = 0.0f;

		LocomoState state = start_state;

		{
			const FootState& support = (state.support & ELFlag) ? state.l : state.r;
			steps.push_back(support.pos);
		}

		{
			const FootState& other = (state.support & ELFlag) ? state.r : state.l;
			steps.push_back(other.pos);
		}

		Random random;

		for (int i = 0; i < 60; ++i)
		{
			LocomoState next_state;
			float duration;

			if (random.randf() > 0.8f)
			{
				walk.stride = random.randr(0.4f, 0.9f);
				//walk.side = random.randr(0.0f, 0.2f);
				walk.turn = random.randr(-15.0f, 15.0f);
			}
			
			if (execute(world, body, state, walk, next_state, duration))
			{
				const FootState& support = (next_state.support & ELFlag) ? next_state.l : next_state.r;
				steps.push_back(support.pos);
			}

			state = next_state;
		}
	}

	glPointSize(3.0f);
	glBegin(GL_POINTS);	
	glColor4ub(0,0,0,196);
	for (size_t i = 0; i < steps.size(); ++i)
	{
		glVertex3f(steps[i].x, 0.0f, steps[i].y);
	}
	glEnd();

#if 0
	{
		mCameraController.Render(mCamera);
	}
#endif

	//switch (mTestMode)
	{
		//default: 
		{
			//RenderTestBasicScene(inRenderer); 
		}
		//break;
	}
	
	mTexture->EndRender();
}
