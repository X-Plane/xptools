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
#include "WED_Group.h"
#include "WED_KeyObjects.h"
#include "WED_Select.h"
#include "WED_Root.h"
#include "WED_Messages.h"
#include "WED_EnumSystem.h"
#include <sqlite3.h>
#include "WED_XMLWriter.h"
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
//	mDB(mFilePath.c_str()),
//	mPackage(inPackage),
	mUndo(&mArchive, this),
	mArchive(this)
{
	mTexMgr = new WED_TexMgr(package);
	mLibraryMgr = new WED_LibraryMgr(package);
	mResourceMgr = new WED_ResourceMgr(mLibraryMgr);
	sDocuments.insert(this);
	mArchive.SetUndoManager(&mUndo);

	string buf;

//	GUI_Resource res = GUI_LoadResource("WED_DataModel.sql");
//	if (res == NULL)
//		WED_ThrowPrintf("Unable to open SQL code resource: %s.", buf.c_str());
//
//	sql_do_bulk_range(mDB.get(), GUI_GetResourceBegin(res), GUI_GetResourceEnd(res));
//	GUI_UnloadResource(res);

#if AIRPORT_ROUTING
//	string buf2;
//
//	GUI_Resource res2 = GUI_LoadResource("WED_DataModel2.sql");
//	if (res2 == NULL)
//		WED_ThrowPrintf("Unable to open SQL code resource: %s.", buf2.c_str());
//
//	sql_do_bulk_range(mDB.get(), GUI_GetResourceBegin(res2), GUI_GetResourceEnd(res2));
//	GUI_UnloadResource(res2);
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

/*
	int result = sql_do(mDB.get(),"BEGIN TRANSACTION;");
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
*/
	string xml = mFilePath;
	xml += ".xml";
	FILE * xml_file = fopen(xml.c_str(),"w");
	if(xml_file)
	{
		fprintf(xml_file,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		{
			WED_XMLElement	top_level("doc",0,xml_file);
			mArchive.SaveToXML(&top_level);
			WED_XMLElement * pref;
			WED_XMLElement * prefs = top_level.add_sub_element("prefs");
			for(map<string,vector<string> >::iterator pi = mDocPrefsItems.begin(); pi != mDocPrefsItems.end(); ++pi)
			{
				pref = prefs->add_sub_element("pref");
				pref->add_attr_stl_str("name",pi->first);
				pref->add_attr_stl_str("value","enum");
				for(vector<string>::iterator i = pi->second.begin(); i != pi->second.end(); ++i)
				{
					WED_XMLElement *item = pref->add_sub_element("item");
					item->add_attr_stl_str("value",*i);
				}
			}
			for(map<string,string>::iterator p = mDocPrefs.begin(); p != mDocPrefs.end(); ++p)
			{
				pref = prefs->add_sub_element("pref");
				pref->add_attr_stl_str("name",p->first);
				pref->add_attr_stl_str("value",p->second);
			}
		}
		fclose(xml_file);
	}
}

void	WED_Document::Revert(void)
{
		mDocPrefs.clear();
	mUndo.__StartCommand("Revert from Saved.",__FILE__,__LINE__);

	try {

		WED_XMLReader	reader;
		reader.PushHandler(this);
		string fname(mFilePath);
		fname+=".xml";
		mArchive.ClearAll();

		// First: try to IO the XML file.
		bool xml_exists;
		string result = reader.ReadFile(fname.c_str(),&xml_exists);
		if(xml_exists && !result.empty())
			WED_ThrowPrintf("Unable to open XML file: %s",result.c_str());

		if(!xml_exists)
		{
			// If XML fails because it's AWOL, go back and do the SQL-style read-in.
			sql_db db(mFilePath.c_str(), SQLITE_OPEN_READWRITE);
			if(db.get())
			{
				GUI_Resource res = GUI_LoadResource("WED_DataModel.sql");
				if (res == NULL)
					WED_ThrowPrintf("Unable to open SQL code resource.",0);

				sql_do_bulk_range(db.get(), GUI_GetResourceBegin(res), GUI_GetResourceEnd(res));
				GUI_UnloadResource(res);





				enum_map_t	mapping;
				ENUM_read(db.get(), mapping);
				mArchive.ClearAll();
				mArchive.LoadFromDB(db.get(), mapping);

				int err;
				{
					sql_command	get_prefs(db.get(),"SELECT key,value FROM WED_doc_prefs WHERE 1;",NULL);

					sql_row2<string, string>	p;
					get_prefs.begin();
					while((err = get_prefs.get_row(p)) == SQLITE_ROW)
					{
						mDocPrefs[p.a] = p.b;
						sGlobalPrefs[p.a] = p.b;
					}
					if (err != SQLITE_DONE)
						WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db.get()),err);
				}
			}
			else
			{
				/* We have a brand new blank doc.  In WED 1.0, we ran a SQL script that built the core objects,
				 * then we IO-ed it in.  In WED 1.1 we just build the world and the few named objs immediately. */

				WED_Root * root = WED_Root::CreateTyped(&mArchive);				// Root object anchors all WED things and supports named searches.
				WED_Select * sel = WED_Select::CreateTyped(&mArchive);			// Sel and key-choice objs are known by name in the root!
				WED_KeyObjects * key = WED_KeyObjects::CreateTyped(&mArchive);
				WED_Group * wrl = WED_Group::CreateTyped(&mArchive);			// The world is a group of name "world" inside the root.
				sel->SetParent(root,0);
				key->SetParent(root,1);
				wrl->SetParent(root,2);
				root->SetName("root");
				sel->SetName("selection");
				key->SetName("choices");
				wrl->SetName("world");
			}

		}
	} catch(...) {
		// don't bail out of the load loop without aborting the command - otherwise undo mgr goes ape.
		mUndo.AbortCommand();
		throw;
	}
	mUndo.CommitCommand();

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

void		WED_Document::ReadEnumIntPref(const char * in_key, vector<int>* out_value)
{
	if(!out_value) return;

	string key(in_key);
	map<string,vector<string> >::iterator i = mDocPrefsItems.find(key);
	if (i == mDocPrefsItems.end()) return;

	for(vector<string>::iterator si = i->second.begin(); si !=  i->second.end(); ++si)
	{
	      out_value->push_back(atoi(si->c_str()));
	}
}

void		WED_Document::WriteEnumIntPref(const char * in_key, vector<int>* in_value)
{
	char buf[256];
	vector<string> v;
	for(vector<int>::iterator i = in_value->begin(); i != in_value->end(); ++i)
	{
	    sprintf(buf,"%d",*i);
	    v.push_back(buf);
	}
	mDocPrefsItems[in_key] = v;
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

void		WED_Document::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	const char * n = NULL, * v = NULL;

	if(strcasecmp(name,"objects")==0)
	{

		reader->PushHandler(&mArchive);
	}
	if(strcasecmp(name,"prefs")==0)
	{
		mDocPrefs.clear();
		mDocPrefsItems.clear();
		mDocPrefsActName = "";
	}
	else if(strcasecmp(name,"pref")==0)
	{
		while(*atts)
		{
			if(strcasecmp(*atts, "name")==0)
			{
				++atts;
				n = *atts;
				++atts;
			}
			else if(strcasecmp(*atts,"value")==0)
			{
				++atts;
				v = *atts;
				++atts;
			}
			else
			{
				++atts;
				++atts;
			}
		}
		if(n && v)
		{
			if(strcasecmp(v,"enum")==0)
			{
				mDocPrefsActName = n;
				mDocPrefsItems[n]= vector<string>();
			}
			else
				mDocPrefs[n] = v;
		}
		else
			reader->FailWithError("Invalid pref: missing key or value.");
	}
	else if(strcasecmp(name,"item")==0)
	{
        while(*atts)
		{
			if(strcasecmp(*atts,"value")==0)
			{
				++atts;
				v = *atts;
				++atts;
			}
			else
			{
				++atts;
				++atts;
			}
		}
		if(v)
		{
			map<string,vector<string> >::iterator i = mDocPrefsItems.find(mDocPrefsActName);
			if (i == mDocPrefsItems.end())
			   reader->FailWithError("Invalid item: no parent entry.");
			else
			   i->second.push_back(v);
		}
		else
			reader->FailWithError("Invalid item: missing  value.");
	}
}

void		WED_Document::EndElement(void)
{
}
void		WED_Document::PopHandler(void)
{
}

void WED_Document::Panic(void)
{
	// Panic case: means undo system blew up.  Try to save off the current project with a special "crash" extension - if we get lucky,
	// we save the user's work.
	string xml = mFilePath;
	xml += ".crash.xml";
	FILE * xml_file = fopen(xml.c_str(),"w");
	if(xml_file)
	{
		fprintf(xml_file,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		{
			WED_XMLElement	top_level("doc",0,xml_file);
			mArchive.SaveToXML(&top_level);
			WED_XMLElement * pref;
			WED_XMLElement * prefs = top_level.add_sub_element("prefs");
			for(map<string,vector<string> >::iterator pi = mDocPrefsItems.begin(); pi != mDocPrefsItems.end(); ++pi)
			{
				pref = prefs->add_sub_element("pref");
				pref->add_attr_stl_str("name",pi->first);
				pref->add_attr_stl_str("value","enum");
				for(vector<string>::iterator i = pi->second.begin(); i != pi->second.end(); ++i)
				{
					WED_XMLElement *item = pref->add_sub_element("item");
					item->add_attr_stl_str("value",*i);
				}
			}
			for(map<string,string>::iterator p = mDocPrefs.begin(); p != mDocPrefs.end(); ++p)
			{
				pref = prefs->add_sub_element("pref");
				pref->add_attr_stl_str("name",p->first);
				pref->add_attr_stl_str("value",p->second);
			}
		}
		fclose(xml_file);
	}
}


