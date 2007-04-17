#ifndef WED_ARCHIVE_H
#define WED_ARCHIVE_H

/*

	A quick note on the concept of "dirtiness"
	
	Objects are saved to and from databases.
	An object is dirty if and only if its in-memory setup has CHANGED from the database.
	
	Therefore the rules for dirtiness are these:
	1. Any NEW objects are "born" dirty - that is, if we make a new obj in mem, it clearly doesn't come from the DB.		[persistent]
	2. Any time we LOAD an obj from the DB, we clear the dirty flag, because we know at that instant that we're clean.		[archive]
	3. Any time we save, if save IF AND ONLY IF we're dirty, and we then get to clear the dirty flag.						[archive]
	
	This saves us database I/O - even though we have to touch the whole DB for read when we load our file (for now), we don't have
	to touch the whole DB for right and blast the hell out of all indices.
	
	Also note that undo DOESN'T restore dirtiness yet - if we delete an obj and undo, the same data WILL be written out to the archive
	because the undo system isn't smart enough to see what happened.

*/

struct sqlite3;

class	WED_Persistent;
class	WED_UndoLayer;
class	WED_UndoMgr;

#include "GUI_Broadcaster.h"

class	WED_Archive : public GUI_Broadcaster {
public:

					WED_Archive();
					~WED_Archive();

	// Find an object in the archive by ID
	WED_Persistent *	Fetch(int in_id) const;

	// Attach an undo layer - must be attached and detached with NULL in sequence.
	void			SetUndo(WED_UndoLayer * inUndo);

	void			LoadFromDB(sqlite3 * db);
	void			SaveToDB(sqlite3 * db);
	
	// Undo convenience API.  
	void			SetUndoManager(WED_UndoMgr * mgr);
	void			StartCommand(const string& inName);		// pass-throughs
	void			CommitCommand(void);
	void			AbortCommand(void);
	
	int				NewID(void);
	
private:

	void			ChangedObject	(WED_Persistent * inObject);	
	void			AddObject		(WED_Persistent * inObject);
	void			RemoveObject	(WED_Persistent * inObject);

	friend class	WED_Persistent;

	typedef hash_map<int, WED_Persistent *>	ObjectMap;	

	ObjectMap		mObjects;		// Our objects!
	bool			mDying;			// Flag to self - WE are killing ourselves - ignore objects.
	WED_UndoLayer *	mUndo;
	WED_UndoMgr *	mUndoMgr;
// Not allowed yet

	WED_Archive(const WED_Archive& rhs);
	WED_Archive& operator=(const WED_Archive& rhs);
	
	int				mID;

};

#endif
