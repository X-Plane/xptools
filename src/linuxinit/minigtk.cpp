#include <dlfcn.h>
#include <cstring>
#include <cstdlib>
#include "minigtk.h"

bool MiniGtk::m_inited = false;
void* MiniGtk::hGtkLib = 0;
void* MiniGtk::hGdkLib = 0;
void* MiniGtk::hGThreadLib = 0;
void* MiniGtk::pDisplay = 0;
Display* MiniGtk::gdk_display = 0;
G_TINIT_PROC MiniGtk::g_thread_init = 0;
GDK_TINIT_PROC MiniGtk::gdk_threads_init = 0;
GDK_FLUSH_PROC MiniGtk::gdk_flush = 0;
GTK_INIT_PROC MiniGtk::gtk_init = 0;
GTK_MSGNEW_PROC MiniGtk::gtk_message_dialog_new = 0;
GTK_ADDBUTTON_PROC MiniGtk::gtk_dialog_add_buttons = 0;
GTK_FILECHS_PROC MiniGtk::gtk_file_chooser_dialog_new = 0;
GTK_FILECHS_FILENAME_PROC MiniGtk::gtk_file_chooser_get_filename = 0;
GTK_RUNDIALOG_PROC MiniGtk::gtk_dialog_run = 0;
GTK_DESTROYWIDGET_PROC MiniGtk::gtk_widget_destroy = 0;
GTK_ABOVE_PROC MiniGtk::gtk_window_set_keep_above = 0;
GTK_POSITION_PROC MiniGtk::gtk_window_set_position = 0;

class GdkLocker
{
private:
	friend class MiniGtk;
	GdkLocker();
	~GdkLocker()
	{
	/**
	** we need to sync manually as we might have two event loops at this point
	** and we need to avoid that our main loop steals events intended for
	** the one of gtk
	**/
		MiniGtk::_flush_gdk();
		XSync(dspl, False);
	//	XUnlockDisplay(dspl);
	}
	GdkLocker(Display* gdk_display)
	{
		dspl = gdk_display;
		if (!dspl)
			throw "invalid GDK display";
	//	XLockDisplay(dspl);
	}
	GdkLocker(const GdkLocker&);
	GdkLocker& operator =(const GdkLocker& other);
	Display* dspl;
};

void MiniGtk::_cleanup()
{
	if (hGtkLib) dlclose(hGtkLib);
	if (hGdkLib) dlclose(hGdkLib);
	if (hGThreadLib) dlclose(hGThreadLib);
}

void MiniGtk::_flush_gdk()
{
	if (!m_inited) return;
	gdk_flush();
}

void MiniGtk::_init(int* argc, char** argv[])
{
	hGtkLib = dlopen(GTK_LIBRARY, RTLD_NOW | RTLD_LOCAL);
	if (!hGtkLib)
		hGtkLib = dlopen(GTK_LIBRARY".0", RTLD_NOW | RTLD_LOCAL);
	if (!hGtkLib)
		throw "'"GTK_LIBRARY "' not found. install GTK+ 2.x";

	hGdkLib = dlopen(GDK_LIBRARY, RTLD_NOW | RTLD_LOCAL);
	if (!hGdkLib)
		hGdkLib = dlopen(GDK_LIBRARY".0", RTLD_NOW | RTLD_LOCAL);
	if (!hGdkLib)
		throw "'"GDK_LIBRARY "' not found. install GTK+ 2.x";

	hGThreadLib = dlopen(GTHREAD_LIBRARY, RTLD_NOW | RTLD_LOCAL);
	if (!hGThreadLib)
		hGThreadLib = dlopen(GTHREAD_LIBRARY".0", RTLD_NOW | RTLD_LOCAL);
	if (!hGThreadLib)
		throw "'"GTHREAD_LIBRARY "' not found. install GTK+ 2.x";

	g_thread_init = (G_TINIT_PROC)dlsym(hGThreadLib, "g_thread_init");
	gdk_threads_init = (GDK_TINIT_PROC)dlsym(hGdkLib, "gdk_threads_init");
	gdk_flush = (GDK_FLUSH_PROC)dlsym(hGdkLib, "gdk_flush");
	gtk_init = (GTK_INIT_PROC)dlsym(hGtkLib, "gtk_init");
	gtk_message_dialog_new = (GTK_MSGNEW_PROC)dlsym(hGtkLib, "gtk_message_dialog_new");
	gtk_dialog_run = (GTK_RUNDIALOG_PROC)dlsym(hGtkLib, "gtk_dialog_run");
	gtk_widget_destroy = (GTK_DESTROYWIDGET_PROC)dlsym(hGtkLib, "gtk_widget_destroy");
	gtk_file_chooser_dialog_new = (GTK_FILECHS_PROC)dlsym(hGtkLib, "gtk_file_chooser_dialog_new");
	gtk_file_chooser_get_filename = (GTK_FILECHS_FILENAME_PROC)dlsym(hGtkLib, "gtk_file_chooser_get_filename");
	gtk_dialog_add_buttons = (GTK_ADDBUTTON_PROC)dlsym(hGtkLib, "gtk_dialog_add_buttons");
	gtk_window_set_keep_above = (GTK_ABOVE_PROC)dlsym(hGtkLib, "gtk_window_set_keep_above");
	gtk_window_set_position = (GTK_POSITION_PROC)dlsym(hGtkLib, "gtk_window_set_position");
	pDisplay = dlsym(hGdkLib, "gdk_display");

	if (g_thread_init &&
		gdk_threads_init &&
		gtk_init &&
		gtk_message_dialog_new &&
		gtk_dialog_run &&
		gtk_widget_destroy &&
		gtk_file_chooser_dialog_new &&
		gtk_file_chooser_get_filename &&
		gdk_flush && pDisplay && gtk_dialog_add_buttons &&
		gtk_window_set_keep_above &&
		gtk_window_set_position)
	{
		//XInitThreads();
		//g_thread_init(0);
		//gdk_threads_init();
		gtk_init(argc, argv);
		gdk_display = *(Display**)pDisplay;
		m_inited = true;
	}
	else
		throw "wasn't able to retrieve all required symbols from GTK+ libraries";
}

void  MiniGtk::WarningMessagebox(const char* message)
{
	if (!m_inited) return;
	GdkLocker gdkl(gdk_display);
	void* dialog = gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE, message);
	gtk_window_set_keep_above(dialog, 1);
	gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
	gtk_dialog_run(dialog);
	gtk_widget_destroy(dialog);
}

int MiniGtk::SaveQuestionBox(const char* message)
{
	if (!m_inited) return GTK_RESPONSE_CANCEL;
	GdkLocker gdkl(gdk_display);
	void* dialog = gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_NONE, message);
	gtk_dialog_add_buttons(dialog, GTK_STOCK_SAVE, GTK_RESPONSE_YES, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_DISCARD, GTK_RESPONSE_NO, NULL);
	gtk_window_set_keep_above(dialog, 1);
	gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
	int res = gtk_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	return res;
}

bool MiniGtk::ChooseFolder(const char* title, char* out, size_t outsize)
{
	if (!m_inited) return false;
	if (!out) return false;
	GdkLocker gdkl(gdk_display);
	void* dialog = gtk_file_chooser_dialog_new(title, 0, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_keep_above(dialog, 1);
	gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(dialog);
		::strncpy(out, filename, outsize);
		::free(filename);
		gtk_widget_destroy(dialog);
		return true;
	}
	gtk_widget_destroy(dialog);
	return false;
}

bool MiniGtk::SaveFile(const char* title, char* out, size_t outsize)
{
	if (!m_inited) return false;
	if (!out) return false;
	GdkLocker gdkl(gdk_display);
	void* dialog = gtk_file_chooser_dialog_new(title, 0, GTK_FILE_CHOOSER_ACTION_SAVE,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_keep_above(dialog, 1);
	gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(dialog);
		::strncpy(out, filename, outsize);
		::free(filename);
		gtk_widget_destroy(dialog);
		return true;
	}
	gtk_widget_destroy(dialog);
	return false;
}

bool MiniGtk::OpenFile(const char* title, char* out, size_t outsize)
{
	if (!m_inited) return false;
	if (!out) return false;
	GdkLocker gdkl(gdk_display);
	void* dialog = gtk_file_chooser_dialog_new(title, 0, GTK_FILE_CHOOSER_ACTION_OPEN,
					GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	gtk_window_set_keep_above(dialog, 1);
	gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
	if (gtk_dialog_run(dialog) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		filename = gtk_file_chooser_get_filename(dialog);
		::strncpy(out, filename, outsize);
		::free(filename);
		gtk_widget_destroy(dialog);
		return true;
	}
	gtk_widget_destroy(dialog);
	return false;
}
