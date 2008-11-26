#ifndef XPOPUP_H
#define XPOPUP_H

#include "XWinGL.h"

class XWin;

typedef struct xwindow
{
	Display*	display;
	Window		window;
	Window		parent;
	Window		root;
	GLXContext	context;
} xwindow;

/*
** we need to lend some stuff from motif protocol
*/
#define MWM_HINTS_DECORATIONS (1L << 1)
#define PROP_MWM_HINTS_ELEMENTS 5

typedef struct MwmHints
{
	unsigned long flags;
	unsigned long functions;
	unsigned long decorations;
	long input_mode;
	unsigned long status;
} MwmHints;


class XPopup
{
	public:
		XPopup();
		virtual ~XPopup();
		void show_at_pointer();
		void register_target(Display* dspl, Window wnd);
	private:
		void _init_opengl(int w, int h);
		void _drawself();
		void _hide();
		void _event_loop();
		void _resize(int w, int h);
		void _move_to_mousepointer();

		xwindow mWnd;
		xwindow mIWnd;
		xwindow mTarget;
		int mX, mY, mW, mH;
		bool mVisible;
		unsigned int displaywidth;
		unsigned int displayheight;
};

#endif

