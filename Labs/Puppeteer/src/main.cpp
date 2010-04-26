#include "NeheGL.h"
#include "app.h"

#define WM_TOGGLEFULLSCREEN (WM_USER+1)									// Application Define Message For Toggling

static BOOL g_isProgramLooping;											// Window Creation Loop, For FullScreen/Windowed Toggle																		// Between Fullscreen / Windowed Mode
static BOOL g_createFullScreen;											// If TRUE, Then Create Fullscreen
App* g_App = NULL;

LRESULT CALLBACK WndProc(	HWND	hWnd,			// Handle For This Window
							UINT	uMsg,			// Message For This Window
							WPARAM	wParam,			// Additional Message Information
							LPARAM	lParam)			// Additional Message Information
{

	switch (uMsg)
	{
		case WM_DROPFILES:
		{
			if (g_App)
			{
				HDROP fDrop = (HDROP) wParam;

				int dropped_file_count = DragQueryFileA(fDrop, 0xFFFFFFFF, NULL, 0);
				char* fName = NULL;
				int buffer_size = 0;

				g_App->OnFilesDropped(dropped_file_count);
				
				for (int i=0; i<dropped_file_count; ++i)
				{
					UINT nBufSize = 1 + DragQueryFileA(fDrop, i, NULL, 0);

					if (nBufSize > buffer_size)
					{
						buffer_size = nBufSize;
						delete fName;
						fName = new char[buffer_size];
					}

					DragQueryFileA(fDrop, i, fName, buffer_size);
					g_App->OnFileDropped(fName);
				}

				delete fName;
				DragFinish(fDrop);
			}

			return 0;
		}
		break;
	}

	return NeheGLWindowProc(hWnd,uMsg,wParam,lParam);
}


BOOL RegisterWindowClass (Application* application)						// Register A Window Class For This Application.
{																		// TRUE If Successful
	// Register A Window Class
	WNDCLASSEXA windowClass;												// Window Class
	ZeroMemory (&windowClass, sizeof (WNDCLASSEXA));						// Make Sure Memory Is Cleared
	windowClass.cbSize			= sizeof (WNDCLASSEXA);					// Size Of The windowClass Structure
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraws The Window For Any Movement / Resizing
	windowClass.lpfnWndProc		= (WNDPROC)(WndProc);				// WindowProc Handles Messages
	windowClass.hInstance		= application->hInstance;				// Set The Instance
	windowClass.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);			// Class Background Brush Color
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	windowClass.lpszClassName	= application->className;				// Sets The Applications Classname
	if (RegisterClassExA (&windowClass) == 0)							// Did Registering The Class Fail?
	{
		// NOTE: Failure, Should Never Happen
		MessageBoxA (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													// Return False (Failure)
	}
	return TRUE;														// Return True (Success)
}


int main()
{
	Application			application;									// Application Structure
	GL_Window			window;											// Window Structure
	Keys				keys;											// Key Structure
	BOOL				isMessagePumpActive;							// Message Pump Active?
	MSG					msg;											// Window Message Structure
	DWORD				tickCount;										// Used For The Tick Counter

	// Fill Out Application Data
	application.className = "Puppeteer";									// Application Class Name
	application.hInstance = GetModuleHandle(NULL);						// Application Instance

	// Fill Out Window
	ZeroMemory (&window, sizeof (GL_Window));							// Make Sure Memory Is Zeroed
	window.keys					= &keys;								// Window Key Structure
	window.init.application		= &application;							// Window Application
	window.init.title			= "Puppeteer";							// Window Title
	window.init.width			= 800;									// Window Width
	window.init.height			= 600;									// Window Height
	window.init.bitsPerPixel	= 32;									// Bits Per Pixel
	window.init.isFullScreen	= FALSE;									// Fullscreen? (Set To TRUE)

	ZeroMemory (&keys, sizeof (Keys));									// Zero keys Structure


	// Ask The User If They Want To Start In FullScreen Mode?
	//if (MessageBoxA (HWND_DESKTOP, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	//{
	//	window.init.isFullScreen = FALSE;								// If Not, Run In Windowed Mode
	//}

	// Register A Class For Our Window To Use
	if (RegisterWindowClass (&application) == FALSE)					// Did Registering A Class Fail?
	{
		// Failure
		MessageBoxA (HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;														// Terminate Application
	}

	g_isProgramLooping = TRUE;											// Program Looping Is Set To TRUE
	g_createFullScreen = window.init.isFullScreen;						// g_createFullScreen Is Set To User Default
	
	App					app;
	
	//while (g_isProgramLooping)											// Loop Until WM_QUIT Is Received
	{
		// Create A Window
		window.init.isFullScreen = g_createFullScreen;					// Set Init Param Of Window Creation To Fullscreen?
		if (CreateWindowGL (&window) == TRUE)							// Was Window Creation Successful?
		{
			// At This Point We Should Have A Window That Is Setup To Render OpenGL
			if (!app.Load(&window))										// Call User Intialization
			{
				// Failure
				TerminateApplication (&window);							// Close Window, This Will Handle The Shutdown
			}
			else														// Otherwise (Start The Message Pump)
			{	
				g_App = &app;
				DragAcceptFiles(window.hWnd, TRUE);

				// Initialize was a success
				isMessagePumpActive = TRUE;								// Set isMessagePumpActive To TRUE
				while (isMessagePumpActive == TRUE)						// While The Message Pump Is Active
				{
					// Success Creating Window.  Check For Window Messages
					if (PeekMessage (&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						// Check For WM_QUIT Message
						if (msg.message != WM_QUIT)						// Is The Message A WM_QUIT Message?
						{
							DispatchMessage (&msg);						// If Not, Dispatch The Message
						}
						else											// Otherwise (If Message Is WM_QUIT)
						{
							isMessagePumpActive = FALSE;				// Terminate The Message Pump
						}
					}
					else												// If There Are No Messages
					{
						//if (window.isVisible == FALSE)					// If Window Is Not Visible
						//{
						//	WaitMessage ();								// Application Is Minimized Wait For A Message
						//}
						//else											// If Window Is Visible
						{
							// Process Application Loop
							tickCount = GetTickCount ();				// Get The Tick Count
							app.Update (tickCount - window.lastTickCount);	// Update The Counter
							window.lastTickCount = tickCount;			// Set Last Count To Current Count
							app.Draw();									// Draw Our Scene

							SwapBuffers (window.hDC);					// Swap Buffers (Double Buffering)
						}
					}
				}														// Loop While isMessagePumpActive == TRUE
			}															// If (Initialize (...

			// Application Is Finished
			app.End();
			//Deinitialize ();											// User Defined DeInitialization
			g_App = NULL;

			DestroyWindowGL (&window);									// Destroy The Active Window
		}
		else															// If Window Creation Failed
		{
			// Error Creating Window
			MessageBoxA (HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									// Terminate The Loop
		}
	}																	// While (isProgramLooping)

	UnregisterClassA (application.className, application.hInstance);		// UnRegister Window Class
	return 0;
}