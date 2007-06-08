#ifndef WED_SELECT_H
#define WED_SELECT_H

/*
	WED_Select - THEORY OF OPERATION
	
	WED_Select implmements ISelection for WED, using a set of object persistent IDs.
	
	WED_Select is itself persistent, that is, changes in the selection are undoable.  This is done
	to maintain sanity -- if we back up the object model without backing up the selection, we could
	have dead objects selected, which would require a bunch of special cases to handle in our editing
	code.
	
*/
#include "WED_Thing.h"
#include "GUI_Broadcaster.h"
#include "ISelection.h"

class	WED_Select : public WED_Thing, public virtual ISelection, public GUI_Broadcaster { 

DECLARE_PERSISTENT(WED_Select)

public:

	// ISelection
	virtual		bool			IsSelected(IBase * who) const;

	virtual		void			Select(IBase * who);
	virtual		void			Clear (void			 );
	virtual		void			Toggle(IBase * who);
	virtual		void			Insert(IBase * who);
	virtual		void			Erase (IBase * who);

	virtual		int				GetSelectionCount(void) const;
	virtual		void			GetSelectionSet(set<IBase *>& sel) const;
	virtual		void			GetSelectionVector(vector<IBase *>& sel) const;
	virtual		IBase *			GetNthSelection(int n) const;

	virtual		int				IterateSelection(int (* func)(IBase * who, void * ref), void * ref) const;

	// WED_Persistent
	virtual		void 			ReadFrom(IOReader * reader);
	virtual		void 			WriteTo(IOWriter * writer);
	virtual		void			FromDB(sqlite3 * db, const map<int,int>& mapping);
	virtual		void			ToDB(sqlite3 * db);
	
private:

	set<int>		mSelected;

};
	
#endif
