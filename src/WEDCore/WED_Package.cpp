#include "WED_Package.h"
#include "PlatformUtils.h"
#include "MemFileUtils.h"
#include "XFileTwiddle.h"
#include "WED_Document.h"
#include "GISUtils.h"
#include "WED_Errors.h"

#define		EDIT_DIR_NAME		"WED" DIR_STR
#define		EARTH_DIR_NAME		"Earth nav data" DIR_STR

inline int lon_lat_to_idx  (int lon, int lat) { return lon + 180 + 360 * (lat + 90); }
inline int lon_lat_to_idx10(int lon, int lat) { return lon + 18  +  36 * (lat +  9); }

static const char * ext = NULL;

static bool	scan_folder_exists(const char * name, bool dir, unsigned long long mod, void * ref)
{
	char * dirs = (char *) ref;
	if (dir)
	if (strlen(name) == 7 && 
		(name[0] == '+' || name[0] == '-') &&
		(name[3] == '+' || name[3] == '-') &&
		isdigit(name[1]) && 
		isdigit(name[2]) &&
		isdigit(name[4]) && 
		isdigit(name[5]) &&
		isdigit(name[6]))
	{
		int lat = (name[1]-'0')*10+(name[2]-'0');
		if (name[0]=='-') lat = -lat;
		int lon = (name[4]-'0')*100+(name[5]-'0')*10+(name[6]-'0');
		if (name[3]=='-') lon = -lon;
		if (lat >= -90 && lat < 90 && lon >= -180 && lon < 180)
			dirs[lon_lat_to_idx10(lon, lat)] = true;
		
	}	
	return true;
}

static bool	scan_file_exists(const char * name, bool dir, unsigned long long mod, void * ref)
{
	unsigned long long * dirs = (unsigned long long *) ref;
	if (!dir)
	if (strlen(name) == 11 && strcmp(name+7, ext) == 0 &&
		(name[0] == '+' || name[0] == '-') &&
		(name[3] == '+' || name[3] == '-') &&
		isdigit(name[1]) && 
		isdigit(name[2]) &&
		isdigit(name[4]) && 
		isdigit(name[5]) &&
		isdigit(name[6]))
	{
		int lat = (name[1]-'0')*10+(name[2]-'0');
		if (name[0]=='-') lat = -lat;
		int lon = (name[4]-'0')*100+(name[5]-'0')*10+(name[6]-'0');
		if (name[3]=='-') lon = -lon;
		if (lat >= -90 && lat < 90 && lon >= -180 && lon < 180)
			dirs[lon_lat_to_idx10(lon, lat)] = mod;
		
	}	
	return true;
}

WED_Package::WED_Package(const char * inPath, bool inCreate)
{
	mPackageBase = inPath;
	memset(mTiles, 0, sizeof(mTiles));

	if (inCreate)
	{
		string wed_folder = mPackageBase + EDIT_DIR_NAME + EARTH_DIR_NAME;
		string earth_folder = mPackageBase + EARTH_DIR_NAME;
		if (!MakeDirExist(mPackageBase.c_str()))
			WED_ThrowPrintf("Unable to create directory %s", mPackageBase.c_str());
		if (!MakeDirExist(wed_folder.c_str()))
			WED_ThrowPrintf("Unable to create directory %s", wed_folder.c_str());
		if (!MakeDirExist(earth_folder.c_str()))
			WED_ThrowPrintf("Unable to create directory %s", earth_folder.c_str());
	}
	
	Rescan();		
}

WED_Package::~WED_Package()
{
	for (int n = 0; n < 360 * 180; ++n)
	if (mTiles[n])
		delete mTiles[n];
}

int				WED_Package::GetTileStatus(int lon, int lat)
{
	return mStatus[lon_lat_to_idx(lon, lat)];
}

WED_Document *	WED_Package::GetTileDocument(int lon, int lat)
{
	return mTiles[lon_lat_to_idx(lon, lat)];
}

WED_Document *	WED_Package::OpenTile(int lon, int lat)
{
	double bounds[4] = { lon, lat, lon + 1, lat + 1 };
	char partial[30];
	sprintf(partial, "%+03d%+04d" DIR_STR "%+03d%+04d.xes", latlon_bucket(lat),latlon_bucket(lon),lat,lon);
	string path = mPackageBase + EDIT_DIR_NAME + EARTH_DIR_NAME + partial;

	WED_Document * tile = new WED_Document(path, this, bounds);
	mTiles[lon_lat_to_idx(lon, lat)] = tile;
	tile->Load();
	return tile;
}

WED_Document *	WED_Package::NewTile(int lon, int lat)
{
	double bounds[4] = { lon, lat, lon + 1, lat + 1 };
	char partial[30];
	sprintf(partial, "%+03d%+04d" DIR_STR "%+03d%+04d.xes", latlon_bucket(lat),latlon_bucket(lon),lat,lon);
	string path = mPackageBase + EDIT_DIR_NAME + EARTH_DIR_NAME + partial;

	WED_Document * tile = new WED_Document(path, this, bounds);
	mTiles[lon_lat_to_idx(lon, lat)] = tile;
	return tile;
}


	
void			WED_Package::Rescan(void)
{
	unsigned long long 		dsf[360*180] = { 0 };
	unsigned long long 		xes[360*180] = { 0 };
	
	char					dsf_dir[36*18] = { 0 };
	char					xes_dir[36*18] = { 0 };
	
	string	dsf_dir_str	= mPackageBase + EARTH_DIR_NAME;
	string	xes_dir_str	= mPackageBase + EDIT_DIR_NAME + EARTH_DIR_NAME;
	
	MF_GetDirectoryBulk(dsf_dir_str.c_str(), scan_folder_exists, dsf_dir);
	MF_GetDirectoryBulk(xes_dir_str.c_str(), scan_folder_exists, xes_dir);
	
	for (int lat = -9; lat < 9; lat++)
	for (int lon = -18; lon < 18; lon++)
	{
		char dname[25];
		sprintf(dname,"%+03d%+04d" DIR_STR, lat*10, lon*10);
		string dsf_subdir_str = dsf_dir_str + dname;
		string xes_subdir_str = xes_dir_str + dname;
		
		if (dsf_dir[lon_lat_to_idx10(lon,lat)])	MF_GetDirectoryBulk(dsf_dir_str.c_str(), scan_file_exists, dsf);
		if (xes_dir[lon_lat_to_idx10(lon,lat)])	MF_GetDirectoryBulk(xes_dir_str.c_str(), scan_file_exists, xes);
	}
	
	for (int lat = -90 ; lat <  90; ++lat)
	for (int lon = -180; lon < 180; ++lon)
	{
		int idx = lon_lat_to_idx(lon, lat);
		
		if (dsf[idx] != 0)
		{
			if (xes[idx] != 0)
			{
				mStatus[idx] = (dsf[idx] > xes[idx]) ? status_UpToDate : status_Stale;
			} else {
				mStatus[idx] = status_DSF;
			}
		} else {
			if (xes[idx] != 0)
			{
				mStatus[idx] = status_XES;
			} else {
				mStatus[idx] = status_None;
			}
		}
	}
}

