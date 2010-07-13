#ifdef WIN32
#include "../include/Bigeye/Bigeye_win_ogl.h"
#include "ARB_Multisample.h"
#include "MagickWand.h"
#include "strtk.hpp"

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


void ChildWidgetContainer::SetDeleteWidgets(bool inDeleteWidgets)
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


bool SimpleButtonWidget::Create(const glm::vec2& inPos, bool inIsToggleButton, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints)
{
	mPos = to3d_point(inPos);
	mTextInfo = inTextInfo;
	mSizeConstraints = inSizeConstraints;
	mIsHighlighted = false;
	mIsPressed = false;
	mIsMousePressed = false;
	mIsToggleButton = inIsToggleButton;
	mIsToggled = false;

	return true;
}

void SimpleButtonWidget::SetIsToggled(bool inIsToggled)
{
	mIsToggled = inIsToggled;
}

void SimpleButtonWidget::CreateTextures(const App& inApp)
{
	if (!mButtonTexture.IsCreated())
	{
		mButtonTexture.AutoCreate();
		mPressedButtonTexture.AutoCreate();
		mHighlightedButtonTexture.AutoCreate();

		GLsizei tex_dims[2];

		inApp.GetWand().MakeButtonTexture(mButtonTexture.mTexture, MagicWand::WIDGET_NORMAL, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
		mButtonTexSize[0] = tex_dims[0];
		mButtonTexSize[1] = tex_dims[1];

		
		inApp.GetWand().MakeButtonTexture(mPressedButtonTexture.mTexture, MagicWand::WIDGET_PRESSED, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
		inApp.GetWand().MakeButtonTexture(mHighlightedButtonTexture.mTexture, MagicWand::WIDGET_HIGHLIGHTED, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
	}
}


bool WidgetUtil::IsMouseInRectangle(const App& inApp, const glm::vec2& inWidgetWorldPos, const glm::vec2& inWidgetSize)
{
	glm::vec2 local_mouse_pos = inApp.GetMousePos() - inWidgetWorldPos;

	if (local_mouse_pos.x < 0.0f || local_mouse_pos.y < 0.0f)
		return false;

	glm::vec2 local_mouse_pos_in_rect = inWidgetSize - local_mouse_pos;
	
	return (local_mouse_pos_in_rect.x > 0.0f && local_mouse_pos_in_rect.y > 0.0f);
}


void SimpleButtonWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = inParentTransform * mPos;

	if (!mButtonTexture.IsCreated())
	{
		CreateTextures(inApp);
	}
	
	bool was_mouse_pressed = mIsMousePressed;

	if (WidgetUtil::IsMouseInRectangle(inApp, to2d_point(world_pos), mButtonTexSize))
	{
		mIsHighlighted = true;

		if (inApp.GetInputState(INPUT_MOUSE_LEFT) > 0.0f)
			mIsMousePressed = true;
		else
			mIsMousePressed = false;
	}
	else
	{
		mIsMousePressed = false;
		mIsHighlighted = false;
	}

	if (mIsToggleButton)
	{
		if (was_mouse_pressed && !mIsMousePressed)
			mIsToggled = !mIsToggled;
	}
	else
	{
		mIsPressed = mIsMousePressed;
	}
}

void SimpleButtonWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec2 world_pos = to2d_point(inParentTransform * mPos);
	
	bool is_pressed = mIsMousePressed || (mIsToggleButton && mIsToggled);
	GLuint tex = is_pressed ? mPressedButtonTexture.mTexture : (mIsHighlighted ? mHighlightedButtonTexture.mTexture : mButtonTexture.mTexture);
	const glm::vec2& tex_size = mButtonTexSize;

	inApp.GetOGLStateManager().Enable(EOGLState_TextureWidget);
	{
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex2f(world_pos.x,world_pos.y);
			 glTexCoord2f(1.0f,0.0f); glVertex2f(world_pos.x+tex_size.x,world_pos.y);
			 glTexCoord2f(1.0f,1.0f); glVertex2f(world_pos.x+tex_size.x,world_pos.y+tex_size.y);
			 glTexCoord2f(0.0f,1.0f); glVertex2f(world_pos.x,world_pos.y+tex_size.y);
		glEnd();
	}
}


bool SimpleSliderWidget::Create(const glm::vec2& inPos, const glm::vec2& inSize, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints)
{
	mType = Horizontal;
	mPos = to3d_point(inPos);
	mSize = inSize;

	mTextInfo = inTextInfo;
	mSizeConstraints = inSizeConstraints;

	mHasMouseSliderFocus = false;
	mIsHighlighted = false;
	mSliderPos = 0.5f;
	
	return true;
}


bool SimpleSliderWidget::CreateVertical(const glm::vec2& inPos, const glm::vec2& inSize)
{
	mType = Vertical;
	mPos = to3d_point(inPos);
	mSize = inSize;

	mHasMouseSliderFocus = false;
	mIsHighlighted = false;
	mSliderPos = 0.5f;

	return true;
}


void SimpleSliderWidget::CreateTextures(const App& inApp)
{
	if (!mFrameTexture.IsCreated())
	{
		mFrameTexture.AutoCreate();
		mMarkerTexture.AutoCreate();
		mPressedMarkerTexture.AutoCreate();
		mHighlightedMarkerTexture.AutoCreate();

		GLsizei tex_dims[2];

		if (mType == Horizontal)
			inApp.GetWand().MakeSliderFrameTexture(mFrameTexture.mTexture, mSize.x, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
		else
			inApp.GetWand().MakeVerticalSliderFrameTexture(mFrameTexture.mTexture, mSize.x, mSize.y, tex_dims[0], tex_dims[1]);

		mFrameTexSize[0] = tex_dims[0];
		mFrameTexSize[1] = tex_dims[1];

		if (mType == Horizontal)
			inApp.GetWand().MakeSliderMarkerTexture(mMarkerTexture.mTexture, MagicWand::WIDGET_NORMAL, tex_dims[1], tex_dims[0], tex_dims[1]);
		else
			inApp.GetWand().MakeVerticalSliderMarkerTexture(mMarkerTexture.mTexture, MagicWand::WIDGET_NORMAL, tex_dims[0], tex_dims[0], tex_dims[1]);

		mMarkerTexSize[0] = tex_dims[0];
		mMarkerTexSize[1] = tex_dims[1];

		if (mType == Horizontal)
			inApp.GetWand().MakeSliderMarkerTexture(mPressedMarkerTexture.mTexture, MagicWand::WIDGET_PRESSED, tex_dims[1], tex_dims[0], tex_dims[1]);
		else
			inApp.GetWand().MakeVerticalSliderMarkerTexture(mPressedMarkerTexture.mTexture, MagicWand::WIDGET_PRESSED, tex_dims[0], tex_dims[0], tex_dims[1]);
	
		if (mType == Horizontal)
			inApp.GetWand().MakeSliderMarkerTexture(mHighlightedMarkerTexture.mTexture, MagicWand::WIDGET_HIGHLIGHTED, tex_dims[1], tex_dims[0], tex_dims[1]);
		else
			inApp.GetWand().MakeVerticalSliderMarkerTexture(mHighlightedMarkerTexture.mTexture, MagicWand::WIDGET_HIGHLIGHTED, tex_dims[0], tex_dims[0], tex_dims[1]);
	}
}


void SimpleSliderWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec2 world_pos = to2d_point(inParentTransform * mPos);

	if (!mMarkerTexture.IsCreated())
	{
		CreateTextures(inApp);
	}

	glm::vec2 marker_world_pos = GetSliderWorldPos(world_pos);
	bool is_mouse_left_down = (inApp.GetInputState(INPUT_MOUSE_LEFT) > 0.0f);
	bool is_mouse_in_rect = WidgetUtil::IsMouseInRectangle(inApp, world_pos, mFrameTexSize);
	bool has_mouse_slider_focus = false;

	if (is_mouse_left_down)
	{
		if (WidgetUtil::IsMouseInRectangle(inApp, marker_world_pos, mMarkerTexSize))
		{
			has_mouse_slider_focus = true;
		}
		else
		{
			has_mouse_slider_focus = mHasMouseSliderFocus;
		}
	}
	
	mIsHighlighted = is_mouse_in_rect;
	
	if (has_mouse_slider_focus)
	{
		glm::vec2 mouse_pos = inApp.GetMousePos();

		if (!mHasMouseSliderFocus)
		{
			mMouseSliderFocusStartMousePos = mouse_pos;
			mMouseSliderFocusStartSliderPos = mSliderPos;
		}

		float pos_diff = 0.0f;
		float new_slider_pos = 0.0f;

		if (mType == Horizontal)
		{
			pos_diff = mouse_pos.x - mMouseSliderFocusStartMousePos.x;
			new_slider_pos = mMouseSliderFocusStartSliderPos  + (pos_diff / (float) mFrameTexSize.x);
		}
		else
		{
			pos_diff = mouse_pos.y - mMouseSliderFocusStartMousePos.y;
			new_slider_pos = mMouseSliderFocusStartSliderPos  + (pos_diff / (float) mFrameTexSize.y);
		}

		new_slider_pos = std::max(0.0f, new_slider_pos);
		new_slider_pos = std::min(1.0f, new_slider_pos);

		SetSliderPos(new_slider_pos);
	}

	mHasMouseSliderFocus = has_mouse_slider_focus;
}


glm::vec2 SimpleSliderWidget::GetSliderWorldPos(const glm::vec2& inWorldPos) const
{
	glm::vec2 marker_world_pos = inWorldPos;

	if (mType == Horizontal)
		horiz2d(marker_world_pos) += 2 + mSliderPos * (horiz2d(mFrameTexSize) - (horiz2d(mMarkerTexSize)+4));
	else
	{
		horiz2d(marker_world_pos) += 0.5f * (horiz2d(mFrameTexSize) - (horiz2d(mMarkerTexSize)));
		vert2d(marker_world_pos) += 2 + mSliderPos * (vert2d(mFrameTexSize) - (vert2d(mMarkerTexSize)+4));
	}


	return marker_world_pos;
}

void SimpleSliderWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec2 world_pos = to2d_point(inParentTransform * mPos);

	glm::vec2 frame_world_pos = world_pos;
	glm::vec2 marker_world_pos = GetSliderWorldPos(world_pos);

	inApp.GetOGLStateManager().Enable(EOGLState_TextureWidget);
	{
		glBindTexture(GL_TEXTURE_2D, mFrameTexture.mTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex2f(frame_world_pos.x,frame_world_pos.y);
			 glTexCoord2f(1.0f,0.0f); glVertex2f(frame_world_pos.x+mFrameTexSize.x,frame_world_pos.y);
			 glTexCoord2f(1.0f,1.0f); glVertex2f(frame_world_pos.x+mFrameTexSize.x,frame_world_pos.y+mFrameTexSize.y);
			 glTexCoord2f(0.0f,1.0f); glVertex2f(frame_world_pos.x,frame_world_pos.y+mFrameTexSize.y);
		glEnd();
	}

	{
		glBindTexture(GL_TEXTURE_2D, mHasMouseSliderFocus ? mPressedMarkerTexture.mTexture : (mIsHighlighted ? mHighlightedMarkerTexture.mTexture : mMarkerTexture.mTexture));
		// I do not know why things break if I do not set these params again here!!! find out!
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			
	
		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex2f(marker_world_pos.x,marker_world_pos.y);
			 glTexCoord2f(1.0f,0.0f); glVertex2f(marker_world_pos.x+mMarkerTexSize.x,marker_world_pos.y);
			 glTexCoord2f(1.0f,1.0f); glVertex2f(marker_world_pos.x+mMarkerTexSize.x,marker_world_pos.y+mMarkerTexSize.y);
			 glTexCoord2f(0.0f,1.0f); glVertex2f(marker_world_pos.x,marker_world_pos.y+mMarkerTexSize.y);
		glEnd();
	}
}


bool SimpleTextWidget::Create(const App& inApp, const glm::vec2& inPos, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints)
{
	mPos = to3d_point(inPos);

	mTextTexture.AutoCreate();
	GLsizei tex_dims[2];
	inApp.GetWand().MakeTextTexture(mTextTexture.mTexture, inTextInfo, inSizeConstraints, tex_dims[0], tex_dims[1]);
	mTextTexSize[0] = tex_dims[0];
	mTextTexSize[1] = tex_dims[1];

	return true;
}


void SimpleTextWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = inParentTransform * mPos;

	inApp.GetOGLStateManager().Enable(EOGLState_TextureWidget);
	{
		glBindTexture(GL_TEXTURE_2D, mTextTexture.mTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex2f(world_pos.x,world_pos.y);
			 glTexCoord2f(1.0f,0.0f); glVertex2f(world_pos.x+mTextTexSize.x,world_pos.y);
			 glTexCoord2f(1.0f,1.0f); glVertex2f(world_pos.x+mTextTexSize.x,world_pos.y+mTextTexSize.y);
			 glTexCoord2f(0.0f,1.0f); glVertex2f(world_pos.x,world_pos.y+mTextTexSize.y);
		glEnd();
	}
}


SimpleTextureWidget::SimpleTextureWidget()
:	mTexture(-1)
{
}

SimpleTextureWidget::~SimpleTextureWidget()
{
	if (mTexture >= 0)
		glDeleteTextures(1, &mTexture);
}

bool SimpleTextureWidget::Create(const App& inApp, const glm::vec2& inPos, const char* inTexturePath)
{
	mPos = to3d_point(inPos);
	mTexture = 0;

	GLsizei dims[2];

	if (!inApp.GetWand().ReadImageToGLTexture(inTexturePath, mTexture, dims[0], dims[1]))
		return false;

	mSize.x = (float) dims[0];
	mSize.y = (float) dims[1];

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


SimpleRenderToTextureWidget::SimpleRenderToTextureWidget()
:	mScene(NULL)
{
}

SimpleRenderToTextureWidget::~SimpleRenderToTextureWidget()
{
}

bool SimpleRenderToTextureWidget::Create(const App& inApp, const glm::vec2& inPos, const glm::vec2& inSize)
{
	mPos = to3d_point(inPos);

	if (!mTexture.Create((GLsizei) inSize.x, (GLsizei) inSize.y))
		return false;

	mSize = inSize;

	return true;
}


void SimpleRenderToTextureWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (mScene)
		mScene->Update(inApp, *this, inTimeSecs);
}


void SimpleRenderToTextureWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	//return;

	glm::vec3 world_pos = inParentTransform * mPos;

	inApp.PushRenderToTexture(to2d_point(world_pos), mTexture);
	{
		if (mScene)
			mScene->Render(inApp, *this, inTimeSecs);
	}
	inApp.PopRenderToTexture();

	inApp.GetOGLStateManager().Enable(EOGLState_TextureWidget);
	{
		mTexture.Bind();
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 // here we invert the coordinates because the texture is stored bottom up (??not sure??)
			 glTexCoord2f(0.0f,1.0f); glVertex2f(world_pos.x,world_pos.y);
			 glTexCoord2f(1.0f,1.0f); glVertex2f(world_pos.x+mSize.x,world_pos.y);
			 glTexCoord2f(1.0f,0.0f); glVertex2f(world_pos.x+mSize.x,world_pos.y+mSize.y);
			 glTexCoord2f(0.0f,0.0f); glVertex2f(world_pos.x,world_pos.y+mSize.y);
		glEnd();
	}
}


MagicWandTestTextureWidget::MagicWandTestTextureWidget()
{
}


MagicWandTestTextureWidget::~MagicWandTestTextureWidget()
{
}


bool MagicWandTestTextureWidget::Create(const App& inApp, const glm::vec2& inPos)
{
	mPos = to3d_point(inPos);

	GLsizei dims[2];

	mTexture.AutoCreate();

	//if (!inApp.GetWand().MakeTestButtonTexture(mTexture.mTexture, dims[0], dims[1]))
	//	return false;

	//if (!MagicWand::MakeSliderFrameTexture(mTexture.mTexture, 100, dims[0], dims[1]))
	//	return false;
	
	//if (!inApp.GetWand().MakeButtonTexture(mTexture.mTexture, "BigEye ;)", 0, 12.0f, false, 10, 2, dims[0], dims[1]))
	//	return false;

	if (!inApp.GetWand().MakeTextTexture(mTexture.mTexture, MagicWand::TextInfo("BigEye floating font AA", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)),MagicWand::SizeConstraints(), dims[0], dims[1]))
		return false;

	//if (!inApp.GetWand().MakeTestTexture(mTexture.mTexture, 50, 180, dims[0], dims[1]))
	//	return false;
	

	mSize.x = (float) dims[0];
	mSize.y = (float) dims[1];

	return true;
}


void MagicWandTestTextureWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = inParentTransform * mPos;

	inApp.GetOGLStateManager().Enable(EOGLState_TextureWidget);
	{
		glBindTexture(GL_TEXTURE_2D, mTexture.mTexture);
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
,	mAutoChildren(false)
{
}


bool SimplePanelWidget::Create(const glm::vec2& inPos, const glm::vec2& inSize, MagicWand::FrameType inType, EOverflowSliderType inOverflowSliderType)
{
	mPos = to3d_point(inPos);
	mSize = inSize;
	mType = inType;
	mOverflowSliderType = inOverflowSliderType;
	mOverflowSpaceSize = 0.0f;
	
	return true;
}


void SimplePanelWidget::CreateTextures(const App& inApp)
{
	// TODO try colors like in : http://www.gameanim.com/2009/08/27/street-fighter-iv-facial-controls/
	if (!mTexture.IsCreated())
	{
		mTexture.AutoCreate();
		GLsizei dims[2];
		dims[0] = mSize.x;
		dims[1] = mSize.y;
		inApp.GetWand().MakeFrameTexture(mType, mTexture.mTexture, dims[0], dims[1], mScissorPos, mScissorSize);
		mSize.x = dims[0];
		mSize.y = dims[1];

		vert2d(mScissorPos) += 1.0f;
		vert2d(mScissorSize) -= 2.0f;
	}
}


void SimplePanelWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (!mTexture.IsCreated())
	{
		CreateTextures(inApp);
	}

	{
		SceneTransform local_transform;
		local_transform[2] = mPos;

		local_transform[2] += to3d_point(mOverflowPosOffset);
		mChildren.Update(inApp, inTimeSecs, inParentTransform, local_transform, inParentTransformDirty || false);
	}

	if (mOverflowSliderType == AutoOverflowSlider)
	{
		mOverflowSpaceSize = 0.0f;
		float max_local_pos_vertical = 0.0f;

		for (size_t i=0; i<mChildren.mChildWidgets.size(); ++i)
		{
			float local_pos_vertical = vert2d(to2d_point(mChildren.mChildWidgets[i]->GetLocalPosition(inApp))) + vert2d(mChildren.mChildWidgets[i]->GetSize(inApp));

			if (local_pos_vertical > max_local_pos_vertical)
				max_local_pos_vertical = local_pos_vertical;
		}

		mOverflowSliderType = max_local_pos_vertical > vert2d(mScissorSize) ? OverflowSlider : NoOverflowSlider;
		
		if (mOverflowSliderType == OverflowSlider)
			mOverflowSpaceSize = (max_local_pos_vertical - vert2d(mScissorSize)) + 4.0f;
	}

	if (mOverflowSliderType == OverflowSlider)
	{
		if (mAutoChildren.mChildWidgets.empty())
		{
			{
				mOverflowSlider.CreateVertical(glm::vec2(horiz2d(mSize) - 14.0f - 8.0f, 6.0f), glm::vec2(14.0f, vert2d(mSize)-24.0f));
				mOverflowSlider.SetSliderPos(0.0f);

				mAutoChildren.mChildWidgets.push_back(&mOverflowSlider);
			}
		}
	}

	{
		SceneTransform local_transform;
		local_transform[2] = mPos;

		mAutoChildren.Update(inApp, inTimeSecs, inParentTransform, local_transform, inParentTransformDirty || false);
	}

	vert2d(mOverflowPosOffset) = -mOverflowSpaceSize * mOverflowSlider.GetSliderPos();
}


void SimplePanelWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = inParentTransform * mPos;

	inApp.GetOGLStateManager().Enable(EOGLState_TextureWidget);
	
	{
		{
			glBindTexture(GL_TEXTURE_2D, mTexture.mTexture);
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


		inApp.PushScissor(to2d_point(world_pos) + mScissorPos, mScissorSize);
		{
			SceneTransform local_transform;
			local_transform[2] = mPos;
			
			mAutoChildren.Render(inApp, inTimeSecs, inParentTransform, local_transform, inParentTransformDirty || false);

			local_transform[2] += to3d_point(mOverflowPosOffset);
			mChildren.Render(inApp, inTimeSecs, inParentTransform, local_transform, inParentTransformDirty || false);
		}
		inApp.PopScissor();
	}
}


void OGLState_TextureWidget::Set()
{
	//glDisable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
}


void NativeWindowWidget::PushScissor(const glm::vec2& inPos, const glm::vec2& inSize) const
{
	mScissorStack.push(ScissorStackElement(inPos, inSize));

	if (mScissorStack.size() == 1)
	{
		glEnable(GL_SCISSOR_TEST);
	}

	const ScissorStackElement& top = mScissorStack.top();
	glScissor(top.mPosX, mViewportHeight-(top.mPosY+top.mHeight), top.mWidth, top.mHeight);
}


void NativeWindowWidget::PopScissor() const
{
	mScissorStack.pop();

	if (mScissorStack.empty())
	{
		glDisable(GL_SCISSOR_TEST);
	}
	else
	{
		const ScissorStackElement& top = mScissorStack.top();
		glScissor(top.mPosX, top.mPosY, top.mWidth, top.mHeight);
	}
}


void NativeWindowWidget::PushRenderToTexture(const glm::vec2& inPos, OGLRenderToTexture& inObject) const
{
	mRenderToTextureStack.push(RenderToTextureStackElement(inPos, inObject));

	const RenderToTextureStackElement& top = mRenderToTextureStack.top();
	top.mObject->BeginRender();
}

OGLRenderToTexture* NativeWindowWidget::PopRenderToTexture() const
{

	OGLRenderToTexture* ret = NULL;

	if (!mRenderToTextureStack.empty())
	{
		const RenderToTextureStackElement& top = mRenderToTextureStack.top();
		ret = top.mObject;
		top.mObject->EndRender();
	}
	
	mRenderToTextureStack.pop();

	if (mRenderToTextureStack.empty())
	{
		mApp->GetOGLStateManager().Enable(EOGLState_Reset);
	}

	return ret;
}

NativeWindowWidget::NativeWindowWidget()
:	mHWND(NULL)
,	mHDC(NULL)
,	mHRC(NULL)
,	mIsWNDCLASSRegistered(false)
,	mChildren(true)
,	mApp(NULL)
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
	mApp = &inApp;
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
		inApp.GetWand().LoadFont("media/DroidSans.ttf");

		inApp.GetOGLStateManager().StartBuild(EOGLState_Count);
		inApp.GetOGLStateManager().BuildSetState(new OGLState(), EOGLState_Reset);
		inApp.GetOGLStateManager().BuildSetState(new OGLState_NativeWindowWidget(*this), EOGLState_NativeWindowWidget);
		//inApp.GetOGLStateManager().BuildSetState(new OGLState_FontRender(mDefaultFont), EOGLState_FontRender, EOGLState_NativeWindowWidget);
		inApp.GetOGLStateManager().BuildSetState(new OGLState_TextureWidget(), EOGLState_TextureWidget, EOGLState_NativeWindowWidget);
		inApp.GetOGLStateManager().EndBuild();
	}

	Test(inApp);
	
	return true;
}


class TestScene : public SimpleRenderToTextureWidget::Scene
{
public:

	void Render(const App& inApp, SimpleRenderToTextureWidget& inParent, float inTimeSecs)	
	{
		glDisable(GL_BLEND);
		glDisable(GL_TEXTURE_2D);

		glClearDepth(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
		glLoadIdentity();									// Reset The Projection Matrix

		// Calculate The Aspect Ratio Of The Window
		gluPerspective(45.0f,(GLfloat)inParent.GetSize(inApp).x/(GLfloat)inParent.GetSize(inApp).y,0.1f,100.0f);

		glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
		glLoadIdentity();		

		glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
		glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

		float rtri = 16.0f * inTimeSecs;
		float rquad = 128.0f * -inTimeSecs;

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
	}
};
TestScene mTestScene;
SimpleRenderToTextureWidget* mTestWidget;

void NativeWindowWidget::Test(App& inApp)
{
	//GLenum err = glewInit();
	//GLuint mFrameBufferID;
	//glGenFramebuffersEXT(1, &mFrameBufferID);

	/*
	{
		MagicWandTestTextureWidget* widget = new MagicWandTestTextureWidget();
		widget->Create(inApp, glm::vec2(400.0f, 400.0f));
		mChildren.mChildWidgets.push_back(widget);
	}
	*/

	/*
	{
		SimpleTextureWidget* widget = new SimpleTextureWidget();
		//widget->Create(glm::vec2(200.0f, 100.0f), "media/tiny_test.bmp");
		widget->Create(inApp, glm::vec2(200.0f, 100.0f), "media/imagick_button.png");
		mChildren.mChildWidgets.push_back(widget);
	}
	*/

	/*
	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(glm::vec2(10.0f, 100.0f), glm::vec2(50.0f, 200.0f));
		mChildren.mChildWidgets.push_back(widget);
	}
	*/

	
	// Takes time to fill gradient
	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(glm::vec2(10.0f, 30.0f), glm::vec2(780.0f, 720.0f), MagicWand::FRAME_NORMAL_CUT_UPPER, SimplePanelWidget::NoOverflowSlider);
		mChildren.mChildWidgets.push_back(widget);
	}

#if 1
	{
		SimpleRenderToTextureWidget* widget = new SimpleRenderToTextureWidget();
		widget->Create(inApp, glm::vec2(10.0f, 30.0f), glm::vec2(640.0f, 480.0f));
		widget->SetScene(&mTestScene);
		mChildren.mChildWidgets.push_back(widget);
		mTestWidget = widget;
	}

#endif

	{
		SimpleTextWidget* text_widget = new SimpleTextWidget();
		text_widget->Create(inApp, glm::vec2(800.0f, 25.0f), MagicWand::TextInfo("Tools", 0, 14.0f, true, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
			
		//pos_vert += vert2d(text_widget->GetSize(inApp)) + 2.0f * height_offset;

		mChildren.mChildWidgets.push_back(text_widget);
	}

	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(glm::vec2(800.0f, 55.0f), glm::vec2(200.0f, 250.0f), MagicWand::FRAME_NORMAL, SimplePanelWidget::AutoOverflowSlider);
		mChildren.mChildWidgets.push_back(widget);

		ChildWidgetContainer& children = widget->GetChildren();

		MagicWand::SizeConstraints sizeConstraints;
		horiz2d(sizeConstraints.mMinSize) = 150.0f;
		horiz2d(sizeConstraints.mMaxSize) = 150.0f;
		
		float pos_vert = 6.0f;
		float height_offset = 6.0f;

		{
			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(glm::vec2(8.0f, pos_vert), glm::vec2(150.0f, 0.0f), MagicWand::TextInfo("Slide", 0, 12.0f, false, glm::vec2(2.0f, 2.0f)), sizeConstraints);
			slider_widget->SetSliderPos(0.3f);
			pos_vert += vert2d(slider_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(slider_widget);
		}

		{
			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(glm::vec2(8.0f, pos_vert), glm::vec2(150.0f, 0.0f), MagicWand::TextInfo("Bold Slide", 0, 12.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);
			slider_widget->SetSliderPos(0.6f);
			
			pos_vert += vert2d(slider_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(slider_widget);
		}

		pos_vert += 4.0f;

		{
			MagicWand::SizeConstraints halfSizeConstraints = sizeConstraints;

			horiz2d(halfSizeConstraints.mMaxSize) = (horiz2d(halfSizeConstraints.mMaxSize) / 2.0f) - 4.0f;
			horiz2d(halfSizeConstraints.mMinSize) = horiz2d(halfSizeConstraints.mMaxSize);

			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), false, MagicWand::TextInfo("Big eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), halfSizeConstraints);
			
			children.mChildWidgets.push_back(button_widget);

			{
				SimpleButtonWidget* button_widget = new SimpleButtonWidget();
				button_widget->Create(glm::vec2(horiz2d(halfSizeConstraints.mMinSize) + 6.0f + 8.0f, pos_vert), false, MagicWand::TextInfo("Bold eye ;)", 0, 12.0f, true, glm::vec2(10.0f, 2.0f)), halfSizeConstraints);
			
				children.mChildWidgets.push_back(button_widget);
			}

			pos_vert += vert2d(button_widget->GetSize(inApp)) + height_offset;
		}

		

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), false, MagicWand::TextInfo("Bigger eye ;)", 0, 16.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
	

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Toggled eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			button_widget->SetIsToggled(true);
			
			pos_vert += vert2d(button_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Untoggled eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}


#if 0
		for (int i=0; i<10; ++i)
		{
			std::string filler_name;
			strtk::construct(filler_name, "", "Filler ", i);

			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo(filler_name.c_str(), 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
#endif

#if 1
		{
			pos_vert += 100.0f;

			int i = 66;

			std::string filler_name;
			strtk::construct(filler_name, "", "Filler ", i);

			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo(filler_name.c_str(), 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
#endif


		/*
		{
			MagicWand::SizeConstraints sizeConstraints;
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Scissorrrrrrrrrrrrrrrrrrrrr testtttttttt", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize(inApp)) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
		*/
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
	GLint viewport_width = mParent.GetViewportWidth();
	GLint viewport_height = mParent.GetViewportHeight();

	glViewport(0, 0, (GLsizei)(viewport_width), (GLsizei)(viewport_height));
	//glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glOrtho(0.0f, window_width, window_height, 0.0f, -1.0f, 1.0f);
	glOrtho(0.0f, viewport_width, viewport_height, 0.0f, -1.0f, 1.0f);

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);

	// For 2D pixel precise mode
	//glTranslatef (0.375f, 0.375f, 0.0f);
	//glTranslatef (0.5f, 0.5f, 0.0f);

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	//PushScissor(glm::vec2(0.0f, 0.0f), glm::vec2(window_width, window_height));
}


void NativeWindowWidget::Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) 
{
	RECT rect;
	GetClientRect(GetHWND(), &rect);							

	mViewportWidth = rect.right-rect.left;							
	mViewportHeight = rect.bottom-rect.top;		

	mChildren.Update(inApp, inTimeSecs, inParentTransform, kIdentitySceneTransform, inParentTransformDirty || false);
}


void NativeWindowWidget::Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) 
{
	inApp.GetOGLStateManager().Enable(EOGLState_NativeWindowWidget);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f);
	//glClearColor(176.0f/255.0f, 176.0f/255.0f, 176.0f/255.0f, 1.0f);
	//glClearColor(49.0f/255.0f, 140.0f/255.0f, 231.0f / 255.0f, 1.0f);
	//glClearColor(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f);
	
	//glClearColor(100.0f/255.0f, 149.0f/255.0f, 237.0f / 255.0f, 1.0f);
	//glClearColor(75.0f/255.0f, 146.0f/255.0f, 219.0f / 255.0f, 1.0f);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	//mTestScene.Render(inApp, *mTestWidget, inTimeSecs);
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
	mMouseMoved = false;
	mLastMouseLeft = false;
	mMouseLeft = false;
	mMouseLeftChanged = false;
	mLastMouseMiddle = false;
	mMouseMiddle = false;
	mMouseMiddleChanged = false;
	mLastMouseRight = false;
	mMouseRight = false;
	mMouseRightChanged = false;
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

float App::GetInputState(int inInputID) const
{
	EInputID input_ID = (EInputID) inInputID;

	if (input_ID > INPUT_MOUSE_START && input_ID < INPUT_MOUSE_END)
	{
		switch (input_ID)
		{
			case INPUT_MOUSE_LEFT:
			{
				return mMouseLeft;
			}
			break;

			case INPUT_MOUSE_LEFT_CHANGED:
			{
				return mMouseLeftChanged;
			}
			break;

			case INPUT_MOUSE_MIDDLE:
			{
				return mMouseMiddle;
			}
			break;

			case INPUT_MOUSE_MIDDLE_CHANGED:
			{
				return mMouseMiddleChanged;
			}
			break;

			case INPUT_MOUSE_RIGHT:
			{
				return mMouseRight;
			}
			break;

			case INPUT_MOUSE_RIGHT_CHANGED:
			{
				return mMouseRightChanged;
			}
			break;

			case INPUT_MOUSE_X:
			{
				return mMousePos.x;
			}
			break;

			case INPUT_MOUSE_Y:
			{
				return mMousePos.y;
			}
			break;

			case INPUT_MOUSE_MOVED:
			{
				return mMouseMoved ? 1.0f : 0.0f;
			}
			break;
		}
	}

	return 0.0f;
}

const glm::vec2& App::GetMousePos() const
{
	return mMousePos;
}

int App::GetInputEventCount() const
{
	return 0;
}

const InputEventInfo& App::GetInputEvent(int inIndex) const
{
	static InputEventInfo info;

	return info;
}


void App::ConsumeInputEvents() 
{
}

void App::PrepareInputForUpdate()
{
	POINT point;
	GetCursorPos(&point);
	ScreenToClient(mWindow->GetHWND(), &point);

	mLastMousePos = mMousePos;
	mMousePos.x = point.x;
	mMousePos.y = point.y;

	mMouseMoved = !glm::areSimilar(mMousePos, mLastMousePos, 0.0f);

	mLastMouseLeft = mMouseLeft;
	mMouseLeft = GetKeyState(VK_LBUTTON) < 0;
	mMouseLeftChanged = mMouseLeft != mLastMouseLeft;

	mLastMouseMiddle = mMouseMiddle;
	mMouseMiddle = GetKeyState(VK_LBUTTON) < 0;
	mMouseMiddleChanged = mMouseMiddle != mLastMouseMiddle;

	mLastMouseRight = mMouseRight;
	mMouseRight = GetKeyState(VK_LBUTTON) < 0;
	mMouseRightChanged = mMouseRight != mLastMouseRight;
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

		PrepareInputForUpdate();
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
