#include "Gui.h"
#include "afx.h"
#include "stdafx.h"
#include "afxcmn.h"    // for CSliderCtrl

// Note that there already is a default black pen
HPEN hColorPen = ::CreatePen(PS_DASHDOTDOT, 0, RGB(150,0,200));
HPEN hFinalPen = ::CreatePen(PS_SOLID, 2, RGB(0,0,0));

void mouseUpFunc(Gui* gui)
{
	// Draw this onto the persistent bitmap !
	gui->drawLine(ePermDC, gui->mouseDownStart, gui->mouseDownEnd, hFinalPen);
	// Get this new situation on the display
	gui->drawPermDC();
}

void mouseMoveFunc(Gui* gui)
{
	if (gui->isMouseDown)
	{
		// Erase previous tentative location of the line
		gui->drawPermDC();
		// Draw the line in the new position
		gui->drawLine(eTempDC, gui->mouseDownStart, gui->mousePrev, hColorPen);
	}
}

int main()
{
	// Create GUI window and some pens
	Gui gui("DrawLines", 640, 480);
	// Fill in some callback functions
	gui.mouseUpFunc   = &mouseUpFunc;
	gui.mouseMoveFunc = &mouseMoveFunc;

	CSliderCtrl *slider = new CSliderCtrl;
	slider->Create(WS_CHILD | WS_VISIBLE, CRect(20, 20, 60, 280), this, IDC_SLIDER_CONTROL);

	while (gui.alive)
		gui.process();

	return 0;
}
