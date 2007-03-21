#ifndef GUI_DEFS_H
#define GUI_DEFS_H

#if !DEV
get this the hell out of here
#endif

#if APL
typedef struct OpaqueMenuRef*           MenuRef;
typedef MenuRef                         MenuHandle;
#endif


#if APL
#define GUI_Menu MenuRef
#elif IBM
#define GUI_Menu HMENU
#endif


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
#define GUI_KEY_DELETE      8
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


/***************************************************************************
 * VIRTUAL KEY CODES
 ***************************************************************************/

#define GUI_VK_BACK         0x08
#define GUI_VK_TAB          0x09
#define GUI_VK_CLEAR        0x0C
#define GUI_VK_RETURN       0x0D
#define GUI_VK_ESCAPE       0x1B
#define GUI_VK_SPACE        0x20
#define GUI_VK_PRIOR        0x21
#define GUI_VK_NEXT         0x22
#define GUI_VK_END          0x23
#define GUI_VK_HOME         0x24
#define GUI_VK_LEFT         0x25
#define GUI_VK_UP           0x26
#define GUI_VK_RIGHT        0x27
#define GUI_VK_DOWN         0x28
#define GUI_VK_SELECT       0x29
#define GUI_VK_PRINT        0x2A
#define GUI_VK_EXECUTE      0x2B
#define GUI_VK_SNAPSHOT     0x2C
#define GUI_VK_INSERT       0x2D
#define GUI_VK_DELETE       0x2E
#define GUI_VK_HELP         0x2F
/* GUI_VK_0 thru GUI_VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39)   */
#define GUI_VK_0            0x30
#define GUI_VK_1            0x31
#define GUI_VK_2            0x32
#define GUI_VK_3            0x33
#define GUI_VK_4            0x34
#define GUI_VK_5            0x35
#define GUI_VK_6            0x36
#define GUI_VK_7            0x37
#define GUI_VK_8            0x38
#define GUI_VK_9            0x39
/* GUI_VK_A thru GUI_VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A)   */
#define GUI_VK_A            0x41
#define GUI_VK_B            0x42
#define GUI_VK_C            0x43
#define GUI_VK_D            0x44
#define GUI_VK_E            0x45
#define GUI_VK_F            0x46
#define GUI_VK_G            0x47
#define GUI_VK_H            0x48
#define GUI_VK_I            0x49
#define GUI_VK_J            0x4A
#define GUI_VK_K            0x4B
#define GUI_VK_L            0x4C
#define GUI_VK_M            0x4D
#define GUI_VK_N            0x4E
#define GUI_VK_O            0x4F
#define GUI_VK_P            0x50
#define GUI_VK_Q            0x51
#define GUI_VK_R            0x52
#define GUI_VK_S            0x53
#define GUI_VK_T            0x54
#define GUI_VK_U            0x55
#define GUI_VK_V            0x56
#define GUI_VK_W            0x57
#define GUI_VK_X            0x58
#define GUI_VK_Y            0x59
#define GUI_VK_Z            0x5A
#define GUI_VK_NUMPAD0      0x60
#define GUI_VK_NUMPAD1      0x61
#define GUI_VK_NUMPAD2      0x62
#define GUI_VK_NUMPAD3      0x63
#define GUI_VK_NUMPAD4      0x64
#define GUI_VK_NUMPAD5      0x65
#define GUI_VK_NUMPAD6      0x66
#define GUI_VK_NUMPAD7      0x67
#define GUI_VK_NUMPAD8      0x68
#define GUI_VK_NUMPAD9      0x69
#define GUI_VK_MULTIPLY     0x6A
#define GUI_VK_ADD          0x6B
#define GUI_VK_SEPARATOR    0x6C
#define GUI_VK_SUBTRACT     0x6D
#define GUI_VK_DECIMAL      0x6E
#define GUI_VK_DIVIDE       0x6F
#define GUI_VK_F1           0x70
#define GUI_VK_F2           0x71
#define GUI_VK_F3           0x72
#define GUI_VK_F4           0x73
#define GUI_VK_F5           0x74
#define GUI_VK_F6           0x75
#define GUI_VK_F7           0x76
#define GUI_VK_F8           0x77
#define GUI_VK_F9           0x78
#define GUI_VK_F10          0x79
#define GUI_VK_F11          0x7A
#define GUI_VK_F12          0x7B
#define GUI_VK_F13          0x7C
#define GUI_VK_F14          0x7D
#define GUI_VK_F15          0x7E
#define GUI_VK_F16          0x7F
#define GUI_VK_F17          0x80
#define GUI_VK_F18          0x81
#define GUI_VK_F19          0x82
#define GUI_VK_F20          0x83
#define GUI_VK_F21          0x84
#define GUI_VK_F22          0x85
#define GUI_VK_F23          0x86
#define GUI_VK_F24          0x87
/* The following definitions are extended and are not based on the Microsoft   *
 * key set.                                                                    */
#define GUI_VK_EQUAL        0xB0
#define GUI_VK_MINUS        0xB1
#define GUI_VK_RBRACE       0xB2
#define GUI_VK_LBRACE       0xB3
#define GUI_VK_QUOTE        0xB4
#define GUI_VK_SEMICOLON    0xB5
#define GUI_VK_BACKSLASH    0xB6
#define GUI_VK_COMMA        0xB7
#define GUI_VK_SLASH        0xB8
#define GUI_VK_PERIOD       0xB9
#define GUI_VK_BACKQUOTE    0xBA
#define GUI_VK_ENTER        0xBB
#define GUI_VK_NUMPAD_ENT   0xBC
#define GUI_VK_NUMPAD_EQ    0xBD

struct	GUI_MenuItem_t {
	const char *	name;
	char			key;
	GUI_KeyFlags	flags;
	int				cmd;
};


#endif