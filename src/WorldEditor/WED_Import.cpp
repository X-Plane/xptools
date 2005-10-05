#include "WED_Import.h"
#include "XPWidgetDialogs.h"
#include "XPWidgets.h"
#include "WED_Globals.h"
#include "MemFileUtils.h"
#include "DEMIO.h"
#include "WED_Notify.h"
#include "EnumSystem.h"
#include "WED_Msgs.h"
#include "WED_DEMGraphics.h"
#include "PlatformUtils.h"
#include "SimpleIO.h"
#include "MapAlgs.h"


//
// TODO: rescale, remapping, geotiff, ida+usgs 
//

static XPWidgetID		sImport = NULL;

static	void	LoadCLUT(XPWidgetID);

vector<int>			sImportForwardMap;
hash_map<int,int>	sImportReverseMap;
vector<char>		sImportCLUT;


static	void	DoImport(XPWidgetID, int);
static	void	ResyncImportDialog(void);
static	void	ImportNotifier(int catagory, int message, void * param);
static	void	InvertRescaling(XPWidgetID);
static	void	ImportEnable(XPWidgetID);
static	void	DimsFromMap(XPWidgetID);

struct	WED_ImportState_t {
	int			dem;
	int			format;
	int			rescale;
	int			byte_off;
	int			byte_width;
	int			byte_height;
	float		offset;
	float		scale;
	int			flip_x;
	int			flip_y;
	int			invert;
	float		west;
	float		south;
	float		east;
	float		north;
};

static WED_ImportState_t	sImportState = { 0, 0, 0, 0, 121, 121, 0.0, 1.0, 0, 0, 0,
				-180.0, -90.0, 180.0, 90.0 };

enum {
	import_PNG=0,
	import_BMP,
	import_TIFF,
	import_Div,
	import_USGSNatural,
	import_IDA,
	import_GeoTIFF,
	import_DTED,
	import_Div2,
	import_Raw8,
	import_Raw16LE,
	import_Raw32LE,
	import_RawFloatLE,
	import_Raw16BE,
	import_Raw32BE,
	import_RawFloatBE
};
static const char * kTitles = 
	"PNG;BMP;TIFF;"
	"-;USGS Natural DEM;IDA;GeoTiff;DTED;"
	"-;8-Bit Raw;16-Bit Raw (IBM);32-Bit Raw (IBM);Floating Point Raw(IBM);16-Bit Raw (Mac);32-Bit Raw (Mac);Floating Point Raw(Mac)";

static	int	kImportLayers[] = {
	dem_Elevation,
	dem_Temperature,
	dem_TemperatureSeaLevel,
	dem_TemperatureRange,
	dem_Rainfall,
	dem_Biomass,				
	dem_Slope,				
	dem_SlopeHeading,
	dem_UrbanDensity,			
	dem_UrbanPropertyValue,
	dem_LandUse,
	dem_Climate				
//	dem_NudeColor,
//	dem_VegetationDensity
};
static	const char * kImportLayerTitles = 
	"dem_Elevation;"
	"dem_Temperature;"
	"dem_TemperatureSeaLevel;"
	"dem_TemperatureRange;"
	"dem_Rainfall;"
	"dem_Biomass;"				
	"dem_Slope;"				
	"dem_SlopeHeading;"
	"dem_UrbanDensity;"			
	"dem_UrbanPropertyValue;"
	"dem_LandUse;"
	"dem_Climate;"				
//	"dem_NudeColor;"
	"dem_VegetationDensity";

const int WED_IMPORT_FORMAT = 1000;
const int WED_IMPORT_RESCALE = 1001;
const int WED_IMPORT_SCALE = 1002;
const int WED_IMPORT_OFFSET = 1003;
const int WED_IMPORT_BYTE_OFFSET = 1004;
const int WED_IMPORT_BYTE_WIDTH = 1005;
const int WED_IMPORT_BYTE_HEIGHT = 1006;
const int WED_IMPORT_TRANSLATION = 1007;

const int WED_IMPORT_NORTH = 1008;
const int WED_IMPORT_SOUTH = 1009;
const int WED_IMPORT_EAST = 1010;
const int WED_IMPORT_WEST = 1011;
const int WED_IMPORT_MAP = 1012;

void	WED_ShowImportDialog(void)
{
	if (sImport != NULL)
	{
		ResyncImportDialog();
		if (!XPIsWidgetVisible(sImport))
			XPShowWidget(sImport);
		XPBringRootWidgetToFront(sImport);
	} else {
		WED_RegisterNotifyFunc(ImportNotifier);
		sImport = XPCreateWidgetLayout(
			0, XP_DIALOG_BOX, "IMPORT", XP_DIALOG_CLOSEBOX, 1, 0, DoImport,
				XP_COLUMN,
					XP_ROW, 
						XP_CAPTION, "Raster Layer:", 
						XP_POPUP_MENU, kImportLayerTitles, &sImportState.dem, 
					XP_END,
					XP_ROW, 
						XP_CAPTION, "Format:", 
						XP_POPUP_MENU, kTitles, &sImportState.format, XP_TAG, WED_IMPORT_FORMAT, XP_NOTIFY, ImportEnable,
					XP_END,
					XP_ROW, 
						XP_BUTTON_ACTION, "Load Translation", LoadCLUT, 
						XP_CAPTION, "(no translation)", XP_TAG, WED_IMPORT_TRANSLATION,
					XP_END,					
					XP_ROW,	
						XP_CHECKBOX, "Rescale", &sImportState.rescale, XP_NOTIFY, ImportEnable, XP_TAG, WED_IMPORT_RESCALE,
						XP_CAPTION, "Offset:", XP_EDIT_FLOAT, 15, 6, 2, &sImportState.offset, XP_TAG, WED_IMPORT_OFFSET,
						XP_CAPTION, "Scale:", XP_EDIT_FLOAT, 15, 6, 2, &sImportState.scale, XP_TAG, WED_IMPORT_SCALE,
					XP_END,
					XP_ROW, 
						XP_CHECKBOX, "Flip X", &sImportState.flip_x, 
						XP_CHECKBOX, "Flip Y", &sImportState.flip_y,
					XP_END,
					XP_ROW,
						XP_CAPTION, "Byte Offset:", 
						XP_EDIT_INT, 15, 6, &sImportState.byte_off, XP_TAG, WED_IMPORT_BYTE_OFFSET,
						XP_CAPTION, "Byte Width:", 
						XP_EDIT_INT, 15, 6, &sImportState.byte_width, XP_TAG, WED_IMPORT_BYTE_WIDTH,
						XP_CAPTION, "Byte Height:", 
						XP_EDIT_INT, 15, 6, &sImportState.byte_height, XP_TAG, WED_IMPORT_BYTE_HEIGHT,
					XP_END,
					XP_ROW,
						XP_CAPTION, "West:", XP_EDIT_FLOAT, 15, 6, 1, &sImportState.west, XP_TAG, WED_IMPORT_WEST,
						XP_CAPTION, "South:", XP_EDIT_FLOAT, 15, 6, 1, &sImportState.south, XP_TAG, WED_IMPORT_SOUTH,
					XP_END,
					XP_ROW,						
						XP_CAPTION, "East:", XP_EDIT_FLOAT, 15, 6, 1, &sImportState.east, XP_TAG, WED_IMPORT_EAST,
						XP_CAPTION, "North:", XP_EDIT_FLOAT, 15, 6, 1, &sImportState.north, XP_TAG, WED_IMPORT_NORTH,
						XP_BUTTON_ACTION, "Map", DimsFromMap, XP_TAG, WED_IMPORT_MAP,
					XP_END,
					XP_ROW, XP_BUTTON_OK, "Import", XP_END,
				XP_END,
			XP_END);
		ResyncImportDialog();
		XPShowWidget(sImport);
	}
}

void	DoImport(XPWidgetID inWidget, int inResult)
{
	if (inResult != xpDialog_ResultOK) return;

	int target_layer = kImportLayers[sImportState.dem];

	char	fileBuf[2048];
	fileBuf[0] = 0;
	if (GetFilePathFromUser(getFile_Open, "Please name your DEM", "Import", 5, fileBuf))
	{
		DEMGeo *	theDem;
		if (sImportState.format == import_PNG || sImportState.format == import_BMP || sImportState.format == import_TIFF)
		{
			int err = 1;
			ImageInfo	info;
			if (sImportState.format == import_BMP) err = CreateBitmapFromFile(fileBuf, &info);
			if (sImportState.format == import_PNG) err = CreateBitmapFromPNG(fileBuf, &info, true);
			if (sImportState.format == import_TIFF) err = CreateBitmapFromTIF(fileBuf, &info);
			if (err != 0)
			{
				DoUserAlert("There was a problem importing the image file.");
				return;
			}
			theDem = &gDem[target_layer];
			theDem->resize(info.width, info.height);
			
			for (int x = 0; x < info.width; ++x)
			for (int y = 0; y < info.height; ++y)
			{
				int ix = x;
				int iy = y;
				if (sImportState.flip_x) ix = info.width - x - 1;
				if (sImportState.flip_y) iy = info.width - y - 1;
				unsigned char * p = info.data + y * (info.width * info.channels + info.pad) + x * info.channels;
				if (info.channels == 4 && p[3] == 0)
					(*theDem)(ix, iy) = NO_DATA;
				else
					(*theDem)(ix, iy) = p[0];					
			}
		}
		
		if (sImportState.format == import_USGSNatural)
		{
			theDem = &gDem[target_layer];		
			if (!ExtractUSGSNaturalFile(*theDem, fileBuf))
			{
				DoUserAlert("Unable to read USGS Natural file.");
				return;
			}
		}
		
		if (sImportState.format == import_IDA)
		{
			theDem = &gDem[target_layer];		
			if (!ExtractIDAFile(*theDem, fileBuf))
			{
				DoUserAlert("Unable to read IDA file.");
				return;
			}
		}

		if (sImportState.format == import_GeoTIFF)
		{
			theDem = &gDem[target_layer];		
			if (!ExtractGeoTiff(*theDem, fileBuf))
			{
				DoUserAlert("Unable to read GeoTIFF file.");
				return;
			}
		}
		
		if (sImportState.format == import_DTED)
		{
			theDem = &gDem[target_layer];		
			if (!ExtractDTED(*theDem, fileBuf))
			{
				DoUserAlert("Unable to read DTED file.");
				return;
			}
		}
		
		if (sImportState.format == import_Raw8 ||
			sImportState.format == import_Raw16LE || sImportState.format == import_Raw16BE ||
			sImportState.format == import_Raw32LE || sImportState.format == import_Raw32BE ||
			sImportState.format == import_RawFloatLE || sImportState.format == import_RawFloatBE)
		{
			theDem = &gDem[target_layer];
			theDem->resize(sImportState.byte_width,sImportState.byte_height);

			MFMemFile *	file = MemFile_Open(fileBuf);
			if (file == NULL)
			{
				DoUserAlert("Unable to open file.");
				return;
			}
	
			MemFileReader	reader(MemFile_GetBegin(file), MemFile_GetEnd(file),
						(sImportState.format == import_Raw16LE || 
						sImportState.format == import_Raw32LE || 
						sImportState.format == import_RawFloatLE) ? platform_LittleEndian : platform_BigEndian);
			{
				for (int n = 0; n < sImportState.byte_off; ++n)
				{
					char k;
					reader.ReadBulk(&k, 1, false);
				}
				
				for (int y = 0; y < theDem->mHeight; ++y)
				for (int x = 0; x < theDem->mWidth; ++x)
				{
					int ix = x, iy = y;
					if (sImportState.flip_x) ix = theDem->mWidth - 1 - ix;
					if (sImportState.flip_y) iy = theDem->mHeight - 1 - iy;

					unsigned char us;
					short		sh;
					int			in;
					float		e;
					
					switch(sImportState.format) {
					case import_Raw8:
						reader.ReadBulk((char *) &us, 1, false);
						e = us;
						break;
					case import_Raw16LE:
					case import_Raw16BE:
						reader.ReadShort(sh);
						e = sh;
						break;
					case import_Raw32LE:
					case import_Raw32BE:
						reader.ReadInt(in);
						e = in;
						break;
					case import_RawFloatLE:
					case import_RawFloatBE:
						reader.ReadFloat(e);
						break;
					}
					(*theDem)(ix,iy) = e;
				}
			}
			
			MemFile_Close(file);
		}
		
		if (sImportState.rescale)
		{
			float new_offset = 0.0;
			float new_scale = 1.0;
			if (sImportState.scale != 0.0)
			{
				new_offset = -sImportState.offset / sImportState.scale;
				new_scale = 1.0 / sImportState.scale;
			}
		
			for (int x = 0; x < theDem->mWidth; ++x)
			for (int y = 0; y < theDem->mHeight; ++y)
			{
				float e = theDem->get(x,y);
				if (e != NO_DATA)
				{
					e = new_offset + e * new_scale;
					(*theDem)(x,y) = e;
				}
			}
		}
		if (!sImportForwardMap.empty())
			TranslateDEMForward(*theDem, sImportForwardMap);	
		
		if (sImportState.format != import_DTED && sImportState.format != import_USGSNatural && sImportState.format != import_GeoTIFF)
		{
			theDem->mWest = sImportState.west;
			theDem->mEast = sImportState.east;
			theDem->mSouth = sImportState.south;
			theDem->mNorth = sImportState.north;
		}
		WED_Notifiable::Notify(wed_Cat_File, wed_Msg_RasterChange,(void*) target_layer);
	}
}

void	ResyncImportDialog(void)
{
	ImportEnable(NULL);
}

void	ImportNotifier(int catagory, int message, void * param)
{
}

void ImportEnable(XPWidgetID)
{
	XPDataFromItem(sImport, WED_IMPORT_RESCALE);
	XPDataFromItem(sImport, WED_IMPORT_FORMAT);
	bool is_raw = sImportState.format >= import_Raw8;
	bool has_geo = sImportState.format == import_DTED || sImportState.format == import_USGSNatural || sImportState.format == import_GeoTIFF;
	XPEnableByTag(sImport, WED_IMPORT_SCALE, sImportState.rescale);
	XPEnableByTag(sImport, WED_IMPORT_OFFSET, sImportState.rescale);
	XPEnableByTag(sImport, WED_IMPORT_BYTE_OFFSET, is_raw);
	XPEnableByTag(sImport, WED_IMPORT_BYTE_WIDTH, is_raw);
	XPEnableByTag(sImport, WED_IMPORT_BYTE_HEIGHT, is_raw);

	XPEnableByTag(sImport, WED_IMPORT_NORTH, !has_geo);
	XPEnableByTag(sImport, WED_IMPORT_SOUTH, !has_geo);
	XPEnableByTag(sImport, WED_IMPORT_EAST , !has_geo);
	XPEnableByTag(sImport, WED_IMPORT_WEST , !has_geo);
	XPEnableByTag(sImport, WED_IMPORT_MAP  , !has_geo);
}

static	void	LoadCLUT(XPWidgetID inID)
{
	XPWidgetID caption = XPFindWidgetByTag(sImport, WED_IMPORT_TRANSLATION);
	if (sImportForwardMap.empty())
	{
		char fileBuf[2048];
		fileBuf[0] = 0;
		if (GetFilePathFromUser(getFile_Open, "Please pick a mapping file", "Open", 3, fileBuf))
		{			
		
			if (LoadTranslationFile(fileBuf, sImportForwardMap, &sImportReverseMap, &sImportCLUT))
			{
				char * s = fileBuf + strlen(fileBuf) - 1;
				while (s > fileBuf && *s != ':' && *s != '\\' && *s != '/')
					--s;
				XPSetWidgetDescriptor(caption, s);
				XPSetWidgetDescriptor(inID, "Clear Translation");
			} else {
				sImportForwardMap.clear();
				sImportReverseMap.clear();
				sImportCLUT.clear();
				XPSetWidgetDescriptor(caption, "(no translation)");
			}			
		}
	} else {
		sImportForwardMap.clear();
		sImportReverseMap.clear();
		sImportCLUT.clear();
		XPSetWidgetDescriptor(caption, "(no translation)");		
		XPSetWidgetDescriptor(inID, "Load Translation");
	}
}

void	DimsFromMap(XPWidgetID)
{
	if (gMap.empty() && gDem.empty())
	{
		sImportState.west = -180.0;
		sImportState.east =  180.0;
		sImportState.south = -90.0;
		sImportState.north =  90.0;
	} else {
			
		Point2	sw(180.0, 90.0), ne(-180.0,-90.0);
		CalcBoundingBox(gMap, sw, ne);
		sImportState.west = sw.x;
		sImportState.east = ne.x;
		sImportState.south = sw.y;
		sImportState.north = ne.y;
		for (DEMGeoMap::iterator i = gDem.begin(); i != gDem.end(); ++i)
		{
			sImportState.west = min(sImportState.west, (float) i->second.mWest);
			sImportState.south = min(sImportState.south, (float) i->second.mSouth);
			sImportState.east = min(sImportState.east, (float) i->second.mEast);
			sImportState.north = min(sImportState.north, (float) i->second.mNorth);
		}
	}	
	XPDataToItem(sImport, WED_IMPORT_SOUTH);
	XPDataToItem(sImport, WED_IMPORT_NORTH);
	XPDataToItem(sImport, WED_IMPORT_EAST );
	XPDataToItem(sImport, WED_IMPORT_WEST );
}
