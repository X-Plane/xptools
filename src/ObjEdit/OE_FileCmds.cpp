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
#include "OE_FileCmds.h"
#include "XPLMMenus.h"
#include "OE_Notify.h"
#include "OE_Globals.h"
#include "XObjReadWrite.h"
#include "OE_Utils.h"

enum {
	fileCmd_Save,
};

const	char *	sFileCmds[] = {
	"&Save",
	0
};

const	char	sFileKeys[] = {
	'S',	xplm_ControlFlag
};



static	void	FileCmdHandler(void * inMenuRef, void * inItemRef);
static	void	FileCmdHandleNotification(int inCatagory, int inMsg, void * inParam);
static	void	FileCmdUpdateItems(void);

static	XPLMMenuID	sFileMenu;

void	SetupFileCmds(void)
{
	OE_RegisterNotifyFunc(FileCmdHandleNotification);

	sFileMenu = XPLMCreateMenu("&File", NULL, 0, FileCmdHandler, NULL);
	for (int n = 0; sFileCmds[n]; ++n)
	{
		XPLMAppendMenuItem(sFileMenu, sFileCmds[n], (void *) n, 1);
		if (sFileKeys[n*2])
			XPLMSetMenuItemKey(sFileMenu,n,sFileKeys[n*2], sFileKeys[n*2+1]);
	}

	FileCmdUpdateItems();

}

void	FileCmdHandler(void * inMenuRef, void * inItemRef)
{
	switch((int) inItemRef) {
	case fileCmd_Save:
		{
			XObj	obj;
			OE_MergeObject(gObjects, gObjectLOD, obj);
			XObjWrite(gFilePath.c_str(), obj);
		}
		break;
	}
}

void	FileCmdHandleNotification(int inCatagory, int inMsg, void * inParam)
{
}

void	FileCmdUpdateItems(void)
{
}



