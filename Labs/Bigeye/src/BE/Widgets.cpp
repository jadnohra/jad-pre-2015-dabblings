#include "Widgets.h"
#include "ARB_Multisample.h"
#include "MainWindow.h"
#include <algorithm>
#include "strtk.hpp"

namespace BE
{

glm::vec3 gWidgetTransform(const SceneTransform& inTransform, const glm::vec3& inPos)
{
	glm::vec4 result = inTransform * glm::vec4(inPos.x, inPos.y, inPos.z, 1.0f);

	return glm::vec3(result.x, result.y, result.z);
}

SceneTransform gWidgetTranslation(const glm::vec3& inPos)
{
	SceneTransform transform;
	transform[3] = glm::vec4(inPos.x, inPos.y, inPos.z, 1.0f);

	return transform;
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


void ChildWidgetContainer::Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty)
{
	SceneTransform parent_world_tfm = inParentTransform * inParentLocalTransform;

	for (size_t i=0; i<mChildWidgets.size(); ++i)
		mChildWidgets[i]->Update(inContext, parent_world_tfm, inParentTransformDirty);
}


void ChildWidgetContainer::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty)
{
	SceneTransform parent_world_tfm = inParentTransform * inParentLocalTransform;

	for (size_t i=0; i<mChildWidgets.size(); ++i)
		mChildWidgets[i]->RenderBuild(inContext, parent_world_tfm, inParentTransformDirty);
}


bool SimpleButtonWidget::Create(const WidgetContext& inContext, const glm::vec2& inPos, bool inIsToggleButton, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints)
{
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;
	mTextInfo = inTextInfo;
	mSizeConstraints = inSizeConstraints;
	mIsHighlighted = false;
	mIsPressed = false;
	mIsMousePressed = false;
	mIsToggleButton = inIsToggleButton;
	mIsToggled = false;

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 1;
	mRenderState.blend_func_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA = 1;
	CreateTextures(inContext);

	return true;
}

void SimpleButtonWidget::SetIsToggled(bool inIsToggled)
{
	mIsToggled = inIsToggled;
}

void SimpleButtonWidget::CreateTextures(const WidgetContext& inContext)
{
	if (!mButtonTexture.IsCreated())
	{
		mButtonTexture.AutoCreate();
		mPressedButtonTexture.AutoCreate();
		mHighlightedButtonTexture.AutoCreate();

		GLsizei tex_dims[2];

		inContext.mMainWindow.GetWand().MakeButtonTexture(mButtonTexture.mTexture, MagicWand::WIDGET_NORMAL, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
		mButtonTexSize[0] = (float) tex_dims[0];
		mButtonTexSize[1] = (float) tex_dims[1];

		
		inContext.mMainWindow.GetWand().MakeButtonTexture(mPressedButtonTexture.mTexture, MagicWand::WIDGET_PRESSED, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
		inContext.mMainWindow.GetWand().MakeButtonTexture(mHighlightedButtonTexture.mTexture, MagicWand::WIDGET_HIGHLIGHTED, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
	}
}

bool WidgetUtil::IsMouseInRectangle(const WidgetContext& inContext, const glm::vec3& inWidgetWorldPos, const glm::vec2& inWidgetSize)
{
	return IsMouseInRectangle(inContext, to2d_point(inWidgetWorldPos), inWidgetSize);
}

bool WidgetUtil::IsMouseInRectangle(const WidgetContext& inContext, const glm::vec2& inWidgetWorldPos, const glm::vec2& inWidgetSize)
{
	glm::vec2 local_mouse_pos = inContext.mMainWindow.GetMousePos() - inWidgetWorldPos;

	if (local_mouse_pos.x < 0.0f || local_mouse_pos.y < 0.0f)
		return false;

	glm::vec2 local_mouse_pos_in_rect = inWidgetSize - local_mouse_pos;
	
	return (local_mouse_pos_in_rect.x > 0.0f && local_mouse_pos_in_rect.y > 0.0f);
}


void SimpleButtonWidget::Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = gWidgetTransform(inParentTransform, mPos);

	bool was_mouse_pressed = mIsMousePressed;

	if (WidgetUtil::IsMouseInRectangle(inContext, to2d_point(world_pos), mButtonTexSize))
	{
		mIsHighlighted = true;

		if (inContext.mMainWindow.GetInputState(INPUT_MOUSE_LEFT) > 0.0f)
			mIsMousePressed = true;
		else
			mIsMousePressed = false;
	}
	else
	{
		mIsMousePressed = false;
		mIsHighlighted = false;
	}

	if (was_mouse_pressed && !mIsMousePressed)
		inContext.mMainWindow.GetWidgetEventManager().AddEvent(WidgetEvent(this, NULL));

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

void SimpleButtonWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);

	inContext.mRenderTreeBuilder.BranchUp(0, *this);
	inContext.mRenderTreeBuilder.BranchDown(0);
}


void SimpleButtonWidget::Render(Renderer& inRenderer)
{
	glm::vec3& world_pos = mRenderWorldPos;
	
	bool is_pressed = mIsMousePressed || (mIsToggleButton && mIsToggled);
	GLuint tex = is_pressed ? mPressedButtonTexture.mTexture : (mIsHighlighted ? mHighlightedButtonTexture.mTexture : mButtonTexture.mTexture);
	const glm::vec2& tex_size = mButtonTexSize;

	mRenderState.Apply(inRenderer);
	{
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex3f(world_pos.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,0.0f); glVertex3f(world_pos.x+tex_size.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,1.0f); glVertex3f(world_pos.x+tex_size.x,world_pos.y+tex_size.y,world_pos.z);
			 glTexCoord2f(0.0f,1.0f); glVertex3f(world_pos.x,world_pos.y+tex_size.y,world_pos.z);
		glEnd();
	}
}


bool SimpleSliderWidget::Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints)
{
	mType = Horizontal;
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;
	mSize = inSize;

	mTextInfo = inTextInfo;
	mSizeConstraints = inSizeConstraints;

	mHasMouseSliderFocus = false;
	mIsHighlighted = false;
	mSliderPos = 0.5f;
	
	CreateTextures(inContext);

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 1;
	mRenderState.blend_func_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA = 1;

	return true;
}


bool SimpleSliderWidget::CreateVertical(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize)
{
	mType = Vertical;
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;
	mSize = inSize;

	mHasMouseSliderFocus = false;
	mIsHighlighted = false;
	mSliderPos = 0.5f;

	CreateTextures(inContext);

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 1;
	mRenderState.blend_func_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA = 1;

	return true;
}


void SimpleSliderWidget::CreateTextures(const WidgetContext& inContext)
{
	if (!mFrameTexture.IsCreated())
	{
		mFrameTexture.AutoCreate();
		mMarkerTexture.AutoCreate();
		mPressedMarkerTexture.AutoCreate();
		mHighlightedMarkerTexture.AutoCreate();

		GLsizei tex_dims[2];

		if (mType == Horizontal)
			inContext.mMainWindow.GetWand().MakeSliderFrameTexture(mFrameTexture.mTexture, (GLsizei) mSize.x, mTextInfo, mSizeConstraints, tex_dims[0], tex_dims[1]);
		else
			inContext.mMainWindow.GetWand().MakeVerticalSliderFrameTexture(mFrameTexture.mTexture, (GLsizei) mSize.x, (GLsizei) mSize.y, tex_dims[0], tex_dims[1]);

		mFrameTexSize[0] = (float) tex_dims[0];
		mFrameTexSize[1] = (float) tex_dims[1];

		if (mType == Horizontal)
			inContext.mMainWindow.GetWand().MakeSliderMarkerTexture(mMarkerTexture.mTexture, MagicWand::WIDGET_NORMAL, tex_dims[1], tex_dims[0], tex_dims[1]);
		else
			inContext.mMainWindow.GetWand().MakeVerticalSliderMarkerTexture(mMarkerTexture.mTexture, MagicWand::WIDGET_NORMAL, tex_dims[0], tex_dims[0], tex_dims[1]);

		mMarkerTexSize[0] = (float) tex_dims[0];
		mMarkerTexSize[1] = (float) tex_dims[1];

		if (mType == Horizontal)
			inContext.mMainWindow.GetWand().MakeSliderMarkerTexture(mPressedMarkerTexture.mTexture, MagicWand::WIDGET_PRESSED, tex_dims[1], tex_dims[0], tex_dims[1]);
		else
			inContext.mMainWindow.GetWand().MakeVerticalSliderMarkerTexture(mPressedMarkerTexture.mTexture, MagicWand::WIDGET_PRESSED, tex_dims[0], tex_dims[0], tex_dims[1]);
	
		if (mType == Horizontal)
			inContext.mMainWindow.GetWand().MakeSliderMarkerTexture(mHighlightedMarkerTexture.mTexture, MagicWand::WIDGET_HIGHLIGHTED, tex_dims[1], tex_dims[0], tex_dims[1]);
		else
			inContext.mMainWindow.GetWand().MakeVerticalSliderMarkerTexture(mHighlightedMarkerTexture.mTexture, MagicWand::WIDGET_HIGHLIGHTED, tex_dims[0], tex_dims[0], tex_dims[1]);
	}
}


void SimpleSliderWidget::Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	glm::vec3 world_pos = gWidgetTransform(inParentTransform, mPos);

	glm::vec3 marker_world_pos = GetSliderWorldPos(world_pos);
	bool is_mouse_left_down = (inContext.mMainWindow.GetInputState(INPUT_MOUSE_LEFT) > 0.0f);
	bool is_mouse_in_rect = WidgetUtil::IsMouseInRectangle(inContext, world_pos, mFrameTexSize);
	bool has_mouse_slider_focus = false;

	if (is_mouse_left_down)
	{
		if (WidgetUtil::IsMouseInRectangle(inContext, marker_world_pos, mMarkerTexSize))
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
		glm::vec2 mouse_pos = inContext.mMainWindow.GetMousePos();

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


glm::vec3 SimpleSliderWidget::GetSliderWorldPos(const glm::vec3& inWorldPos) const
{
	glm::vec2 marker_world_pos = to2d_point(inWorldPos);

	if (mType == Horizontal)
		horiz2d(marker_world_pos) += 2 + mSliderPos * (horiz2d(mFrameTexSize) - (horiz2d(mMarkerTexSize)+4));
	else
	{
		horiz2d(marker_world_pos) += 0.5f * (horiz2d(mFrameTexSize) - (horiz2d(mMarkerTexSize)));
		vert2d(marker_world_pos) += 2 + mSliderPos * (vert2d(mFrameTexSize) - (vert2d(mMarkerTexSize)+4));
	}

	glm::vec3 marker_world_pos_3d = to3d_point(marker_world_pos);
	marker_world_pos_3d.z = inWorldPos.z + 0.01f;

	return marker_world_pos_3d;
}



void SimpleSliderWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);

	inContext.mRenderTreeBuilder.BranchUp(0, *this);
	inContext.mRenderTreeBuilder.BranchDown(0);
}


void SimpleSliderWidget::Render(Renderer& inRenderer)
{
	glm::vec3 world_pos = mRenderWorldPos;

	glm::vec3 frame_world_pos = world_pos;
	glm::vec3 marker_world_pos = GetSliderWorldPos(world_pos);

	mRenderState.Apply(inRenderer);
	{
		glBindTexture(GL_TEXTURE_2D, mFrameTexture.mTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex3f(frame_world_pos.x,frame_world_pos.y,frame_world_pos.z);
			 glTexCoord2f(1.0f,0.0f); glVertex3f(frame_world_pos.x+mFrameTexSize.x,frame_world_pos.y,frame_world_pos.z);
			 glTexCoord2f(1.0f,1.0f); glVertex3f(frame_world_pos.x+mFrameTexSize.x,frame_world_pos.y+mFrameTexSize.y,frame_world_pos.z);
			 glTexCoord2f(0.0f,1.0f); glVertex3f(frame_world_pos.x,frame_world_pos.y+mFrameTexSize.y,frame_world_pos.z);
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
			 glTexCoord2f(0.0f,0.0f); glVertex3f(marker_world_pos.x,marker_world_pos.y,marker_world_pos.z);
			 glTexCoord2f(1.0f,0.0f); glVertex3f(marker_world_pos.x+mMarkerTexSize.x,marker_world_pos.y,marker_world_pos.z);
			 glTexCoord2f(1.0f,1.0f); glVertex3f(marker_world_pos.x+mMarkerTexSize.x,marker_world_pos.y+mMarkerTexSize.y,marker_world_pos.z);
			 glTexCoord2f(0.0f,1.0f); glVertex3f(marker_world_pos.x,marker_world_pos.y+mMarkerTexSize.y,marker_world_pos.z);
		glEnd();
	}
}

bool SimpleTextWidget::Create(const WidgetContext& inContext, const glm::vec2& inPos, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints)
{
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;

	mTextTexture.AutoCreate();
	GLsizei tex_dims[2];
	inContext.mMainWindow.GetWand().MakeTextTexture(mTextTexture.mTexture, inTextInfo, inSizeConstraints, tex_dims[0], tex_dims[1]);
	mTextTexSize[0] = (float) tex_dims[0];
	mTextTexSize[1] = (float) tex_dims[1];

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 1;
	mRenderState.blend_func_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA = 1;

	return true;
}


void SimpleTextWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);

	inContext.mRenderTreeBuilder.BranchUp(0, *this);
	inContext.mRenderTreeBuilder.BranchDown(0);
}


void SimpleTextWidget::Render(Renderer& inRenderer)
{
	glm::vec3& world_pos = mRenderWorldPos;
	
	mRenderState.Apply(inRenderer);
	{
		glBindTexture(GL_TEXTURE_2D, mTextTexture.mTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex3f(world_pos.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,0.0f); glVertex3f(world_pos.x+mTextTexSize.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,1.0f); glVertex3f(world_pos.x+mTextTexSize.x,world_pos.y+mTextTexSize.y,world_pos.z);
			 glTexCoord2f(0.0f,1.0f); glVertex3f(world_pos.x,world_pos.y+mTextTexSize.y,world_pos.z);
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

bool SimpleTextureWidget::Create(const WidgetContext& inContext, const glm::vec2& inPos, const char* inTexturePath)
{
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;
	mTexture = 0;

	GLsizei dims[2];

	if (!inContext.mMainWindow.GetWand().ReadImageToGLTexture(inTexturePath, mTexture, dims[0], dims[1]))
		return false;

	mSize.x = (float) dims[0];
	mSize.y = (float) dims[1];

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 1;
	mRenderState.blend_func_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA = 1;

	return true;
}



void SimpleTextureWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);

	inContext.mRenderTreeBuilder.BranchUp(0, *this);
	inContext.mRenderTreeBuilder.BranchDown(0);
}


void SimpleTextureWidget::Render(Renderer& inRenderer)
{
	glm::vec3& world_pos = mRenderWorldPos;
	
	mRenderState.Apply(inRenderer);
	{
		glBindTexture(GL_TEXTURE_2D, mTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex3f(world_pos.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,0.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,1.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y+mSize.y,world_pos.z);
			 glTexCoord2f(0.0f,1.0f); glVertex3f(world_pos.x,world_pos.y+mSize.y,world_pos.z);
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

bool SimpleRenderToTextureWidget::Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize, int inMultiSampleCount)
{
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;

	if (!mTexture.Create((GLsizei) inSize.x, (GLsizei) inSize.y, inMultiSampleCount))
		return false;

	mSize = inSize;

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 0;

	return true;
}


void SimpleRenderToTextureWidget::Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (mScene)
	{
		mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);
		mScene->Update(inContext, *this, mTexture);
	}
}


void SimpleRenderToTextureWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (mScene)
	{
		mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);

		mRenderNodeDependency.mTrees.clear();

		inContext.mRenderTreeBuilder.BranchUp(0, *this);
		inContext.mRenderTreeBuilder.BranchDown(0);

		int render_to_texture_tree = inContext.mRenderTreeBuilder.BranchUpNewTree(mScene, false, this);
		inContext.mRenderTreeBuilder.BranchDown(render_to_texture_tree);
	}
}


void SimpleRenderToTextureWidget::Render(Renderer& inRenderer)
{
	glm::vec3& world_pos = mRenderWorldPos;
	
	mRenderState.Apply(inRenderer);
	{
		mTexture.Bind();
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 // here we invert the coordinates because the texture is stored bottom up (??not sure??)
			 glTexCoord2f(0.0f,1.0f); glVertex3f(world_pos.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,1.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,0.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y+mSize.y,world_pos.z);
			 glTexCoord2f(0.0f,0.0f); glVertex3f(world_pos.x,world_pos.y+mSize.y,world_pos.z);
		glEnd();
	}
}


bool SimpleRenderToTextureWidget::IsMainWindowPosInViewport(const WidgetContext& inContext, const glm::vec2& inMainWindowPos, glm::vec2& outViewportPos)
{
	outViewportPos = to2d_point(to3d_point(inMainWindowPos) - mRenderWorldPos);
	return WidgetUtil::IsMouseInRectangle(inContext, mRenderWorldPos, mSize);
}


MagicWandTestTextureWidget::MagicWandTestTextureWidget()
{
}


MagicWandTestTextureWidget::~MagicWandTestTextureWidget()
{
}


bool MagicWandTestTextureWidget::Create(const WidgetContext& inContext, const glm::vec2& inPos)
{
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;

	GLsizei dims[2];

	mTexture.AutoCreate();

	//if (!inContext.mMainWindow.GetWand().MakeTestButtonTexture(mTexture.mTexture, dims[0], dims[1]))
	//	return false;

	//if (!MagicWand::MakeSliderFrameTexture(mTexture.mTexture, 100, dims[0], dims[1]))
	//	return false;
	
	//if (!inContext.mMainWindow.GetWand().MakeButtonTexture(mTexture.mTexture, "Bigeye ;)", 0, 12.0f, false, 10, 2, dims[0], dims[1]))
	//	return false;

	if (!inContext.mMainWindow.GetWand().MakeTextTexture(mTexture.mTexture, MagicWand::TextInfo("Bigeye floating font AA", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)),MagicWand::SizeConstraints(), dims[0], dims[1]))
		return false;

	//if (!inContext.mMainWindow.GetWand().MakeTestTexture(mTexture.mTexture, 50, 180, dims[0], dims[1]))
	//	return false;
	

	mSize.x = (float) dims[0];
	mSize.y = (float) dims[1];

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 1;
	mRenderState.blend_func_GL_SRC_ALPHA_GL_ONE_MINUS_SRC_ALPHA = 1;

	return true;
}


void MagicWandTestTextureWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);

	inContext.mRenderTreeBuilder.BranchUp(0, *this);
	inContext.mRenderTreeBuilder.BranchDown(0);
}

void MagicWandTestTextureWidget::Render(Renderer& inRenderer)
{
	glm::vec3& world_pos = mRenderWorldPos;
	
	mRenderState.Apply(inRenderer);
	{
		glBindTexture(GL_TEXTURE_2D, mTexture.mTexture);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
		
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		//glColor4f(1.0f,1.0f,1.0f,1.0f);			

		glBegin(GL_QUADS);
			 glTexCoord2f(0.0f,0.0f); glVertex3f(world_pos.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,0.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y,world_pos.z);
			 glTexCoord2f(1.0f,1.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y+mSize.y,world_pos.z);
			 glTexCoord2f(0.0f,1.0f); glVertex3f(world_pos.x,world_pos.y+mSize.y,world_pos.z);
		glEnd();
	}
}


SimplePanelWidget::SimplePanelWidget()
:	mChildren(true)
,	mAutoChildren(false)
,	mIsVisible(true)
{
}


bool SimplePanelWidget::Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize, MagicWand::FrameType inType, EOverflowSliderType inOverflowSliderType, bool inUseGradient)
{
	mPos = to3d_point(inPos);
	mPos.z = 0.01f;
	mSize = inSize;
	mType = inType;
	mOverflowSliderType = inOverflowSliderType;
	mOverflowSpaceSize = 0.0f;
	
	CreateTextures(inContext, inUseGradient);

	mRenderState.enable_depth = 1;
	mRenderState.enable_texture = 1;
	mRenderState.enable_blend = 1;

	return true;
}


void SimplePanelWidget::CreateTextures(const WidgetContext& inContext, bool inUseGradient)
{
	if (!mTexture.IsCreated())
	{
		mTexture.AutoCreate();
		GLsizei dims[2];
		dims[0] = (GLsizei) mSize.x;
		dims[1] = (GLsizei) mSize.y;
		inContext.mMainWindow.GetWand().MakeFrameTexture(mType, inUseGradient, mTexture.mTexture, dims[0], dims[1], mScissorPos, mScissorSize);
		mSize.x = (float) dims[0];
		mSize.y = (float) dims[1];
	}
}


void SimplePanelWidget::Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (!mIsVisible)
		return;

	{
		SceneTransform local_transform = gWidgetTranslation(mPos + to3d_point(mOverflowPosOffset));
		mChildren.Update(inContext, inParentTransform, local_transform, inParentTransformDirty || false);
	}

	if (mOverflowSliderType == AutoOverflowSlider)
	{
		mOverflowSpaceSize = 0.0f;
		float max_local_pos_vertical = 0.0f;

		for (size_t i=0; i<mChildren.mChildWidgets.size(); ++i)
		{
			float local_pos_vertical = vert2d(to2d_point(mChildren.mChildWidgets[i]->GetLocalPosition())) + vert2d(mChildren.mChildWidgets[i]->GetSize());

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
				mOverflowSlider.CreateVertical(inContext, glm::vec2(horiz2d(mSize) - 14.0f - 8.0f, 6.0f), glm::vec2(14.0f, vert2d(mSize)-24.0f));
				mOverflowSlider.SetSliderPos(0.0f);

				mAutoChildren.mChildWidgets.push_back(&mOverflowSlider);
			}
		}
	}

	{
		SceneTransform local_transform = gWidgetTranslation(mPos);

		mAutoChildren.Update(inContext, inParentTransform, local_transform, inParentTransformDirty || false);
	}

	vert2d(mOverflowPosOffset) = -mOverflowSpaceSize * mOverflowSlider.GetSliderPos();
}



void SimplePanelWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	if (!mIsVisible)
		return;

	mRenderWorldPos = gWidgetTransform(inParentTransform, mPos);

	inContext.mRenderTreeBuilder.BranchUp(0, *this);
	{

		mPushScissorNode.mPushPopStack = &inContext.mRenderTreeBuilder.GetScissorPushPopNodeStack(0);
		mPushScissorNode.mScissorInfo = PushScissorNode::ScissorInfo(to2d_point(mRenderWorldPos) + mScissorPos, mScissorSize);

		inContext.mRenderTreeBuilder.BranchUp(0, mPushScissorNode);
		{
		
			SceneTransform local_transform = gWidgetTranslation(mPos);
			mAutoChildren.RenderBuild(inContext, inParentTransform, local_transform, inParentTransformDirty || false);

			SceneTransform overflow_local_transform = gWidgetTranslation(mPos + to3d_point(mOverflowPosOffset));
			mChildren.RenderBuild(inContext, inParentTransform, overflow_local_transform, inParentTransformDirty || false);
		}
		inContext.mRenderTreeBuilder.BranchDown(0);

		mPopScissorNode.mPushPopStack = mPushScissorNode.mPushPopStack;
		inContext.mRenderTreeBuilder.BranchUp(0, mPopScissorNode);
		inContext.mRenderTreeBuilder.BranchDown(0);
	}
	inContext.mRenderTreeBuilder.BranchDown(0);

}


void SimplePanelWidget::Render(Renderer& inRenderer)
{
	glm::vec3 world_pos = mRenderWorldPos;

	mRenderState.Apply(inRenderer);
	{
		{
			glBindTexture(GL_TEXTURE_2D, mTexture.mTexture);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);	// Linear Filtering
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);	// Linear Filtering
			
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
			//glColor4f(1.0f,1.0f,1.0f,1.0f);			

			glBegin(GL_QUADS);
				 glTexCoord2f(0.0f,0.0f); glVertex3f(world_pos.x,world_pos.y,world_pos.z);
				 glTexCoord2f(1.0f,0.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y,world_pos.z);
				 glTexCoord2f(1.0f,1.0f); glVertex3f(world_pos.x+mSize.x,world_pos.y+mSize.y,world_pos.z);
				 glTexCoord2f(0.0f,1.0f); glVertex3f(world_pos.x,world_pos.y+mSize.y,world_pos.z);
			glEnd();
		}
	}
}


NativeWindowWidget::NativeWindowWidget()
:	mHWND(NULL)
,	mHDC(NULL)
,	mHRC(NULL)
,	mIsWNDCLASSRegistered(false)
,	mChildren(true)
,	mMainWindow(NULL)
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
	
	if (pTHIS != NULL)
		return pTHIS->WindowProc(hWnd, uMsg, wParam, lParam);
	else
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
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

		case WM_DROPFILES:
		{
			if (mMainWindow && mMainWindow->GetClient())
			{
				HDROP fDrop = (HDROP) wParam;

				int dropped_file_count = DragQueryFileA(fDrop, 0xFFFFFFFF, NULL, 0);
				char* fName = NULL;
				int buffer_size = 0;

				mMainWindow->GetClient()->OnFilesDropped(mMainWindow, dropped_file_count);
				
				for (int i=0; i<dropped_file_count; ++i)
				{
					UINT nBufSize = 1 + DragQueryFileA(fDrop, i, NULL, 0);

					if (nBufSize > (UINT) buffer_size)
					{
						buffer_size = nBufSize;
						delete fName;
						fName = new char[buffer_size];
					}

					DragQueryFileA(fDrop, i, fName, buffer_size);
					mMainWindow->GetClient()->OnFileDropped(mMainWindow, fName);
				}

				delete fName;
				DragFinish(fDrop);
			}

			return 0;
		}

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


bool NativeWindowWidget::Create(const WidgetContext& inContext, MainWindow& inApp, const WideString& inWindowName, int inWidth, int inHeight, bool inTitleBar)
{
	mMainWindow = &inApp;
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
if (inTitleBar)
	AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	
	// Register A Window Class
	if (!mIsWNDCLASSRegistered)
	{
		WNDCLASSEX windowClass;											// Window Class
		ZeroMemory (&windowClass, sizeof (WNDCLASSEXA));					// Make Sure Memory Is Cleared
		windowClass.cbSize			= sizeof (WNDCLASSEXA);					// Size Of The windowClass Structure
		windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
		windowClass.lpfnWndProc		= NativeWindowWidget::WindowProcProxy;	// WindowProc Handles Messages
		windowClass.hInstance		= inContext.mMainWindow.GetHINSTANCE();					// Set The Instance
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
								   inContext.mMainWindow.GetHINSTANCE(), // Pass The Window Instance
								   this);

	if (mHWND == 0)
		return false;
	
	SetWindowLongPtr(mHWND, GWL_USERDATA, (LONG) this);
	if (inApp.GetClient() && inApp.GetClient()->SupportsDragAndDrop())
		DragAcceptFiles(mHWND, TRUE);

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
		if(InitMultisample(inContext.mMainWindow.GetHINSTANCE(), mHWND, pfd))
		{
			
			Destroy();
			return Create(inContext, inApp, inWindowName, inWidth, inHeight, inTitleBar);
		}
	}
		

	ShowWindow (mHWND, SW_NORMAL);	

	if (!inTitleBar)
	{
		DWORD dwStyle = GetWindowLong(mHWND, GWL_STYLE);
		dwStyle &= ~(WS_CAPTION|WS_SIZEBOX);
		SetWindowLong(mHWND, GWL_STYLE, dwStyle);
	}


	//ReshapeGL (window->init.width, window->init.height);				// Reshape Our GL Window

	{
		//mMainWindow->GetWand().LoadFont("D:/jad/Dev/AiGameDev/Bigfoot/bin/media/test.ttf");
		//mMainWindow->GetWand().LoadFont("media/test.ttf");
		mMainWindow->GetWand().LoadFont("media/DroidSans.ttf", "media/DroidSans-Bold.ttf");
		//mMainWindow->GetWand().LoadFont("media/DroidSans-Bold.ttf");
	}

	// Test(inContext);
	
	return true;
}


class TestScene : public SimpleRenderToTextureWidget::Scene
{
public:

	float mRenderTime;
	glm::vec2 mSize;
	OGLRenderToTexture* mTexture;

	virtual void Update(const WidgetContext& inContext, SimpleRenderToTextureWidget& inParent, OGLRenderToTexture& inTexture)	
	{
		mSize.x = inParent.GetSize().x;
		mSize.y = inParent.GetSize().y;
		mRenderTime = inContext.mTimeSecs;
		mTexture = &inTexture;
	}

	void Render(Renderer& inRenderer)	
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
};
TestScene mTestScene;
SimpleRenderToTextureWidget* mTestWidget;

void NativeWindowWidget::Test(const WidgetContext& inContext)
{
	/*
	{
		MagicWandTestTextureWidget* widget = new MagicWandTestTextureWidget();
		widget->Create(inContext, glm::vec2(400.0f, 400.0f));
		mChildren.mChildWidgets.push_back(widget);
	}
	*/

	/*
	{
		SimpleTextureWidget* widget = new SimpleTextureWidget();
		//widget->Create(glm::vec2(200.0f, 100.0f), "media/tiny_test.bmp");
		widget->Create(inContext, glm::vec2(200.0f, 100.0f), "media/imagick_button.png");
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
#if 1
	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(inContext, glm::vec2(10.0f, 30.0f), glm::vec2(780.0f, 720.0f), MagicWand::FRAME_NORMAL_CUT_UPPER, SimplePanelWidget::NoOverflowSlider, false);
		mChildren.mChildWidgets.push_back(widget);

		SimplePanelWidget* parent_widget = widget;
		ChildWidgetContainer& children = widget->GetChildren();

#if 1
		{
			SimpleRenderToTextureWidget* widget = new SimpleRenderToTextureWidget();
			widget->Create(inContext, glm::vec2(0.0f, 0.0f), parent_widget->GetInternalSize());
			widget->SetScene(&mTestScene);
			children.mChildWidgets.push_back(widget);
		}
#endif
	}
#endif



	{
		SimpleTextWidget* text_widget = new SimpleTextWidget();
		text_widget->Create(inContext, glm::vec2(800.0f, 25.0f), MagicWand::TextInfo("Tools", 0, 14.0f, true, glm::vec2(0.0f, 0.0f)), MagicWand::SizeConstraints());
			
		//pos_vert += vert2d(text_widget->GetSize()) + 2.0f * height_offset;

		mChildren.mChildWidgets.push_back(text_widget);
	}

	{
		SimplePanelWidget* widget = new SimplePanelWidget();
		widget->Create(inContext, glm::vec2(800.0f, 55.0f), glm::vec2(200.0f, 250.0f), MagicWand::FRAME_NORMAL, SimplePanelWidget::AutoOverflowSlider, true);
		mChildren.mChildWidgets.push_back(widget);

		ChildWidgetContainer& children = widget->GetChildren();

		MagicWand::SizeConstraints sizeConstraints;
		horiz2d(sizeConstraints.mMinSize) = 150.0f;
		horiz2d(sizeConstraints.mMaxSize) = 150.0f;
		
		float pos_vert = 6.0f;
		float height_offset = 6.0f;

		{
			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(inContext, glm::vec2(8.0f, pos_vert), glm::vec2(150.0f, 0.0f), MagicWand::TextInfo("Slide", 0, 12.0f, false, glm::vec2(2.0f, 2.0f)), sizeConstraints);
			slider_widget->SetSliderPos(0.3f);
			pos_vert += vert2d(slider_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(slider_widget);
		}

#if 1
		{
			SimpleSliderWidget* slider_widget = new SimpleSliderWidget();
			slider_widget->Create(inContext, glm::vec2(8.0f, pos_vert), glm::vec2(150.0f, 0.0f), MagicWand::TextInfo("Bold Slide", 0, 12.0f, true, glm::vec2(2.0f, 2.0f)), sizeConstraints);
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
			button_widget->Create(inContext, glm::vec2(8.0f, pos_vert), false, MagicWand::TextInfo("Big eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), halfSizeConstraints);
			
			children.mChildWidgets.push_back(button_widget);

			{
				SimpleButtonWidget* button_widget = new SimpleButtonWidget();
				button_widget->Create(inContext, glm::vec2(horiz2d(halfSizeConstraints.mMinSize) + 6.0f + 8.0f, pos_vert), false, MagicWand::TextInfo("Bold eye ;)", 0, 12.0f, true, glm::vec2(10.0f, 2.0f)), halfSizeConstraints);
			
				children.mChildWidgets.push_back(button_widget);
			}

			pos_vert += vert2d(button_widget->GetSize()) + height_offset;
		}

		

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(inContext, glm::vec2(8.0f, pos_vert), false, MagicWand::TextInfo("Bigger eye ;)", 0, 16.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
	

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(inContext, glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Toggled eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			button_widget->SetIsToggled(true);
			
			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}

		{
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(inContext, glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Untoggled eye ;)", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
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

			int i = 66;

			std::string filler_name;
			strtk::construct(filler_name, "", "Filler ", i);

			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(inContext, glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo(filler_name.c_str(), 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

			children.mChildWidgets.push_back(button_widget);
		}
#endif


		/*
		{
			MagicWand::SizeConstraints sizeConstraints;
			SimpleButtonWidget* button_widget = new SimpleButtonWidget();
			button_widget->Create(glm::vec2(8.0f, pos_vert), true, MagicWand::TextInfo("Scissorrrrrrrrrrrrrrrrrrrrr testtttttttt", 0, 12.0f, false, glm::vec2(10.0f, 2.0f)), sizeConstraints);
			
			pos_vert += vert2d(button_widget->GetSize()) + height_offset;

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



void NativeWindowWidget::Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty) 
{
	RECT rect;
	GetClientRect(GetHWND(), &rect);							

	mViewportWidth = rect.right-rect.left;							
	mViewportHeight = rect.bottom-rect.top;		

	mChildren.Update(inContext, inParentTransform, kIdentitySceneTransform, inParentTransformDirty || false);
}


void NativeWindowWidget::RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty)
{
	int render_tree = inContext.mRenderTreeBuilder.BranchUpNewTree(this, false, NULL);
	mChildren.RenderBuild(inContext, inParentTransform, kIdentitySceneTransform, inParentTransformDirty || false);
	inContext.mRenderTreeBuilder.BranchDown(render_tree);
}

void NativeWindowWidget::Render(Renderer& inRenderer)
{
	{
		GLint viewport_width = GetViewportWidth();
		GLint viewport_height = GetViewportHeight();

		glViewport(0, 0, (GLsizei)(viewport_width), (GLsizei)(viewport_height));
		//glClear(GL_COLOR_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		//glOrtho(0.0f, window_width, window_height, 0.0f, -1.0f, 1.0f);
		glOrtho(0.0f, viewport_width, viewport_height, 0.0f, -1.0f, 1.0f);

		glMatrixMode( GL_MODELVIEW );
		glLoadIdentity();

		//glDisable(GL_DEPTH_TEST);
		glEnable(GL_DEPTH_TEST);

		// For 2D pixel precise mode
		//glTranslatef (0.375f, 0.375f, 0.0f);
		//glTranslatef (0.5f, 0.5f, 0.0f);

		//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		//glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

		//PushScissor(glm::vec2(0.0f, 0.0f), glm::vec2(window_width, window_height));
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(39.0f/255.0f, 39.0f/255.0f, 39.0f/255.0f, 1.0f);
	//glClearColor(59.0f/255.0f, 59.0f/255.0f, 59.0f/255.0f, 1.0f);
	//glClearColor(176.0f/255.0f, 176.0f/255.0f, 176.0f/255.0f, 1.0f);
	//glClearColor(49.0f/255.0f, 140.0f/255.0f, 231.0f / 255.0f, 1.0f);
	//glClearColor(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.0f);
	
	//glClearColor(100.0f/255.0f, 149.0f/255.0f, 237.0f / 255.0f, 1.0f);
	//glClearColor(75.0f/255.0f, 146.0f/255.0f, 219.0f / 255.0f, 1.0f);
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear Screen And Depth Buffer

	//mTestScene.Render(inContext, *mTestWidget, inContext.mTimeSecs);
}

}