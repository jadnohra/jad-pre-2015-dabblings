#ifdef WIN32
#ifndef _INCLUDED_BIGEYE_MAINWINDOW_H
#define _INCLUDED_BIGEYE_MAINWINDOW_H

#include "Platform.h"
#include "Widgets.h"

namespace BE
{
	
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

	class MainWindow
	{
	public:
		
								MainWindow();
								~MainWindow();

		bool					Create(const char* inWindowName, int inWidth, int inHeight);

		float					GetInputState(int inInputID) const;
		int						GetInputEventCount() const;
		const InputEventInfo&	GetInputEvent(int inIndex) const;
		void					ConsumeInputEvents();
		const glm::vec2&		GetMousePos() const;

		bool					Update(float inTimeSecs);

		HINSTANCE				GetHINSTANCE() const			{ return mHINSTANCE; }
		MagicWand&				GetWand() const					{ return mWand; }

	protected:

		void					PrepareInputForUpdate();

		HINSTANCE				mHINSTANCE;
		NativeWindowWidget*		mWindow;
		mutable MagicWand		mWand;

		Renderer				mRenderer;


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

	
}

#endif 
#endif