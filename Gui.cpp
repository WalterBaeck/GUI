#include "Gui.h"
#include "ErrorWindow.h"  // For ErrorExit()

using namespace std;


COLORREF cBlack    = RGB(0,0,0);
COLORREF cRed      = RGB(255,0,0);
COLORREF cGreen    = RGB(0,255,0);
COLORREF cDGreen   = RGB(0,160,0);
COLORREF cCyan     = RGB(0,200,255);
COLORREF cPurple   = RGB(200,80,240);
COLORREF cBrown    = RGB(160,120,80);
COLORREF cBeige    = RGB(240,224,176);
COLORREF cDGrey    = RGB(50,50,50);
COLORREF cLGrey    = RGB(200,200,200);
COLORREF cWhite    = RGB(255,255,255);
COLORREF cPink     = RGB(255,200,224);


////////////////////////// Gui class-handling functions /////////////////

Gui::Gui(string name, int sizeX, int sizeY) :
	size(sizeX, sizeY), isMouseDown(false), alive(true),
	mouseDownFunc(NULL), mouseUpFunc(NULL), mouseMoveFunc(NULL)
{
	HINSTANCE hInstance_ = GetModuleHandle(NULL);
	if (!hInstance_)
	{
		cout << "GetModuleHandle() failed" << endl;
		ErrorExit();
	}

	string className(name);
	className.append("Gui");

	int wflags = 0;
	WNDCLASS wc;
	wc.lpszClassName = className.c_str();
	wc.lpfnWndProc = &Gui::windowProc;
	wc.style = wflags;
	wc.hInstance = hInstance_;
	wc.hIcon = LoadIcon(hInstance_, MAKEINTRESOURCE(105));
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

	wflags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU |
		WS_MINIMIZEBOX | WS_MAXIMIZEBOX;	//	wflags |= WS_THICKFRAME;
	// Depending on Windows version and theme, the requested size in CreateWindow()
	//   will lose some pixels to the Windows elements. Request a larger size:
	RECT rect;
	rect.left = rect.top = 0;
	rect.right = sizeX;
	rect.bottom = sizeY;
	::AdjustWindowRect(&rect, wflags, false);
	hWindow = ::CreateWindow(className.c_str(), name.c_str(), wflags,
			60, 35, rect.right - rect.left, rect.bottom - rect.top,
			0, 0, hInstance_, 0);
	if(hWindow == 0)
	{
		cout << "CreateWindow() failed" << endl;
		ErrorExit();
	}
	::ShowWindow(hWindow, SW_SHOWNORMAL);                //Display @ normal size

	// Now that the Window was created, populate the Gui object with some members
	tempDC = ::GetDC(hWindow);
	permDC = ::CreateCompatibleDC(tempDC);
	// This gives us just a 1x1 monochrome bitmap, need to get more:
	HBITMAP permBmp = ::CreateCompatibleBitmap(tempDC, sizeX, sizeY);
	::SelectObject(permDC, permBmp);
	// Make sure the window handle leads to the Gui object that created it
	SetWindowLongPtr(hWindow, GWLP_USERDATA, (LONG)this);

	// Make the window initially white iso. black
	::GetClientRect(hWindow, &rect);
	::FillRect(permDC, &rect, (HBRUSH) (COLOR_WINDOW+1));

	// Superfluous stuff:
	//::MoveWindow(hWindow, 70, 50, sizeX, sizeY, FALSE);  //Change pos and size
	//::UpdateWindow(hWindow);                             //Send a WM_PAINT msg

} // Gui::Gui()

Gui::~Gui()
{
	::DeleteDC(permDC);
	::ReleaseDC(hWindow, tempDC);
}


////////////////////////////// Gui housekeeping ////////////////////

void Gui::process()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			alive = false;
			exit(0);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg); // This invokes windowProc()
	}
}

void Gui::grab()
{
	tempDC = ::GetDC(hWindow);
}

void Gui::release()
{
	::ReleaseDC(hWindow, tempDC);
}

void Gui::waitUntilClicked()
{
	while (!isMouseDown)
		process();
	// Need to wait for the release too, or else many events fall through in a single shot
	while (isMouseDown)
		process();
}

/////////////////////// Gui drawing functions ////////////////////

void Gui::drawPermDC()
{
	grab();
	::BitBlt(tempDC, 0, 0, size.x, size.y, permDC, 0, 0, SRCCOPY);
	release();
}

void Gui::drawPixel(enum eGuiDC dc, XY at, unsigned char r, unsigned char g, unsigned char b)
{
	if (dc==eTempDC)  grab();
	COLORREF newColor = (((DWORD)b)<<16) | (((DWORD)g)<<8) | (DWORD)r;
	::SetPixel(getDC(dc), at.x, at.y, newColor);
	if (dc==eTempDC)  release();
}

void Gui::drawLine(enum eGuiDC dc, XY from, XY to, HPEN pen)
{
	if (dc==eTempDC)  grab();
	::SelectObject(getDC(dc), pen);
	::MoveToEx(getDC(dc), from.x, from.y, 0);
	::LineTo(getDC(dc), to.x, to.y);
	if (dc==eTempDC)  release();
}

void Gui::drawRectangle(enum eGuiDC dc, XY from, XY to, HPEN pen)
{
	if (dc==eTempDC)  grab();
	::SelectObject(getDC(dc), pen);
	::SelectObject(getDC(dc), GetStockObject(NULL_BRUSH)); // No fill
	::Rectangle(getDC(dc), from.x, from.y, to.x, to.y);
	if (dc==eTempDC)  release();
}

void Gui::fillRectangle(enum eGuiDC dc, XY from, XY to,
		HBRUSH brush, COLORREF bkColor)
{
	RECT frect;
	frect.left = from.x;
	frect.top = from.y;
	frect.right = to.x;
	frect.bottom = to.y;
	if (dc==eTempDC)  grab();
	COLORREF prevBkColor = ::GetBkColor(getDC(dc));
	::SetBkColor(getDC(dc), bkColor);
	::FillRect(getDC(dc), &frect, brush);
	::SetBkColor(getDC(dc), prevBkColor);
	if (dc==eTempDC)  release();
}

LOGFONT Gui::font(string name, XY size,
			enum Bold bold, enum Italic italic,
			enum Underline underline,
			enum Strikethrough strikethrough)
{
	// Prepare the font
	LOGFONT logFont;
	strcpy(logFont.lfFaceName, name.c_str());
	logFont.lfHeight = size.y;
	logFont.lfWidth = size.x;
	logFont.lfWeight = (bold==eNotBold ? FW_NORMAL : FW_BOLD);
	logFont.lfItalic = (italic==eNotItalic ? false : true);
	logFont.lfUnderline = (underline==eNotUnderline ? false : true);
	logFont.lfStrikeOut = (strikethrough==eNotStrikethrough ? false : true);
	logFont.lfEscapement = 0;
	logFont.lfOrientation = 0;
	return logFont;
}

void Gui::writeText(enum eGuiDC dc, XY pos, string text, LOGFONT logFont, COLORREF color)
{
	if (dc==eTempDC)  grab();
	::SelectObject(getDC(dc), ::CreateFontIndirect(&logFont));
	::SetTextColor(getDC(dc), color);
	::TextOut(getDC(dc), pos.x, pos.y, (LPCTSTR)(text.c_str()), text.length());
	if (dc==eTempDC)  release();
}

//////////////////// Static : the Window Procedure ///////////////

// Helper function for use in the Window Procedure
Gui* getGuiPtr(HWND hWnd)
{
	// Because the Window Procedure is a static method, need to get object ptr first
	// See blogs.microsoft.com/oldnewthing/20050422-08
	Gui* self = (Gui*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	if (!self)
	{
		cout << "getGuiPtr failed" << endl;
		ErrorExit();
	}

	return self;
}

// Implementation of the Window Procedure (registered in the Window Class)
LRESULT CALLBACK Gui::windowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		////  ------------------   Startup and shutdown  ------------------  ////
		case WM_CREATE:
			return 0;
		case WM_DESTROY:
		{
			::PostQuitMessage(0);  // This will post a WM_QUIT message
			return 0;
		}

		////   ------------------  Display routine   ------------------  ////
    case WM_PAINT:
		{
			Gui* self = getGuiPtr(hWnd);
			PAINTSTRUCT ps;
			self->tempDC = ::BeginPaint(hWnd, &ps);
			self->drawPermDC();
			::EndPaint(hWnd, &ps);
			return 0;
		}
		case WM_ERASEBKGND:
			return TRUE; // tell Windows that we handled it (but actually do nothing)

		////  ------------------  Mouse handling  ------------------  ////
		case WM_LBUTTONDOWN:
		{
			XY xy(lParam);
			Gui* self = getGuiPtr(hWnd);
			self->isMouseDown = true;
			self->mouseDownStart = xy;
			self->mousePrev = xy;
			if (self->mouseDownFunc)
			{
				self->tempDC = ::GetDC(hWnd);
				self->mouseDownFunc(self);
				::ReleaseDC(hWnd, self->tempDC);
			}
			return 0;
		}
		case WM_LBUTTONUP:
		{
			XY xy(lParam);
			Gui* self = getGuiPtr(hWnd);
			self->isMouseDown = false;
			self->mouseDownEnd = xy;
			if (self->mouseUpFunc)
			{
				self->tempDC = ::GetDC(hWnd);
				self->mouseUpFunc(self);
				::ReleaseDC(hWnd, self->tempDC);
			}
			return 0;
		}
		case WM_MOUSEMOVE:
		{
			XY xy(lParam);
			Gui* self = getGuiPtr(hWnd);
			// Actual change in position ?
			if (xy != self->mousePrev)
			{
				self->mousePrev = xy;  // Note that this is the NEW position !
				if (self->mouseMoveFunc)
				{
					self->tempDC = ::GetDC(hWnd);
					self->mouseMoveFunc(self);
					::ReleaseDC(hWnd, self->tempDC);
				}
			}
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			// Distance in units of 120, negative for "scroll down"
			short dist = HIWORD(wParam);
			XY xy(lParam);
			Gui* self = getGuiPtr(hWnd);
			if (self->mouseWheelFunc)
			{
				self->tempDC = ::GetDC(hWnd);
				self->mouseWheelFunc(self, dist/(-120));
				::ReleaseDC(hWnd, self->tempDC);
			}
			return 0;
		}
	} // switch(msg)

	// All unhandled messages are treated by the Windows default
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
} // Gui::windowProc()
