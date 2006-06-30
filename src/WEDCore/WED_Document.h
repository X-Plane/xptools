#ifndef WED_DOCUMENT_H
#define WED_DOCUMENT_H

#include "WED_Archive.h"
#include "WED_UndoMgr.h"
#include "WED_Globals.h"
#include "MeshDefs.h"
#include "AptDefs.h"
#include "MapDefs.h"
#include "DEMDefs.h"

class	WED_ObjectRoot;
class	WED_Package;

#include "GUI_Broadcaster.h"

class	WED_Document : public GUI_Broadcaster{
public:

						WED_Document(
								const string& 		path, 
								WED_Package * 		inPackage,
								double				inBounds[4]);
						~WED_Document();

	// Management
	string				GetFilePath(void) const;
	bool				GetDirty(void) const;
	
	void				Save(void);
	void				Load(void);
	void				GetBounds(double bounds[4]);

	// OBJECT PLACEMENT
	
	WED_ObjectRoot *	GetObjectRoot(void);
	
	// LEGACY STUFF
	
	Pmwx				gMap;
	DEMGeoMap			gDem;
	CDT					gTriangulationHi;
	AptVector			gApts;
	AptIndex			gAptIndex;

private:

	WED_Archive		mArchive;
	WED_UndoMgr		mUndo;

	WED_Package *		mPackage;
	double				mBounds[4];

	string				mFilePath;
	bool				mDirty;

	WED_ObjectRoot *	mObjectRoot;

	WED_Document();
	WED_Document(const WED_Document&);
	WED_Document& operator=(const WED_Document&);

};

#endif
