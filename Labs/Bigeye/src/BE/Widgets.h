#ifndef _INCLUDED_BIGEYE_BIGEYEWIDGETS_H
#define _INCLUDED_BIGEYE_BIGEYEWIDGETS_H

#include <stack>
#include "BEMath.h"
#include "OGL.h"
#include "MagickWand.h"
#include "RenderTreeBuilder.h"

namespace BE 
{
	class MainWindow;

	struct WidgetContext
	{
		const MainWindow& mMainWindow;
		float mTimeSecs;

		mutable RenderTreeBuilder mRenderTreeBuilder;

		WidgetContext(const MainWindow& inMainWindow, float inTimeSecs)
		:	mMainWindow(inMainWindow)
		,	mTimeSecs(inTimeSecs)
		{
		}
	};

	class WidgetUtil
	{
	public:

		static bool IsMouseInRectangle(const WidgetContext& inContext, const glm::vec3& inWidgetWorldPos, const glm::vec2& inWidgetSize);
		static bool IsMouseInRectangle(const WidgetContext& inContext, const glm::vec2& inWidgetWorldPos, const glm::vec2& inWidgetSize);
	};


	class Widget : public RenderNode
	{
	public:

		virtual void Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}
		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}
		virtual void Render(Renderer& inRenderer) {}

		virtual glm::vec3 GetLocalPosition() { return glm::vec3(); }
		virtual glm::vec2 GetSize() { return glm::vec2(); }
	};


	class ChildWidgetContainer
	{
	public:

		ChildWidgetContainer(bool inDeleteWidgets);
		~ChildWidgetContainer();

		void SetDeleteWidgets(bool inDeleteWidgets);

		void	Delete();
		void	Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty);
		void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty);
		
	public:

		typedef std::vector<Widget*> aWidgetPtr;
		
		bool mDeleteWidgets;
		aWidgetPtr mChildWidgets;
	};

	class MagicWandTestTextureWidget : public Widget
	{
	public:

		MagicWandTestTextureWidget();
		virtual ~MagicWandTestTextureWidget();

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos);

		virtual glm::vec3 GetLocalPosition() { return mPos; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

	protected:

		OGLTexture mTexture;
		glm::vec3 mPos;
		glm::vec2 mSize;

		CompactRenderState mRenderState;
		glm::vec3 mRenderWorldPos;
	};

	class SimpleTextureWidget : public Widget
	{
	public:

		SimpleTextureWidget();
		virtual ~SimpleTextureWidget();

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const char* inTexturePath);

		virtual glm::vec2 GetSize() { return mSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

	protected:

		GLuint mTexture;
		glm::vec3 mPos;
		glm::vec2 mSize;

		CompactRenderState mRenderState;
		glm::vec3 mRenderWorldPos;
	};


	class SimpleRenderToTextureWidget : public Widget
	{
	public:

		class Scene : public RenderNode
		{
		public:

			virtual void Update(const WidgetContext& inContext, SimpleRenderToTextureWidget& inParent, OGLRenderToTexture& inTexture)	{}
			virtual void Render(Renderer& inRenderer)	{}
		};

		SimpleRenderToTextureWidget();
		virtual ~SimpleRenderToTextureWidget();

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize);
		void		SetScene(Scene* inScene)		{ mScene = inScene; }

		virtual void Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		virtual glm::vec2 GetSize() { return mSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

		virtual RenderNodeDependency*	GetDependency()					{ return &mRenderNodeDependency; }

	protected:

		OGLRenderToTexture mTexture;
		glm::vec3 mPos;
		glm::vec2 mSize;

		Scene* mScene;

		RenderNodeDependency mRenderNodeDependency;
		CompactRenderState mRenderState;
		glm::vec3 mRenderWorldPos;
	};

	class SimpleTextWidget : public Widget
	{
	public:

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints);

		virtual glm::vec2 GetSize() { return mTextTexSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

	protected:

		glm::vec3 mPos;
		glm::vec2 mTextTexSize;

		OGLTexture mTextTexture;

		CompactRenderState mRenderState;
		glm::vec3 mRenderWorldPos;
	};


	class SimpleButtonWidget : public Widget
	{
	public:

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, bool inIsToggleButton, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints);

		void		SetIsToggled(bool inIsToggled);

		virtual void Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		
		virtual glm::vec2 GetSize() {  return mButtonTexSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

	protected:

		void CreateTextures(const WidgetContext& inContext);

		MagicWand::TextInfo mTextInfo;
		MagicWand::SizeConstraints mSizeConstraints;
		bool mIsToggleButton;
		bool mIsMousePressed;
		bool mIsPressed;
		bool mIsToggled;
		bool mIsHighlighted;

		glm::vec3 mPos;
		glm::vec2 mButtonTexSize;

		OGLTexture mButtonTexture;
		OGLTexture mHighlightedButtonTexture;
		OGLTexture mPressedButtonTexture;

		CompactRenderState mRenderState;
		glm::vec3 mRenderWorldPos;
	};

	class SimpleSliderWidget : public Widget
	{
	public:

		enum EType
		{
			Horizontal, Vertical
		};

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints);
		bool		CreateVertical(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize);

		void		SetSliderPos(float inPos)	{ mSliderPos = inPos; }
		float		GetSliderPos() const		{ return mSliderPos; }

		virtual void Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		
		virtual glm::vec2 GetSize() {  return mMarkerTexSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

	protected:

		glm::vec3 GetSliderWorldPos(const glm::vec3& inWorldPos) const;
		void CreateTextures(const WidgetContext& inContext);

		MagicWand::TextInfo mTextInfo;
		MagicWand::SizeConstraints mSizeConstraints;
		EType mType;

		glm::vec3 mPos;
		glm::vec2 mSize;
		float mSliderPos;

		OGLTexture mFrameTexture;
		glm::vec2 mFrameTexSize;
		OGLTexture mPressedMarkerTexture;
		OGLTexture mHighlightedMarkerTexture;
		OGLTexture mMarkerTexture;
		glm::vec2 mMarkerTexSize;

		bool mIsHighlighted;
		bool mHasMouseSliderFocus;
		glm::vec2 mMouseSliderFocusStartMousePos;
		float mMouseSliderFocusStartSliderPos;

		CompactRenderState mRenderState;
		glm::vec3 mRenderWorldPos;
	};

	class SimplePanelWidget : public Widget
	{
	public:

		enum EOverflowSliderType
		{
			NoOverflowSlider, OverflowSlider, AutoOverflowSlider
		};

		SimplePanelWidget();

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize, MagicWand::FrameType inType, EOverflowSliderType inOverflowSliderType, bool inUseGradient);

		ChildWidgetContainer& GetChildren() { return mChildren; } 

		virtual void Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		
		virtual glm::vec2 GetSize() {  return mSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		glm::vec2 GetInternalSize() {  return mScissorSize; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

	protected:

		void CreateTextures(const WidgetContext& inContext, bool inUseGradient);

		MagicWand::FrameType mType;
		EOverflowSliderType mOverflowSliderType;
		
		glm::vec3 mPos;
		glm::vec2 mSize;
		glm::vec2 mScissorPos;
		glm::vec2 mScissorSize;
		glm::vec2 mOverflowPosOffset;
		SimpleSliderWidget mOverflowSlider;
		float mOverflowSpaceSize;

		OGLTexture mTexture;
		ChildWidgetContainer mChildren;
		ChildWidgetContainer mAutoChildren;

		CompactRenderState mRenderState;
		glm::vec3 mRenderWorldPos;
		PushScissorNode mPushScissorNode;
		PopScissorNode mPopScissorNode;
	};


	class NativeWindowWidget : public Widget
	{
	public:

						NativeWindowWidget();
						~NativeWindowWidget();

		bool			Create(const WidgetContext& inContext, MainWindow& inApp, const WideString& inWindowName, int inWidth, int inHeight);

		virtual void	Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		
		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

		LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND			GetHWND() const	{ return mHWND; }
		HDC				GetHDC() const	{ return mHDC; }

		GLint			GetViewportWidth() const { return mViewportWidth; }
		GLint			GetViewportHeight() const { return mViewportHeight; }

		ChildWidgetContainer& GetChildren() { return mChildren; } 

	protected:

		void			Destroy();
		void			Test(const WidgetContext& inContext);

		MainWindow*	mMainWindow;
		bool	mIsWNDCLASSRegistered;
		HWND	mHWND;
		HDC		mHDC;
		HGLRC	mHRC;
		ChildWidgetContainer mChildren;
		GLint mViewportWidth;
		GLint mViewportHeight;
	};

}


#endif