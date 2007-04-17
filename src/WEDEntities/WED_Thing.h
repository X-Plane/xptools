#ifndef WED_THING_H
#define WED_THING_H

/*
	WED_Thing - THEORY OF OPERATION
	
	WED_Thing is the base persistent object for WorldEditor.  Quite literally EVERYTHIGN is a WED_Thing.  
	
	WED_Things have a few tricks:
	
		- They are property helpers, and thus meet the property objecct interface for generalized editing.
		- They are persistent.
		- They are proeprty helpers, so all subclasses can easily embed "properties".
		- They use the property helper's streaming code.  Thus you don't have to worry about undo for prop-helper-type code.
		- They provide nesting via a parent-child relationship to other WED persistent objs, managing undo safely.
		- All things have names (at least until I decide that this is silly).

 */

#include "WED_Persistent.h"
#include "WED_PropertyHelper.h"
#include "IArray.h"
#include "IDirectory.h"
#include "IOperation.h"

class	WED_Thing : public WED_Persistent, public WED_PropertyHelper, public virtual IArray, public virtual IDirectory, public virtual IOperation {

DECLARE_INTERMEDIATE(WED_Thing)

public:

			int					CountChildren(void) const;
			WED_Thing *			GetNthChild(int n) const;
			WED_Thing *			GetNamedChild(const string& s) const;
			WED_Thing *			GetParent(void) const;
			void				SetParent(WED_Thing * parent, int nth);

			void				GetName(string& name) const;
			void				SetName(const string& name);

	// WED_Persistent
	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual void			FromDB(sqlite3 * db);
	virtual void			ToDB(sqlite3 * db);	
	
	// From WED_PropertyHelper...
	virtual	void			PropEditCallback(int before);	
	
	// IArray
	virtual	int				Array_Count (void );
	virtual IUnknown *		Array_GetNth(int n);
	
	// IDirectory
	virtual	IUnknown *	Directory_Find(const char * name);

	// IOperation
	virtual		void			StartOperation(const char * op_name);
	virtual		void			CommitOperation(void);
	virtual		void			AbortOperation(void);		
	
private:
			void				AddChild(int id, int n);
			void				RemoveChild(int id);

	int				parent_id;
	vector<int>		child_id;
	
	WED_PropStringText			name;

};
	

#endif /* WED_THING_H */
