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

#ifndef WED_UNDOMGR_H
#define WED_UNDOMGR_H

#include "GUI_MemoryHog.h"

#include <list>
using std::list;

class	WED_Archive;
class	WED_UndoLayer;

// Panic handler.  Basically if the undo system hits a command in a command, it can go totally ape - it has no way to record the command because a command is already running.  
// If we abort the previous command, the world goes inconsistent from underneath us.  And if we're called from inside a UI tracker like the vertex tool, we're just going to get 
// a ton of calls AS IF the command was running.  Oh shit!
//
// For now, to avoid work loss: when this happens, call a panic handler - the doc will use this to do a safe-save if we can.
//
// In the future, mmmaybe we find a way to "back off" a gesture (hrm - the UI tracker would have to "drop" the click - but even that isn't always enough to know when the cmd
// is over).  Truth is, this represents a finite number of bugs that we should just squash.

class	WED_UndoFatalErrorHandler {
public:	
	virtual	void	Panic(void)=0;
};

class	WED_UndoMgr : public GUI_MemoryHog {
public:

	WED_UndoMgr(WED_Archive * inArchive, WED_UndoFatalErrorHandler * panic_handler);
	~WED_UndoMgr();

	void	__StartCommand(const string& inName, const char * inFile, int inLine);
	void	CommitCommand(void);
	void	AbortCommand(void);

	bool	HasUndo(void) const;
	bool	HasRedo(void) const;
	string	GetUndoName(void) const;
	string	GetRedoName(void) const;

	void	Undo(void);
	void	Redo(void);

	void	PurgeUndo(void);
	void	PurgeRedo(void);

	// From GUI_MemoryHog
	virtual	bool	ReleaseMemory(void);

private:

	typedef list<WED_UndoLayer *>	LayerList;

	LayerList 		mUndo;
	LayerList		mRedo;

	WED_UndoLayer *				mCommand;
	WED_Archive *				mArchive;
	WED_UndoFatalErrorHandler *	mPanicHandler;

};
#endif
