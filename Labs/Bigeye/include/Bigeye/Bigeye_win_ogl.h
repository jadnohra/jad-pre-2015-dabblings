#ifdef WIN32
#ifndef _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H
#define _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H

#include <windows.h>
#include "BEMath.h"
#include "OGL.h"
#include "OGLState.h"
#include "OGLFontRender.h"
#include "RoundedRectangle.h"

namespace BE
{
	class App;
	class WideString;

	enum InputID
	{
	};


	class Widget
	{
	public:

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty) {}
	};


	enum EOGLWidgetState
	{
		EOGLState_Reset,
		EOGLState_NativeWindowWidget,
		EOGLState_NormalWidget,
		EOGLState_ShadowWidget,
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


	class OGLState_NormalWidget : public OGLState
	{
	public:

		virtual void	Set();
	};


	class OGLState_ShadowWidget : public OGLState
	{
	public:

		virtual void	Set();
	};

	
	class ChildWidgetContainer
	{
	public:

		ChildWidgetContainer(bool inDeleteWidgets);
		~ChildWidgetContainer();

		void SetDelectWidgets(bool inDeleteWidgets);

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

		bool		Create(const glm::vec2& inPos);

		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

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

		bool		Create(const glm::vec2& inPos, const char* inTexturePath);

		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

	protected:

		GLuint mTexture;
		glm::vec3 mPos;
		glm::vec2 mSize;
	};

	class SimpleTextWidget : public Widget
	{
	public:

		bool		Create(const glm::vec2& inPos, const char* inText = "");

		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

	protected:

		glm::vec3 mPos;
		std::string mText;
	};


	class SimpleSliderWidget : public Widget
	{
	public:

		bool		Create(const glm::vec2& inPos, const glm::vec2& inSize);

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

	protected:

		void UpdateGeometry(const glm::vec2& inWorldPos);

		glm::vec3 mPos;
		glm::vec2 mSize;
		float mSliderPos;

		OGLTexture mFrameTexture;
		glm::vec2 mFrameTexSize;
		OGLTexture mMarkerTexture;
		glm::vec2 mMarkerTexSize;
	};

	class SimplePanelWidget : public Widget
	{
	public:

		SimplePanelWidget();

		bool		Create(const glm::vec2& inPos, const glm::vec2& inSize);

		ChildWidgetContainer& GetChildren() { return mChildren; } 

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, bool inParentTransformDirty);

	protected:

		void UpdateGeometry(const glm::vec2& inWorldPos);

		glm::vec3 mPos;
		glm::vec2 mSize;

		OGLTexture mTexture;
		ChildWidgetContainer mChildren;
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

	protected:

		void			Destroy();
		void			Test(App& inApp);

		bool	mIsWNDCLASSRegistered;
		HWND	mHWND;
		HDC		mHDC;
		HGLRC	mHRC;
		ChildWidgetContainer mChildren;
		OGLFontInstance		 mDefaultFont;	
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

		float					GetInputState(int inInputID);
		int						GetInputEventCount();
		const InputEventInfo&	GetInputEvent(int inIndex);
		void					ConsumeInputEvents();

		bool					Update(float inTimeSecs);

		HINSTANCE				GetHINSTANCE() const			{ return mHINSTANCE; }
		const OGLStateManager&	GetOGLStateManager() const		{ return mOGLStateManager; }
		OGLStateManager&		GetOGLStateManager()			{ return mOGLStateManager; }

	protected:

		HINSTANCE				mHINSTANCE;
		NativeWindowWidget*		mWindow;
		OGLStateManager			mOGLStateManager;
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