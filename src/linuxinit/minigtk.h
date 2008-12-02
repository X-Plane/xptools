#if !LIN
	#error Use the MiniGtk class under Linux only.
#endif

#ifndef MINIGTK_H
#define MINIGTK_H

#include <X11/Xlib.h>

#define GTK_LIBRARY "libgtk-x11-2.0.so"
#define GDK_LIBRARY "libgdk-x11-2.0.so"
#define GTHREAD_LIBRARY "libgthread-2.0.so"

#define GTK_STOCK_CANCEL	"gtk-cancel"
#define GTK_STOCK_OPEN		"gtk-open"
#define GTK_STOCK_SAVE		"gtk-save"
#define GTK_STOCK_DISCARD	"gtk-discard"

typedef void (*GTK_INIT_PROC)(void*, void*);
typedef void* (*GTK_MSGNEW_PROC)(void*, int, int, int, const char*, ...);
typedef int (*GTK_RUNDIALOG_PROC)(void*);
typedef void (*GTK_DESTROYWIDGET_PROC)(void*);
typedef void* (*GTK_FILECHS_PROC)(const char*, void*, int, const char*, ...);
typedef char* (*GTK_FILECHS_FILENAME_PROC)(void*);
typedef void (*G_TINIT_PROC)(void*);
typedef void (*GDK_TINIT_PROC)(void);
typedef void (*GDK_FLUSH_PROC)(void);
typedef void (*GTK_ADDBUTTON_PROC)(void*, const char*, ...);
typedef void (*GTK_ABOVE_PROC)(void*, int);
typedef void (*GTK_POSITION_PROC)(void*, int);

typedef enum
{
  GTK_DIALOG_MODAL = 1 << 0,
  GTK_DIALOG_DESTROY_WITH_PARENT = 1 << 1,
  GTK_DIALOG_NO_SEPARATOR = 1 << 2
} GtkDialogFlags;

typedef enum
{
  GTK_MESSAGE_INFO,
  GTK_MESSAGE_WARNING,
  GTK_MESSAGE_QUESTION,
  GTK_MESSAGE_ERROR,
  GTK_MESSAGE_OTHER
} GtkMessageType;

typedef enum
{
  GTK_BUTTONS_NONE,
  GTK_BUTTONS_OK,
  GTK_BUTTONS_CLOSE,
  GTK_BUTTONS_CANCEL,
  GTK_BUTTONS_YES_NO,
  GTK_BUTTONS_OK_CANCEL
} GtkButtonsType;

typedef enum
{
  GTK_FILE_CHOOSER_ACTION_OPEN,
  GTK_FILE_CHOOSER_ACTION_SAVE,
  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
} GtkFileChooserAction;

typedef enum
{
  GTK_RESPONSE_NONE = -1,
  GTK_RESPONSE_REJECT = -2,
  GTK_RESPONSE_ACCEPT = -3,
  GTK_RESPONSE_DELETE_EVENT = -4,
  GTK_RESPONSE_OK = -5,
  GTK_RESPONSE_CANCEL = -6,
  GTK_RESPONSE_CLOSE = -7,
  GTK_RESPONSE_YES = -8,
  GTK_RESPONSE_NO = -9,
  GTK_RESPONSE_APPLY = -10,
  GTK_RESPONSE_HELP = -11
} GtkResponseType;

typedef enum
{
  GTK_WIN_POS_NONE,
  GTK_WIN_POS_CENTER,
  GTK_WIN_POS_MOUSE,
  GTK_WIN_POS_CENTER_ALWAYS,
  GTK_WIN_POS_CENTER_ON_PARENT
} GtkWindowPosition;


class MiniGtk
{
public:
	static void WarningMessagebox(const char* message);
	static int SaveQuestionBox(const char* message);
	static bool ChooseFolder(const char* title, char* out, size_t outsize);
	static bool SaveFile(const char* title, char* out, size_t outsize);
	static bool OpenFile(const char* title, char* out, size_t outsize);
private:
	// do not intantiate this class, it's just a bunch of functions
	MiniGtk();
	~MiniGtk();
	MiniGtk(const MiniGtk&);
	MiniGtk& operator =(const MiniGtk& other);
	friend class Initializer;
	friend class GdkLocker;
	// for Initializer only
	static void _init(int* argc, char** argv[]);
	static void _cleanup();
	// for GdkLocker only
	static void _flush_gdk();

	static bool m_inited;
	static void* hGtkLib;
	static void* hGdkLib;
	static void* hGThreadLib;
	// private function pointers;
	static GTK_INIT_PROC gtk_init;
	static GTK_MSGNEW_PROC gtk_message_dialog_new;
	static GTK_ADDBUTTON_PROC gtk_dialog_add_buttons;
	static GTK_FILECHS_PROC gtk_file_chooser_dialog_new;
	static GTK_FILECHS_FILENAME_PROC gtk_file_chooser_get_filename;
	static GTK_RUNDIALOG_PROC gtk_dialog_run;
	static GTK_DESTROYWIDGET_PROC gtk_widget_destroy;
	static G_TINIT_PROC g_thread_init;
	static GDK_TINIT_PROC gdk_threads_init;
	static GDK_FLUSH_PROC gdk_flush;
	static GTK_ABOVE_PROC gtk_window_set_keep_above;
	static GTK_POSITION_PROC gtk_window_set_position;
	static void* pDisplay;
	static Display* gdk_display;
};

#endif /*  MINIGTK_H */
