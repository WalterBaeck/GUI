#ifndef GUI_H
#define GUI_H

#include <iostream>
#include <windows.h>      // For GUI elements

using namespace std;


extern COLORREF cBlack;
extern COLORREF cRed;
extern COLORREF cGreen;
extern COLORREF cDGreen;
extern COLORREF cCyan;
extern COLORREF cPurple;
extern COLORREF cBrown;
extern COLORREF cBeige;
extern COLORREF cDGrey;
extern COLORREF cLGrey;
extern COLORREF cWhite;
extern COLORREF cPink;


struct XY {
	int x, y;

  XY() : x(0), y(0) {}
  XY(int x, int y) : x(x), y(y) {}
  XY(LPARAM lParam) : x(LOWORD(lParam)), y(HIWORD(lParam)) {}

	// Copy assignment operator is default member-by-member

	bool operator==(XY &rhs) { return ((x==rhs.x) && (y==rhs.y)); }
	bool operator!=(XY &rhs) { return ((x!=rhs.x) || (y!=rhs.y)); }
}; // struct XY


enum eGuiDC {eTempDC, ePermDC};

enum Bold {eNotBold, eBold};
enum Italic {eNotItalic, eItalic};
enum Underline {eNotUnderline, eUnderline};
enum Strikethrough {eNotStrikethrough, eStrikethrough};


class Gui {
public:
	Gui(string name, int sizeX, int sizeY);
	~Gui();

	void process();
	void grab();
	void release();
	void waitUntilClicked();

	void drawPermDC();
	void drawPixel(enum eGuiDC dc, XY at, unsigned char r, unsigned char g, unsigned char b);
	void drawLine(enum eGuiDC dc, XY from, XY to, HPEN pen);
	void drawRectangle(enum eGuiDC dc, XY from, XY to, HPEN pen);
	void fillRectangle(enum eGuiDC dc, XY from, XY to,
			HBRUSH brush, COLORREF bkColor = (COLOR_WINDOW + 1));
	static LOGFONT font(string name, XY size,
			enum Bold bold = eNotBold, enum Italic italic = eNotItalic,
			enum Underline underline = eNotUnderline,
			enum Strikethrough strikethrough = eNotStrikethrough);
	void writeText(enum eGuiDC dc, XY pos, string text, LOGFONT logFont,
			COLORREF color = cBlack);


	// Function callbacks on mouse events
	void (*mouseDownFunc)(Gui*);
	void (*mouseUpFunc)  (Gui*);
	void (*mouseMoveFunc)(Gui*);
	void (*mouseWheelFunc)(Gui*, int dist); // +1 for each ScrollDown

	// Dimensions of the window
	XY size;

	// Graphics buffering
	HDC tempDC;
	HDC permDC;
	HDC getDC(enum eGuiDC dc) { return (dc==ePermDC ? permDC : tempDC); }
	
	// Mouse status
	bool isMouseDown;
	XY mousePrev;
	XY mouseDownStart;
	XY mouseDownEnd;
	
	// Window status
	HWND hWindow;
	bool alive;

private:
	// the Window Procedure must be static, to conform with WNDPROC signature
  static LRESULT CALLBACK windowProc(
		HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

}; // class Gui

#endif // GUI_H
