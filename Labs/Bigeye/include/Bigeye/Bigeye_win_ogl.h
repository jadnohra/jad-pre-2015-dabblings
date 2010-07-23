#ifdef WIN32
#ifndef _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H
#define _INCLUDED_BIGEYE_BIGEYE_WIN_OGL_H

#include "Platform.h"
#include "BigeyeWidgets.h"

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

		void			PushRenderToTexture(const glm::vec2& inPos, OGLRenderToTexture& inObject) const { mWindow->PushRenderToTexture(inPos, inObject); }
		OGLRenderToTexture*			PopRenderToTexture() const											{ return mWindow->PopRenderToTexture(); }

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

	
}

#endif 
#endif