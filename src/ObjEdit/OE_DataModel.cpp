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
#include "OE_DataModel.h"
#include "OE_Globals.h"
#include "XPLMMenus.h"
#include "OE_Notify.h"
#include "OE_Msgs.h"

#define	kMaxUndo 30

enum {
	undoCmd_Undo = 0,
	undoCmd_Redo = 1
};

static const char * sUndoCmds [] = {
	"&Undo",
	"&Redo",
	0
};

static const char	sUndoKeys [] = { 
	'Z',				xplm_ControlFlag,
	'Z',				xplm_ControlFlag + xplm_ShiftFlag,
	0,					0
};

static	XPLMMenuID	sUndoCmdMenu;

static	void	UndoCmdHandler(void * inMenuRef, void * inItemRef);
static	void	UndoCmdNotification(int inCatagory, int inMsg, void * inParam);
static	void	UndoCmdUpdateItems(void);

struct	UndoState_t {
	string			cmd;
	vector<XObj>	objects;
	set<int>		selection;
	int				levelOfDetail;
	int				rebuildStep;
	
	void	FromGlobals(void);
	void	ToGlobals(void);
};

typedef	vector<UndoState_t>	UndoVector_t;

// Undo stack's back item is the most recently done command.
// Redo stack's back item is the most recently undone command.

static UndoVector_t		sUndo;
static UndoVector_t		sRedo;
static UndoState_t		sCommand;

static bool				sCommandInProgress = false;
	
void	UndoState_t::FromGlobals(void)
{
	objects 		= gObjects		;
	selection 		= gSelection	;
	levelOfDetail 	= gLevelOfDetail;
	rebuildStep 	= gRebuildStep	;
}

void	UndoState_t::ToGlobals(void)
{
	gObjects		= objects 		;
	gSelection		= selection 	;
	gLevelOfDetail	= levelOfDetail ;
	gRebuildStep	= rebuildStep 	;
}

void	OE_BeginCommand(const char * inCommandName)
{
	sCommand.cmd = inCommandName;
	sCommand.FromGlobals();
	sCommandInProgress = true;
}

void	OE_CompleteCommand(void)
{
	if (sCommandInProgress);
		sUndo.push_back(sCommand);
	sCommandInProgress = false;
	sRedo.clear();
	if (sUndo.size() > kMaxUndo)
	{
		int undo_amount = sUndo.size() - kMaxUndo;
		sUndo.erase(sUndo.begin(), sUndo.begin() + undo_amount);
	}
	OE_Notifiable::Notify(catagory_Object, msg_ObjectTexturingChanged, NULL);	
}

void	OE_AbortCommand(void)
{
	if (sCommandInProgress)
	{
		sCommand.ToGlobals();
	}
	sCommandInProgress = false;
}

void	OE_Undo(void)
{
	if (sUndo.empty())	return;
	UndoState_t	curState;
	curState.cmd = sUndo.back().cmd;
	curState.FromGlobals();
	sRedo.push_back(curState);
	sUndo.back().ToGlobals();
	sUndo.pop_back();
	OE_Notifiable::Notify(catagory_Object, msg_ObjectTexturingChanged, NULL);	
}

void	OE_Redo(void)
{
	if (sRedo.empty())	return;
	UndoState_t	curState;
	curState.cmd = sRedo.back().cmd;
	curState.FromGlobals();
	sUndo.push_back(curState);
	sRedo.back().ToGlobals();
	sRedo.pop_back();
	OE_Notifiable::Notify(catagory_Object, msg_ObjectTexturingChanged, NULL);	
}

void	OE_PurgeUndo(void)
{
	sUndo.clear();
	sRedo.clear();
}

bool	OE_HasUndo(char * outCmdName)
{
	if (outCmdName && !sUndo.empty())
		strcpy(outCmdName, sUndo.back().cmd.c_str());
	return !sUndo.empty();
}

bool	OE_HasRedo(char * outCmdName)
{
	if (outCmdName && !sRedo.empty())
		strcpy(outCmdName, sRedo.back().cmd.c_str());
	return !sRedo.empty();
}

#pragma mark -

void	OE_SetupUndoCmds()
{
	OE_RegisterNotifyFunc(UndoCmdNotification);
	
	sUndoCmdMenu = XPLMCreateMenu("&Edit", NULL, 0, UndoCmdHandler, NULL);
	for (int n = 0; sUndoCmds[n]; ++n)
	{
		XPLMAppendMenuItem(sUndoCmdMenu, sUndoCmds[n], (void *) n, 1);
		if (sUndoKeys[n*2])
			XPLMSetMenuItemKey(sUndoCmdMenu,n,sUndoKeys[n*2],sUndoKeys[n*2+1]);
	}
	
	UndoCmdUpdateItems();
}

static	void	UndoCmdHandler(void * inMenuRef, void * inItemRef)
{
	int item = (int) inItemRef;
	switch(item) {
	case undoCmd_Undo:	OE_Undo();	break;
	case undoCmd_Redo:	OE_Redo();	break;
	}
	UndoCmdUpdateItems();
}

static	void	UndoCmdNotification(int inCatagory, int inMsg, void * inParam)
{
	UndoCmdUpdateItems();
}

static	void	UndoCmdUpdateItems(void)
{
	char	undoCmd[100], redoCmd[100];
	char	undoStr[120], redoStr[120];
	bool	undoOk = OE_HasUndo(undoCmd);
	bool	redoOk = OE_HasRedo(redoCmd);
	
	if (undoOk)
		sprintf(undoStr, "Undo %s", undoCmd);
	else	
		sprintf(undoStr, "Can't Undo", undoCmd);
	XPLMSetMenuItemName(sUndoCmdMenu, undoCmd_Undo, undoStr, 1);

	if (redoOk)
		sprintf(redoStr, "Redo %s", redoCmd);
	else
		sprintf(redoStr, "Can't Redo");		
	XPLMSetMenuItemName(sUndoCmdMenu, undoCmd_Redo, redoStr, 1);

	XPLMEnableMenuItem(sUndoCmdMenu, undoCmd_Undo, undoOk);
	XPLMEnableMenuItem(sUndoCmdMenu, undoCmd_Redo, redoOk);
}

