#ifdef WIN32
#ifndef _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H
#define _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H

#include <windows.h>
#include <stack>
#include "BEMath.h"
#include "OGL.h"
#include "OGLState.h"
#include "OGLFontRender.h"
#include "RoundedRectangle.h"
#include "MagickWand.h"

namespace BE
{
	class App;
	class WideString;

	enum InputID
	{
	};


	class WidgetUtil
	{
	public:

		static bool IsMouseInRectangle(const App& inApp, const glm::vec2& inWidgetWorldPos, const glm::vec2& inWidgetSize);
	};


	class Widget
	{
	public:

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}
		virtual glm::vec3 GetLocalPosition(const App& inApp) { return glm::vec3(); }
		virtual glm::vec2 GetSize(const App& inApp) { return glm::vec2(); }
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
		void	Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty);
		void	Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform, bool inParentTransformDirty);

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

		bool		Create(const App& inApp, const glm::vec2& inPos);

		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual glm::vec3 GetLocalPosition(const App& inApp) { return mPos; }

	protected:

		OGLTexture mTexture;
		glm::vec3 mPos;
		glm::vec2 mSize;
	};

	class SimpleTextureWidget : public Widget
	{
	public:

		SimpleTextureWidget();
		virtual ~SimpleTextureWidget();

		bool		Create(const App& inApp, const glm::vec2& inPos, const char* inTexturePath);

		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual glm::vec2 GetSize(const App& inApp) { return mSize; }
		virtual glm::vec3 GetLocalPosition(const App& inApp) { return mPos; }

	protected:

		GLuint mTexture;
		glm::vec3 mPos;
		glm::vec2 mSize;
	};

	class SimpleTextWidget : public Widget
	{
	public:

		bool		Create(const App& inApp, const glm::vec2& inPos, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints);

		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual glm::vec2 GetSize(const App& inApp) { return mTextTexSize; }
		virtual glm::vec3 GetLocalPosition(const App& inApp) { return mPos; }

	protected:

		glm::vec3 mPos;
		glm::vec2 mTextTexSize;

		OGLTexture mTextTexture;
	};


	class SimpleButtonWidget : public Widget
	{
	public:

		bool		Create(const glm::vec2& inPos, bool inIsToggleButton, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints);

		void		SetIsToggled(bool inIsToggled);

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		virtual glm::vec2 GetSize(const App& inApp) { CreateTextures(inApp); return mButtonTexSize; }
		virtual glm::vec3 GetLocalPosition(const App& inApp) { return mPos; }

	protected:

		void CreateTextures(const App& inApp);

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
	};

	class SimpleSliderWidget : public Widget
	{
	public:

		enum EType
		{
			Horizontal, Vertical
		};

		bool		Create(const glm::vec2& inPos, const glm::vec2& inSize, const MagicWand::TextInfo& inTextInfo, const MagicWand::SizeConstraints& inSizeConstraints);
		bool		CreateVertical(const glm::vec2& inPos, const glm::vec2& inSize);

		void		SetSliderPos(float inPos)	{ mSliderPos = inPos; }
		float		GetSliderPos() const		{ return mSliderPos; }

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		virtual glm::vec2 GetSize(const App& inApp) { CreateTextures(inApp); return mMarkerTexSize; }
		virtual glm::vec3 GetLocalPosition(const App& inApp) { return mPos; }

	protected:

		glm::vec2 GetSliderWorldPos(const glm::vec2& inWorldPos) const;
		void CreateTextures(const App& inApp);

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
	};

	class SimplePanelWidget : public Widget
	{
	public:

		enum EOverflowSliderType
		{
			NoOverflowSlider, OverflowSlider, AutoOverflowSlider
		};

		SimplePanelWidget();

		bool		Create(const glm::vec2& inPos, const glm::vec2& inSize, MagicWand::FrameType inType, EOverflowSliderType inOverflowSliderType);

		ChildWidgetContainer& GetChildren() { return mChildren; } 

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		virtual glm::vec2 GetSize(const App& inApp) { CreateTextures(inApp); return mSize; }
		virtual glm::vec3 GetLocalPosition(const App& inApp) { return mPos; }

	protected:

		void CreateTextures(const App& inApp);

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
	};


	class NativeWindowWidget : public Widget
	{
	public:

						NativeWindowWidget();
						~NativeWindowWidget();

		bool			Create(App& inApp, const WideString& inWindowName, int inWidth, int inHeight);

		virtual void	Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void	Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

		LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND			GetHWND() const	{ return mHWND; }

		void			PushScissor(const glm::vec2& inPos, const glm::vec2& inSize) const; 
		void			PopScissor() const; 

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

		typedef std::stack<ScissorStackElement> ScissorStack;

		void			Destroy();
		void			Test(App& inApp);

		bool	mIsWNDCLASSRegistered;
		HWND	mHWND;
		HDC		mHDC;
		HGLRC	mHRC;
		ChildWidgetContainer mChildren;
		mutable ScissorStack mScissorStack;	
		GLint mViewportWidth;
		GLint mViewportHeight;
	};

	enum EInputID
	{
		INPUT_INVALID,
		INPUT_MOUSE_START,
		INPUT_MOUSE_LEFT,
		INPUT_MOUSE_LEFT_CHANGED,
		INPUT_MOUSE_MIDDLE,
		INPUT_MOUSE_MIDDLE_CHANGED,
		INPUT_MOUSE_RIGHT,
		INPUT_MOUSE_RIGHT_CHANGED,
		INPUT_MOUSE_X,
		INPUT_MOUSE_Y,
		INPUT_MOUSE_MOVED,
		INPUT_MOUSE_END,
	};

	struct InputEventInfo
	{
		int inputID;
		float value;
		float action;
	};

	class App
	{
	public:
		
								App();
								~App();

		bool					Create(const char* inWindowName, int inWidth, int inHeight);

		float					GetInputState(int inInputID) const;
		int						GetInputEventCount() const;
		const InputEventInfo&	GetInputEvent(int inIndex) const;
		void					ConsumeInputEvents();
		const glm::vec2&		GetMousePos() const;

		bool					Update(float inTimeSecs);

		HINSTANCE				GetHINSTANCE() const			{ return mHINSTANCE; }
		const OGLStateManager&	GetOGLStateManager() const		{ return mOGLStateManager; }
		OGLStateManager&		GetOGLStateManager()			{ return mOGLStateManager; }
		MagicWand&				GetWand() const					{ return mWand; }

		void					PushScissor(const glm::vec2& inPos, const glm::vec2& inSize) const	{ mWindow->PushScissor(inPos, inSize); }
		void					PopScissor() const													{ mWindow->PopScissor(); }

	protected:

		void					PrepareInputForUpdate();

		HINSTANCE				mHINSTANCE;
		NativeWindowWidget*		mWindow;
		OGLStateManager			mOGLStateManager;
		mutable MagicWand		mWand;

		bool					mMouseMoved;
		glm::vec2				mMousePos;
		glm::vec2				mLastMousePos;
		bool					mLastMouseLeft;
		bool					mMouseLeft;
		bool					mMouseLeftChanged;
		bool					mLastMouseMiddle;
		bool					mMouseMiddle;
		bool					mMouseMiddleChanged;
		bool					mLastMouseRight;
		bool					mMouseRight;
		bool					mMouseRightChanged;
	};

	class WideString
	{
	public:

		WideString();
		WideString(const char* inString);
		void Set(const char* inString);

		operator LPCWSTR() const { return mBuffer; }

	private:

		LPWSTR mBuffer;
		int mSize;
	};
}

#endif 
#endif