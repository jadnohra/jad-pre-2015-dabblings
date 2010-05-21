#ifdef WIN32
#ifndef _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H
#define _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H

#include <windows.h>
#include "glm/glm.hpp"
#include "OGL.h"
#include "OGLState.h"
#include "OGLFontRender.h"


namespace BE
{
	class App;
	class WideString;

	enum InputID
	{
	};

	typedef glm::mat3 SceneTransform;
	static const SceneTransform kIdentitySceneTransform;

	class Widget
	{
	public:

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform) {}
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform) {}
	};


	enum EOGLWidgetState
	{
		EOGLState_Reset,
		EOGLState_NativeWindowWidget,
		EOGLState_NormalWidget,
		EOGLState_WidgetShadow,
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


	class OGLState_NormalWidget : public OGLState
	{
	public:

		virtual void	Set();
	};


	class OGLState_WidgetShadow : public OGLState
	{
	public:

		virtual void	Set();
	};

	
	class ChildWidgetContainer
	{
	public:

		void	Delete();
		void	Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform);
		void	Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform, const SceneTransform& inParentLocalTransform);

	public:

		typedef std::vector<Widget*> aWidgetPtr;
		aWidgetPtr mChildWidgets;
	};

	class SimpleRectangleWidget : public Widget
	{
	public:

		bool		Create(const glm::vec2& inPos, const glm::vec2& inSize);

		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform);

	protected:

		glm::vec3 mPos;
		glm::vec2 mSize;
	};

	class NativeWindowWidget : public Widget
	{
	public:

						NativeWindowWidget();
						~NativeWindowWidget();

		bool			Create(App& inApp, const WideString& inWindowName, int inWidth, int inHeight);

		virtual void	Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform);
		virtual void	Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform);

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