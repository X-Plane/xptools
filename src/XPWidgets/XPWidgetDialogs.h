/* 
 * Copyright (c) 2007, Laminar Research.
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

#ifndef XPWIDGETDIALOGS_H
#define XPWIDGETDIALOGS_H

/*

	XPWidgetDialogs - THEORY OF OPERATION
	
	XPWidgetDialogs is a set of 4 APIs that makes it easy to build complex dialog box using
	plugin widgets:
	
	1. Layout management APIs.  These provide a central facility for organizing widgets,
	   something that was not provided in the original widgets kit.
	2. Wrapper widgets.  These are specializations of a few common widgets for the use in
	   dialog boxes.  Wrapper widgets understand dialog box data transfer, callbacks, and
	   layout management using the above APIs.
	3. Layout widgets.  These widgets serve as intermediates for organizing a dialog box.
	4. Dialog box constructor and utilities.  These APIs let you easily create and access
	   a dialog box.
	   
	LAYOUT CALCULATION
	
	Layout management is done in two passes.  First in the measurement phase, messeages
	flow from the top down in a depth-first search to find the minimum dimensions of 
	every widget, first horizontally then vertically.  Basically the leaf widgets drive
	the process by defining their minimum sizes.  Then in the second phase, widgets are
	layed out from the top down, conforming to the layout dictated by the size calculations.
	
	Three new properties and three new messages are defined for layout management:
	
	xpProperty_WidgetClass - this property contains the enum value of a widget's class.
		This is done to provide a numeric idea of what kind of widget we have for margin
		calculation (see below).
	xpProperty_MinWidth/Height - these are the calculated minimum dimensions of a widget.
		These fields are used as I/O fields by the messages below.
		
	xpMsg_RecalcMinSizeH/V - this message is sent to a widget to calculate its minimum
		size.  A widget that responds to this message should store its minimum size in
		the xpProperty_MinWidth/Height fields.
	
	xpMsg_DoReshape - this message is sent to tell a widget to resize for layout
		management.  The widget should change its size but not position based on the 
		values of xpProperty_MinWidth and xpProperty_MinHeight, which may have changed
		since last set.  (For example, if a widget is set to full justify in a large
		container, its widget may be expanded.) 
		
	Values of -1 for width and height mean "don't care" - widgets should leave their
	dimensions alone.
	
	The layout manager calls are separate APIs to hide the casing code that handles the
	situation where the layout messages are not handled.
	
	MARGIN HANDLING
	
	The layout manager also provides margin information - spacing between two widgets.
	This is always done based on the class of a widget, e.g. the space between a button
	and text field.  A default is provided, but overrides can be entered by the host
	program, and a few widgets come special cased.  Two kind sof margins are provided:
	
	1. Sibbling margins (H & V) - two widgets next to each other in the same dialog
	   box.  This is the spacing between their bounding rects.
	2. Containment margins - for a widget containing another, how much must the 
	   contained widget be inset.
	
	DIALOG BOX TAGS
	
	The xpProperty_DialogTag property contains a unique 'tag' that lets you identify
	each widget.  Since widgets are created dynamically you don't have their Widget IDs
	to work with your dialog.  You can optionally attach an arbitrary numeric tag
	to widgets that you will need during the dialog box's period.  Tags are optional.
	Typically you'll need tags to:
	
	 - Manually exchange data to and from your dialog box during its operation.
	 - Enable and disable items.
	 - Change the captions on text, update a popup's choices, etc.
	
	DIALOG BOX DDX
	
	Data-dialog-exchange (DDX) is the process of transferring data between your dialog
	box and application.  The widget dialogs code contains some helpers to do this
	for you.  Some general ideas:
	
	1. You must provide fixed-location storage for your dialog's data that lasts at
	   least as long as the dialog box.
	2. You will always access your dialog box via this storage.  You should not need
	   to decode the values of specific text fields, for example.
	   
	DDX has two parts: first leaf widgets (like text fields) contain a data pointer
	property that points to your variable that will store the widget's variable.
	Second, these widgets respond to recursive messages to copy their values from
	or to the variables.  Each widget knows its data type and handles type
	coercion, string processing, etc.  Generally types make sense for the widgets, e.g.
	you can have an int check box but not a string text box.
	
	DDX happens automatically when an OK button is clicked or when the dialog box 
	is shown.  It can also happen manually in your code.  If you want to preserve
	the state of variables for when 'cancel' is clicked and you do your own DDX,
	make sure to preserve your dialog box variables yourself.  Dialog boxes are
	not modal unless you shut off the rest of the world separately, so use caution
	in this case.  Manual dialog DDX is done via tags, so you'll need to tag any
	widget that you want to manually DDX.
	
	DIALOG BOX NOTIFICATIONS
	
	You can also attach a notification function to any of the leaf widgets that make
	up a dialog box.  This function is a callback that is called when the widget
	changes.  The function receives a pointer to that widget.  A typical use of
	notifications is to dynamically update the dialog box.  For example, you can
	set up a notification to automatically disable certain text fields when a radio
	button is picked.  NOTE: notifications are available for action push buttons and
	are called after the action callback is called.
	
	AUTOMATIC BEHAVIOR
	
	The dialog APIs provide the following automatic behavior:

	 - Radio buttons within a single parent widget are mutually exclusive.
	 - Tabs (if compiled in) automatically hide all but one of their child widgets
	   for automatic tabbing.
	 - Edit fields: cut, copy and paste.

*/


#include "XPWidgetDefs.h"


// Define these to stub out the popup and tab code if you do not have the supplimental code
// for these widgets.
#define NO_POPUPS 0
#define NO_TABS 0



/**********************************************************************
 * LAYOUT MANAGEMENT APIS
 **********************************************************************
 *
 * Theory: the layout manager APIs externalize the question of how small things
 * can be squished.  From that point all classes just squish themselves.
 *
 * There are two ways to extend this API:
 * 1. You can provide a callback for a given widget class that will answer
 *    these questions, and
 * 2. You can make the widget respond to the layout APIs directly.
 *
 */

enum {
	xpProperty_WidgetClass					= 28300,	// If the widget class is stored in this property, layout mgr can easily find it
	xpProperty_MinWidth						= 28301,	// where your layout leaves its min dimensions 
	xpProperty_MinHeight					= 28302,	// after it gets the RecalcMinSize msg
	
	xpMsg_RecalcMinSizeH					= 28300,	// tells your widget to recalc its min size.
	xpMsg_RecalcMinSizeV					= 28301,	// tells your widget to recalc its min size.
	xpMsg_DoReshape							= 28302		// tells your widget to reshape - min size props have been set
};

/* Query a given widget for its size.  Pass 0 if you do not know the widget's
 * class. */
int		XPLayout_GetMinimumWidth(XPWidgetClass inClass, XPWidgetID inWidget);
int		XPLayout_GetMinimumHeight(XPWidgetClass inClass, XPWidgetID inWidget);

void	XPLayout_Reshape(XPWidgetClass inClass, XPWidgetID inWidget, int inIdealWidth, int inIdealHeight);

/* Given two widget classes, get minimum margins.  Pass 0 for each for default
 * margins or 0 for one for margins against the container side! */
int		XPLayout_GetMinimumMarginH(XPWidgetClass inLeft, XPWidgetClass inRight);
int		XPLayout_GetMinimumMarginV(XPWidgetClass inBottom, XPWidgetClass inTop);
void	XPLayout_GetContainMargins(XPWidgetClass inParent, XPWidgetClass inChild, int& outLeft, int& outTop, int& outRight, int& outBottom);

/* Register exceptional layout properties. */
void	XPLayout_RegisterSpecialMarginsH(XPWidgetClass inLeft, XPWidgetClass inRight, int sibling_margin);
void	XPLayout_RegisterSpecialMarginsV(XPWidgetClass inBottom, XPWidgetClass inTop, int sibling_margin);
void	XPLayout_RegisterSpecialMarginsContains(XPWidgetClass inParent, XPWidgetClass inChild, int inLeft, int inTop, int inRight, int inBottom);

/**********************************************************************
 * WRAPPER WIDGETS
 **********************************************************************
 *
 * These are wrapper functions around the regular widgets, except that 
 * they can have added behavior: they have a data ptr property which is
 * a ptr to some kind of storage.  When the dialog box exchange messages
 * go down the widget hierarchy, they copy data to and from this pointer.
 * The comments to the right indicate the pointer type.
 *
 * (There are a few ways to extend the behavior of a widget.  You can
 * do it at run time by adding a second widget handler function using
 * XPAddWidgetCallback, or you can simply write a widget function that
 * calls the old widget function.  These classes do the later - they
 * use XPGetWidgetClassFunc to find the 'official' implementations for
 * various dialog box widgets, and call these functions if they don't
 * handle the message themselves.  So you can use these widget functions
 * instead of the default widget functions.)
 *
 */
enum {
	xpProperty_DataPtr					= 28400,		// Ptr to location nof data
	xpProperty_FieldVisChars			= 28401,		// How many chars visible for layout management
	xpProperty_FieldPrecision			= 28402,		// How many chars precision for floating point
	xpProperty_RadioButtonEnum			= 28403,		// Radio button sets int to this value.
	xpProperty_NotifyPtr				= 28404			// void(*)(XPWidgetID) - called when this thing changes.
};

int XPWF_Caption			(XPWidgetMessage,XPWidgetID,long,long);	// not used
int	XPWF_EditText_String	(XPWidgetMessage,XPWidgetID,long,long); // char *
int	XPWF_EditText_Int		(XPWidgetMessage,XPWidgetID,long,long);	// int *
int	XPWF_EditText_Float		(XPWidgetMessage,XPWidgetID,long,long);	// float *
int	XPWF_PushButton_Action	(XPWidgetMessage,XPWidgetID,long,long);	// void(*)(XPWidgetID)
int	XPWF_CheckBox_Int		(XPWidgetMessage,XPWidgetID,long,long);	// int *
int	XPWF_RadioButton_Int	(XPWidgetMessage,XPWidgetID,long,long);	// int *
#if !NO_POPUPS
int	XPWF_Popup_Int			(XPWidgetMessage,XPWidgetID,long,long);	// int *
#endif
#if !NO_TABS
int	XPWF_Tabs_IntShowHide	(XPWidgetMessage,XPWidgetID,long,long);	// int *
#endif

/**********************************************************************
 * LAYOUT WIDGETS
 **********************************************************************
 *
 * These widgets manage their children's dimensions.  NOTE: justification
 * is not yet implemented!
 *
 */

enum {
	xpWidgetClass_RowColumn = 11,

	// Justifications for a row column in their non-stacked direction, e.g. horizontal just for vertical column.
	xpRowColumn_JustifyMin    = 0,
	xpRowColumn_JustifyCenter = 1,
	xpRowColumn_JustifyMax 	  = 2,
	xpRowColumn_JustifySpread = 3,
	xpRowColumn_JustifyStretch = 4,

	xpProperty_RowColumnIsVertical			= 28500,	// 1 = vertical 0 = horizontal
	xpProperty_JustifyH						= 28501,		// justify, default = 0 = min (left/bot)
	xpProperty_JustifyV						= 28502		// justify, default = 0 = min (left/bot)
};
int	XPWF_RowColumn	(XPWidgetMessage,XPWidgetID,long,long); 

/**********************************************************************
 * DIALOG BOX STUFF
 **********************************************************************
 *
 * These widgets manage their children's dimensions
 *
 */

enum {

	xpWidgetClass_Dialog = 12,

	xpMsg_DataToDialog					= 28600,	// Sent by dialog to all children to copy data to widgets
	xpMsg_DataFromDialog				= 28601,	// Sent by dialog to all children to copy data from widgets
	xpMsg_DialogDone					= 28602,	// Sent to dialog to close, - param 1 = result	
	
	xpDialog_ResultOK 		= 1,						// OK button pressed
	xpDialog_ResultCancel 	= 0,						// Dialog canceled
	
	xpProperty_DialogSelfDestroy			= 28600,	// if 1, dialog deallocates when dismissed.
	xpProperty_DialogSelfHide				= 28601,	// if 1, dialog deallocates when dismissed.
	xpProperty_DialogDismissCB				= 28602,	// Ptr to a void (*)(XPWidgetID, int), called when dismissed.
	xpProperty_DialogTag					= 280603
};

// These are action procs your push button can use to
// nuke a dialog box.
void	PBAction_DoneDialogOK(XPWidgetID);	
void	PBAction_DoneDialogCancel(XPWidgetID);

// This is the dialog box top widget - manages layout with children and dialog box data exchange.
int	XPWF_DialogBox	(XPWidgetMessage,XPWidgetID,long,long); 

/**********************************************************************
 * CONSTRUCTION HELPER
 **********************************************************************
 *
 *
 */

// Basic construction tokens - these are used to create a dialog box.
const	int		XP_DIALOG_BOX	=	1;	//	XP_DIALOG_BOX, <title>, [XP_DIALOG_CLOSEBOX,] <hide> <destroy> <callback>
const	int		XP_ROW			=	2;	//	XP_ROW
const	int		XP_COLUMN		=	3;	//	XP_COLUMN
const	int		XP_END			=	4;	//	XP_END
const	int		XP_BUTTON_ACTION=	5;	//	XP_BUTTON_ACTION, "Help", MyHelpFunc
const	int		XP_BUTTON_OK	=	6;	//	XP_BUTTON_OK, "OK"
const	int		XP_BUTTON_CANCEL=	7;	//	XP_BUTTON_CANCEL, "CANCEL"
const	int		XP_CAPTION		=	8;	//	XP_CAPTION, "This is a caption"
const	int		XP_EDIT_STRING	=	9;	//	XP_EDIT_STRING, [XP_EDIT_PASSWORD], <max len>, <vis len>, <string ptr> 
const	int		XP_EDIT_INT		=	10;	//	XP_EDIT_INT, [XP_EDIT_PASSWORD], <max char len>, <vis len>, <int ptr> 
const	int		XP_EDIT_FLOAT	=	11;	//	XP_EDIT_FLOAT_,  [XP_EDIT_PASSWORD], <max char len>, <vis len>, <precision len>, <float ptr>
const	int		XP_POPUP_MENU	=	12;	//	XP_POPUP_MENU, "Title", <int ptr>
const	int		XP_TABS			=	13;	//	XP_TABS, "Title", <int ptr>
const	int		XP_CHECKBOX		=	14;	// 	XP_CHECKBOX, "Title", <int ptr>
const	int		XP_RADIOBUTTON	=	15;	// 	XP_CHECKBOX, "Title", <int ptr>, <enum_value>

const	int		XP_TAG			=	16;	//	XP_TAG, <int>,	
const	int		XP_NOTIFY		=	17;	//	XP_NOTIFY, <func>

// Special flags - these can be inserted optionally in certain locations in the string, as shown above.
// The use of negative numbers allows us to get them via context.
const	int		XP_EDIT_PASSWORD=	-100;
const	int		XP_DIALOG_CLOSEBOX=	-101;
 
XPWidgetID		XPCreateWidgetLayout(int dummy, ...);
XPWidgetID		XPFindWidgetByTag(XPWidgetID dialog, int tag);
void			XPDataFromItem(XPWidgetID dialog, int tag);
void			XPDataToItem(XPWidgetID dialog, int tag);
void			XPEnableByTag(XPWidgetID dialog, int tag, int enable);

void			XPRegisterWidgetCreateHandler(int inToken, XPWidgetID (* inHandler)(XPWidgetID inParent, va_list& ioList));

void			XPInitDefaultMargins(void);

#endif /* XPWIDGETDIALOGS_H */
