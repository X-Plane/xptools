#ifndef WED_DOCUMENT_H
#define WED_DOCUMENT_H

#include "WED_Globals.h"
#include "GUI_Destroyable.h"
#include "MeshDefs.h"
#include "AptDefs.h"
#include "ILibrarian.h"
#include "IDocPrefs.h"
#include "MapDefs.h"
#include "DEMDefs.h"
//#include "WED_Properties.h"
#include "SQLUtils.h"
#include "WED_Archive.h"
#include "WED_UndoMgr.h"

class	WED_Thing;
class	WED_TexMgr;

typedef struct sqlite3 sqlite3;

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


class	WED_Document : public GUI_Broadcaster, public GUI_Destroyable, public virtual IResolver, public virtual ILibrarian, public IDocPrefs {
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

//	virtual void *		QueryInterface(const char * class_id);
	virtual	IBase *		Resolver_Find(const char * path);
	virtual void		LookupPath(string& io_path);		// Input: a relative or library path
	virtual void		ReducePath(string& io_path);		// Output: actual disk location
	virtual	int			ReadIntPref(const char * in_key, int in_default);
	virtual	void		WriteIntPref(const char * in_key, int in_value);
	virtual	double		ReadDoublePref(const char * in_key, double in_default);
	virtual	void		WriteDoublePref(const char * in_key, double in_value);
	

	bool				TryClose(void);
	void				Save(void);
	void				Revert(void);
	bool				IsDirty(void);

	// LEGACY STUFF
	
	Pmwx				gMap;
	DEMGeoMap			gDem;
	CDT					gTriangulationHi;
	AptVector			gApts;
	AptIndex			gAptIndex;

	static	bool	TryCloseAll(void);

private:

	double				mBounds[4];

	string				mFilePath;
	string				mPackage;
	
	sql_db				mDB;
	WED_Archive			mArchive;
	WED_UndoMgr			mUndo;

	WED_TexMgr *		mTexMgr;

//	WED_Properties	mProperties;

	WED_Document();
	WED_Document(const WED_Document&);
	WED_Document& operator=(const WED_Document&);

	map<string,string>	mDocPrefs;

};

#endif
