#ifndef WED_ARCHIVE_H
#define WED_ARCHIVE_H

#include "WED_GUID.h"	// Strange STL note - if we forward declare it this WILL compile
						// but with some @#$@ed implicit hash func that is autogeneated -
						// we get a cryptic double-declare later.  Creepy.

class	WED_Persistent;
class	WED_UndoLayer;


class	WED_Archive {
public:

					WED_Archive();
					~WED_Archive();

	// Find an object in the archive by GUID
	WED_Persistent *	Fetch(const WED_GUID& inGUID) const;

	// Attach an undo layer - must be attached and detached with NULL in sequence.
	void				SetUndo(WED_UndoLayer * inUndo);

private:

	void			ChangedObject(WED_Persistent * inObject);	
	void			AddObject(WED_Persistent * inObject);
	void			RemoveObject(WED_Persistent * inObject);

	friend class	WED_Persistent;

	typedef hash_map<WED_GUID, WED_Persistent *>	ObjectMap;	

	ObjectMap		mObjects;		// Our objects!
	bool			mDying;			// Flag to self - WE are killing ourselves - ignore objects.
	WED_UndoLayer *	mUndo;
	
// Not allowed yet

	WED_Archive(const WED_Archive& rhs);
	WED_Archive& operator=(const WED_Archive& rhs);

};

#endif
