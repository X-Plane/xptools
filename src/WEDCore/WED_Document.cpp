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
#include "WED_PackageMgr.h"
#include "FileUtils.h"
#include "MathUtils.h"
#include "PlatformUtils.h"
#include "WED_ToolUtils.h"
#include "WED_Thing.h"
#include "WED_Group.h"
#include "WED_Airport.h"
#include "WED_KeyObjects.h"
#include "WED_Select.h"
#include "WED_Root.h"
#include "WED_Messages.h"
#include "WED_EnumSystem.h"
#include "WED_XMLWriter.h"
#include "WED_Errors.h"
#include "WED_TexMgr.h"
#include "WED_LibraryMgr.h"
#include "WED_ResourceMgr.h"
#include "WED_SceneryImport.h"
#include "WED_GroupCommands.h"
#include "WED_Version.h"

#include "GUI_Fonts.h"
#include "GUI_Resources.h"
#include "GUI_Prefs.h"

#if IBM
#include "GUI_Unicode.h"
#endif

#include "WED_Globals.h"
int gIsFeet;
int gInfoDMS;
int gModeratorMode;
int gFontSize;
string gCustomSlippyMap;
int gOrthoExport;

static set<WED_Document *> sDocuments;
static map<string,string>	sGlobalPrefs;

WED_Document::WED_Document(
	const string& package,
	double				inBounds[4]) :
	mPackage(package),
	mFilePath(gPackageMgr->ComputePath(package, "earth.wed")),
#if WITHNWLINK
	mServer(NULL),
	mNWLink(NULL),
#endif
	mOnDisk(false),
	mPrefsChanged(false),
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

	// Help WED novices with the frustrating "opened scenery, WED just shows empty screen" by looking for importable stuff
	if (!mOnDisk)
	{
		int this_pkg;
		for (this_pkg = gPackageMgr->CountCustomPackages(); this_pkg > 0; this_pkg--)
		{
			string tmp;
			gPackageMgr->GetNthPackageName(this_pkg, tmp);
			if (tmp == package)
				break;
		}
		if (this_pkg > 0 && gPackageMgr->HasAPT(this_pkg))
		{
			if (ConfirmMessage("This scenery package does not include the WED design files.\n"
				               "But it does include already exported X-Plane scenery.\n"
				               "WED can import these, but some loss of functionality or features may inccurr.\n"
							   "For best results, always start editing with the original WED design files\n"
							   "\n"
							   "YES - try recovering scenery\n"
							   "NO  - start with new, empty scenery\n", "Yes", "No") == 1)
			{
			string scn_path = gPackageMgr->ComputePath(package, "Earth nav data");

			mUndo.__StartCommand("Revert from Saved.", __FILE__, __LINE__);
			if (WED_SceneryImport(scn_path, WED_GetWorld(this), true))
				mUndo.CommitCommand();
            else
				mUndo.AbortCommand();
			}
		}
	}

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

#include <chrono>

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

	auto t0 = std::chrono::high_resolution_clock::now();

#if 0
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
#else
	rename(xml.c_str(), bakXML.c_str());
#endif
	auto t1 = std::chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = t1 - t0;
	LOG_MSG("\nrename b4 save %.3lf s\n", elapsed.count());
	t0 = t1;

	//Create an xml file by opening the file located on the hard drive (windows)
	//open a file for writing creating/nukeing if neccessary

	FILE * xml_file = fopen(xml.c_str(),"w");

	if(xml_file == NULL)
	{
		string msg = "Can not open '" + xml + "' for writing.";
		DoUserAlert(msg.c_str());
		return;
	}

	int ferrorErr = ferror(xml_file);
		//If everything else has worked
	if(ferrorErr == 0)
	{
		WriteXML(xml_file);
	}

	int fcloseErr = fclose(xml_file);

	t1 = std::chrono::high_resolution_clock::now();
	elapsed = t1 - t0;
	LOG_MSG("xml write %.3lf s\n", elapsed.count());
	t0 = t1;

	if(ferrorErr != 0 || fcloseErr != 0)
	{
		string msg =  "Error while writing '" + xml + "'";
#if 0
		switch(stage)
		{
			case none:
				FILE_delete_file(xml.c_str(), false);
				break;
			case nobackup:
				//Delete's the bad save
				FILE_delete_file(xml.c_str(), false);
				//un-renames the old one
				FILE_rename_file(bakXML.c_str(),xml.c_str());
				msg += " or creating backup '" + bakXML;
				break;
			case both:
				//delete incomplete file
				FILE_delete_file(xml.c_str(), false);

				//un-rename earth.wed.bak.xml to earth.wed.xml
				FILE_rename_file(bakXML.c_str(), xml.c_str());

				//un-rename earth.wed.bak.bak.xml to earth.wed.bak.xml
				FILE_rename_file(tempBakBak.c_str(), bakXML.c_str());
				msg += " or renaming backups to '" + bakXML + "' or '" + tempBakBak;
				break;
		}
#else
		rename(bakXML.c_str(), xml.c_str());
#endif
		msg += "'.";
		DoUserAlert(msg.c_str());
	}
	else
	{
		// This is the save-was-okay case.
		mOnDisk=true;
		mPrefsChanged=false;
	}
#if 0
	//if the second backup still exists after the error handling
	if(FILE_exists(tempBakBak.c_str()) == true)
	{
		//Delete it
		FILE_delete_file(tempBakBak.c_str(), false);
	}
#endif
	t1 = std::chrono::high_resolution_clock::now();
	elapsed = t1 - t0;
	LOG_MSG("delete aft save %.3lf s\n", elapsed.count());
}

void	WED_Document::Revert(void)
{
	if(this->IsDirty())
	{
		string msg = "Are you sure you want to revert the document '" + mPackage + "' to the saved version on disk?";
		if(!ConfirmMessage(msg.c_str(), "Revert", "Cancel"))
			return;
	}
	mDocPrefs.clear();
	auto t0 = std::chrono::high_resolution_clock::now();

	try {
		mUndo.__StartCommand("Revert from Saved.", __FILE__, __LINE__);
		bool xml_exists;

		WED_XMLReader	reader;
		reader.PushHandler(this);
		string fname(mFilePath + ".xml");
		mArchive.ClearAll();

		LOG_MSG("I/Doc reading XML from %s\n", fname.c_str());
		string result = reader.ReadFile(fname.c_str(), &xml_exists);

		if(xml_exists && !result.empty())
		{
			LOG_MSG("E/Doc Error reading XML %s",result.c_str());
			string msg("An error '");
			msg += result + "'ocurred while opening the XML file.\n";

			WED_XMLReader	reader_bak;
			reader_bak.PushHandler(this);
			mArchive.ClearAll();
			fname = mFilePath + ".bak.xml";

			LOG_MSG("I/Doc reading backup XML from %s\n", fname.c_str());
			result = reader_bak.ReadFile(fname.c_str(), &xml_exists);

			if (result.empty())
			{
				msg += "But a good backup file exists.\n";
				if (ConfirmMessage(msg.c_str(), "Open backup", "Cancel") == 0)
					throw;
			}
			else
			{
				LOG_MSG("E/Doc Error reading backup XML %s", result.c_str());
				WED_ThrowPrintf("%s\nAnd the backup XML resulted in error: %s", msg.c_str(), result.c_str());
			}
		}

		for (auto sp : mDocPrefs)
			LOG_MSG("I/Doc prefs %s = %s\n", sp.first.c_str(), sp.second.c_str());

		if(xml_exists)
		{
			mOnDisk=true;
			mPrefsChanged = false;
		}
		else
		{
				// We have a brand new blank doc.

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
				LOG_MSG("I/Doc brand new empty doc\n");
		}
		mUndo.CommitCommand();
	} catch(...) {
		// don't bail out of the load loop without aborting the command - otherwise undo mgr goes ape.
		mUndo.AbortCommand();
		throw;
	}

	auto t1 = std::chrono::high_resolution_clock::now();
	chrono::duration<double> elapsed = t1 - t0;
	LOG_MSG("XML reading %.3lf s\n", elapsed.count());

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
	return mArchive.IsDirty() != 0 || mPrefsChanged;
}

void	WED_Document::SetDirty(void)
{
	mPrefsChanged = true;
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


int			WED_Document::ReadIntPref(const char * in_key, int in_default, unsigned type)
{
	std::string value;
	if ((type & pref_type_doc) && ReadPrefInternal(in_key, pref_type_doc, value))
	{
		return atoi(value.c_str());
	}
	else if ((type & pref_type_global) && ReadPrefInternal(in_key, pref_type_global, value))
	{
		if (strcmp(in_key, "doc/export_target") == 0)        // ignore a few things when creating new, empty docs. Too many users get confused by inheriting "unusual" settings
			return wet_latest_xplane;
		else if (strcmp(in_key, "map/obj_density") == 0)
			return in_default;
		else
			return atoi(value.c_str());
	}
	else
	{
		return in_default;
	}
}

void		WED_Document::WriteIntPref(const char * in_key, int in_value, unsigned type)
{
	char buf[256];
	sprintf(buf,"%d",in_value);
	WriteStringPref(in_key, buf, type);
}

double			WED_Document::ReadDoublePref(const char * in_key, double in_default, unsigned type)
{
	std::string value;
	if (ReadPrefInternal(in_key, type, value))
		return atof(value.c_str());
	else
		return in_default;
}

void		WED_Document::WriteDoublePref(const char * in_key, double in_value, unsigned type)
{
	char buf[256];
	sprintf(buf,"%lf",in_value);
	WriteStringPref(in_key, buf, type);
}

string			WED_Document::ReadStringPref(const char * in_key, const string& in_default, unsigned type)
{
	std::string value;
	if (ReadPrefInternal(in_key, type, value))
		return value;
	else
		return in_default;
}

void		WED_Document::WriteStringPref(const char * in_key, const string& in_value, unsigned type)
{
	if (type & pref_type_doc)
		mDocPrefs[in_key] = in_value;
	if (type & pref_type_global)
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

	gIsFeet  = atoi(GUI_GetPrefString("preferences","use_feet","0"));
	gInfoDMS = atoi(GUI_GetPrefString("preferences","InfoDMS","0"));
	gCustomSlippyMap = GUI_GetPrefString("preferences","CustomSlippyMap","");
	int FontSize = atoi(GUI_GetPrefString("preferences","FontSize","12"));
	gFontSize = intlim(FontSize, 10, 18);
	GUI_SetFontSizes(gFontSize);
	gOrthoExport = atoi(GUI_GetPrefString("preferences","OrthoExport","1"));
}

void	WED_Document::WriteGlobalPrefs(void)
{
	GUI_SetPrefString("preferences","use_feet",gIsFeet ? "1" : "0");
	GUI_SetPrefString("preferences","InfoDMS",gInfoDMS ? "1" : "0");
	GUI_SetPrefString("preferences","CustomSlippyMap",gCustomSlippyMap.c_str());
	string FontSize(to_string(gFontSize));
	GUI_SetPrefString("preferences","FontSize",FontSize.c_str());
	GUI_SetPrefString("preferences","OrthoExport",gOrthoExport ? "1" : "0");

	for (map<string,string>::iterator i = sGlobalPrefs.begin(); i != sGlobalPrefs.end(); ++i)
		if(i->first != "doc/xml_compatibility")          // why NOT write that ? Cuz WED 2.0 ... 2.2 read that and if an PRE wed-2.0 document
			                                             // is opened - it uses this instead, resulting in false warnings
			GUI_SetPrefString("doc_prefs", i->first.c_str(), i->second.c_str());
}

void		WED_Document::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	const char * n = NULL, * v = NULL;

	if(strcmp(name,"objects")==0)
	{
		reader->PushHandler(&mArchive);
	}
	if(strcmp(name,"prefs")==0)
	{
		mDocPrefs.clear();
		mDocPrefsItems.clear();
		mDocPrefsActName = "";
	}
	else if(strcmp(name,"pref")==0)
	{
		while(*atts)
		{
			if(strcmp(*atts, "name")==0)
			{
				++atts;
				n = *atts;
				++atts;
			}
			else if(strcmp(*atts,"value")==0)
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
	else if(strcmp(name,"item")==0)
	{
        while(*atts)
		{
			if(strcmp(*atts,"value")==0)
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

bool		WED_Document::ReadPrefInternal(const char * in_key, unsigned type, string &out_value) const
{
	string key(in_key);
	map<string, string>::const_iterator i;
	bool found = false;
	if (type & pref_type_doc)
	{
		i = mDocPrefs.find(key);
		found = (i != mDocPrefs.end());
	}
	if (!found && (type & pref_type_global))
	{
		i = sGlobalPrefs.find(key);
		found = (i != sGlobalPrefs.end());
	}
	if (found)
		out_value = i->second;
	return found;
}

void		WED_Document::WriteXML(FILE * xml_file)
{
	//print to file the xml file passed in with the following encoding
	fprintf(xml_file,"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	fprintf(xml_file,"<!-- written by WED " WED_VERSION_STRING " -->\n");
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


