#ifdef WIN32
#include "../include/Bigeye/Bigeye_win_ogl.h"

namespace BE
{


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

	if (!mWindow->Create(WidgetContext(*this, 0.0f), *this, WideString(inWindowName), inWidth, inHeight))
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
		mWindow->Update(WidgetContext(*this, inTimeSecs), identity, false);
		mWindow->Render(WidgetContext(*this, inTimeSecs), identity, false);

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
