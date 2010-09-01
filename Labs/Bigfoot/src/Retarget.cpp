#include "BE/Bigeye.h"
#include "BF/Camera.h"
#include "BF/BFMath.h"
#include "BF/Skeleton.h"
#include "BF/SkeletonRenderer.h"
#include "BF/GridRenderer.h"
#include "BF/LoaderBVH.h"

class BigfootRetargetScene : public BE::SimpleRenderToTextureWidget::Scene, public BE::MainWindowClient
{
public:

	enum ETestMode
	{
		ETestBasicScene,
		ETestSkeletonScene,
	};

	struct AnimPlayback
	{
		float mLastRenderTime;
		float mPlaybackTime;

		void Reset()
		{
			mLastRenderTime = -1.0f;
			mPlaybackTime = 0.0f;
		}

		void UpdatePlay(float inRenderTime)
		{
			if (mLastRenderTime == -1.0f)
				mLastRenderTime = inRenderTime;

			mPlaybackTime += inRenderTime - mLastRenderTime;
			mLastRenderTime = inRenderTime;
		}

		void UpdatePause(float inRenderTime)
		{
			mLastRenderTime = inRenderTime;
		}
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

	AnimPlayback mAnimPlayback;
	bool mIncludeDefaultPoseInPlayback;

	BE::SimpleButtonWidget* mPlayButton;
	BE::SimpleButtonWidget* mLoopButton;
	BE::SimpleButtonWidget* mRewindButton;
	BE::SimpleButtonWidget* mHelpButton;
	BE::SimplePanelWidget* mHelpPanel;
	BE::SimpleButtonWidget* mIncludeDefaultPoseButton;
	BE::SimpleSliderWidget* mFrameSlider;
	BE::SimpleSliderWidget* mSpeedSlider;

	BigfootRetargetScene::BigfootRetargetScene() 
	:	mTestMode(ETestBasicScene)
	,	mAutoSetupBasicScene(true)
	,	mRenderTime(-1.0f)
	,	mPlayButton(NULL)
	,	mLoopButton(NULL)
	,	mRewindButton(NULL)
	,	mHelpButton(NULL)
	,	mHelpPanel(NULL)
	,	mIncludeDefaultPoseButton(NULL)
	,	mFrameSlider(NULL)
	,	mSpeedSlider(NULL)
	,	mIncludeDefaultPoseInPlayback(true)
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
	void RenderTestSkeletonScene(BE::Renderer& inRenderer);

	void OnFirstUpdate();

	int SliderToFrameIndex(float inSliderPos) const;
	float FrameIndexToSlider(int inFrameIndex) const;

	float GetPlaySpeed();
};


void CreateWidgets(BE::MainWindow& inWindow, BigfootRetargetScene& inScene);


int RetargetMain()
{
	BigfootRetargetScene scene;
	BE::MainWindow main_window(&scene);
	
	if (main_window.Create("Bigfoot [Retarget]", 1024, 768))
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


void CreateWidgets(BE::MainWindow& inWindow, BigfootRetargetScene& inScene)
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
			widget->Create(context, glm::vec2(0.0f, 0.0f), parent_widget->GetInternalSize());
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
			button_widget->Create(context, glm::vec2(pos_horiz, pos_vert), true, MagicWand::TextInfo(" > ", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);

			pos_horiz += 30.0f;

			children.mChildWidgets.push_back(button_widget);
			inScene.mPlayButton = button_widget;
		}

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(pos_horiz, pos_vert), false, MagicWand::TextInfo(" << ", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);

			pos_horiz += 30.0f;

			children.mChildWidgets.push_back(button_widget);
			inScene.mRewindButton = button_widget;
		}

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(pos_horiz, pos_vert), true, MagicWand::TextInfo("oo", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);

			pos_horiz += 30.0f;

			children.mChildWidgets.push_back(button_widget);
			inScene.mLoopButton = button_widget;
		}

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(context, glm::vec2(pos_horiz, pos_vert), true, MagicWand::TextInfo(" T ", 0, 14.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);
			button_widget->SetIsToggled(true);

			pos_horiz += 30.0f;

			children.mChildWidgets.push_back(button_widget);
			inScene.mIncludeDefaultPoseButton = button_widget;
		}

		{
			MagicWand::SizeConstraints sizeConstraints;
			vert2d(sizeConstraints.mMinSize) = 18.0f;
			vert2d(sizeConstraints.mMaxSize) = 18.0f;

			pos_horiz += 5.0f;

			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(context, glm::vec2(pos_horiz, pos_vert+1.0f), glm::vec2(100.0f, 0.0f), MagicWand::TextInfo("Speed", 0, 12.0f, true, glm::vec2(4.0f, 2.0f)), sizeConstraints);
			slider_widget->SetSliderPos(0.5f);
			
			pos_horiz += horiz2d(slider_widget->GetSize()) + 5.0f;

			children.mChildWidgets.push_back(slider_widget);
			inScene.mSpeedSlider = slider_widget;
		}


		{
			MagicWand::SizeConstraints sizeConstraints;
			vert2d(sizeConstraints.mMinSize) = 18.0f;
			vert2d(sizeConstraints.mMaxSize) = 18.0f;

			pos_horiz += 25.0f;

			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(context, glm::vec2(pos_horiz, pos_vert+1.0f), glm::vec2(680.0f, 0.0f), MagicWand::TextInfo("Frame", 0, 12.0f, true, glm::vec2(4.0f, 2.0f)), sizeConstraints);
			slider_widget->SetSliderPos(0.0f);
			
			pos_horiz += horiz2d(slider_widget->GetSize()) + 5.0f;

			children.mChildWidgets.push_back(slider_widget);
			inScene.mFrameSlider = slider_widget;
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
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo("BigFoot [viewer] ver. 0.1b", 0, 12.0f, true, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
					
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;

				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo("© 2010 Jad Nohra", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
					
				pos_vert += vert2d(text_widget->GetSize()) + 3.0f * height_offset;

				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo(" * Load .BVH mocap file : Drag and drop into window", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;
				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo(" * Rotate camera : Drag middle mouse button", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;
				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo(" * Zoom camera : Mouse scroll", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;
				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo(" * > : Play / Pause", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;
				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo(" * << : Rewind to start", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;
				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo(" * oo : Loop toggle", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;
				children.mChildWidgets.push_back(text_widget);
			}

			{
				SimpleTextWidget* text_widget = new SimpleTextWidget();
				text_widget->Create(context, glm::vec2(pos_horiz, pos_vert), MagicWand::TextInfo(" * T : Show / Hide first frame toggle", 0, 10.0f, false, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
				pos_vert += vert2d(text_widget->GetSize()) + height_offset;
				children.mChildWidgets.push_back(text_widget);
			}
		}
	}
}


void BigfootRetargetScene::RenderTestBasicScene(BE::Renderer& inRenderer)
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


int BigfootRetargetScene::SliderToFrameIndex(float inSliderPos) const
{
	int frame_index = -1;

	if (mFrameSlider != NULL && !mTestSkeletonAnim.mSkeletonAnimationFrames.empty())
	{
		if (mIncludeDefaultPoseInPlayback || mTestSkeletonAnim.mSkeletonAnimationFrames.size() == 1)
			frame_index = (int) (inSliderPos * (float) (mTestSkeletonAnim.mSkeletonAnimationFrames.size()-1));
		else
			frame_index = ((int) (inSliderPos * (float) (mTestSkeletonAnim.mSkeletonAnimationFrames.size()-2)))+1;
	}

	frame_index = std::min(frame_index, (int) mTestSkeletonAnim.mSkeletonAnimationFrames.size()-1);
	frame_index = std::max(frame_index, -1);
	
	return frame_index;
}


float BigfootRetargetScene::FrameIndexToSlider(int inFrameIndex) const
{
	float slider_pos = 0.0f;

	if (mFrameSlider != NULL && !mTestSkeletonAnim.mSkeletonAnimationFrames.empty())
	{
		if (mIncludeDefaultPoseInPlayback || mTestSkeletonAnim.mSkeletonAnimationFrames.size() == 1)
			slider_pos = (float) inFrameIndex / ((float) mTestSkeletonAnim.mSkeletonAnimationFrames.size()-1);
		else
			slider_pos = (float) (inFrameIndex-1) / ((float) mTestSkeletonAnim.mSkeletonAnimationFrames.size()-2);
	}

	return slider_pos;
}


void BigfootRetargetScene::RenderTestSkeletonScene(BE::Renderer& inRenderer)
{
	BF::AAB skeleton_bounds;

	mIncludeDefaultPoseInPlayback = (mIncludeDefaultPoseButton == NULL || mIncludeDefaultPoseButton->IsToggled());
	int frame_index = -1;

	if (mSpeedSlider != NULL && mSpeedSlider->IsBeingDraggedByMouse())
	{
		frame_index = SliderToFrameIndex(mFrameSlider != NULL ? mFrameSlider->GetSliderPos() : 0.0f);

		mAnimPlayback.UpdatePause(mRenderTime);
		mAnimPlayback.mPlaybackTime = ((float) frame_index * mTestSkeletonAnim.mFrameTime) / GetPlaySpeed();
	}
	else if (mFrameSlider != NULL && mFrameSlider->IsBeingDraggedByMouse())
	{
		frame_index = SliderToFrameIndex(mFrameSlider != NULL ? mFrameSlider->GetSliderPos() : 0.0f);

		mAnimPlayback.UpdatePause(mRenderTime);
		mAnimPlayback.mPlaybackTime = ((float) frame_index * mTestSkeletonAnim.mFrameTime) / GetPlaySpeed();
	}
	else
	{
		if (mPlayButton == NULL || mPlayButton->IsToggled())
		{
			mAnimPlayback.UpdatePlay(mRenderTime);

			if (!mTestSkeletonAnim.mSkeletonAnimationFrames.empty())
			{
				frame_index = (int) (mAnimPlayback.mPlaybackTime * GetPlaySpeed() / mTestSkeletonAnim.mFrameTime);

				if (frame_index >= (int) mTestSkeletonAnim.mSkeletonAnimationFrames.size())
				{
					if (mLoopButton->IsToggled())
					{
						mAnimPlayback.Reset();
						frame_index = (!mIncludeDefaultPoseInPlayback && mTestSkeletonAnim.mSkeletonAnimationFrames.size() >= 2) ? 1 : 0;
						mAnimPlayback.mPlaybackTime = ((float) frame_index * mTestSkeletonAnim.mFrameTime) / GetPlaySpeed();
					}
					else
						frame_index = mTestSkeletonAnim.mSkeletonAnimationFrames.size() - 1;
				}
			}
		}
		else
		{
			frame_index = SliderToFrameIndex(mFrameSlider != NULL ? mFrameSlider->GetSliderPos() : 0.0f);
			mAnimPlayback.UpdatePause(mRenderTime);
		}

		if (mFrameSlider != NULL)
		{
			float slider_pos = FrameIndexToSlider(frame_index);
			mFrameSlider->SetSliderPos(slider_pos);
		}
	}
	
	// We are one frame off with the camera!
	mTestSkeletonRenderer.Render(mTestSkeleton, frame_index, &mTestSkeletonAnim, 
									mCamera.GetViewMatrix(), true, true, skeleton_bounds);
}


bool BigfootRetargetScene::Create()
{
	mCameraSetup.SetupDepthPlanes(glm::vec2(0.1f, 100.0f));
	BF::AAB default_gid_aab;
	default_gid_aab.Include(glm::vec3(-50.0f, -50.0f, -50.0f));
	default_gid_aab.Include(glm::vec3(50.0f, 50.0f, 50.0f));
	mGrid.Setup(default_gid_aab, glm::vec2(100.0f, 100.0f));
	mCameraController.AttachCamera(mCamera);
	
	return true;
}


void BigfootRetargetScene::OnFileDropped(BE::MainWindow* inWindow, const char* inFilePath)
{
	if (BF::LoaderBVH::Load(inFilePath, mTestSkeleton, &mTestSkeletonAnim))
	{
		mTestMode = ETestSkeletonScene;
		{
			BF::AAB render_bounds;

			BF::AAB frame_render_bounds;
			BF::AAB default_pose_frame_render_bounds;
			mTestSkeletonRenderer.GetRenderBounds(mTestSkeleton, -1, &mTestSkeletonAnim, true, true, frame_render_bounds);
			default_pose_frame_render_bounds = frame_render_bounds;
			render_bounds.Include(frame_render_bounds);

			// TODO use grid AAB and extract from animation
			if (!mTestSkeletonAnim.mSkeletonAnimationFrames.empty())
			{
				BF::AAB frame_render_bounds;

				for (size_t i=0; i<mTestSkeletonAnim.mSkeletonAnimationFrames.size(); ++i)
				{
					mTestSkeletonRenderer.GetRenderBounds(mTestSkeleton, i, &mTestSkeletonAnim, true, true, frame_render_bounds);
					render_bounds.Include(frame_render_bounds);
				}
			}
			
			// include origin
			render_bounds.Include(glm::vec3(0.0f));

			BF::Sphere anim_sphere;
			anim_sphere.InitFrom(render_bounds);

			BF::Sphere skeleton_sphere;
			skeleton_sphere.InitFrom(default_pose_frame_render_bounds);
			printf("Model scale: %f units\n", skeleton_sphere.mRadius);

			BF::CameraFollowSphereAutoSetup cam_setup;

			cam_setup.SetFollowParams(glm::vec3(1.0f, 1.0f, -1.0f));
			glm::vec2 auto_depth_planes;
			cam_setup.SetupCamera(anim_sphere, mCameraSetup.GetFOV(), glm::vec2(0.001f, 10000.0f), mCamera, auto_depth_planes);
			auto_depth_planes.y *= 50.0f;
			mCameraSetup.SetupDepthPlanes(auto_depth_planes);

			float grid_unit_scale = skeleton_sphere.mRadius / 4.0f;
			float nice_grid_unit_scale = 1.0f;
			{
				float scale_test = grid_unit_scale;
				if (scale_test > 1.0f)
				{
					while (scale_test > 1.0f)
					{
						scale_test /= 10.0f;
						nice_grid_unit_scale *= 10.0f;
					}
				}
				else
				{
					while (scale_test < 0.1f)
					{
						scale_test *= 10.0f;
						nice_grid_unit_scale /= 10.0f;
					}
				}

				nice_grid_unit_scale /= 10.0f;
			}
			
			glm::vec2 grid_division_count;
			grid_division_count.x = render_bounds.GetExtents().x / nice_grid_unit_scale;
			grid_division_count.y = render_bounds.GetExtents().z / nice_grid_unit_scale;
			mGrid.Setup(render_bounds, grid_division_count);

			mAnimPlayback.Reset();
			if (mFrameSlider != NULL)
				mFrameSlider->SetSliderPos(0.0f);
		}
	}
	else
	{
		mTestMode = ETestBasicScene;
		mAutoSetupBasicScene = true;

		mCameraSetup.SetupDepthPlanes(glm::vec2(0.1f, 100.0f));
		BF::AAB default_gid_aab;
		default_gid_aab.Include(glm::vec3(-50.0f, -50.0f, -50.0f));
		mGrid.Setup(default_gid_aab, glm::vec2(100.0f, 100.0f));
	}
}

float BigfootRetargetScene::GetPlaySpeed() 
{
	if (mSpeedSlider == NULL)
		return 1.0f;

	if (mSpeedSlider->GetSliderPos() < 0.5f)
	{
		float factor = (mSpeedSlider->GetSliderPos() / 0.5f);

		return 0.01f + factor * (1.0f - 0.01f);
	}
	else
	{
		float factor = (mSpeedSlider->GetSliderPos() - 0.5f) / 0.5f;

		return 1.0f + factor * (10.0f - 1.0f);
	}
}

void BigfootRetargetScene::ProcessWidgetEvents(BE::MainWindow* inWindow, BE::WidgetEventManager& inManager)
{
	for (size_t i=0; i<inManager.GetEventCount(); ++i)
	{
		const BE::WidgetEvent& widget_event = inManager.GetEvent(i);
		if (widget_event.mWidget == mRewindButton)
		{
			int frame_index = (!mIncludeDefaultPoseInPlayback && mTestSkeletonAnim.mSkeletonAnimationFrames.size() >= 2) ? 1 : 0;
			
			mAnimPlayback.Reset();
			mAnimPlayback.mPlaybackTime = ((float) frame_index * mTestSkeletonAnim.mFrameTime) / GetPlaySpeed();
			

			if (mFrameSlider != NULL)
				mFrameSlider->SetSliderPos(FrameIndexToSlider(frame_index));
		}
	}
}


void BigfootRetargetScene::Update(const BE::WidgetContext& context, BE::SimpleRenderToTextureWidget& inParent, BE::OGLRenderToTexture& inTexture)	
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
}


void BigfootRetargetScene::Unproject(const glm::vec2& inViewportPoint, glm::vec3& outSceneWorldPoint, glm::vec3* outSceneWorldRayDir)
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


void BigfootRetargetScene::OnFirstUpdate()
{
	//OnFileDropped(NULL, "../media/test.bvh");
	OnFileDropped(NULL, "../media/113_25.bvh");
}


void BigfootRetargetScene::Render(BE::Renderer& inRenderer)	
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
			RenderTestSkeletonScene(inRenderer); 
		}
		break;

		default: 
		{
			RenderTestBasicScene(inRenderer); 
		}
		break;
	}
	
	mTexture->EndRender();
}
