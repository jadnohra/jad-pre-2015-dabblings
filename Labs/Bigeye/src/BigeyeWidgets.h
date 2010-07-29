#ifndef _INCLUDED_BIGEYE_BIGEYEWIDGETS_H
#define _INCLUDED_BIGEYE_BIGEYEWIDGETS_H

#include <stack>
#include "BEMath.h"
#include "OGL.h"
#include "OGLState.h"
#include "MagickWand.h"
#include "BigeyeRenderTreeBuilder.h"

namespace BE 
{
	class App;

	struct WidgetContext
	{
		const App& mApp;
		float mTimeSecs;

		mutable RenderTreeBuilder mRenderTreeBuilder;

		WidgetContext(const App& inApp, float inTimeSecs)
		:	mApp(inApp)
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
		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}
		virtual void Render(Renderer& inRenderer) {}

		virtual glm::vec3 GetLocalPosition() { return glm::vec3(); }
		virtual glm::vec2 GetSize() { return glm::vec2(); }
	};


	enum EOGLWidgetState
	{
		EOGLState_Reset,
		EOGLState_NativeWindowWidget,
		EOGLState_TextureWidget,
		EOGLState_FontRender,
		EOGLState_Count,
	};


	class NativeWindowWidget;
	class OGLState_NativeWindowWidget : public OGLState
	{
	public:

						OGLState_NativeWindowWidget(NativeWindowWidget& inParent);

		virtual void	Set();

	protected:

		NativeWindowWidget& mParent;
	};

	class OGLState_TextureWidget : public OGLState
	{
	public:

		virtual void	Set();
	};

	
	class ChildWidgetContainer
	{
	public:

		ChildWidgetContainer(bool inDeleteWidgets);
		~ChildWidgetContainer();

		void SetDeleteWidgets(bool inDeleteWidgets);

		void	Delete();
		void	Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty);
		void	Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty);

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

		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
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

		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
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

		class Scene
		{
		public:

			virtual void Update(const WidgetContext& inContext, SimpleRenderToTextureWidget& inParent)	{}
			virtual void Render(const WidgetContext& inContext, SimpleRenderToTextureWidget& inParent)	{}
		};

		SimpleRenderToTextureWidget();
		virtual ~SimpleRenderToTextureWidget();

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize);
		void		SetScene(Scene* inScene)		{ mScene = inScene; }

		virtual void Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		virtual glm::vec2 GetSize() { return mSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		//virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		//virtual void Render(Renderer& inRenderer);

	protected:

		OGLRenderToTexture mTexture;
		glm::vec3 mPos;
		glm::vec2 mSize;

		Scene* mScene;
	};

	class SimpleTextWidget : public Widget
	{
	public:

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints);

		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
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
		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);

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
		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);

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

		bool		Create(const WidgetContext& inContext, const glm::vec2& inPos, const glm::vec2& inSize, MagicWand::FrameType inType, EOverflowSliderType inOverflowSliderType);

		ChildWidgetContainer& GetChildren() { return mChildren; } 

		virtual void Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		virtual glm::vec2 GetSize() {  return mSize; }
		virtual glm::vec3 GetLocalPosition() { return mPos; }

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

	protected:

		void CreateTextures(const WidgetContext& inContext);

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

		bool			Create(const WidgetContext& inContext, App& inApp, const WideString& inWindowName, int inWidth, int inHeight);

		virtual void	Update(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void	Render(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		virtual void RenderBuild(const WidgetContext& inContext, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(Renderer& inRenderer);

		LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND			GetHWND() const	{ return mHWND; }
		HDC				GetHDC() const	{ return mHDC; }

		void			PushScissor(const glm::vec2& inPos, const glm::vec2& inSize) const; 
		void			PopScissor() const; 

		void			PushRenderToTexture(const glm::vec2& inPos, OGLRenderToTexture& inObject) const;
		OGLRenderToTexture*			PopRenderToTexture() const;

		GLint			GetViewportWidth() const { return mViewportWidth; }
		GLint			GetViewportHeight() const { return mViewportHeight; }

	protected:

		struct ScissorStackElement
		{
			GLint mPosX;
			GLint mPosY;
			GLint mWidth;
			GLint mHeight;

			ScissorStackElement()
			:	mPosX(0)
			,	mPosY(0)
			,	mWidth(0)
			,	mHeight(0)
			{
			}

			ScissorStackElement(const glm::vec2& inPos, const glm::vec2& inSize)
			:	mPosX((GLint) inPos.x)
			,	mPosY((GLint) inPos.y)
			,	mWidth((GLint) inSize.x)
			,	mHeight((GLint) inSize.y)
			{
			}
		};

		struct RenderToTextureStackElement
		{
			GLint mPosX;
			GLint mPosY;

			OGLRenderToTexture*	mObject;

			RenderToTextureStackElement()
			:	mPosX(0)
			,	mPosY(0)
			,	mObject(NULL)
			{
			}

			RenderToTextureStackElement(const glm::vec2& inPos, OGLRenderToTexture& inObject)
			:	mPosX((GLint) inPos.x)
			,	mPosY((GLint) inPos.y)
			,	mObject(&inObject)
			{
			}
		};

		typedef std::stack<ScissorStackElement> ScissorStack;
		typedef std::stack<RenderToTextureStackElement> RenderToTextureStack;

		void			Destroy();
		void			Test(const WidgetContext& inContext);

		App*	mApp;
		bool	mIsWNDCLASSRegistered;
		HWND	mHWND;
		HDC		mHDC;
		HGLRC	mHRC;
		ChildWidgetContainer mChildren;
		mutable ScissorStack mScissorStack;	
		mutable RenderToTextureStack mRenderToTextureStack;	
		GLint mViewportWidth;
		GLint mViewportHeight;
	};

}


#endif