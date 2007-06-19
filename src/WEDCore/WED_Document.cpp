#include "WED_Document.h"
#include "WED_Progress.h"
#include "GUI_Resources.h"
#include "FileUtils.h"
#include "XESIO.h"
#include "AptIO.h"
#include "MapAlgs.h"
#include "WED_Thing.h"
#include "WED_Messages.h"
#include "WED_EnumSystem.h"
#include <sqlite3.h>
#include "WED_Errors.h"
#include "GUI_Resources.h"
#include "PlatformUtils.h"
// TODO: 
// migrate all old stuff
// wire dirty to obj persistence

static string	process_path(const string& package_path)
{
	string ret(package_path);
	FILE_make_dir_exist(ret.c_str());
	ret += DIR_STR "WED";
	FILE_make_dir_exist(ret.c_str());
	ret += DIR_STR "earth.wed";
	return ret;
}

static set<WED_Document *> sDocuments;

WED_Document::WED_Document(
								const string& 		path, 
								double				inBounds[4]) :
//	mProperties(mDB.get()),
	mFilePath(process_path(path)),
	mDB(mFilePath.c_str()),
//	mPackage(inPackage),
	mUndo(&mArchive)
{
	sDocuments.insert(this);
	mArchive.SetUndoManager(&mUndo);
	
	string buf;
	
	GUI_Resource res = GUI_LoadResource("WED_DataModel.sql");
	if (res == NULL)
		WED_ThrowPrintf("Unable to open SQL code: %s.", buf.c_str());
	
	sql_do_bulk_range(mDB.get(), GUI_GetResourceBegin(res), GUI_GetResourceEnd(res));
	GUI_UnloadResource(res);


	mBounds[0] = inBounds[0];
	mBounds[1] = inBounds[1];
	mBounds[2] = inBounds[2];
	mBounds[3] = inBounds[3];	
	
	mUndo.StartCommand("Load from disk.");
	enum_map_t	mapping;
	ENUM_read(mDB.get(), mapping);
	mArchive.LoadFromDB(mDB.get(),mapping);
	mUndo.CommitCommand();
	mUndo.PurgeUndo();
	mUndo.PurgeRedo();
}

WED_Document::~WED_Document()
{
	sDocuments.erase(this);
	BroadcastMessage(msg_DocumentDestroyed, 0);
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
	int result = sql_do(mDB.get(),"BEGIN TRANSACTION;");
	#if ERROR_CHECK
	hello
	#endif

	mArchive.SaveToDB(mDB.get());
	ENUM_write(mDB.get());
	result = sql_do(mDB.get(),"COMMIT TRANSACTION;");
}

void	WED_Document::Revert(void)
{
	mUndo.StartCommand("Revert from Saved.");
	
	enum_map_t	mapping;
	ENUM_read(mDB.get(), mapping);
	mArchive.ClearAll();
	mArchive.LoadFromDB(mDB.get(), mapping);
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

bool	WED_Document::TryCloseAll(void)
{
	set<WED_Document *>	documents(sDocuments);
	for (set<WED_Document *>::iterator p = documents.begin(); p != documents.end(); ++p)
	{
		if (!(*p)->TryClose()) return false;
	}
	return true;
}


