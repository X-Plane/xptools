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

#include "WED_UndoMgr.h"
#include "WED_Archive.h"
#include "WED_UndoLayer.h"
#include "AssertUtils.h"
#include "WED_Messages.h"
#include "PlatformUtils.h"
// UNDO STACK ORDER IS:

// CHRONOLOGICAL FROM BEGIN TO END
// So...the last op DONE is in undo.back()
// The first op UNDONE is redo.front()

#define WARN_IF_LESS_LEVEL	10
#define MAX_UNDO_LEVELS 20

WED_UndoMgr::WED_UndoMgr(WED_Archive * inArchive, WED_UndoFatalErrorHandler * panic_handler) : mCommand(NULL), mArchive(inArchive), mPanicHandler(panic_handler)
{
}

WED_UndoMgr::~WED_UndoMgr()
{
	DebugAssert(mCommand == NULL);
	if (mCommand) delete mCommand;
	mCommand = NULL;
	PurgeUndo();
	PurgeRedo();
}

static const char * trim_file(const char * p)
{
	const char * ret = p;
	while(*p)
	{
		if(*p == '\\' || *p == ':' || *p == '/')
			ret = p+1;
		++p;
	}
	return ret;
}

void	WED_UndoMgr::__StartCommand(const string& inName, const char * file, int line)
{
	while(mUndo.size() > MAX_UNDO_LEVELS)
	{
		delete mUndo.front();
		mUndo.pop_front();
	}
	
	// This is the asset case that often burns us: a command is started WHILE another command is going on.  This happens due to
	// either bad UI code or unknown weird shit from the window mgr.
	if (mCommand != NULL)
	{
		if (mPanicHandler) 
		{
			// If we have a panic handler, abort the old cmd, don't start the new one. 
			mArchive->SetUndo(NULL);
			mArchive->SetUndo(UNDO_DISCARD);
			mCommand->Execute();
			// Now when we call panic, the previous unfinished cmd is backed out and we are in the last sane state.  The user loses the
			// TWO half-done cmds that clashed, and that's it.
			mPanicHandler->Panic();
		}
		AssertPrintf("Command %s (%s:%d) started while command %s (%s:%d) still active.",inName.c_str(), trim_file(file), line, mCommand->GetName().c_str(), trim_file(mCommand->GetFile()), mCommand->GetLine());
	}
	mCommand = new WED_UndoLayer(mArchive, inName, file, line);
	mArchive->SetUndo(mCommand);
}

void	WED_UndoMgr::CommitCommand(void)
{
	Assert(mCommand != NULL);
	mArchive->SetUndo(NULL);
	if (mCommand->Empty())
	{
		delete mCommand;
		mCommand = NULL;
		return;
	}
	PurgeRedo();
	mUndo.push_back(mCommand);
	int change_mask = mCommand->GetChangeMask();
	mCommand = NULL;
	mArchive->BroadcastMessage(msg_ArchiveChanged,change_mask);
}

void	WED_UndoMgr::AbortCommand(void)
{
	mArchive->SetUndo(NULL);
	mArchive->SetUndo(UNDO_DISCARD);
	Assert(mCommand != NULL);
	mCommand->Execute();
	delete mCommand;
	mCommand = NULL;
	mArchive->SetUndo(NULL);
}

bool	WED_UndoMgr::HasUndo(void) const
{
	return !mUndo.empty();
}

bool	WED_UndoMgr::HasRedo(void) const
{
	return !mRedo.empty();
}

string	WED_UndoMgr::GetUndoName(void) const
{
	DebugAssert(!mUndo.empty());
	return "&Undo " + mUndo.back()->GetName();
}

string	WED_UndoMgr::GetRedoName(void) const
{
	DebugAssert(!mRedo.empty());
	return "&Redo " + mRedo.front()->GetName();
}

void	WED_UndoMgr::Undo(void)
{
	DebugAssert(!mUndo.empty());
	WED_UndoLayer * undo = mUndo.back();
	WED_UndoLayer * redo = new WED_UndoLayer(mArchive, undo->GetName(), undo->GetFile(), undo->GetLine());
	mArchive->SetUndo(redo);
	int change_mask = undo->GetChangeMask();
	undo->Execute();
	mArchive->SetUndo(NULL);
	mRedo.push_front(redo);
	delete undo;
	mUndo.pop_back();
	mArchive->mOpCount--;
	mArchive->mCacheKey++;
	mArchive->BroadcastMessage(msg_ArchiveChanged,change_mask);
}

void	WED_UndoMgr::Redo(void)
{
	DebugAssert(!mRedo.empty());
	WED_UndoLayer * redo = mRedo.front();
	WED_UndoLayer * undo = new WED_UndoLayer(mArchive, redo->GetName(), redo->GetFile(), redo->GetLine());
	mArchive->SetUndo(undo);
	int change_mask = redo->GetChangeMask();
	redo->Execute();
	mArchive->SetUndo(NULL);
	mUndo.push_back(undo);
	delete redo;
	mRedo.pop_front();
	mArchive->mOpCount++;
	mArchive->mCacheKey++;
	mArchive->BroadcastMessage(msg_ArchiveChanged,change_mask);
}

void	WED_UndoMgr::PurgeUndo(void)
{
	for (LayerList::iterator l = mUndo.begin(); l != mUndo.end(); ++l)
		delete *l;
	mUndo.clear();
}

void	WED_UndoMgr::PurgeRedo(void)
{
	for (LayerList::iterator l = mRedo.begin(); l != mRedo.end(); ++l)
		delete *l;
	mRedo.clear();
}

bool	WED_UndoMgr::ReleaseMemory(void)
{
	if (mUndo.empty() && mRedo.empty()) return false;
	if (mUndo.size() > WARN_IF_LESS_LEVEL)
	{
		delete mUndo.front();
		mUndo.pop_front();
		return true;
	}

//	if(!ConfirmMessage("WED is low on memory.  May I purge the undo list to free up memory?", "Purge", "Cancel")) return false;

	if (!mRedo.empty())
	{
		PurgeRedo();
		return true;
	}

	delete mUndo.front();
	mUndo.pop_front();
	return true;

}
