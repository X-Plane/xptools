#include "WED_Document.h"
#include "WED_Progress.h"
#include "GUI_Resources.h"
#include "XESIO.h"
#include "AptIO.h"
#include "MapAlgs.h"
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
	mProperties(mDB.get()),
	mFilePath(path),
	mPackage(inPackage)
{
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
	
	mArchive.LoadFromDB(mDB.get());
}

WED_Document::~WED_Document()
{
	mArchive.SaveToDB(mDB.get());
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
