#include "BE/Bigeye.h"
#include "BF/Camera.h"
#include "BF/BFMath.h"
#include "BF/Skeleton.h"
#include "BF/SkeletonRenderer.h"
#include "BF/GridRenderer.h"
#include "BF/LoaderBVH.h"

class BigfootScene : public BE::SimpleRenderToTextureWidget::Scene, public BE::MainWindowClient
{
public:

	enum ETestMode
	{
		ETestBasicScene,
		ETestSkeletonScene,
	};

	float mRenderTime;
	glm::vec2 mSize;
	BE::OGLRenderToTexture* mTexture;
	ETestMode mTestMode;
	BF::Camera mCamera;
	BF::ViewportSetup mViewportSetup;
	BF::CameraSetup mCameraSetup;
	BF::GridRenderer mGrid;
	BF::CameraTurnTableRotationController mCameraController;
	bool mAutoSetupBasicScene;
	
	BF::Skeleton mTestSkeleton;
	BF::SkeletonAnimationFrames mTestSkeletonAnim;
	BF::SkeletonRenderer mTestSkeletonRenderer;


	BigfootScene::BigfootScene() 
	:	mTestMode(ETestBasicScene)
	,	mAutoSetupBasicScene(true)
	,	mRenderTime(-1.0f)
	{
	}

	bool Create();

	virtual bool SupportsDragAndDrop() { return true; }
	virtual void OnFileDropped(BE::MainWindow* inWindow, const char* inFilePath);

	virtual void Update(const BE::WidgetContext& context, BE::SimpleRenderToTextureWidget& inParent, BE::OGLRenderToTexture& inTexture);
	virtual void Render(BE::Renderer& inRenderer);

	virtual void Unproject(const glm::vec2& inViewportPoint, glm::vec3& outSceneWorldPoint, glm::vec3* outSceneWorldRayDir);

	void RenderTestBasicScene(BE::Renderer& inRenderer);
	void RenderTestSkeletonScene(BE::Renderer& inRenderer);

	void OnFirstUpdate();
};


void CreateWidgets(BE::MainWindow& inWindow, BigfootScene& inScene);


int main()
{
	BigfootScene scene;
	BE::MainWindow main_window(&scene);
	
	if (main_window.Create("Bigfoot", 1280, 1024))
	{
		CreateWidgets(main_window, scene);

		if (scene.Create())
		{
			TimerMillis timer;

			while (main_window.Update(timer.GetTimeSecs()))
			{
				timer.CacheTime();
			}
		}
	}
	
	return 0;
}


void CreateWidgets(BE::MainWindow& inWindow, BigfootScene& inScene)
{
	BE::NativeWindowWidget* root =  inWindow.GetRootWidget();
	BE::WidgetContext context(inWindow, 0.0f);

	using namespace BE;

	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(context, glm::vec2(10.0f, 30.0f), glm::vec2(1200.0f, 960.0f), MagicWand::FRAME_NORMAL_CUT_UPPER, SimplePanelWidget::NoOverflowSlider, false);
		root->GetChildren().mChildWidgets.push_back(widget);

		SimplePanelWidget* parent_widget = widget;
		ChildWidgetContainer& children = widget->GetChildren();

		{
			SimpleRenderToTextureWidget* widget = new SimpleRenderToTextureWidget();
			widget->Create(context, glm::vec2(0.0f, 0.0f), parent_widget->GetInternalSize());
			widget->SetScene(&inScene);
			children.mChildWidgets.push_back(widget);
		}
	}
}


void BigfootScene::RenderTestBasicScene(BE::Renderer& inRenderer)
{
	bool test_controller = true;
	glm::mat4 view_matrix;

	float rtri = 16.0f * mRenderTime;
	float rquad = 128.0f * -mRenderTime;

	if (test_controller)
	{
		if (mAutoSetupBasicScene)
		{
			BF::CameraFollowSphereAutoSetup camera_auto_setup;
			BF::Sphere sphere; sphere.mPosition = glm::vec3(-1.5f,0.0f,-6.0f); sphere.mRadius = 5.0f;
			glm::vec2 auto_depth_planes;
			camera_auto_setup.SetFollowParams(glm::vec3(1.0f, 1.0f, 0.0f));
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

void BigfootScene::RenderTestSkeletonScene(BE::Renderer& inRenderer)
{
	BF::AAB skeleton_bounds;

	static int frame_index = -1;

	frame_index = (int) (10.0f * mRenderTime) % mTestSkeletonAnim.mSkeletonAnimationFrames.size();
	frame_index = frame_index % 30;
	//if (frame_index > mTestSkeletonAnim.mSkeletonAnimationFrames.size())
	//{
	//	frame_index = -1;
	//}

	// We are one frame off with the camera!
	mTestSkeletonRenderer.Render(mTestSkeleton, frame_index, &mTestSkeletonAnim, 
								mCamera.GetViewMatrix(), true, true, skeleton_bounds);
}


bool BigfootScene::Create()
{
	mCameraSetup.SetupDepthPlanes(glm::vec2(0.1f, 100.0f));
	mGrid.Setup(50, 50, 1.0f);
	mCameraController.AttachCamera(mCamera);
	
	return true;
}


void BigfootScene::OnFileDropped(BE::MainWindow* inWindow, const char* inFilePath)
{
	if (BF::LoaderBVH::Load(inFilePath, mTestSkeleton, &mTestSkeletonAnim))
	{
		mTestMode = ETestSkeletonScene;
		{
			BF::AAB render_bounds;
			mTestSkeletonRenderer.GetRenderBounds(mTestSkeleton, 0, true, true, render_bounds);

			BF::Sphere skeleton_sphere;
			skeleton_sphere.InitFrom(render_bounds);

			BF::CameraFollowSphereAutoSetup cam_setup;

			cam_setup.SetFollowParams(glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec2 auto_depth_planes;
			cam_setup.SetupCamera(skeleton_sphere, mCameraSetup.GetFOV(), glm::vec2(0.001f, 10000.0f), mCamera, auto_depth_planes);
			mCameraSetup.SetupDepthPlanes(auto_depth_planes);

			{
				float scale = 1.0f;
				float scale_test = skeleton_sphere.mRadius;
				if (scale_test > 1.0f)
				{
					while (scale_test > 0.6f)
					{
						scale_test /= 10.0f;
						scale *= 10.0f;
					}
				}
				else
				{
					while (scale_test < 4.0f)
					{
						scale_test *= 10.0f;
						scale /= 10.0f;
					}
				}

				mGrid.Setup(50, 50, scale / 100.0f);
			}
		}
	}
	else
	{
		mTestMode = ETestBasicScene;
		mAutoSetupBasicScene = true;

		mCameraSetup.SetupDepthPlanes(glm::vec2(0.1f, 100.0f));
		mGrid.Setup(50, 50, 1.0f);
	}
}


void BigfootScene::Update(const BE::WidgetContext& context, BE::SimpleRenderToTextureWidget& inParent, BE::OGLRenderToTexture& inTexture)	
{
	mTexture = &inTexture;

	mSize.x = inParent.GetSize().x;
	mSize.y = inParent.GetSize().y;
	mViewportSetup.mWindowSize = mSize;
	mCameraSetup.SetupFOVY(mViewportSetup);

	if (mRenderTime < 0.0f)
		OnFirstUpdate();
	
	mRenderTime = context.mTimeSecs;
	
	mCameraController.Update(context, inParent, mViewportSetup, mCameraSetup);
}


void BigfootScene::Unproject(const glm::vec2& inViewportPoint, glm::vec3& outSceneWorldPoint, glm::vec3* outSceneWorldRayDir)
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


void BigfootScene::OnFirstUpdate()
{
	//OnFileDropped(NULL, "../media/test.bvh");
}


void BigfootScene::Render(BE::Renderer& inRenderer)	
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
	glClearColor(115.0f/255.0f, 115.0f/255.0f, 115.0f/255.0f, 1.0f); // Blender

	glClearDepth(1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mCameraSetup.SetGlProjectionMatrix();

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();		

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations


	{
		glLoadMatrixf(glm::value_ptr(mCamera.GetViewMatrix()));
		mGrid.Render();
	}

#if 0
	{
		mCameraController.Render(mCamera);
	}
#endif

	switch (mTestMode)
	{
		case ETestSkeletonScene: 
		{
			RenderTestSkeletonScene(inRenderer); break;
		}

		default: RenderTestBasicScene(inRenderer); break;
	}
	
	mTexture->EndRender();
}
