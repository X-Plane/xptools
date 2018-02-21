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

#include <stdint.h>

#if WITHNWLINK
#include "WED_Server.h"
#include "WED_NWLinkAdapter.h"
#endif
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
#include "WED_XMLWriter.h"
#include "WED_Errors.h"
#include "GUI_Resources.h"
#include "GUI_Prefs.h"
#include "PlatformUtils.h"
#include "WED_TexMgr.h"
#include "WED_LibraryMgr.h"
#include "WED_ResourceMgr.h"
#include "WED_GroupCommands.h"

#if IBM
#include "GUI_Unicode.h"
#endif
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
#if WITHNWLINK
	mServer(NULL),
	mNWLink(NULL),
	mOnDisk(false),
#endif
	mUndo(&mArchive, this),
	mArchive(this)
{

	mTexMgr = new WED_TexMgr(package);
	mLibraryMgr = new WED_LibraryMgr(package);
	mResourceMgr = new WED_ResourceMgr(mLibraryMgr);
	sDocuments.insert(this);
	mArchive.SetUndoManager(&mUndo);

	string buf;

	mBounds[0] = inBounds[0];
	mBounds[1] = inBounds[1];
	mBounds[2] = inBounds[2];
	mBounds[3] = inBounds[3];

	Revert();
	mUndo.PurgeUndo();
	mUndo.PurgeRedo();

#if WITHNWLINK
	if(sDocuments.size()==1)// only first document
	{
		mServer = new WED_Server(mPackage,ReadIntPref("network/port",10300));
		mNWLink	= new WED_NWLinkAdapter(mServer,&mArchive);
		mServer->AddListener(mNWLink);
		AddListener(mNWLink);
		mArchive.SetNWLinkAdapter(mNWLink);
	}
#endif
}

WED_Document::~WED_Document()
{
	delete mTexMgr;
	delete mResourceMgr;
	delete mLibraryMgr;
#if WITHNWLINK
	delete mNWLink;
	delete mServer;
#endif
	sDocuments.erase(this);
	BroadcastMessage(msg_DocumentDestroyed, 0);
}

string				WED_Document::GetFilePath(void) const
{
	return mFilePath + ".xml";
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
#if WITHNWLINK
WED_Server *	WED_Document::GetServer(void)
{
	return mServer;
}
WED_NWLinkAdapter *	WED_Document::GetNWLink(void)
{
	return mNWLink;
}
#endif
void	WED_Document::Save(void)
{
	BroadcastMessage(msg_DocWillSave, reinterpret_cast<uintptr_t>(static_cast<IDocPrefs *>(this)));

	enum {none,nobackup,both};
	int stage = none;

	//Create the strings path.
	//earth.wed.xml,
	//earth.wed.bak.xml,
	//earth.wed.bak.bak.xml, deleted before the user sees it.

	//All or some of these are used through out
	
	string xml = mFilePath;
	xml += ".xml";
	
	string bakXML = xml;	
	bakXML = bakXML.insert((bakXML.length()-4),".bak");

	string tempBakBak = bakXML;
	tempBakBak = tempBakBak.insert((bakXML.length()-4),".bak");
	
	bool earth_wed_xml = FILE_exists(xml.c_str());
	bool earth_wed_bak_xml = FILE_exists(bakXML.c_str());

	//If there is no earth.wed.xml file for any reason
	if(earth_wed_xml == false)
	{
		stage = none;
	}
	//If there is no back up and there is an earth.wed.xml
	else if(earth_wed_xml == true && earth_wed_bak_xml == false)
	{
		stage = nobackup;
	}
	//If there is a back up and an earth.wed.xml
	else if(earth_wed_xml == true && earth_wed_bak_xml == true)
	{
		stage = both;
	}

	//This is the renaming switch
	switch(stage)
	{
	case none:
		break;
	case nobackup:
		//Rename the current earth.wed.xml to earth.wed.bak.xml
		FILE_rename_file(xml.c_str(),bakXML.c_str());
		break;
	case both:
		//next rename the current earth.wed.bak.xml to earth.wed.bak.bak.xml
		FILE_rename_file(bakXML.c_str(),tempBakBak.c_str());
		
		//Rename the current earth.wed.xml to earth.wed.bak.xml
		FILE_rename_file(xml.c_str(),bakXML.c_str());
		break;
	}
	
	//Create an xml file by opening the file located on the hard drive (windows)
	//open a file for writing creating/nukeing if necissary

	FILE * xml_file = fopen(xml.c_str(),"w");

	if(xml_file == NULL)
	{
		DoUserAlert("Please check file path for errors or missing parts");
		return;
	}

	int ferrorErr = ferror(xml_file);
		//If everything else has worked
	if(ferrorErr == 0)
	{
		WriteXML(xml_file);
	}
	int fcloseErr = fclose(xml_file);
	if(ferrorErr != 0 || fcloseErr != 0)
	{
		//This is the error handling switch
		switch(stage)
		{
			case none:
				FILE_delete_file(xml.c_str(), false);
				DoUserAlert("Please check file path for errors or missing parts");
				break;
			case nobackup:
				//Delete's the bad save
				FILE_delete_file(xml.c_str(), false);
				//un-renames the old one
				FILE_rename_file(bakXML.c_str(),xml.c_str());
				DoUserAlert("Please check file path for errors or missing parts");
				break;
			case both:
				//delete incomplete file
				FILE_delete_file(xml.c_str(), false);

				//un-rename earth.wed.bak.xml to earth.wed.xml
				FILE_rename_file(bakXML.c_str(), xml.c_str());

				//un-rename earth.wed.bak.bak.xml to earth.wed.bak.xml
				FILE_rename_file(tempBakBak.c_str(), bakXML.c_str());
				DoUserAlert("Please check file path for errors or missing parts");
				break;
		}
	}	
	else
	{
		// This is the save-was-okay case.
		mOnDisk=true;
	}
	
	//if the second backup still exists after the error handling
	if(FILE_exists(tempBakBak.c_str()) == true)
	{
		//Delete it
		FILE_delete_file(tempBakBak.c_str(), false);
	}
}

void	WED_Document::Revert(void)
{
	if(this->IsDirty())
	{
		string msg = "Are you sure you want to revert the document '" + mPackage + "' to the saved version on disk?";
		if(!ConfirmMessage(msg.c_str(),"Revert","Cancel"))
			return;
	}

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

		if(xml_exists)
		{
			mOnDisk=true;
		}
		else
		{
				/* We have a brand new blank doc.  In WED 1.0, we ran a SQL script that built the core objects,
				 * then we IO-ed it in.  In WED 1.1 we just build the world and the few named objs immediately. */
				 
				// BASIC DOCUMENT STRUCTURE:
				// The first object ever made gets ID 1 and is the "root" - the one known object.  The WED doc goes
				// to "object 1" to get started.
				// THEN the root contains three well-known objects by name - so their order isn't super-important as
				// children or in the archive.  The "World" is the outer most spatial group, the "selection" is our one
				// and only selection object, and "choices" is our key-value dictionary for various random crap we'll
				// need.  (Currently only current airprt is set in the key-value lookup.)

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
	} catch(...) {
		// don't bail out of the load loop without aborting the command - otherwise undo mgr goes ape.
		mUndo.AbortCommand();
		throw;
	}
	mUndo.CommitCommand();

	if(WED_Repair(this))
	{
		string msg = string("Warning: the package '") + mPackage + string("' has some corrupt contents."
					"They have been deleted so that the document can be opened.  If you have a better backup of the project, do not save these changes.");
		DoUserAlert(msg.c_str());
		mUndo.PurgeUndo();
	}

	BroadcastMessage(msg_DocLoaded, reinterpret_cast<uintptr_t>(static_cast<IDocPrefs *>(this)));
}

bool	WED_Document::IsDirty(void)
{
	return mArchive.IsDirty() != 0;
}

bool	WED_Document::IsOnDisk(void)
{
	return mOnDisk;
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
#if WITHNWLINK
	if(mServer)
	{
		mServer->DoStop();
		WriteIntPref("network/port", mServer->GetPort());
	}
#endif
	AsyncDestroy();     // This prevents most class deconstructors from being executed.
	
	delete this;        // So we do that ... its needed by WED_ResourceMgr
	return true;
}


IBase *	WED_Document::Resolver_Find(const char * in_path)
{
	const char * sp = in_path;
	const char * ep;

	if (strcmp(in_path,"librarian")==0) return (ILibrarian *) this;
	if (strcmp(in_path,"texmgr")==0) return (ITexMgr *) mTexMgr;
	if (strcmp(in_path,"resmgr")==0) return (WED_ResourceMgr *) mResourceMgr;
	if (strcmp(in_path,"libmgr")==0) return (WED_LibraryMgr *) mLibraryMgr;
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

void		WED_Document::ReadIntSetPref(const char * in_key, set<int>& out_value)
{
	string key(in_key);
	map<string,set<int> >::iterator i = mDocPrefsItems.find(key);
	if (i == mDocPrefsItems.end()) return;
	out_value = i->second;
}

void		WED_Document::WriteIntSetPref(const char * in_key, const set<int>& in_value)
{
	mDocPrefsItems[in_key] = in_value;
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
				mDocPrefs[n] = v;
		}
		else if(n)
		{
			mDocPrefsActName = n;
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
			set<int>& ip(mDocPrefsItems[mDocPrefsActName]);
			ip.insert(atoi(v));
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
		WriteXML(xml_file);
		fclose(xml_file);
	}
}

void		WED_Document::WriteXML(FILE * xml_file)
{
	//print to file the xml file passed in with the following encoding
	fprintf(xml_file,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	{
		WED_XMLElement	top_level("doc",0,xml_file);
		mArchive.SaveToXML(&top_level);
		WED_XMLElement * pref;
		WED_XMLElement * prefs = top_level.add_sub_element("prefs");
		for(map<string,set<int> >::iterator pi = mDocPrefsItems.begin(); pi != mDocPrefsItems.end(); ++pi)
		{
			pref = prefs->add_sub_element("pref");
			pref->add_attr_stl_str("name",pi->first);
			for(set<int>::iterator i = pi->second.begin(); i != pi->second.end(); ++i)
			{
				WED_XMLElement *item = pref->add_sub_element("item");
				item->add_attr_int("value",*i);
			}
		}
		for(map<string,string>::iterator p = mDocPrefs.begin(); p != mDocPrefs.end(); ++p)
		{
			pref = prefs->add_sub_element("pref");
			pref->add_attr_stl_str("name",p->first);
			pref->add_attr_stl_str("value",p->second);
		}
	}
}


