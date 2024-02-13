#include "Gui.h"
#include <time.h>     // For srand(time())

int main()
{
	// Create GUI window
	Gui gui("Random", 640, 480);
	// Fill in some callback functions
//gui.mouseDownFunc = &mouseDownFunc;

	// Seed random engine
	srand( (unsigned)time( NULL ) );

	while (gui.alive)
	{
		gui.process();

		// Draw 1K more random pixels
		for (int pixels=0; pixels<1000; pixels++)
			::SetPixel(gui.permDC, rand()%gui.size.x, rand()%gui.size.y,
					RGB(rand()%256, rand()%256, rand()%256));

		// Also write a character
		char c = '0' + (rand()%0x4A);
		::TextOut(gui.permDC, rand()%gui.size.x, rand()%gui.size.y,
				(LPCTSTR)(&c), 1);

		// Get a visible update
		gui.drawPermDC();
	}

	return 0;
}
