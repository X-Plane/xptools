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

WED_UndoMgr::WED_UndoMgr(WED_Archive * inArchive) : mCommand(NULL), mArchive(inArchive)
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

void	WED_UndoMgr::StartCommand(const string& inName)
{
	while(mUndo.size() > MAX_UNDO_LEVELS)
	{
		delete mUndo.front();
		mUndo.pop_front();
	}
	if (mCommand != NULL)
		AssertPrintf("Command %s started while command %s still active.",inName.c_str(), mCommand->GetName().c_str());
	mCommand = new WED_UndoLayer(mArchive, inName);
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
	WED_UndoLayer * redo = new WED_UndoLayer(mArchive, undo->GetName());
	mArchive->SetUndo(redo);
	int change_mask = undo->GetChangeMask();
	undo->Execute();
	mArchive->SetUndo(NULL);
	mRedo.push_front(redo);
	delete undo;
	mUndo.pop_back();
	mArchive->mOpCount--;
	mArchive->BroadcastMessage(msg_ArchiveChanged,change_mask);		
}

void	WED_UndoMgr::Redo(void)
{
	DebugAssert(!mRedo.empty());
	WED_UndoLayer * redo = mRedo.front();
	WED_UndoLayer * undo = new WED_UndoLayer(mArchive, redo->GetName());
	mArchive->SetUndo(undo);
	int change_mask = redo->GetChangeMask();
	redo->Execute();
	mArchive->SetUndo(NULL);	
	mUndo.push_back(undo);
	delete redo;
	mRedo.pop_front();
	mArchive->mOpCount++;
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
