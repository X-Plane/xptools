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
#include "OE_SelCmds.h"
#include "XPLMMenus.h"
#include "OE_Notify.h"
#include "OE_Globals.h"
#include "OE_Msgs.h"
#include "OE_Utils.h"

enum {
	selCmd_SelectNone,
	selCmd_SelectAll,
	selCmd_SelectFirst,	
	selCmd_SelectPrev,
	selCmd_SelectPrevUntextured,
	selCmd_SelectNext,
	selCmd_SelectNextUntextured,
	selCmd_SelectLast,
	selCmd_Divider1,
	selCmd_FirstLOD,
	selCmd_PrevLOD,
	selCmd_NextLOD,
	selCmd_LastLOD
};		

const	char *	sSelCmds[] = {
	"Select &None",
	"Select &All",
	"Select &First",
	"Select &Previous",
	"Select Previous Untextured",
	"Select &Next",
	"Select Next Untextured",
	"Select &Last",
	"-",
	"First LOD",
	"Prev LOD",
	"Next LOD",
	"Last LOD",
	0
};

const	char	sSelKeys[] = {
	'D',			xplm_ControlFlag,
	'A',			xplm_ControlFlag,
	XPLM_KEY_UP,	xplm_ControlFlag,
	XPLM_KEY_LEFT,	xplm_ControlFlag,
	XPLM_KEY_LEFT,	xplm_ControlFlag + xplm_ShiftFlag,
	XPLM_KEY_RIGHT,	xplm_ControlFlag,
	XPLM_KEY_RIGHT,	xplm_ControlFlag + xplm_ShiftFlag,
	XPLM_KEY_DOWN,	xplm_ControlFlag,
	
	0,				0,
	
	XPLM_KEY_UP,	xplm_ControlFlag + xplm_OptionAltFlag,
	XPLM_KEY_LEFT,	xplm_ControlFlag + xplm_OptionAltFlag,
	XPLM_KEY_RIGHT,	xplm_ControlFlag + xplm_OptionAltFlag,
	XPLM_KEY_DOWN,	xplm_ControlFlag + xplm_OptionAltFlag
};



static	void	SelCmdHandler(void * inMenuRef, void * inItemRef);
static	void	SelCmdHandleNotification(int inCatagory, int inMsg, void * inParam);
static	void	SelCmdUpdateItems(void);

static	XPLMMenuID	sSelMenu;

void	SetupSelCmds(void)
{
	OE_RegisterNotifyFunc(SelCmdHandleNotification);
	
	sSelMenu = XPLMCreateMenu("&Select", NULL, 0, SelCmdHandler, NULL);
	for (int n = 0; sSelCmds[n]; ++n)
	{
		XPLMAppendMenuItem(sSelMenu, sSelCmds[n], (void *) n, 1);
		
		if (sSelKeys[n*2])
			XPLMSetMenuItemKey(sSelMenu, n, sSelKeys[n*2], sSelKeys[n*2+1]);		
	}
	
	SelCmdUpdateItems();

}

void	SelCmdHandler(void * inMenuRef, void * inItemRef)
{
	if (gObjects.empty())	return;
	int	sel;
	int	cmdCount = gObjects[gLevelOfDetail].cmds.size();
	int lodCount = gObjects.size();
	
	switch((int) inItemRef) {
	case selCmd_SelectNone:
		gSelection.clear();
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		break;
	case selCmd_SelectAll:
		for (int n = 0; n < cmdCount; ++n)
			gSelection.insert(n);
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		break;
	case selCmd_SelectFirst:
		gSelection.clear();
		if (cmdCount > 0)
			gSelection.insert(0);
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		break;
	case selCmd_SelectPrev:
		if (cmdCount > 0)
		{
			if (gSelection.empty())
			sel = 0; 
				else
			sel = *gSelection.begin();
			--sel;
			if (sel < 0)
				sel = cmdCount - 1;
			gSelection.clear();
			gSelection.insert(sel);
			gRebuildStep = -1;
			OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		}
		break;	
	case selCmd_SelectNext:		
		if (cmdCount > 0)
		{
			sel = OE_MaxSelected() + 1;
			if (sel >= cmdCount)
				sel = 0;
			gSelection.clear();
			gSelection.insert(sel);
			gRebuildStep = -1;
			OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		}
		break;
	case selCmd_SelectNextUntextured:
	case selCmd_SelectPrevUntextured:
		if (cmdCount > 0)
		{
			sel = OE_NextPrevUntextured(((int) inItemRef == selCmd_SelectPrevUntextured) ? -1 : 1);
			gSelection.clear();
			gSelection.insert(sel);
			gRebuildStep = -1;
			OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		}
		break;
	case selCmd_SelectLast:
		gSelection.clear();
		if (cmdCount > 0)
			gSelection.insert(cmdCount - 1);
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		break;

	case selCmd_FirstLOD:
		gLevelOfDetail = 0;		
		gSelection.clear();
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		OE_Notifiable::Notify(catagory_Object, msg_ObjectLODChanged, NULL);
		break;
	case selCmd_PrevLOD:
		gLevelOfDetail++;
		if (gLevelOfDetail >= lodCount)
			gLevelOfDetail = 0;
		gSelection.clear();
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		OE_Notifiable::Notify(catagory_Object, msg_ObjectLODChanged, NULL);
		break;
	case selCmd_NextLOD:
		gLevelOfDetail--;
		if (gLevelOfDetail < 0)
			gLevelOfDetail = lodCount - 1;
		gSelection.clear();
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		OE_Notifiable::Notify(catagory_Object, msg_ObjectLODChanged, NULL);
		break;
	case selCmd_LastLOD:
		gLevelOfDetail = lodCount - 1;		
		gSelection.clear();
		gRebuildStep = -1;
		OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
		OE_Notifiable::Notify(catagory_Object, msg_ObjectLODChanged, NULL);
		break;
	}

}

void	SelCmdHandleNotification(int inCatagory, int inMsg, void * inParam)
{
	switch(inCatagory) {
	case catagory_Object:
		switch(inMsg) {
		case msg_ObjectLoaded:
		case msg_ObjectSelectionChanged:
		case msg_ObjectLODChanged:
			SelCmdUpdateItems();
			break;
		}
		break;
	}
}

void	SelCmdUpdateItems(void)
{
	bool	hasLODs = gObjects.size() > 1;
	bool	hasItems = (!gObjects.empty() && !gObjects[gLevelOfDetail].cmds.empty());
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectNone, hasItems);
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectAll,hasItems);
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectFirst,	hasItems);
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectPrev,hasItems);
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectPrevUntextured,hasItems);
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectNext,hasItems);
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectNextUntextured,hasItems);
	XPLMEnableMenuItem(sSelMenu, selCmd_SelectLast, hasItems);

	XPLMEnableMenuItem(sSelMenu, selCmd_FirstLOD, hasLODs);
	XPLMEnableMenuItem(sSelMenu, selCmd_PrevLOD, hasLODs);
	XPLMEnableMenuItem(sSelMenu, selCmd_NextLOD, hasLODs);
	XPLMEnableMenuItem(sSelMenu, selCmd_LastLOD, hasLODs);

}
