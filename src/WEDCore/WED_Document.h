#ifndef WED_DOCUMENT_H
#define WED_DOCUMENT_H

#include "WED_Globals.h"
#include "MeshDefs.h"
#include "AptDefs.h"
#include "MapDefs.h"
#include "DEMDefs.h"
#include "WED_Properties.h"
#include "WED_Archive.h"

class	WED_Package;

typedef struct sqlite3 sqlite3;

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
	
	void				GetBounds(double bounds[4]);

	WED_Archive *		GetArchive(void);

	// LEGACY STUFF
	
	Pmwx				gMap;
	DEMGeoMap			gDem;
	CDT					gTriangulationHi;
	AptVector			gApts;
	AptIndex			gAptIndex;

private:

	WED_Package *		mPackage;
	double				mBounds[4];

	string				mFilePath;
	
	sql_db				mDB;
	WED_Archive			mArchive;

	WED_Properties	mProperties;

	WED_Document();
	WED_Document(const WED_Document&);
	WED_Document& operator=(const WED_Document&);

};

#endif
