#ifndef WED_UNDOMGR_H
#define WED_UNDOMGR_H

#include <list>
using std::list;

class	WED_Archive;
class	WED_UndoLayer;

class	WED_UndoMgr {
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

private:

	typedef list<WED_UndoLayer *>	LayerList;
	
	LayerList 		mUndo;
	LayerList		mRedo;
	
	WED_UndoLayer *	mCommand;
	WED_Archive *	mArchive;

};
#endif
