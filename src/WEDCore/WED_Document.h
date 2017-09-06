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

#ifndef WED_DOCUMENT_H
#define WED_DOCUMENT_H

#include "WED_XMLReader.h"
//#include "WED_Globals.h"
#include "GUI_Destroyable.h"
//#include "MeshDefs.h"
#include "AptDefs.h"
#include "ILibrarian.h"
#include "IDocPrefs.h"
//#include "MapDefs.h"
//#include "DEMDefs.h"
#include "PlatformUtils.h"
#include "WED_Archive.h"
#include "WED_UndoMgr.h"


class	WED_Thing;
class	WED_TexMgr;
class	WED_LibraryMgr;
class	WED_ResourceMgr;
#if WITHNWLINK
class	WED_Server;
class	WED_NWLinkAdapter;
#endif

#include "GUI_Broadcaster.h"
#include "IResolver.h"

/*
	WED_Document - THEORY OF OPERATION

	UI-DATAMODEL

	Because the data model is made of persistent objects (whose pointer memory addresses may vary), we can't just pass pointers to the UI.  Instead the UI
	finds abstract interfaces into the data model using the IResolver interface.

	In the case of the document, each component uses the IDirectory, and each array index uses the IArray interface.  In turn, WED_Thing actually implements
	both of these, so we can index into the hierarchial datamodel by index or "thing name".

	Object with ID 1 is by definition "the document root" - that is, it is used as a starting point for all resolutions.

*/


class	WED_Document : public GUI_Broadcaster, public GUI_Destroyable, public virtual IResolver, public virtual ILibrarian, public IDocPrefs, public WED_XMLHandler, public WED_UndoFatalErrorHandler {
public:

						WED_Document(
								const string& 		package,
								double				inBounds[4]);
						~WED_Document();

	static		void	ReadGlobalPrefs(void);
	static		void	WriteGlobalPrefs(void);

	// Management
	string				GetFilePath(void) const;

	void				GetBounds(double bounds[4]);

	WED_Archive *		GetArchive(void);
	WED_Thing *			GetRoot(void);
	WED_UndoMgr *		GetUndoMgr(void);
#if WITHNWLINK
	WED_Server *		GetServer(void);
	WED_NWLinkAdapter *	GetNWLink(void);
#endif
//	virtual void *		QueryInterface(const char * class_id);
	virtual	IBase *		Resolver_Find(const char * path);
	virtual void		LookupPath(string& io_path);		// Input: a relative or library path
	virtual void		ReducePath(string& io_path);		// Output: actual disk location
	virtual	int			ReadIntPref(const char * in_key, int in_default);
	virtual	void		WriteIntPref(const char * in_key, int in_value);
	virtual	double		ReadDoublePref(const char * in_key, double in_default);
	virtual	void		WriteDoublePref(const char * in_key, double in_value);
	virtual	string		ReadStringPref(const char * in_key, const string& in_default);
	virtual	void		WriteStringPref(const char * in_key, const string& in_value);
	virtual	void		ReadIntSetPref(const char * in_key, set<int>& out_value);
	virtual	void		WriteIntSetPref(const char * in_key, const set<int>& in_value);

	WED_LibraryMgr *	GetLibrary(void) { return mLibraryMgr; }
	WED_ResourceMgr *	GetResourceMgr(void) { return mResourceMgr; }

	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);

	virtual	void		Panic(void);

	bool				TryClose(void);

	//Saves the file, returns true if successful, false if not.
	void				Save(void);
	void				Revert(void);
	bool				IsDirty(void);
	bool				IsOnDisk(void);

	// LEGACY STUFF

//	Pmwx				gMap;
//	DEMGeoMap			gDem;
//	CDT					gTriangulationHi;
//	AptVector			gApts;
//	AptIndex			gAptIndex;

	static	bool	TryCloseAll(void);

private:
	void				WriteXML(FILE * fi);

	//Member Variables

	double				mBounds[4];

	string				mFilePath;
	string				mPackage;
	bool				mOnDisk;

	//sql_db				mDB;
	WED_Archive			mArchive;
	WED_UndoMgr			mUndo;

	WED_TexMgr *		mTexMgr;
	WED_LibraryMgr *	mLibraryMgr;
	WED_ResourceMgr *	mResourceMgr;
	//WED_Properties	mProperties;
#if WITHNWLINK
	WED_Server *		mServer;
	WED_NWLinkAdapter *	mNWLink;
#endif
	WED_Document();
	WED_Document(const WED_Document&);
	WED_Document& operator=(const WED_Document&);

	string						mDocPrefsActName;		// Temporary for tracking the current int-set on read-i.
	map<string,string>			mDocPrefs;				// All string, int and double (non-set) prefs
	map<string,set<int> >		mDocPrefsItems;			// The int-set prefs, separated out.

};

#endif
