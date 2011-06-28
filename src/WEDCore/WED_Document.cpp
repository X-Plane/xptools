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

#include "WED_Document.h"
#include "GUI_Resources.h"
#include "WED_PackageMgr.h"
#include "FileUtils.h"
//#include "XESIO.h"
#include "AptIO.h"
//#include "MapAlgs.h"
#include "WED_Thing.h"
#include "WED_Messages.h"
#include "WED_EnumSystem.h"
#include <sqlite3.h>
#include "WED_Errors.h"
#include "GUI_Resources.h"
#include "GUI_Prefs.h"
#include "PlatformUtils.h"
#include "WED_TexMgr.h"
#include "WED_LibraryMgr.h"
#include "WED_ResourceMgr.h"

// TODO:
// migrate all old stuff
// wire dirty to obj persistence


static set<WED_Document *> sDocuments;

static map<string,string>	sGlobalPrefs;

WED_Document::WED_Document(
								const string& 		package,
								double				inBounds[4]) :
//	mProperties(mDB.get()),
	mPackage(package),
	mFilePath(gPackageMgr->ComputePath(package, "earth.wed")),
	mDB(mFilePath.c_str()),
//	mPackage(inPackage),
	mUndo(&mArchive),
	mArchive(this)
{
	mTexMgr = new WED_TexMgr(package);
	mLibraryMgr = new WED_LibraryMgr(package);
	mResourceMgr = new WED_ResourceMgr(mLibraryMgr);
	sDocuments.insert(this);
	mArchive.SetUndoManager(&mUndo);

	string buf;

	GUI_Resource res = GUI_LoadResource("WED_DataModel.sql");
	if (res == NULL)
		WED_ThrowPrintf("Unable to open SQL code resource: %s.", buf.c_str());

	sql_do_bulk_range(mDB.get(), GUI_GetResourceBegin(res), GUI_GetResourceEnd(res));
	GUI_UnloadResource(res);

#if AIRPORT_ROUTING
	string buf2;

	GUI_Resource res2 = GUI_LoadResource("WED_DataModel2.sql");
	if (res2 == NULL)
		WED_ThrowPrintf("Unable to open SQL code resource: %s.", buf2.c_str());

	sql_do_bulk_range(mDB.get(), GUI_GetResourceBegin(res2), GUI_GetResourceEnd(res2));
	GUI_UnloadResource(res2);
#endif

	mBounds[0] = inBounds[0];
	mBounds[1] = inBounds[1];
	mBounds[2] = inBounds[2];
	mBounds[3] = inBounds[3];

	Revert();
	mUndo.PurgeUndo();
	mUndo.PurgeRedo();
}

WED_Document::~WED_Document()
{
	delete mTexMgr;
	delete mResourceMgr;
	delete mLibraryMgr;
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
	BroadcastMessage(msg_DocWillSave, reinterpret_cast<long>(static_cast<IDocPrefs *>(this)));
	int result = sql_do(mDB.get(),"BEGIN TRANSACTION;");
	#if ERROR_CHECK
	hello
	#endif

	mArchive.SaveToDB(mDB.get());
	ENUM_write(mDB.get());

	int err;
	{
		sql_command	clear_table(mDB.get(),"DELETE FROM WED_doc_prefs WHERE 1;",NULL);
		err = clear_table.simple_exec();
		if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(mDB.get()),err);
	}

	{
		sql_command add_item(mDB.get(),"INSERT INTO WED_doc_prefs VALUES(@k,@v);","@k,@v");
		for(map<string,string>::iterator i = mDocPrefs.begin(); i != mDocPrefs.end(); ++i)
		{
			sql_row2<string,string>	r;
			r.a = i->first;
			r.b = i->second;
			err = add_item.simple_exec(r);
			if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(mDB.get()),err);
		}
	}

	result = sql_do(mDB.get(),"COMMIT TRANSACTION;");
}

void	WED_Document::Revert(void)
{
	mUndo.__StartCommand("Revert from Saved.",__FILE__,__LINE__);

	enum_map_t	mapping;
	ENUM_read(mDB.get(), mapping);
	mArchive.ClearAll();
	mArchive.LoadFromDB(mDB.get(), mapping);
	mUndo.CommitCommand();

	mDocPrefs.clear();
	int err;
	{
		sql_command	get_prefs(mDB.get(),"SELECT key,value FROM WED_doc_prefs WHERE 1;",NULL);

		sql_row2<string, string>	p;
		get_prefs.begin();
		while((err = get_prefs.get_row(p)) == SQLITE_ROW)
		{
			mDocPrefs[p.a] = p.b;
			sGlobalPrefs[p.a] = p.b;
		}
		if (err != SQLITE_DONE)
			WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(mDB.get()),err);
	}

	BroadcastMessage(msg_DocLoaded, reinterpret_cast<long>(static_cast<IDocPrefs *>(this)));
}

bool	WED_Document::IsDirty(void)
{
	return mArchive.IsDirty() != 0;
}

bool	WED_Document::TryClose(void)
{
	if (IsDirty())
	{
		string msg = string("Save changes to scenery package ") + mPackage + string(" before closing?");

		switch(DoSaveDiscardDialog("Save changes before closing...",msg.c_str())) {
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

	if (strcmp(in_path,"librarian")==0) return (ILibrarian *) this;
	if (strcmp(in_path,"texmgr")==0) return (ITexMgr *) mTexMgr;
	if (strcmp(in_path,"resmgr")==0) return (WED_ResourceMgr *) mResourceMgr;
	if (strcmp(in_path,"docprefs")==0) return (IDocPrefs *) this;

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



void	WED_Document::LookupPath(string& io_path)
{
	io_path = gPackageMgr->ComputePath(mPackage, io_path);
}

void	WED_Document::ReducePath(string& io_path)
{
	io_path = gPackageMgr->ReducePath(mPackage, io_path);
}


int			WED_Document::ReadIntPref(const char * in_key, int in_default)
{
	string key(in_key);
	map<string,string>::iterator i = mDocPrefs.find(key);
	if (i == mDocPrefs.end())
	{
		i = sGlobalPrefs.find(key);
		if (i == sGlobalPrefs.end())
			return in_default;
		return atoi(i->second.c_str());
	}
	return atoi(i->second.c_str());
}

void		WED_Document::WriteIntPref(const char * in_key, int in_value)
{
	char buf[256];
	sprintf(buf,"%d",in_value);
	mDocPrefs[in_key] = buf;
	sGlobalPrefs[in_key] = buf;
}

double			WED_Document::ReadDoublePref(const char * in_key, double in_default)
{
	string key(in_key);
	map<string,string>::iterator i = mDocPrefs.find(key);
	if (i == mDocPrefs.end())
	{
		i = sGlobalPrefs.find(key);
		if (i == sGlobalPrefs.end())
			return in_default;
		return atof(i->second.c_str());
	}
	return atof(i->second.c_str());
}

void		WED_Document::WriteDoublePref(const char * in_key, double in_value)
{
	char buf[256];
	sprintf(buf,"%lf",in_value);
	mDocPrefs[in_key] = buf;
	sGlobalPrefs[in_key] = buf;
}



string			WED_Document::ReadStringPref(const char * in_key, const string& in_default)
{
	string key(in_key);
	map<string,string>::iterator i = mDocPrefs.find(key);
	if (i == mDocPrefs.end())
	{
		i = sGlobalPrefs.find(key);
		if (i == sGlobalPrefs.end())
			return in_default;
		return i->second;
	}
	return i->second;
}

void		WED_Document::WriteStringPref(const char * in_key, const string& in_value)
{
	mDocPrefs[in_key] = in_value;
	sGlobalPrefs[in_key] = in_value;
}

static void PrefCB(const char * key, const char * value, void * ref)
{
	sGlobalPrefs[key] = value;
}


void	WED_Document::ReadGlobalPrefs(void)
{
	GUI_EnumSection("doc_prefs", PrefCB, NULL);
}

void	WED_Document::WriteGlobalPrefs(void)
{
	for (map<string,string>::iterator i = sGlobalPrefs.begin(); i != sGlobalPrefs.end(); ++i)
		GUI_SetPrefString("doc_prefs", i->first.c_str(), i->second.c_str());
}
