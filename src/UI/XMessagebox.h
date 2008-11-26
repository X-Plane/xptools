#ifndef XMESSAGEBOX_H
#define XMESSAGEBOX_H

#include <X11/Xlib.h>

enum _msgbox
{
	GLDLG_OK,
	GLDLG_CANCEL,
	GLDLG_YES,
	GLDLG_NO,
	GLDLG_SAVE,
	GLDLG_DISCARD,
	GLDLG_CLOSED,
	GLDLG_ERROR
};

int XMessagebox(Window parent, const char* title, const char* message, int type, int defaultaction);

#endif

