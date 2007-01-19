#include "XPLMDisplay.h"
#include "XPLMPrivate.h"
#include "XPWidgetWin.h"

#include <algorithm>
#include <vector>
#include <list>

// Log the drawing callbacks, useful for debugging glue compliance.
#define DEBUG_DRAWING_PHASES 0

#if APL
#if defined(__MWERKS__)
#include <Events.h>
#else 
#include <Carbon/Carbon.h>
#endif
#endif

#if IBM
int ShiftToggle = 0;
int ControlToggle = 0;
int OptionKeyToggle = 0;
#endif

// BAS TODO:
// Resolve the "dragging" issue.

struct	XPLMDrawCallback_t {
	XPLMDrawCallback_f		callback;
	XPLMDrawingPhase		phase;
	int						preCallback;
	void *					refcon;
	
	bool	operator==(const XPLMDrawCallback_t& rhs) const {
		return ((callback == rhs.callback) && (phase == rhs.phase) &&
			(preCallback == rhs.preCallback) && (refcon == rhs.refcon));
		}
	
};
typedef	std::vector<XPLMDrawCallback_t>	XPLMDrawCallbackVector;

struct	XPLMKeySniffer_t {
	XPLMKeySniffer_f		callback;
	XPLMPluginID			plugin;
	int						beforeWindows;
	void * 					refcon;
	
	bool	operator==(const XPLMKeySniffer_t& rhs) const {
		return ((callback == rhs.callback) && (beforeWindows == rhs.beforeWindows) &&
			(refcon == rhs.refcon) && (plugin == rhs.plugin));
		}
};
typedef	std::vector<XPLMKeySniffer_t>	XPLMKeySnifferVector;

struct	XPLMWindow_t {
	int						left;
	int						top;
	int						right;
	int						bottom;
	int						visible;
	XPLMDrawWindow_f		drawCallback;
	XPLMHandleKey_f			keyCallback;
	XPLMHandleMouseClick_f	mouseCallback;
	XPLMPluginID			plugin;
	void *					refcon;
};

typedef	XPLMWindow_t *	XPLMWindowPtr;
typedef	std::list<XPLMWindowPtr>	XPLMWindowList;

XPLMDrawCallbackVector				gDrawCallbacks;
XPLMKeySnifferVector				gKeyCallbacks;
XPLMWindowList						gWindows;
XPLMWindowPtr						gFocusWindow = NULL;	// Keyboard focus
XPLMWindowPtr						gDragWindow = NULL;		// Mouse trapping

struct	XPLMHotKeyInfo_t {
	char			vkey;
	XPLMKeyFlags	flags;
	std::string		description;
	XPLMHotKey_f	callback;
	void *			refcon;
};

typedef	XPLMHotKeyInfo_t *				XPLMHotKeyInfoPtr;
typedef std::vector<XPLMHotKeyInfoPtr>	XPLMHotKeyVector;
XPLMHotKeyVector						gHotKeys;

static	XPLMWindowPtr		XPLMWindowIsValid(XPLMWindowID inWindow, XPLMWindowList::iterator * outIter);
static	int				XPLMDispatchDrawingHook(XPLMDrawingPhase inPhase, int isBefore);
static	XPLMHotKeyInfoPtr	XPLMValidateHotKey(XPLMHotKeyID inID, XPLMHotKeyVector::iterator * outIterator);
static	int					XPLMDispatchHotKey(char inVKey, XPLMKeyFlags inFlags);

// The following tables map virtual keys from their native versions to the x-platform version


#if APL

const char	xplm_Key_Map [256] = {
/*			00					01					02					03					04					05					06					07*/
/* 00 */	XPLM_VK_A,			XPLM_VK_S,			XPLM_VK_D,			XPLM_VK_F,			XPLM_VK_H,			XPLM_VK_G,			XPLM_VK_Z,			XPLM_VK_X,
/* 08 */	XPLM_VK_C,			XPLM_VK_V,			0,					XPLM_VK_B,			XPLM_VK_Q,			XPLM_VK_W,			XPLM_VK_E,			XPLM_VK_R,
/* 10 */	XPLM_VK_Y,			XPLM_VK_T,			XPLM_VK_1,			XPLM_VK_2,			XPLM_VK_3,			XPLM_VK_4,			XPLM_VK_6,			XPLM_VK_5,
/* 18 */	XPLM_VK_EQUAL,		XPLM_VK_9,			XPLM_VK_7,			XPLM_VK_MINUS,		XPLM_VK_8,			XPLM_VK_0,			XPLM_VK_RBRACE,		XPLM_VK_O,
/* 20 */	XPLM_VK_U,			XPLM_VK_LBRACE,		XPLM_VK_I,			XPLM_VK_P,			XPLM_VK_RETURN,		XPLM_VK_L,			XPLM_VK_J,			XPLM_VK_QUOTE,
/* 28 */	XPLM_VK_K,			XPLM_VK_SEMICOLON,	XPLM_VK_BACKSLASH,	XPLM_VK_COMMA,		XPLM_VK_SLASH,		XPLM_VK_N,			XPLM_VK_M,			XPLM_VK_PERIOD,
/* 30 */	XPLM_VK_TAB,		XPLM_VK_SPACE,		XPLM_VK_BACKQUOTE,	XPLM_VK_DELETE,		XPLM_VK_ENTER,		XPLM_VK_ESCAPE,		0,					0,
/* 38 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 40 */	0,					XPLM_VK_DECIMAL,	0,					XPLM_VK_MULTIPLY,	0,					XPLM_VK_ADD,		0,					XPLM_VK_CLEAR,
/* 48 */	0,					0,					0,					XPLM_VK_DIVIDE,		XPLM_VK_NUMPAD_ENT,	0,					XPLM_VK_SUBTRACT,	0,
/* 50 */	0,					XPLM_VK_NUMPAD_EQ,	XPLM_VK_NUMPAD0,	XPLM_VK_NUMPAD1,	XPLM_VK_NUMPAD2,	XPLM_VK_NUMPAD3,	XPLM_VK_NUMPAD4,	XPLM_VK_NUMPAD5,
/* 58 */	XPLM_VK_NUMPAD6,	XPLM_VK_NUMPAD7,	0,					XPLM_VK_NUMPAD8,	XPLM_VK_NUMPAD9,	0,					0,					0,
/* 60 */	XPLM_VK_F5,			XPLM_VK_F6,			XPLM_VK_F7,			XPLM_VK_F3,			XPLM_VK_F8,			XPLM_VK_F9,			0,					XPLM_VK_F11	,	
/* 68 */	0,					0,					0,					0,					0,					XPLM_VK_F10,		0,					XPLM_VK_F12,
/* 70 */	0,					0,					0,					XPLM_VK_HOME,		XPLM_VK_PRIOR,		0,					XPLM_VK_F4,			XPLM_VK_END,		
/* 78 */	XPLM_VK_F2,			XPLM_VK_NEXT,		XPLM_VK_F1,			XPLM_VK_LEFT,		XPLM_VK_RIGHT,		XPLM_VK_DOWN,		XPLM_VK_UP,			0
};

#else

const char	xplm_Key_Map [256] = {
/*			00					01					02					03					04					05					06					07*/
/* 00 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 08 */	XPLM_VK_BACK,		XPLM_VK_TAB,		0,					0,					XPLM_VK_CLEAR,		XPLM_VK_RETURN,		0,					0,
/* 10 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 18 */	0,					0,					0,					XPLM_VK_ESCAPE,		0,					0,					0,					0,
/* 20 */	XPLM_VK_SPACE,		XPLM_VK_PRIOR,		XPLM_VK_NEXT,		XPLM_VK_END,		XPLM_VK_HOME,		XPLM_VK_LEFT,		XPLM_VK_UP,			XPLM_VK_RIGHT,
/* 28 */	XPLM_VK_DOWN,		XPLM_VK_SELECT,		XPLM_VK_PRINT,		XPLM_VK_EXECUTE,	XPLM_VK_SNAPSHOT,	XPLM_VK_INSERT,		XPLM_VK_DELETE,		XPLM_VK_HELP,
/* 30 */	XPLM_VK_0,			XPLM_VK_1,			XPLM_VK_2,			XPLM_VK_3,			XPLM_VK_4,			XPLM_VK_5,			XPLM_VK_6,			XPLM_VK_7,
/* 38 */	XPLM_VK_8,			XPLM_VK_9,			0,					0,					0,					0,					0,					0,
/* 40 */	0,					XPLM_VK_A,			XPLM_VK_B,			XPLM_VK_C,			XPLM_VK_D,			XPLM_VK_E,			XPLM_VK_F,			XPLM_VK_G,
/* 48 */	XPLM_VK_H,			XPLM_VK_I,			XPLM_VK_J,			XPLM_VK_K,			XPLM_VK_L,			XPLM_VK_M,			XPLM_VK_N,			XPLM_VK_O,
/* 50 */	XPLM_VK_P,			XPLM_VK_Q,			XPLM_VK_R,			XPLM_VK_S,			XPLM_VK_T,			XPLM_VK_U,			XPLM_VK_V,			XPLM_VK_W,
/* 58 */	XPLM_VK_X,			XPLM_VK_Y,			XPLM_VK_Z,			0,					0,					0,					0,					0,
/* 60 */	XPLM_VK_NUMPAD0,	XPLM_VK_NUMPAD1,	XPLM_VK_NUMPAD2,	XPLM_VK_NUMPAD3,	XPLM_VK_NUMPAD4,	XPLM_VK_NUMPAD5,	XPLM_VK_NUMPAD6,	XPLM_VK_NUMPAD7,
/* 68 */	XPLM_VK_NUMPAD8,	XPLM_VK_NUMPAD9,	XPLM_VK_MULTIPLY,	XPLM_VK_ADD,		0,					XPLM_VK_SUBTRACT,	XPLM_VK_DECIMAL,	XPLM_VK_DIVIDE,
/* 70 */	XPLM_VK_F1,			XPLM_VK_F2,			XPLM_VK_F3,			XPLM_VK_F4,			XPLM_VK_F5,			XPLM_VK_F6,			XPLM_VK_F7,			XPLM_VK_F8,
/* 78 */	XPLM_VK_F9,			XPLM_VK_F10,		XPLM_VK_F11,		XPLM_VK_F12,		XPLM_VK_F13,		XPLM_VK_F14,		XPLM_VK_F15,		XPLM_VK_F16,
/* 80 */	XPLM_VK_F17,		XPLM_VK_F18,		XPLM_VK_F19,		XPLM_VK_F20,		XPLM_VK_F21,		XPLM_VK_F22,		XPLM_VK_F23,		XPLM_VK_F24,
/* 88 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 90 */	0,					0,					0,					0,					0,					0,					0,					0,
/* 88 */	0,					0,					0,					0,					0,					0,					0,					0,
/* A0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* A8 */	0,					0,					0,					0,					0,					0,					0,					0,
/* B0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* B8 */	0,					0,					XPLM_VK_SEMICOLON,	XPLM_VK_EQUAL,		XPLM_VK_COMMA,		XPLM_VK_MINUS,		XPLM_VK_PERIOD,		XPLM_VK_SLASH,
/* C0 */	XPLM_VK_BACKQUOTE,	0,					0,					0,					0,					0,					0,					0,
/* C8 */	0,					0,					0,					0,					0,					0,					0,					0,
/* D0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* D8 */	0,					0,					0,					XPLM_VK_LBRACE,		XPLM_VK_BACKSLASH,	XPLM_VK_RBRACE,		XPLM_VK_QUOTE,		XPLM_VK_BACKQUOTE,
/* E0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* E8 */	0,					0,					0,					0,					0,					0,					0,					0,
/* F0 */	0,					0,					0,					0,					0,					0,					0,					0,
/* F8 */	0,					0,					0,					0,					0,					0,					0,					0
};


#endif

int	XPLMRegisterDrawCallback(
					XPLMDrawCallback_f	inCallback,
					XPLMDrawingPhase	inPhase,
					int					inWantsBefore,
					void *				inRefcon)
{
		XPLMDrawCallback_t		newCB;
		
	newCB.callback = inCallback;
	newCB.phase = inPhase;
	newCB.preCallback = inWantsBefore;
	newCB.refcon = inRefcon;
	
	if (std::find(gDrawCallbacks.begin(), gDrawCallbacks.end(), newCB) ==
		gDrawCallbacks.end())
	{
		gDrawCallbacks.push_back(newCB);
		return 1;
	} else
		return 0;
	
}					

int	XPLMUnregisterDrawCallback(
					XPLMDrawCallback_f	inCallback,
					XPLMDrawingPhase	inPhase,
					int					inWantsBefore,
					void *				inRefcon)
{
		XPLMDrawCallback_t		newCB;
		
	newCB.callback = inCallback;
	newCB.phase = inPhase;
	newCB.preCallback = inWantsBefore;
	newCB.refcon = inRefcon;

	XPLMDrawCallbackVector::iterator	iter = std::find(gDrawCallbacks.begin(), gDrawCallbacks.end(), newCB);
	if (iter != gDrawCallbacks.end())
	{
		gDrawCallbacks.erase(iter);
		return 1;
	} else
		return 0;
	
}					

int XPLMRegisterKeySniffer(
					XPLMKeySniffer_f	inCallback,
					int					inBeforeWindows,
					void *				inRefcon)
{
		XPLMKeySniffer_t		newCB;
		
	newCB.callback = inCallback;
	newCB.beforeWindows = inBeforeWindows;
	newCB.refcon = inRefcon;
	
	if (std::find(gKeyCallbacks.begin(), gKeyCallbacks.end(), newCB) ==
		gKeyCallbacks.end())
	{
		gKeyCallbacks.push_back(newCB);
		return 1;
	} else
		return 0;	
}

int	XPLMUnregisterKeySniffer(
					XPLMKeySniffer_f	inCallback,
					int					inBeforeWindows,
					void *				inRefcon)
{
		XPLMKeySniffer_t		newCB;
		
	newCB.callback = inCallback;
	newCB.beforeWindows = inBeforeWindows;
	newCB.refcon = inRefcon;
	
	XPLMKeySnifferVector::iterator iter = std::find(gKeyCallbacks.begin(), gKeyCallbacks.end(), newCB);
	
	if (iter != gKeyCallbacks.end())
	{
		gKeyCallbacks.erase(iter);
		return 1;
	} else
		return 0;	
}					

void		XPLMGetScreenSize(
					int *				outWidth,
					int *				outHeight)
{
	gWidgetWin->GetBounds(outWidth, outHeight);
}

void		XPLMGetMouseLocation(
					int *				outX,
					int *				outY)
{
		int	h, v;
	gWidgetWin->GetBounds(&h, &v);

#if APL
	Point	p;
	GetMouse(&p);
	if (outX) *outX = p.h;
	if (outY) 
	{
		*outY = v - p.v;
	}
#endif
#if IBM
	gWidgetWin->GetMouseLoc(outX, outY);
	if (outY) *outY = v - *outY;
#endif
}					

XPLMKeyFlags XPLMGetModifiers(void)
{
#if APL
	UInt32	mods = GetCurrentKeyModifiers();
	
	XPLMKeyFlags	flags = 0;
	
	if (mods & shiftKey)
		flags |= xplm_ShiftFlag;
	if (mods & cmdKey)
		flags |= xplm_ControlFlag;
	if (mods & optionKey)
		flags |= xplm_OptionAltFlag;
	return flags;
#endif
#if IBM
	XPLMKeyFlags	flags = 0;
	
	if (::GetKeyState(VK_SHIFT) & ~1)
		flags |= xplm_ShiftFlag;
	if (::GetKeyState(VK_CONTROL) & ~1)
		flags |= xplm_ControlFlag;
	if (::GetKeyState(VK_MENU) & ~1)
		flags |= xplm_OptionAltFlag;
	return flags;
#endif
}


#pragma mark -

XPLMWindowID XPLMCreateWindow(
					int						inLeft,
					int						inTop,
					int						inRight,
					int						inBottom,
					int						inIsVisible,
					XPLMDrawWindow_f		inDrawCallback,
					XPLMHandleKey_f			inKeyCallback,
					XPLMHandleMouseClick_f	inMouseCallback,
					void *					inRefCon)
{
		XPLMWindowPtr	newWindow = new XPLMWindow_t;
	
	newWindow->left = inLeft;
	newWindow->top = inTop;
	newWindow->right = inRight;
	newWindow->bottom = inBottom;
	newWindow->visible = inIsVisible;
	newWindow->drawCallback = inDrawCallback;
	newWindow->keyCallback = inKeyCallback;
	newWindow->mouseCallback = inMouseCallback;
	newWindow->refcon = inRefCon;
	
	gWindows.push_front(newWindow);
	
	return (XPLMWindowID) newWindow;
}					

void		XPLMDestroyWindow(
					XPLMWindowID 		inWindowID)
{
	XPLMWindowList::iterator iter;
	XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, &iter);
	if (theWindow == NULL)
		return;
		
	if (theWindow == gFocusWindow)
		XPLMTakeKeyboardFocus(NULL);

	if (theWindow == gDragWindow)
		gDragWindow = NULL;

	gWindows.erase(iter);
	delete theWindow;
	
}					

void		XPLMGetWindowGeometry(
					XPLMWindowID		inWindowID,
					int	*				outLeft,
					int	*				outTop,
					int	*				outRight,
					int	*				outBottom)
{
		XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, NULL);

	if (theWindow == NULL)
		return;
		
	if (outLeft)
		*outLeft = theWindow->left;
	if (outTop)
		*outTop = theWindow->top;
	if (outRight)
		*outRight = theWindow->right;
	if (outBottom)
		*outBottom = theWindow->bottom;
}					

void		XPLMSetWindowGeometry(
					XPLMWindowID		inWindowID,
					int					inLeft,
					int					inTop,
					int					inRight,
					int					inBottom)
{	
		XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, NULL);

	if (theWindow == NULL)
		return;

	theWindow->left = inLeft;
	theWindow->top = inTop;
	theWindow->right = inRight;
	theWindow->bottom = inBottom;
}					
					

int			XPLMGetWindowIsVisible(
					XPLMWindowID			inWindowID)
{
		XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, NULL);

	if (theWindow == NULL)
		return 0;
		
	return theWindow->visible;
}					
	
void		XPLMSetWindowIsVisible(
					XPLMWindowID			inWindowID,
					int						inIsVisible)
{
		XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, NULL);

	if (theWindow == NULL)
		return;
		
	if ((theWindow->visible) && (!inIsVisible) && (theWindow == gFocusWindow))
		XPLMTakeKeyboardFocus(NULL);
	theWindow->visible = inIsVisible;
}					
					

void *		XPLMGetWindowRefCon(
					XPLMWindowID			inWindowID)
{
		XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, NULL);

	if (theWindow == NULL)
		return NULL;
	
	return theWindow->refcon;
}					

void 		XPLMSetWindowRefCon(
					XPLMWindowID			inWindowID,
					void *					inRefcon)
{
		XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, NULL);

	if (theWindow == NULL)
		return;
	
	theWindow->refcon = inRefcon;
}					


void		XPLMTakeKeyboardFocus(
					XPLMWindowID			inWindowID)
{
		XPLMWindowPtr	theWindow = XPLMWindowIsValid(inWindowID, NULL);
		
	if (theWindow == gFocusWindow)
		return;

	// DO NOT focus a window for a disabled plugin!  (Unlikely, but a concern.)
		
	if (gFocusWindow != NULL)
	{
		gFocusWindow->keyCallback(
					inWindowID, 			
					0,						// ASCII
					0,						// Flags
					0,						// Virtual Key
					gFocusWindow->refcon,	// Refcon
					1);						// Losing focus
	}	
	gFocusWindow = theWindow;
}
					

void		XPLMBringWindowToFront(
					XPLMWindowID			inWindowID)
{
		XPLMWindowList::iterator	iter;
		XPLMWindowPtr				theWindow = XPLMWindowIsValid(inWindowID, &iter);
	
	if (theWindow == NULL)
		return;
	
	gWindows.erase(iter);
	gWindows.push_front(theWindow);	
}					
					
int			XPLMIsWindowInFront(
					XPLMWindowID			inWindowID)
{
		XPLMWindowPtr		theWindow = XPLMWindowIsValid(inWindowID, NULL);
		
	if (theWindow == NULL)
		return false;
	
	for (XPLMWindowList::iterator iter = gWindows.begin(); iter != gWindows.end(); ++iter)
	{
		if ((*iter)->visible)
			return (*iter) == theWindow;
	}

		return false;
}	

#pragma mark -

void			XPLMDisplayDisableHook(XPLMPluginID inWho)
{
	if (gFocusWindow && gFocusWindow->plugin == inWho)
	{
		XPLMTakeKeyboardFocus(NULL);
	}
	if (gDragWindow && gDragWindow->plugin == inWho)
	{
		int	x, y;
		XPLMGetMouseLocation(&x, &y);
		XPLMDisplayMouseClickHook(x, y, xplm_MouseUp, 0);
	}
}

void		XPLMDisplayDoDrawingHook(
						void)
{
	for (XPLMWindowList::reverse_iterator iter = gWindows.rbegin(); iter != gWindows.rend(); ++iter)
	{
		if ((*iter)->visible)
		{
			(*iter)->drawCallback(
							(XPLMWindowID) (*iter),
							(*iter)->refcon);
		}
	}	
}

void		XPLMDisplayMouseClickHook(
						int 			inX, 
						int 			inY,
						XPLMMouseStatus	inStatus,
						int				inButton)
{
	// If the mouse status is a down click, we dispatch directly to that window.  If it is a drag or up click,
	// it always goes to the previously clicked window.

	if (inStatus == xplm_MouseDown || inStatus == xplm_MouseWheel)
	{
		for (XPLMWindowList::iterator iter = gWindows.begin(); iter != gWindows.end(); ++iter)
		{
			if (((*iter)->visible) && 
				(inX >= (*iter)->left) && 
				(inX < (*iter)->right) && 
				(inY < (*iter)->top) &&
				(inY >= (*iter)->bottom))
			{
				XPLMWindowPtr	clickWindow = *iter; // BAS - assign iter first, once we dispatch the mouse click, our iterator could be toast.
										
				{
					if ((*iter)->mouseCallback((XPLMWindowID) *iter, inX, inY, inStatus, inButton, (*iter)->refcon))
					{
						// Only if the window click returned full do we remember what window we are dragging in and eat the click.
						if (inStatus == xplm_MouseDown)
							gDragWindow = clickWindow;
						return; // eat the click
					}
				}
				// If we didn't dispatch, treat us as transparent.
			}
		}
		return; // Don't eat the click
	} else {
		int	retVal = 1; // Default is to not eat the click
		if (gDragWindow == NULL)	
			return; // Pass to x-plane
		{
			{
				gDragWindow->mouseCallback((XPLMWindowID) gDragWindow, inX, inY, inStatus, inButton, gDragWindow->refcon);
				retVal = 0; // Eat it
			}
		}
		if (inStatus == xplm_MouseUp)
			gDragWindow = NULL;
		return;
	}
}

int		XPLMDisplayKeyPressHook(
						long			inMsg,
						long			inParam1,
						long			inParam2)
{
	XPLMKeyFlags		flags = 0;

#if APL
	char		charCode = 0;
	char		virtualCode = 0;
#elif IBM
	unsigned char	charCode = 0;
	unsigned char	virtualCode = 0;

	HKL hKL;
	unsigned int vKey, RetCode, ScanCode;
	unsigned short Char = 0;
	BYTE KeyState[256];
	long shiftKey, controlKey, optionKey, keyDown, keyUp, charCodeMask,	keyCodeMask;
	long ExtKeyMask, ShiftControlMask, scrollLockKey, capsLockKey, numLockKey;
	int ExtendedKey;
#endif

#if APL

	charCode = inParam1 & charCodeMask;
	virtualCode = (inParam1 & keyCodeMask) >> 8L;
	if (inParam2 & shiftKey)
		flags |= xplm_ShiftFlag;
	if (inParam2 & cmdKey)
		flags |= xplm_ControlFlag;
	if (inParam2 & optionKey)
		flags |= xplm_OptionAltFlag;
	if (inMsg == keyDown)
		flags |= xplm_DownFlag;
	if (inMsg == keyUp)
		flags |= xplm_UpFlag;
		
	// NOTE: the XPLM_KEY ASCII defines are all mac-compatible.

	// Finally, control and option keys are not available as ASCII because
	// the ASCII codes are whacko.
	if (inParam2 & (controlKey + optionKey + cmdKey))
		charCode = 0;

#elif IBM
	numLockKey = 0x01450000;
	capsLockKey =0x003a0000;
	scrollLockKey = 0x00460000;
	shiftKey = 0x002a0000;
	controlKey = 0x001d0000;
	optionKey = 0x00380000;
	keyDown = WM_KEYDOWN;
	keyUp = WM_KEYUP;
	charCodeMask = 0xff;
	keyCodeMask = 0xff;
	ShiftControlMask = 0x00ff0000;
	ExtKeyMask = 0x01ff0000;

	if (((inParam2 & ExtKeyMask) == numLockKey) || ((inParam2 & ShiftControlMask) == capsLockKey) || ((inParam2 & ShiftControlMask) == scrollLockKey))
		return 1;

	if (((inParam2 & ShiftControlMask) == shiftKey) || (inParam1 == VK_SHIFT))
	{
		if (inMsg == keyDown)
			ShiftToggle = 1;
		else
			ShiftToggle = 0;

		return 1;
	}

	if ((inParam2 & ShiftControlMask) == controlKey)
	{
		if (inMsg == keyDown)
			ControlToggle = 1;
		else
			ControlToggle = 0;

		return 1;
	}

	/// SB
	if ((inParam2 & ShiftControlMask) == optionKey)
	{
		if (inMsg == keyDown)
			OptionKeyToggle = 1;
		else
			OptionKeyToggle = 0;

		return 1;
	}

	hKL = GetKeyboardLayout(NULL); 
	ScanCode = ((inParam2 >> 16) & 0xff);
	ExtendedKey =  ((inParam2 >> 24) & 0x01);
	vKey = MapVirtualKeyEx(ScanCode, 1, hKL); 
	RetCode = GetKeyboardState((unsigned char*)&KeyState);
	RetCode = ToAsciiEx(vKey, ScanCode, (unsigned char*)&KeyState, &Char, 0, hKL); 

	if (RetCode != 0)
	{
		charCode = Char;
		if (ExtendedKey == 0)
			virtualCode = vKey;
		else
			virtualCode = inParam1 & keyCodeMask;
	}
	else
	{
		charCode = inParam1 & charCodeMask;
		virtualCode = inParam1 & keyCodeMask;
	}

	if (ShiftToggle)
		flags |= xplm_ShiftFlag;
	if (ControlToggle)
		flags |= xplm_ControlFlag;
///	SB if ((inParam2 & ShiftControlMask) == optionKey)
	if (OptionKeyToggle)
		flags |= xplm_OptionAltFlag;
	if (inMsg == keyDown)
		flags |= xplm_DownFlag;
	if (inMsg == keyUp)
		flags |= xplm_UpFlag;
		
	// NOTE: the XPLM_KEY ASCII defines are all mac-compatible.
		
	// Finally, control and option keys are not available as ASCII because
	// the ASCII codes are whacko.
	if ( ((inParam2 & ShiftControlMask) == controlKey) || ((inParam2 & ShiftControlMask) == optionKey))
		charCode = 0;
				
#else
	#error "Must port XPLM to a new OS...key bindings come in in an OS native form."
#endif

	virtualCode = xplm_Key_Map[virtualCode];

#if IBM
	switch (virtualCode)
	{
		case XPLM_VK_RETURN:
			charCode = XPLM_KEY_RETURN;
			break;
		case XPLM_VK_ESCAPE:
			charCode = XPLM_KEY_ESCAPE;
			break;
		case XPLM_VK_TAB:
			charCode = XPLM_KEY_TAB;
			break;
		case XPLM_VK_BACK:
			charCode = XPLM_KEY_DELETE;
			break;
		case XPLM_VK_LEFT:
			charCode = XPLM_KEY_LEFT;
			break;
		case XPLM_VK_UP:
			charCode = XPLM_KEY_UP;
			break;
		case XPLM_VK_RIGHT:
			charCode = XPLM_KEY_RIGHT;
			break;
		case XPLM_VK_DOWN:
			charCode = XPLM_KEY_DOWN;
			break;
		case XPLM_VK_NUMPAD0:
			charCode = XPLM_KEY_0;
			break;
		case XPLM_VK_NUMPAD1:
			charCode = XPLM_KEY_1;
			break;
		case XPLM_VK_NUMPAD2:
			charCode = XPLM_KEY_2;
			break;
		case XPLM_VK_NUMPAD3:
			charCode = XPLM_KEY_3;
			break;
		case XPLM_VK_NUMPAD4:
			charCode = XPLM_KEY_4;
			break;
		case XPLM_VK_NUMPAD5:
			charCode = XPLM_KEY_5;
			break;
		case XPLM_VK_NUMPAD6:
			charCode = XPLM_KEY_6;
			break;
		case XPLM_VK_NUMPAD7:
			charCode = XPLM_KEY_7;
			break;
		case XPLM_VK_NUMPAD8:
			charCode = XPLM_KEY_8;
			break;
		case XPLM_VK_NUMPAD9:
			charCode = XPLM_KEY_9;
			break;
		case XPLM_VK_DECIMAL:
			charCode = XPLM_KEY_DECIMAL;
			break;
	}
#endif

	if ((flags == 0) && (charCode == 0) && (virtualCode == 0))
		return 1;

	// First we have to find out if any pre-window key sniffers care about the key...
	
	XPLMKeySnifferVector::iterator iter;
	for (iter = gKeyCallbacks.begin(); iter != gKeyCallbacks.end(); ++iter)
	{
		if (iter->beforeWindows)
		{
			if (!iter->callback(charCode, flags, virtualCode, iter->refcon))
				return 0;
	}
	}
	
	if (XPLMDispatchHotKey(virtualCode, flags))
		return 0;
	
	// Next if we have a window in focus, it gets a crack at it.
	// Note that in focus windows ALWAYS eat the keystroke.  If they don't want it,
	// they should lose focus!  (This may change once I think about it carefully. :-)

	if (gFocusWindow != NULL)
	{
		{
		gFocusWindow->keyCallback(
					(XPLMWindowID) gFocusWindow,
					charCode,
					flags,
					virtualCode,
					gFocusWindow->refcon,
					0);		// not losing focus
		return 0;
	}
	}

	// No window in focus, we try post-window key sniffers...

	for (iter = gKeyCallbacks.begin(); iter != gKeyCallbacks.end(); ++iter)
	{
		if (!iter->beforeWindows)
		{
			if (!iter->callback(charCode, flags, virtualCode, iter->refcon))
				return 0;
	}
	}
	
	// If we get this far, XP gets the keystroke.
	
	return 1;
}						

#pragma mark -

static	XPLMWindowPtr	XPLMWindowIsValid(XPLMWindowID inWindow, XPLMWindowList::iterator * outIter)
{
		XPLMWindowPtr	theWindow = (XPLMWindowPtr) inWindow;
	XPLMWindowList::iterator iter = std::find(gWindows.begin(), gWindows.end(),theWindow);
	if (outIter != NULL)
		*outIter = iter;
	return (iter != gWindows.end()) ? theWindow : NULL;
}

static	int				XPLMDispatchDrawingHook(XPLMDrawingPhase inPhase, int isBefore)
{
		int	result = 1;

	for (XPLMDrawCallbackVector::iterator iter = gDrawCallbacks.begin(); 
		iter != gDrawCallbacks.end(); ++iter)
	{
		if ((iter->phase == inPhase) &&
			(iter->preCallback == isBefore))
		{
			result &= iter->callback(inPhase, isBefore, iter->refcon);
		}
	}
	return result;
}

#pragma mark -

XPLMHotKeyID	XPLMRegisterHotKey(
									char			inVirtualKey,
									XPLMKeyFlags	inFlags,
									const char *	inDescription,
									XPLMHotKey_f	inCallback,
									void *			inRefcon)
{
	XPLMHotKeyInfoPtr newID = new XPLMHotKeyInfo_t;
	newID->vkey = inVirtualKey;
	newID->flags = inFlags;
	newID->description = std::string(inDescription);
	newID->callback = inCallback;
	newID->refcon = inRefcon;
	gHotKeys.push_back(newID);
	return newID;
}									
									
void			XPLMUnregisterHotKey(
									XPLMHotKeyID	inHotKey)
{
	XPLMHotKeyVector::iterator iter;
	if (XPLMValidateHotKey(inHotKey, &iter))
	{
		gHotKeys.erase(iter);
	}
}									
									
long			XPLMCountHotKeys(void)
{
	return gHotKeys.size();
}

XPLMHotKeyID	XPLMGetNthHotKey(
									long 			inIndex)
{
	if ((inIndex >= 0) && (inIndex < gHotKeys.size()))
		return gHotKeys[inIndex];
	else
		return NULL;									
}
									
void			XPLMGetHotKeyInfo(
									XPLMHotKeyID	inHotKey,
									char *			outVirtualKey,
									XPLMKeyFlags *	outFlags,
									char *			outDescription,
									XPLMPluginID *	outPlugin)
{
	XPLMHotKeyInfoPtr	info = XPLMValidateHotKey(inHotKey, NULL);
	if (info)
	{
		if (outVirtualKey)
			*outVirtualKey = info->vkey;
		if (outFlags)
			*outFlags = info->flags;
		if (outDescription)
			strcpy(outDescription, info->description.c_str());
	}
}									
									
void			XPLMSetHotKeyCombination(
									XPLMHotKeyID	inHotKey,
									char			inVirtualKey,
									XPLMKeyFlags	inFlags)
{
	XPLMHotKeyInfoPtr	info = XPLMValidateHotKey(inHotKey, NULL);
	if (info)
	{
		info->vkey = inVirtualKey;
		info->flags = inFlags;
	}
}									

XPLMHotKeyInfoPtr	XPLMValidateHotKey(XPLMHotKeyID inID, XPLMHotKeyVector::iterator * outIterator)
{
	XPLMHotKeyVector::iterator iter = std::find(gHotKeys.begin(), gHotKeys.end(), (XPLMHotKeyInfoPtr) inID);
	if (outIterator)
		*outIterator = iter;
	if (iter == gHotKeys.end())
		return NULL;
	return *iter;
}

int					XPLMDispatchHotKey(char inVKey, XPLMKeyFlags inFlags)
{
	for (XPLMHotKeyVector::iterator iter = gHotKeys.begin(); iter != gHotKeys.end(); ++iter)
	{
		if ((*iter)->vkey == inVKey && (*iter)->flags == inFlags)
		{
			{
				(*iter)->callback((*iter)->refcon);
				return 1;
			}
		}
	}
	return 0;
}

void XPLMDisplayCleanupHook(void)
{
	gDrawCallbacks.clear();
	gKeyCallbacks.clear();
	for (XPLMWindowList::iterator iter = gWindows.begin(); iter != gWindows.end(); ++iter)
	{
		delete (*iter);
	}
	gWindows.clear();
	gFocusWindow = NULL;
	gDragWindow = NULL;
	for (XPLMHotKeyVector::iterator iter2 = gHotKeys.begin(); iter2 != gHotKeys.end(); ++iter2)
	{
		delete (*iter2);
	}
	gHotKeys.clear();
}