/*
 * Copyright (c) 2004, Laminar Research.
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
#include "PlatformUtils.h"
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

/**
** janos says:
** if someone complains 'bout this code slap him. i know that it's ugly
** (specifically it is _not_ threadsafe), but this way
** we have an intermediate solution until i have something else. we don't
** have gtk dependencies as well (during compile and runtime) and can gracefully
** shut the program down if the library isn't available.
**/

Display* gdk_display = 0;

/* GTK for minimalists */

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

static GTK_INIT_PROC gtk_init = 0;
static GTK_MSGNEW_PROC gtk_message_dialog_new = 0;
static GTK_ADDBUTTON_PROC gtk_dialog_add_buttons = 0;
static GTK_FILECHS_PROC gtk_file_chooser_dialog_new = 0;
static GTK_FILECHS_FILENAME_PROC gtk_file_chooser_get_filename = 0;
static GTK_RUNDIALOG_PROC gtk_dialog_run = 0;
static GTK_DESTROYWIDGET_PROC gtk_widget_destroy = 0;
static G_TINIT_PROC g_thread_init = 0;
static GDK_TINIT_PROC gdk_threads_init = 0;
static GDK_FLUSH_PROC gdk_flush = 0;
static GTK_ABOVE_PROC gtk_window_set_keep_above = 0;
static GTK_POSITION_PROC gtk_window_set_position = 0;

typedef enum
{
  GTK_DIALOG_MODAL               = 1 << 0,
  GTK_DIALOG_DESTROY_WITH_PARENT = 1 << 1,
  GTK_DIALOG_NO_SEPARATOR        = 1 << 2
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
  GTK_RESPONSE_OK     = -5,
  GTK_RESPONSE_CANCEL = -6,
  GTK_RESPONSE_CLOSE  = -7,
  GTK_RESPONSE_YES    = -8,
  GTK_RESPONSE_NO     = -9,
  GTK_RESPONSE_APPLY  = -10,
  GTK_RESPONSE_HELP   = -11
} GtkResponseType;

typedef enum
{
  GTK_WIN_POS_NONE,
  GTK_WIN_POS_CENTER,
  GTK_WIN_POS_MOUSE,
  GTK_WIN_POS_CENTER_ALWAYS,
  GTK_WIN_POS_CENTER_ON_PARENT
} GtkWindowPosition;


#define GTK_STOCK_CANCEL           "gtk-cancel"
#define GTK_STOCK_OPEN             "gtk-open"
#define GTK_STOCK_SAVE             "gtk-save"
#define GTK_STOCK_DISCARD          "gtk-discard"

static int found_gtk = 0;

int __init_minimalist_gtk(int* argc, char*** argv)
{
	void* moduleHandle = dlopen("libgtk-x11-2.0.so", RTLD_NOW | RTLD_GLOBAL);
	void* gdkModuleHandle = dlopen("libgdk-x11-2.0.so", RTLD_NOW | RTLD_GLOBAL);
	void* gModuleHandle = dlopen("libgthread-2.0.so", RTLD_NOW | RTLD_GLOBAL);

	if (!moduleHandle)
	{
		printf("libgtk-x11-2.0.so not found. exiting\n");
		return 0;
	}
	if (!gdkModuleHandle)
	{
		printf("libgdk-x11-2.0.so not found. exiting\n");
		return 0;
	}
	if (!gModuleHandle)
	{
		printf("libgthread-2.0.so not found. exiting\n");
		return 0;
	}

	g_thread_init = (G_TINIT_PROC)dlsym(gModuleHandle, "g_thread_init");
	gdk_threads_init = (GDK_TINIT_PROC)dlsym(gdkModuleHandle, "gdk_threads_init");
	gdk_flush = (GDK_FLUSH_PROC)dlsym(gdkModuleHandle, "gdk_flush");
	void* d = dlsym(gdkModuleHandle, "gdk_display");
	gtk_init = (GTK_INIT_PROC)dlsym(moduleHandle, "gtk_init");
	gtk_message_dialog_new = (GTK_MSGNEW_PROC)dlsym(moduleHandle, "gtk_message_dialog_new");
	gtk_dialog_run = (GTK_RUNDIALOG_PROC)dlsym(moduleHandle, "gtk_dialog_run");
	gtk_widget_destroy = (GTK_DESTROYWIDGET_PROC)dlsym(moduleHandle, "gtk_widget_destroy");
	gtk_file_chooser_dialog_new = (GTK_FILECHS_PROC)dlsym(moduleHandle, "gtk_file_chooser_dialog_new");
	gtk_file_chooser_get_filename = (GTK_FILECHS_FILENAME_PROC)dlsym(moduleHandle, "gtk_file_chooser_get_filename");
	gtk_dialog_add_buttons = (GTK_ADDBUTTON_PROC)dlsym(moduleHandle, "gtk_dialog_add_buttons");
	gtk_window_set_keep_above = (GTK_ABOVE_PROC)dlsym(moduleHandle, "gtk_window_set_keep_above");
	gtk_window_set_position = (GTK_POSITION_PROC)dlsym(moduleHandle, "gtk_window_set_position");
	if (g_thread_init &&
		gdk_threads_init &&
		gtk_init &&
		gtk_message_dialog_new &&
		gtk_dialog_run &&
		gtk_widget_destroy &&
		gtk_file_chooser_dialog_new &&
		gtk_file_chooser_get_filename &&
		gdk_flush && d && gtk_dialog_add_buttons &&
		gtk_window_set_keep_above &&
		gtk_window_set_position)
		found_gtk = 1;

	if (found_gtk)
	{
		gdk_display = *(Display**)d;
		g_thread_init(0);
		gdk_threads_init();
		gtk_init(argc, argv);
	}
	return found_gtk;
}

const char * GetApplicationPath(char * pathBuf, int sz)
{
	return ".";
}

int		GetFilePathFromUser(
					int					inType,
					const char * 		inPrompt,
					const char *		inAction,
					int					inID,
					char * 				outFileName,
					int					inBufSize)
{
	if (!found_gtk) return 0;
	switch(inType) {
		case getFile_Open:
		{
			XLockDisplay(gdk_display);
			void* dialog = gtk_file_chooser_dialog_new(inPrompt, 0, GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
			gtk_window_set_keep_above(dialog, 1);
			gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
			if (gtk_dialog_run(dialog) == GTK_RESPONSE_ACCEPT)
			{
				char *filename;
				filename = gtk_file_chooser_get_filename(dialog);
				strncpy(outFileName, filename, inBufSize);
				free(filename);
				gtk_widget_destroy(dialog);
				// we need to sync manually as we have two event loops at this point
				// and we need to avoid that our main loop steals events intented for
				// the one of gtk
				gdk_flush();
				XSync(gdk_display, True);
				XUnlockDisplay(gdk_display);
				return 1;
			}
			else
			{
				gtk_widget_destroy(dialog);
				gdk_flush();
				XSync(gdk_display, True);
				XUnlockDisplay(gdk_display);
				return 0;
			}
		}
		case getFile_Save:
		{
			XLockDisplay(gdk_display);
			void* dialog = gtk_file_chooser_dialog_new(inPrompt, 0, GTK_FILE_CHOOSER_ACTION_SAVE,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
			gtk_window_set_keep_above(dialog, 1);
			gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
			if (gtk_dialog_run(dialog) == GTK_RESPONSE_ACCEPT)
			{
				char *filename;
				filename = gtk_file_chooser_get_filename(dialog);
				strncpy(outFileName, filename, inBufSize);
				free(filename);
				gtk_widget_destroy(dialog);
				gdk_flush();
				XSync(gdk_display, True);
				XUnlockDisplay(gdk_display);
				return 1;
			}
			else
			{
				gdk_flush();
				XSync(gdk_display, True);
				gtk_widget_destroy(dialog);
				XUnlockDisplay(gdk_display);
				return 0;
			}
		}
		case getFile_PickFolder:
		{
			XLockDisplay(gdk_display);
			void* dialog = gtk_file_chooser_dialog_new(inPrompt, 0, GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
			gtk_window_set_keep_above(dialog, 1);
			gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
			if (gtk_dialog_run(dialog) == GTK_RESPONSE_ACCEPT)
			{
				char *filename;
				filename = gtk_file_chooser_get_filename(dialog);
				strncpy(outFileName, filename, inBufSize);
				free(filename);
				gtk_widget_destroy(dialog);
				gdk_flush();
				XSync(gdk_display, True);
				XUnlockDisplay(gdk_display);
				return 1;
			}
			else
			{
				gtk_widget_destroy(dialog);
				gdk_flush();
				XSync(gdk_display, True);
				XUnlockDisplay(gdk_display);
				return 0;
			}
		}
	}
}

void	DoUserAlert(const char * inMsg)
{
	if (!found_gtk) return;
	XLockDisplay(gdk_display);
	void* dialog = gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING,
					GTK_BUTTONS_CLOSE, inMsg);
	gtk_window_set_keep_above(dialog, 1);
	gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
	gtk_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	gdk_flush();
	XSync(gdk_display, True);
	XUnlockDisplay(gdk_display);
}

void	ShowProgressMessage(const char * inMsg, float * inProgress)
{
	if(inProgress)	fprintf(stderr,"%s: %f\n",inMsg,100.0f * *inProgress);
	else			fprintf(stderr,"%s\n",inMsg);
}

int		ConfirmMessage(const char * inMsg, const char * proceedBtn, const char * cancelBtn)
{
	if (!found_gtk) return 0;
	fprintf(stderr,"%s (%s/%s)\n", inMsg, proceedBtn, cancelBtn);
	return 0;
}

int DoSaveDiscardDialog(const char * inMessage1, const char * inMessage2)
{
	if (!found_gtk) return close_Cancel;
	XLockDisplay(gdk_display);
	void* dialog = gtk_message_dialog_new(0, GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_NONE, inMessage2);
	gtk_dialog_add_buttons(dialog, GTK_STOCK_SAVE, GTK_RESPONSE_YES, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_DISCARD, GTK_RESPONSE_NO, NULL);
	gtk_window_set_keep_above(dialog, 1);
	gtk_window_set_position(dialog, GTK_WIN_POS_CENTER_ALWAYS);
	int res = gtk_dialog_run(dialog);
	gtk_widget_destroy(dialog);
	gdk_flush();
	XSync(gdk_display, True);
	XUnlockDisplay(gdk_display);
	switch (res)
	{
		case GTK_RESPONSE_YES:
			return close_Save;
		case GTK_RESPONSE_NO:
			return close_Discard;
		case GTK_RESPONSE_CANCEL:
		default:
			return close_Cancel;
	}
}
