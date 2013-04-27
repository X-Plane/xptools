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

#ifndef GUI_DEFS_H
#define GUI_DEFS_H

/***************************************************************************
 * KEY FLAGS
 ***************************************************************************/
enum {
     gui_ShiftFlag                           = 1,
     gui_OptionAltFlag                       = 2,
     gui_ControlFlag                         = 4,
     gui_DownFlag                            = 8,
     gui_UpFlag                              = 16
};
typedef int GUI_KeyFlags;

/***************************************************************************
 * ASCII CONTROL KEY CODES
 ***************************************************************************/
#define GUI_KEY_RETURN      13
#define GUI_KEY_ESCAPE      27
#define GUI_KEY_TAB         9
#define GUI_KEY_BACK        8
#define GUI_KEY_LEFT        28
#define GUI_KEY_RIGHT       29
#define GUI_KEY_UP          30
#define GUI_KEY_DOWN        31
#define GUI_KEY_0           48
#define GUI_KEY_1           49
#define GUI_KEY_2           50
#define GUI_KEY_3           51
#define GUI_KEY_4           52
#define GUI_KEY_5           53
#define GUI_KEY_6           54
#define GUI_KEY_7           55
#define GUI_KEY_8           56
#define GUI_KEY_9           57
#define GUI_KEY_DECIMAL     46
#define GUI_KEY_DELETE      127



/***************************************************************************
 * VIRTUAL KEY CODES
 ***************************************************************************/

#define GUI_VK_BACK         ((const char)0x08)
#define GUI_VK_TAB          ((const char)0x09)
#define GUI_VK_CLEAR        ((const char)0x0C)
#define GUI_VK_RETURN       ((const char)0x0D)
#define GUI_VK_ESCAPE       ((const char)0x1B)
#define GUI_VK_SPACE        ((const char)0x20)
#define GUI_VK_PRIOR        ((const char)0x21)
#define GUI_VK_NEXT         ((const char)0x22)
#define GUI_VK_END          ((const char)0x23)
#define GUI_VK_HOME         ((const char)0x24)
#define GUI_VK_LEFT         ((const char)0x25)
#define GUI_VK_UP           ((const char)0x26)
#define GUI_VK_RIGHT        ((const char)0x27)
#define GUI_VK_DOWN         ((const char)0x28)
#define GUI_VK_SELECT       ((const char)0x29)
#define GUI_VK_PRINT        ((const char)0x2A)
#define GUI_VK_EXECUTE      ((const char)0x2B)
#define GUI_VK_SNAPSHOT     ((const char)0x2C)
#define GUI_VK_INSERT       ((const char)0x2D)
#define GUI_VK_DELETE       ((const char)0x2E)
#define GUI_VK_HELP         ((const char)0x2F)
/* GUI_VK_0 thru GUI_VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39)   */
#define GUI_VK_0            ((const char)0x30)
#define GUI_VK_1            ((const char)0x31)
#define GUI_VK_2            ((const char)0x32)
#define GUI_VK_3            ((const char)0x33)
#define GUI_VK_4            ((const char)0x34)
#define GUI_VK_5            ((const char)0x35)
#define GUI_VK_6            ((const char)0x36)
#define GUI_VK_7            ((const char)0x37)
#define GUI_VK_8            ((const char)0x38)
#define GUI_VK_9            ((const char)0x39)
/* GUI_VK_A thru GUI_VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A)   */
#define GUI_VK_A            ((const char)0x41)
#define GUI_VK_B            ((const char)0x42)
#define GUI_VK_C            ((const char)0x43)
#define GUI_VK_D            ((const char)0x44)
#define GUI_VK_E            ((const char)0x45)
#define GUI_VK_F            ((const char)0x46)
#define GUI_VK_G            ((const char)0x47)
#define GUI_VK_H            ((const char)0x48)
#define GUI_VK_I            ((const char)0x49)
#define GUI_VK_J            ((const char)0x4A)
#define GUI_VK_K            ((const char)0x4B)
#define GUI_VK_L            ((const char)0x4C)
#define GUI_VK_M            ((const char)0x4D)
#define GUI_VK_N            ((const char)0x4E)
#define GUI_VK_O            ((const char)0x4F)
#define GUI_VK_P            ((const char)0x50)
#define GUI_VK_Q            ((const char)0x51)
#define GUI_VK_R            ((const char)0x52)
#define GUI_VK_S            ((const char)0x53)
#define GUI_VK_T            ((const char)0x54)
#define GUI_VK_U            ((const char)0x55)
#define GUI_VK_V            ((const char)0x56)
#define GUI_VK_W            ((const char)0x57)
#define GUI_VK_X            ((const char)0x58)
#define GUI_VK_Y            ((const char)0x59)
#define GUI_VK_Z            ((const char)0x5A)
#define GUI_VK_NUMPAD0      ((const char)0x60)
#define GUI_VK_NUMPAD1      ((const char)0x61)
#define GUI_VK_NUMPAD2      ((const char)0x62)
#define GUI_VK_NUMPAD3      ((const char)0x63)
#define GUI_VK_NUMPAD4      ((const char)0x64)
#define GUI_VK_NUMPAD5      ((const char)0x65)
#define GUI_VK_NUMPAD6      ((const char)0x66)
#define GUI_VK_NUMPAD7      ((const char)0x67)
#define GUI_VK_NUMPAD8      ((const char)0x68)
#define GUI_VK_NUMPAD9      ((const char)0x69)
#define GUI_VK_MULTIPLY     ((const char)0x6A)
#define GUI_VK_ADD          ((const char)0x6B)
#define GUI_VK_SEPARATOR    ((const char)0x6C)
#define GUI_VK_SUBTRACT     ((const char)0x6D)
#define GUI_VK_DECIMAL      ((const char)0x6E)
#define GUI_VK_DIVIDE       ((const char)0x6F)
#define GUI_VK_F1           ((const char)0x70)
#define GUI_VK_F2           ((const char)0x71)
#define GUI_VK_F3           ((const char)0x72)
#define GUI_VK_F4           ((const char)0x73)
#define GUI_VK_F5           ((const char)0x74)
#define GUI_VK_F6           ((const char)0x75)
#define GUI_VK_F7           ((const char)0x76)
#define GUI_VK_F8           ((const char)0x77)
#define GUI_VK_F9           ((const char)0x78)
#define GUI_VK_F10          ((const char)0x79)
#define GUI_VK_F11          ((const char)0x7A)
#define GUI_VK_F12          ((const char)0x7B)
#define GUI_VK_F13          ((const char)0x7C)
#define GUI_VK_F14          ((const char)0x7D)
#define GUI_VK_F15          ((const char)0x7E)
#define GUI_VK_F16          ((const char)0x7F)
#define GUI_VK_F17          ((const char)0x80)
#define GUI_VK_F18          ((const char)0x81)
#define GUI_VK_F19          ((const char)0x82)
#define GUI_VK_F20          ((const char)0x83)
#define GUI_VK_F21          ((const char)0x84)
#define GUI_VK_F22          ((const char)0x85)
#define GUI_VK_F23          ((const char)0x86)
#define GUI_VK_F24          ((const char)0x87)
/* The following definitions are extended and are not based on the Microsoft   *
 * key set.                                                                    */
#define GUI_VK_EQUAL        ((const char)0xB0)
#define GUI_VK_MINUS        ((const char)0xB1)
#define GUI_VK_RBRACE       ((const char)0xB2)
#define GUI_VK_LBRACE       ((const char)0xB3)
#define GUI_VK_QUOTE        ((const char)0xB4)
#define GUI_VK_SEMICOLON    ((const char)0xB5)
#define GUI_VK_BACKSLASH    ((const char)0xB6)
#define GUI_VK_COMMA        ((const char)0xB7)
#define GUI_VK_SLASH        ((const char)0xB8)
#define GUI_VK_PERIOD       ((const char)0xB9)
#define GUI_VK_BACKQUOTE    ((const char)0xBA)
#define GUI_VK_ENTER        ((const char)0xBB)
#define GUI_VK_NUMPAD_ENT   ((const char)0xBC)
#define GUI_VK_NUMPAD_EQ    ((const char)0xBD)

/***************************************************************************
 * CURSORS
 ***************************************************************************/
enum {
	gui_Cursor_None = 0,
	gui_Cursor_Arrow,
	gui_Cursor_Resize_H,
	gui_Cursor_Resize_V
};

/***************************************************************************
 * MENU DEFINES
 ***************************************************************************/
typedef void *	GUI_Menu;

struct	GUI_MenuItem_t {
	const char *	name;			// Item Name
	char			key;			// Menu Key - note that this is ASCII, not a vkey code
	GUI_KeyFlags	flags;			// Modifier Flags
	int				checked;		// Checked
	int				cmd;			// Command enum for this menu item
};

/***************************************************************************
 * KEY CLIPBOARD
 ***************************************************************************/

// GUI clipboard type.  Declared here to avoid having to grab GUI_Clipboard.h all
// over the place.
typedef	int	GUI_ClipType;

// Free function - used to deallocate memory.
typedef	void			(* GUI_FreeFunc_f)(const void * mem, void * ref);

// GUI data fetcher function.  This routine is passed around as a way of getting data
// for clipboard/drag and drop later.
// Passed in: clip_type.
// Returned: set out_start and out_end to the range of memory for the data, or set
// out_start to a NULL ptr on failure.
// Return: if the memory needs to be deallocated, return a free func that can be used
// on the start ptr.
typedef GUI_FreeFunc_f	(* GUI_GetData_f)(GUI_ClipType clip_type, const void ** out_start, const void ** out_end, void * ref);

// Drag & Drop Operation Enums.
// These defines the kind of drag & drop gestures we support - copy, or move.  See GUI_Pane for how they are used.
enum {
	gui_Drag_None	=	0,
	gui_Drag_Move	=	1,
	gui_Drag_Copy	=	2,
};
typedef int GUI_DragOperation;

#endif

