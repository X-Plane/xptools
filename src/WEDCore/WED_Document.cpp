#include "WED_Document.h"
#include "WED_Progress.h"
#include "GUI_Resources.h"
#include "XESIO.h"
#include "AptIO.h"
#include "MapAlgs.h"
#include "WED_Thing.h"
#include "WED_Messages.h"
#include <sqlite3.h>
#include "WED_Errors.h"
#include "GUI_Resources.h"

// TODO: 
// migrate all old stuff
// wire dirty to obj persistence

WED_Document::WED_Document(
								const string& 		path, 
								WED_Package * 		inPackage,
								double				inBounds[4]) :
	mDB(path.c_str()),
//	mProperties(mDB.get()),
	mFilePath(path),
	mPackage(inPackage),
	mUndo(&mArchive)
{
	mArchive.SetUndoManager(&mUndo);
	
	string buf;
	if (!GUI_GetResourcePath("WED_DataModel.sql",buf))
		WED_ThrowPrintf("Unable to open SQL code: %s.", buf.c_str());
	
	MFMemFile * f = MemFile_Open(buf.c_str());
	if (f == NULL) WED_ThrowPrintf("Unable t open %s.\n", buf.c_str());
	sql_do_bulk_range(mDB.get(), MemFile_GetBegin(f), MemFile_GetEnd(f));
	MemFile_Close(f);


	mBounds[0] = inBounds[0];
	mBounds[1] = inBounds[1];
	mBounds[2] = inBounds[2];
	mBounds[3] = inBounds[3];	
	
	mUndo.StartCommand("Load from disk.");
	mArchive.LoadFromDB(mDB.get());
	mUndo.CommitCommand();
	mUndo.PurgeUndo();
	mUndo.PurgeRedo();
}

WED_Document::~WED_Document()
{
	printf("Starting doc dtor.\n");
//	mArchive.SaveToDB(mDB.get());
	printf("Doc saved, broadcasting.\n");
	BroadcastMessage(msg_DocumentDestroyed, 0);
	printf("Ending doc dtor.\n");
}

string				WED_Document::GetFilePath(void) const
{
	return mFilePath;
}

void		WED_Document::GetBounds(double bounds[4])
{
	for (int n = 0; n < 4; ++n)
		bounds[n] = mBounds[n];
}

/*
CREATE TABLE properties (key VARCHAR PRIMARY KEY,value DOUBLE);
CREATE INDEX key_idx ON properties (key);
SELECT COUNT(*) FROM properties WHERE key = 'key' AND value IS NOT NULL;
SELECT value FROM properties WHERE key = 'key';
DELETE FROM propeties WHERE key = 'key';
*/

WED_Archive *		WED_Document::GetArchive(void)
{
	return &mArchive;
}

WED_Thing *		WED_Document::GetRoot(void)
{
	return SAFE_CAST(WED_Thing,mArchive.Fetch(1));
}

WED_UndoMgr *	WED_Document::GetUndoMgr(void)
{
	return &mUndo;
}

void	WED_Document::Save(void)
{
	mArchive.SaveToDB(mDB.get());
}

void	WED_Document::Revert(void)
{
	mUndo.StartCommand("Revert from Saved.");
	mArchive.ClearAll();
	mArchive.LoadFromDB(mDB.get());
	mUndo.CommitCommand();
}

bool	WED_Document::IsDirty(void)
{
	return mArchive.IsDirty() != 0;
}

bool	WED_Document::TryClose(void)
{
	if (IsDirty())
	{
		switch(DoSaveDiscardDialog("Save this document.","It needs saving.")) {
		case close_Save:	Save();	break;
		case close_Discard:			break;
		case close_Cancel:	return false;
		}
	}
	AsyncDestroy();
	return true;
}


IBase *	WED_Document::Resolver_Find(const char * in_path)
{
	const char * sp = in_path;
	const char * ep;
	
	IBase * who = mArchive.Fetch(1);
	
	while(*sp != 0)
	{
		if(*sp == '[')
		{
			++sp;
			int idx = atoi(sp);
			while(*sp != 0 && *sp != ']') ++sp;
			if (*sp == 0) return NULL;
			++sp;
			
			IArray * arr = SAFE_CAST(IArray, who);
			if(arr == NULL) return NULL;
			who = arr->Array_GetNth(idx);
			if (who == NULL) return NULL;
			
			if (*sp == '.') ++sp;			
		}
		else
		{
			ep = sp;
			while (*ep != 0 && *ep != '[' && *ep != '.') ++ep;			
			string comp(sp,ep);
			
			IDirectory * dir = SAFE_CAST(IDirectory,who);
			if (dir == NULL) return NULL;
			who = dir->Directory_Find(comp.c_str());
			if (who == NULL) return NULL;
			sp = ep;
			
			if (*sp == '.') ++sp;
		}		
	}
	return who;
}

/*
void *		WED_Document::QueryInterface(const char * class_id)
{
	if (strcmp(class_id,"IResolver")==0)
		return (IResolver *) this;
	return NULL;
}

*/