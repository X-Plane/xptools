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

class	WED_UndoMgr : public GUI_MemoryHog {
public:

	WED_UndoMgr(WED_Archive * inArchive);
	~WED_UndoMgr();
	
	void	StartCommand(const string& inName);
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
	
	WED_UndoLayer *	mCommand;
	WED_Archive *	mArchive;

};
#endif
