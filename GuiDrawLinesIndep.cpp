#include <iostream>
#include <windows.h>  // For GUI elements
#include <strsafe.h>  // For StringCchPrintf() in ErrorExit()

using namespace std;

// Predeclarations
static LRESULT CALLBACK window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
void ErrorExit();

int main()
{
	int wflags = 0;
	//	wflags |= WS_THICKFRAME | WS_MAXIMIZEBOX;

	HINSTANCE _hInstance;
	HWND _hWindow;
	_hInstance = GetModuleHandle(NULL);
	if (!_hInstance)
		cout << "GetModuleHandle() failed" << endl;

	WNDCLASS wc;
	wc.lpszClassName = "MyGUI";
	wc.lpfnWndProc = window_proc;
	wc.style = wflags;
	wc.hInstance = _hInstance;
	wc.hIcon = LoadIcon(_hInstance, MAKEINTRESOURCE(105));
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = "AppMenu";
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	if (::RegisterClass(&wc) == 0)
	{
		cout << "RegisterClass() failed" << endl;
		ErrorExit();
	}

	wflags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	_hWindow = ::CreateWindow("MyGUI", "GuiTrial",	wflags, 100,	100, 640, 480, 0, 0, _hInstance, 0);
	if(_hWindow == 0)
	{
		cout << "CreateWindow() failed" << endl;
		ErrorExit();
	}

	RECT rct;
	::GetClientRect(_hWindow, &rct);
	::MoveWindow(_hWindow,			   // handle to window
                 70,                  // horizontal position
                 50,                  // vertical position
			640, 480,
                 FALSE);

	::ShowWindow(_hWindow, SW_SHOWNORMAL);
	::UpdateWindow(_hWindow);


	// Main loop
	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg); // This invokes window_proc()
		
		// Perhaps do some other stuff here
	}

	return 0;
}

// Global variables used in the Window Procedure
int downAtX = 0;
int downAtY = 0;
int upAtX = 0;
int upAtY = 0;
int prevX = 0;
int prevY = 0;

bool isDown = false;
HDC memDC;
RECT clientRect;

HPEN hColorPen;
HPEN hFinalPen;

static LRESULT CALLBACK window_proc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LRESULT ret = 0;

	switch (msg)
	{
		////  ------------------   Startup and shutdown  ------------------  ////
		case WM_CREATE:
		{
			// Start with a blank window
			HDC hdc = ::GetDC(hWnd);
			::GetClientRect(hWnd,&clientRect);
			memDC = ::CreateCompatibleDC(hdc);
			// This gives us just a 1x1 monochrome bitmap, need to get more:
			HBITMAP memBmp = ::CreateCompatibleBitmap(hdc,640,480);
			::SelectObject(memDC, memBmp);
			// Now we can draw into memDC all we want
			::FillRect(memDC, &clientRect, (HBRUSH) (COLOR_WINDOW+1));
			::ReleaseDC(hWnd, hdc);

			// Note that there already is a default black pen
			hColorPen = ::CreatePen(PS_DASHDOTDOT, 0, RGB(150,0,200));
			hFinalPen = ::CreatePen(PS_SOLID, 2, RGB(0,0,0));
			return 0;
		}
		case WM_DESTROY:
		{
			::PostQuitMessage(0);
			return 0;
		}

		////   ------------------  Display routine   ------------------  ////
    case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = ::BeginPaint(hWnd, &ps);
			// Get the device context from memory (with memory bitmap) drawn into this DC
			BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
			::EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_ERASEBKGND:
			return TRUE; // tell Windows that we handled it. (but don't actually draw anything)

		////  ------------------  Mouse handling  ------------------  ////
		case WM_LBUTTONDOWN:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			downAtX = x;
			downAtY = y;
			isDown = true;
			prevX = x;
			prevY = y;
			return 0;
		}
		case WM_LBUTTONUP:
		{
			isDown = false;
			upAtX = LOWORD(lParam);
			upAtY = HIWORD(lParam);
			// Draw this onto the persistent bitmap !
			::SelectObject(memDC, hFinalPen);
			::MoveToEx(memDC, downAtX, downAtY, 0);
			::LineTo(memDC, upAtX, upAtY);
			// Get this new situation on the display
			HDC hdc = ::GetDC(hWnd);
			BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
			::ReleaseDC(hWnd, hdc);
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			int x = LOWORD(lParam);
			int y = HIWORD(lParam);
			// Busy deciding about a line ?
			if ((isDown) && ((prevX != x) || (prevY != y)))
			{
				HDC hdc = ::GetDC(hWnd);
				// Erase previous tentative location of the line
				BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, memDC, 0, 0, SRCCOPY);
				prevX = x;
				prevY = y;
				// Draw the line in the new position
				::SelectObject(hdc, hColorPen);
				::MoveToEx(hdc, downAtX, downAtY, 0);
				::LineTo(hdc, x, y);
				::ReleaseDC(hWnd, hdc);
			}
			return 0;
		}
	} // switch(msg)

	// All unhandled messages are treated by the Windows default
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}


// From MSDN
void ErrorExit()
{ 
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
        (lstrlen((LPCTSTR)lpMsgBuf) + 40) * sizeof(TCHAR)); 
    StringCchPrintf((LPTSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("Last error %d: %s"),  dw, lpMsgBuf); 
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK); 

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
    ExitProcess(dw); 
}
