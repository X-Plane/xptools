/* 
 * Copyright (c) 2004, Ben Supnik and Sandy Barbour.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */
#include "XPWidgets.h"
#include "XPLMDisplay.h"
#include "XPLMUtilities.h"
#include <algorithm>
#include <list>
#include <string>
#include <string.h>
#if APL
#include <OpenGL/gl.h>
#else
#include <gl.h>
#endif

#include <stdio.h>
#include "XPStandardWidgetsPrivate.h"
#include <map>
#include <set>
#include <vector>

#define	NO_ROUGH_WIDGET_CULL	0

// Callbacks are kept in a simple list.
typedef	std::vector<XPWidgetFunc_t>		XPCallbackList;

// And children in a simple vector (for indexing)
typedef	std::vector<XPWidgetID>			XPWidgetList;

// Properties are kept in a map for speed
typedef	std::map<XPWidgetPropertyID, long>	XPPropertyTable;

// This is the info in an actual widget.

struct	XPWidgetInfo {
	
	XPCallbackList			callbacks;
	
	// Our geometry.
	int						left;
	int						top;
	int						right;
	int						bottom;

	// Our visible geometry.
	int						vis_left;
	int						vis_top;
	int						vis_right;
	int						vis_bottom;
	int						vis_valid;	// 1 if the visible geometry is up to date.

	int						visible;

	XPLMWindowID			root;
	
	XPWidgetID				parent;
	XPWidgetList			children;
	
	XPPropertyTable			properties;
	std::string				descriptor;

};

typedef	XPWidgetInfo *			XPWidgetPtr;

// We maintain a set of widget pointers...this lets us very quickly validate whether
// a widget ID is valid.  We don't ever need to traverse the whole list so the fact
// that the order is arbitrary is ok.
typedef	std::set<XPWidgetPtr>	XPWidgetSet;

static	XPWidgetSet			gWidgets;						// All widets
static	XPWidgetID			gFocusWidget = NULL;			// Keyboard focused widget
static	int					gInternalFocusChange = 0;		// Reentrancy flag
static	XPWidgetID			gMouseWidget = NULL;			// Widget handling mouse drag

// Helper and callback routines
static	XPWidgetPtr		XPFindWidgetInfo(XPWidgetID inID, XPWidgetSet::iterator * outIter);
static	void			XPWidgetDraw(XPLMWindowID inWindowID, void * inRefcon);
static	void			XPWidgetKey(XPLMWindowID inWindowID, char inKey, XPLMKeyFlags inFlags, char vkey, void * inRefcon, int losingFocus);
static	int				XPWidgetMouse(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, int button, void * inRefcon);
static	void			XPRebuildExposureCache(XPWidgetID inWidget);
static	void			XPInvalidateExposureCache(XPWidgetID inWidget);
static	void			XPPaintOneWidgetRecursive(XPWidgetID inWidget);

XPWidgetID		XPCreateWidget(
						int				inLeft,
						int				inTop,
						int				inRight,
						int				inBottom,
						int				inVisible,
						const char *	inDescriptor,
						int				inIsRoot,
						XPWidgetID		inContainer,
						XPWidgetClass	inClass)
{
		XPWidgetFunc_t	func;
	
	func = XPGetWidgetClassFunc(inClass);
	if (func == NULL)
		return NULL;
		
	return XPCreateCustomWidget(inLeft, inTop, inRight, inBottom,
		inVisible, inDescriptor, inIsRoot, inContainer, func);
}						

XPWidgetID		XPCreateCustomWidget(
						int				inLeft,
						int				inTop,
						int				inRight,
						int				inBottom,
						int				inVisible,
						const char *	inDescriptor,
						int				inIsRoot,
						XPWidgetID		inContainer,
						XPWidgetFunc_t  inCallback)
{
		XPLMWindowID	rootID = NULL;
		
	if (inIsRoot && (inContainer != NULL))
		return NULL;
		
	if (inIsRoot)
	{
		rootID = XPLMCreateWindow(inLeft, inTop, inRight, inBottom, inVisible,
					XPWidgetDraw, XPWidgetKey, XPWidgetMouse, NULL);
		if (rootID == NULL)
			return NULL;
	}
		
	XPWidgetInfo *		info = new XPWidgetInfo;
	
	info->callbacks.insert(info->callbacks.begin(), inCallback);
	info->left = inLeft;
	info->top = inTop;
	info->right = inRight;
	info->bottom = inBottom;
	info->visible = inVisible;
	info->root = rootID;
	info->parent = NULL;
	info->vis_valid = 0;
	if (inDescriptor != NULL)
		info->descriptor = std::string(inDescriptor);

	XPLMSetWindowRefCon(rootID, info);
	
	gWidgets.insert(info);
	
	XPSendMessageToWidget((XPWidgetID) info, xpMsg_Create, xpMode_Direct, 0/*creating*/, 0);

	if (inContainer != NULL)
		XPPlaceWidgetWithin((XPWidgetID) info, inContainer);

	return (XPWidgetID)	info;
}						


void			XPDestroyWidget(
						XPWidgetID		inWidget,
						int				inDestroyChildren)
{
	static	int		recursive = 0;

		XPWidgetSet::iterator	iter;

	XPWidgetPtr	me = XPFindWidgetInfo(inWidget, &iter);
	if (me == NULL)
		return;
	
	XPLoseKeyboardFocus(inWidget);

	if (me->parent != NULL)
		XPPlaceWidgetWithin(inWidget, NULL);
	
	XPWidgetList	children = me->children;
	for (XPWidgetList::iterator iter2 = children.begin(); iter2 != children.end(); ++iter2)
		XPPlaceWidgetWithin(*iter2, NULL/*no parent*/);

	if (inDestroyChildren)
	{
		recursive = 1;
		for (XPWidgetList::iterator iter3 = children.begin(); iter3 != children.end(); ++iter3)
			XPDestroyWidget(*iter3, inDestroyChildren);
		recursive = 0;
	}

	XPSendMessageToWidget(inWidget, xpMsg_Destroy, xpMode_DirectAllCallbacks, recursive/*delete by par*/, 0);
	
	if (me->root != NULL)
		XPLMDestroyWindow(me->root);
	
	gWidgets.erase(iter);
	
	if (gMouseWidget == inWidget)
		gMouseWidget = NULL;
	
	delete me;
}
						
int				XPSendMessageToWidget(
						XPWidgetID		inWidget,				
						XPWidgetMessage	inMessage,
						XPDispatchMode	inMode,
						long			inParam1,
						long			inParam2)
{
	XPWidgetPtr	me = XPFindWidgetInfo(inWidget, NULL);
	XPWidgetList::iterator childIter;

	if (me == NULL)
		return 0;
	
		int handled = 0;	
	
	for (XPCallbackList::iterator iter = me->callbacks.begin(); iter !=
		me->callbacks.end(); ++iter)
	{
		int	result = (*iter)(inMessage, inWidget, inParam1, inParam2);
		
		if (result)
			handled = 1;
			
		if (inMode == xpMode_Once)
			return handled;
			
		if (result && (inMode != xpMode_DirectAllCallbacks))
			break;
	}
	
	switch(inMode) {
	case xpMode_Direct:
	case xpMode_DirectAllCallbacks:
		return handled;
	case xpMode_UpChain:
		if (!handled && me->parent != NULL)
			return XPSendMessageToWidget(me->parent, inMessage, inMode, inParam1, inParam2);
		else
			return handled;
	case xpMode_Recursive:
		for (childIter = me->children.begin(); 
			childIter != me->children.end(); ++childIter)
		{
			XPSendMessageToWidget(*childIter, inMessage, inMode, inParam1, inParam2);
		}
		return handled;
	// BAS - xpMode_RecursiveVisible is depreciated....it has been replaced with a more complex
	// and industrial drawing mechanism...see XPPaintOneWidgetRecursive.
#if 0
	case xpMode_RecursiveVisible:
		for (childIter = me->children.begin(); 
			childIter != me->children.end(); ++childIter)
		{
			XPWidgetInfo * child = XPFindWidgetInfo(*childIter, NULL);
			if ((child != NULL) && (child->visible))
				XPSendMessageToWidget(*childIter, inMessage, inMode, inParam1, inParam2);		
		}
		return handled;
#endif		
	default:
		return handled;
	}
}						
						
#pragma mark -

void			XPPlaceWidgetWithin(
						XPWidgetID		inSubWidget,
						XPWidgetID		inContainer)
{
	XPWidgetInfo *	child = XPFindWidgetInfo(inSubWidget, NULL);
	if (child == NULL)
		return;
	
	XPWidgetInfo *	oldParent = XPFindWidgetInfo(child->parent, NULL);
	XPWidgetInfo *	newParent = XPFindWidgetInfo(inContainer, NULL);
	
	if (oldParent)
	{
		XPSendMessageToWidget(child->parent, xpMsg_LoseChild, xpMode_Direct,
				(long) inSubWidget, 0);
		XPWidgetList::iterator iter = std::find(oldParent->children.begin(),
			oldParent->children.end(), inSubWidget);
		if (iter != oldParent->children.end())
			oldParent->children.erase(iter);
	}
	
	XPInvalidateExposureCache(inSubWidget);
	
	XPSendMessageToWidget(inSubWidget, xpMsg_AcceptParent, xpMode_Direct,
				(long) inContainer, 0);
	child->parent = inContainer;
	if (newParent)
	{
		newParent->children.push_back(inSubWidget);
		XPSendMessageToWidget(inContainer, xpMsg_AcceptChild, xpMode_Direct,
				(long) inSubWidget, 0);
	}
}

int			XPCountChildWidgets(
						XPWidgetID		inWidget)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return 0;
	return me->children.size();
}
						
XPWidgetID		XPGetNthChildWidget(
						XPWidgetID		inWidget,
						long			inIndex)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if ((me == NULL) ||
		(inIndex < 0) ||
		(inIndex >= me->children.size()))
	{
		return NULL;
	}
	
	return me->children[inIndex];
}						

XPWidgetID		XPGetParentWidget(
						XPWidgetID		inWidget)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);

	return (me == NULL) ? NULL : me->parent;
}						

void			XPShowWidget(
						XPWidgetID		inWidget)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me && !me->visible)
	{
		me->visible = 1;
		if (me->root != NULL)
			XPLMSetWindowIsVisible(me->root, 1);
		XPSendMessageToWidget(inWidget, xpMsg_Shown, xpMode_UpChain, (long) inWidget, 0);
	}
}

void			XPHideWidget(
						XPWidgetID		inWidget)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me && me->visible)
	{
		me->visible = 0;
		if (me->root != NULL)
			XPLMSetWindowIsVisible(me->root, 0);
		XPSendMessageToWidget(inWidget, xpMsg_Hidden, xpMode_UpChain, (long) inWidget, 0);
	}
}						

int				XPIsWidgetVisible(
						XPWidgetID		inWidget)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	while (1)
	{
		// If we're a bad widget or we're hidden, that's it.
		if ((me == NULL) || (!me->visible))
			return 0;
		// If we're not hidden and we have no parent, the whole chain is visible.
		if (me->parent == NULL)
			return 1;
		// We're not hidden and we do have a parent, crawl up the chain.
		me = XPFindWidgetInfo(me->parent, NULL);
	}
}						

XPWidgetID		XPFindRootWidget(XPWidgetID inWidget)
{
	XPWidgetPtr	me = XPFindWidgetInfo(inWidget, NULL);
	while (me != NULL)
	{
		if (me->root != NULL)
			return (XPWidgetID) me;
		me = XPFindWidgetInfo(me->parent, NULL);
	}
	return NULL;
}

void			XPBringRootWidgetToFront(
						XPWidgetID		inWidget)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me != NULL)
	{
		if (me->root != NULL)
			XPLMBringWindowToFront(me->root);
		else
			XPBringRootWidgetToFront(me->parent);
	}
}						

int			XPIsWidgetInFront(
						XPWidgetID		inWidget)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	while (me != NULL)
	{	
		if (me->root != NULL)
		{
			return XPLMIsWindowInFront(me->root);
		}	
			
		me = XPFindWidgetInfo(me->parent, NULL);
	}
	return 0;
}						
						
void			XPGetWidgetGeometry(
						XPWidgetID		inWidgetID,
						int *			outLeft,
						int *			outTop,
						int *			outRight,
						int *			outBottom)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidgetID, NULL);
	if (me == NULL)
		return;
	if (outLeft)	*outLeft = me->left;
	if (outTop)		*outTop = me->top;
	if (outRight)	*outRight = me->right;
	if (outBottom)	*outBottom = me->bottom;
}						

void			XPSetWidgetGeometry(
						XPWidgetID 		inWidgetID,
						int 			inLeft,
						int 			inTop,
						int 			inRight,
						int 			inBottom)
{
	XPWidgetPtr me = XPFindWidgetInfo(inWidgetID, NULL);
	if (me == NULL)
		return;

	XPWidgetGeometryChange_t	deltas;
	deltas.dx = inLeft - me->left;
	deltas.dy = inBottom - me->bottom;
	deltas.dwidth = (inRight - inLeft) - (me->right - me->left);
	deltas.dheight = (inTop - inBottom) - (me->top - me->bottom);
		
	me->left = inLeft;
	me->top = inTop;
	me->right = inRight;
	me->bottom = inBottom;
	
	if (me->root != NULL)
		XPLMSetWindowGeometry(me->root, inLeft, inTop, inRight, inBottom);
	
	XPInvalidateExposureCache(inWidgetID);
	
	XPSendMessageToWidget(inWidgetID, xpMsg_Reshape, xpMode_UpChain, 
		(long) inWidgetID, (long) &deltas);
}						
 
XPWidgetID		XPGetWidgetForLocation(
						XPWidgetID		inContainer,
						int				x,
						int				y,
						int 			inRecursive,
						int				inVisibleOnly)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inContainer, NULL);
	if (me == NULL)
		return NULL;
	
	if ((x < me->left) ||
		(x > me->right) ||
		(y < me->bottom) ||
		(y > me->top) ||
		(!me->visible && inVisibleOnly))
	{
		return NULL;
	}
	
	if (inRecursive)
	{
		for (XPWidgetList::reverse_iterator iter = me->children.rbegin(); iter
			!= me->children.rend(); ++iter)
		{
			XPWidgetID	subTarget = XPGetWidgetForLocation(*iter, x, y, inRecursive, inVisibleOnly);
			if (subTarget != NULL)
				return subTarget;
		}
	}
	return inContainer;
}

void			XPGetWidgetExposedGeometry(
						XPWidgetID		inWidgetID,
						int *			outLeft,
						int *			outTop,
						int *			outRight,
						int *			outBottom)
{
	XPWidgetPtr me = XPFindWidgetInfo(inWidgetID, NULL);
	if (me == NULL)
		return;
		
	if (!me->vis_valid)
	{
		XPWidgetID	root = XPFindRootWidget(inWidgetID);
		if (root != NULL)
			XPRebuildExposureCache(root);
		else {
			// Weird...they want the exposed region of a non-rooted widget.  Well,
			// give 'em the whole bounds.
			if (outLeft)
				*outLeft = me->left;
			if (outTop)
				*outTop = me->top;
			if (outRight)
				*outRight = me->right;
			if (outBottom)
				*outBottom = me->bottom;
			return;
		}
	}
	
	// Now we're valid and in a rooted hierarchy, etc.
	if (outLeft)
		*outLeft = me->vis_left;
	if (outTop)
		*outTop = me->vis_top;
	if (outRight)
		*outRight = me->vis_right;
	if (outBottom)
		*outBottom = me->vis_bottom;
}						

#pragma mark -

void			XPSetWidgetDescriptor(
						XPWidgetID			inWidget,
						const char *		inDescriptor)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return;
	me->descriptor = std::string((inDescriptor) ? inDescriptor : "");
	XPSendMessageToWidget(inWidget, xpMsg_DescriptorChanged, xpMode_Direct, 0, 0);
}

long			XPGetWidgetDescriptor(
						XPWidgetID			inWidget,
						char *				outDescriptor,
						long				inMaxDescLength)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return 0;
	if (outDescriptor != NULL)
		strncpy(outDescriptor, me->descriptor.c_str(), inMaxDescLength);
	
	return me->descriptor.size();
}						

void			XPSetWidgetProperty(
						XPWidgetID			inWidget,
						XPWidgetPropertyID	inProperty,
						long				inValue)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return;
	
	me->properties[inProperty] = inValue;	
	XPSendMessageToWidget(inWidget, xpMsg_PropertyChanged, xpMode_Direct, inProperty, inValue);
}						
						
long			XPGetWidgetProperty(
						XPWidgetID			inWidget,
						XPWidgetPropertyID	inProperty,
						int *				inExists)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
	{
		if (inExists)
			*inExists = 0;
		return 0;
	}
	
	XPPropertyTable::iterator iter = me->properties.find(inProperty);
	if (inExists)
		*inExists = (iter != me->properties.end());
	
	return (iter == me->properties.end()) ? 0 : iter->second;
}						

#pragma mark -

XPWidgetID		XPSetKeyboardFocus(
						XPWidgetID		inWidget)
{
	// Short circuit
	if (inWidget == gFocusWidget)
		return gFocusWidget;
		
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return NULL;
		
	XPWidgetID	root = XPFindRootWidget(inWidget);
	if (root == NULL)
		return NULL;

	XPWidgetInfo *	rootInfo = XPFindWidgetInfo(root, NULL);
	if (rootInfo == NULL)
		return NULL;

			
	// First we have to preflight whether the widget wants focus.
	long	result = XPSendMessageToWidget(inWidget, xpMsg_KeyTakeFocus, xpMode_Direct,
					0/*explicit keybd focus*/, 0);

	// If the widget don't want it, the widget don't get it
	if (result == 0)
		return NULL;
		
	if (gFocusWidget != NULL)
	{
		// Someone else is losing focus for us to get it.
		XPSendMessageToWidget(gFocusWidget, xpMsg_KeyLoseFocus, xpMode_Direct, 1/*stolen*/, 0);
	}

	// Flag ourselves that we're messing with focus so that if the XPLM calls us back
	// we don't care (since all will be well when done.)
	gInternalFocusChange = 1;
	XPLMTakeKeyboardFocus(rootInfo->root);
	gInternalFocusChange = 0;
	gFocusWidget = inWidget;
	
	return inWidget;
}						


void			XPLoseKeyboardFocus(
						XPWidgetID		inWidget)
{
	if (inWidget != gFocusWidget)
		return;

	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return;
		
	// Ok, this widget wants to lose focus...first tell it what happened.
	XPSendMessageToWidget(inWidget, xpMsg_KeyLoseFocus, xpMode_Direct, 0/*commanded*/, 0);

	// Now figure out if anyone wants it.
	me = XPFindWidgetInfo(me->parent, NULL);
	while (me != NULL)
	{
		if(XPSendMessageToWidget((XPWidgetID) me, xpMsg_KeyTakeFocus, xpMode_Direct,
					1/*child gave up keybd focus*/, 0))
		{
			// Ok, we found someone who wants it.
			gFocusWidget = (XPWidgetID) me;
			return;
		}
		
		// This guy doesn't want it.  If he's the root we're done.
		if (me->root != NULL)
		{
			// Flag ourselves as messing with focus so we don't get called back.
			gInternalFocusChange = 1;
			XPLMTakeKeyboardFocus(0);	// Back to x-plane
			gInternalFocusChange = 0;
			gFocusWidget = NULL;
			return;
		}
		
		// Try the next guy
		me = XPFindWidgetInfo(me->parent, NULL);	
	}
	
	// If no one is going to have focus, and we had focus, some root
	/// must have had the XPLMDisplay focus.  Best return it to x-plane!
	gFocusWidget = NULL;	
	gInternalFocusChange = 1;
	XPLMTakeKeyboardFocus(0);	// Back to x-plane
	gInternalFocusChange = 0;
	gFocusWidget = NULL;	
}
						
XPWidgetID		XPGetWidgetWithFocus(void)
{
	return	gFocusWidget;
}

#pragma mark -

void			XPAddWidgetCallback(
						XPWidgetID			inWidget,
						XPWidgetFunc_t	 	inNewCallback)
{
	XPWidgetInfo *	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return;
	me->callbacks.insert(me->callbacks.begin(), inNewCallback);
	
	XPSendMessageToWidget(inWidget, xpMsg_Create, xpMode_Once, 1/*subclassing*/, 0);
}						
						
XPWidgetFunc_t			XPGetWidgetClassFunc(
						XPWidgetClass		inWidgetClass)
{
	return gStandardWidgets[inWidgetClass];
}						

#pragma mark -

// This routine returns a widget casted to its info if it exists or NULL if not.
// It also optionally returns an iterator in the master widget set.
// This is used to sanity check all widgets.

XPWidgetPtr		XPFindWidgetInfo(XPWidgetID inID, XPWidgetSet::iterator * outIter)
{
	XPWidgetSet::iterator iter = gWidgets.find((XPWidgetPtr) inID);
	if (outIter)
		*outIter = iter;
	if (iter == gWidgets.end())
		return NULL;
	else
		return *iter;
}

// This is the draw handler for XPLMDisplay; it revalidates the caches
// and goes down the line drawing the widgets.

void			XPWidgetDraw(XPLMWindowID inWindowID, void * inRefcon)
{
	XPWidgetID	me = (XPWidgetID) inRefcon;
	XPRebuildExposureCache(me);
	
	XPPaintOneWidgetRecursive(me);
}

// This is the keyboard handler.  It dispatches keys and also defocuses
// widgets when keyboard focus is stolen.

void			XPWidgetKey(XPLMWindowID inWindowID, 
							char inKey, 
							XPLMKeyFlags inFlags, 
							char inVirtualKey,
							void * inRefcon, 
							int losingFocus)
{
	if (losingFocus)
	{
		// If we're in focus code, the XPLM will call us back.  Ignore us
		if (!gInternalFocusChange)
		{
			// Otherwise focus is being taken by something external.
			if (gFocusWidget != NULL)
				XPSendMessageToWidget(gFocusWidget, xpMsg_KeyLoseFocus, xpMode_Direct,
					1/*stolen*/, 0);
			gFocusWidget = NULL;
		}
	} else {
		// A valid key.
		if (gFocusWidget != NULL)
		{
			XPKeyState_t	st;
			st.key = inKey;
			st.flags = inFlags;
			st.vkey = inVirtualKey;
			XPSendMessageToWidget(gFocusWidget, xpMsg_KeyPress, xpMode_UpChain,
				(long) &st, 0);
		}
	}
}

// The XPLMDisplay mouse handler.  It finds the widget responsible and always 
// dispatches to that one.

int			XPWidgetMouse(XPLMWindowID inWindowID, int x, int y, XPLMMouseStatus inMouse, int button, void * inRefcon)
{

	XPWidgetID	top = (XPWidgetID) inRefcon;
	XPMouseState_t	st;
	st.x = x;
	st.y = y;
	st.button = button;
	switch(inMouse) {
	case xplm_MouseWheel:
		{
			XPWidgetID	target = XPGetWidgetForLocation(top, x, y, 1/*recursive*/, 1/*vis only*/);
			while (target != NULL)
			{
				if (XPSendMessageToWidget(target, xpMsg_MouseWheel, xpMode_Direct, (long) &st, 0))
				{
					break;
				} else {
					target = XPGetParentWidget(target);
				}
			}
		}
		break;
	case xplm_MouseDown:
		{
			XPWidgetID	target = XPGetWidgetForLocation(top, x, y, 1/*recursive*/, 1/*vis only*/);
			gMouseWidget = NULL;
			while (target != NULL)
			{
				if (XPSendMessageToWidget(target, xpMsg_MouseDown, xpMode_Direct, (long) &st, 0))
				{
					gMouseWidget = target;
					break;
				} else {
					target = XPGetParentWidget(target);
				}
			}
		}
		break;
	case xplm_MouseDrag:	
		if (gMouseWidget != NULL)
			XPSendMessageToWidget(gMouseWidget, xpMsg_MouseDrag, xpMode_Direct, (long) &st, 0);
		break;
	case xplm_MouseUp:
		if (gMouseWidget != NULL)
			XPSendMessageToWidget(gMouseWidget, xpMsg_MouseUp, xpMode_Direct, (long) &st, 0);
		gMouseWidget = NULL;
		break;
	}
	// BAS - note: if we supported funny-shaped widgets, we'd check the property
	// on mouse down for non-square widgets and only return 1 if we used the mouse click.
	return 1;	// Always solid for now.
}

// This routine rebuilds the exposure catche from this widget on down.

void			XPRebuildExposureCache(XPWidgetID inWidget)
{
	XPWidgetPtr	me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return;

	if (!me->vis_valid)
	{
		XPWidgetPtr	par = XPFindWidgetInfo(me->parent, NULL);
		
		// BAS - this is a hack to use ourselves as a parent...
		// this effectively leaves the exposure cache of parent widgets wide open.
		int	l, r, t, b;

		if (par == NULL)
		{
			l = me->left;
			r = me->right;
			b = me->bottom;
			t = me->top;
		} else {

			l = WIDGET_TMAX(me->left, par->vis_left);
			r = WIDGET_TMIN(me->right, par->vis_right);
			b = WIDGET_TMAX(me->bottom, par->vis_bottom);
			t = WIDGET_TMIN(me->top, par->vis_top);
		}
		
		if ((l != me->vis_left) ||
			(r != me->vis_right) ||
			(t != me->vis_top) ||
			(b != me->vis_bottom))
		{
			XPSendMessageToWidget(inWidget, xpMsg_ExposedChanged, xpMode_Direct, 0, 0);
			me->vis_left = l;
			me->vis_top = t;
			me->vis_right = r;
			me->vis_bottom = b;
		}
		me->vis_valid = 1;
	}
	
	for (XPWidgetList::iterator iter = me->children.begin(); 
		iter != me->children.end(); ++iter)
	{
		XPRebuildExposureCache(*iter);
	}
}

// This routine invalidates the exposure catche from this widget on down.

void			XPInvalidateExposureCache(XPWidgetID inWidget)
{
	XPWidgetPtr me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return;
	
	me->vis_valid = 0;
	for (XPWidgetList::iterator iter = me->children.begin();
		iter != me->children.end(); ++iter)
	{
		XPInvalidateExposureCache(*iter);
	}
}

// This routine draws a widget and all of its visible children.

void 			XPPaintOneWidgetRecursive(XPWidgetID	inWidget)
{
	XPWidgetPtr me = XPFindWidgetInfo(inWidget, NULL);
	if (me == NULL)
		return;
		
	if ((!me->visible)
#if !NO_ROUGH_WIDGET_CULL	
		|| (me->vis_right <= me->vis_left)
		|| (me->vis_top <= me->vis_bottom)
#endif		
		)
	{
		return;
	}		
		
	if (!XPSendMessageToWidget(inWidget, xpMsg_Paint, xpMode_Direct, 0, 0))
	{
		if (XPGetWidgetProperty(inWidget, xpProperty_Clip, NULL))
		{
			glEnable(GL_SCISSOR_TEST);
			glScissor(me->vis_left, me->vis_bottom,
					me->vis_right - me->vis_left,
					me->vis_top - me->vis_bottom);
		}
		XPSendMessageToWidget(inWidget, xpMsg_Draw, xpMode_Direct, 0, 0);
		if (XPGetWidgetProperty(inWidget, xpProperty_Clip, NULL))
			glDisable(GL_SCISSOR_TEST);
		for (XPWidgetList::iterator iter = me->children.begin();
			iter != me->children.end(); ++iter)
		{
			XPPaintOneWidgetRecursive(*iter);
		}
	}
}
