#ifdef WIN32
#ifndef _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H
#define _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H

#include "windows.h"
#define FREEGLUT_STATIC
#include "freeglut/GL/freeglut.h"
#include "freeglut/GL/freeglut_ext.h"
#include "glm/glm.hpp"

namespace BE
{
	class App;

	enum InputID
	{
	};

	typedef glm::mat3 SceneTransform;

	class Widget
	{
	public:

		virtual void Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform) {}
		virtual void Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform) {}
	};

	class NativeWindowWidget : public Widget
	{
	public:

						NativeWindowWidget();
						~NativeWindowWidget();

		bool			Create(const App& inApp, const char* inWindowName, int inWidth, int inHeight);

		virtual void	Update(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform) {}
		virtual void	Render(const App& inApp, float inTimeSecs, const SceneTransform& inParentTransform) {}

		LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK WindowProcProxy(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		HWND			GetHWND()	{ return mHWND; }

	protected:

		void			Destroy();

		HWND	mHWND;
		HDC		mHDC;
		HGLRC	mHRC;
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

	protected:

		HINSTANCE				mHINSTANCE;
		NativeWindowWidget*		mWindow;
	};
}

#endif 
#endif