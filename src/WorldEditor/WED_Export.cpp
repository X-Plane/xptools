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

#include "WED_Export.h"
#include "XPWidgetDialogs.h"
#include "XPWidgets.h"
#include "WED_Globals.h"
#include "WED_Notify.h"
#include "EnumSystem.h"
#include "WED_Msgs.h"
#include "WED_DEMGraphics.h"
#include "PlatformUtils.h"
#include "SimpleIO.h"
#include "DEMIO.h"
#include "GISTool_Globals.h"

static XPWidgetID		sExport = NULL;

static	void	DoExport(XPWidgetID, int);
static	void	ResyncExportDialog(void);
static	void	ExportNotifier(int catagory, int message, void * param);
static	void	CalculateRescaling(XPWidgetID);
static	void	LoadCLUT(XPWidgetID);
static	void	SetEnables(XPWidgetID);

vector<int>			sExportForwardMap;
hash_map<int,int>	sExportReverseMap;
vector<char>		sExportCLUT;


struct	WED_ExportState_t {
	int			dem;
	int			format;
	int			color;
	int			rescale;
	float		offset;
	float		scale;
	int			flip_x;
	int			flip_y;
	int			invert;
	int			alpha;
};

static WED_ExportState_t	sExportState = { 0, 0, 0, 0, 0.0, 1.0, 0, 0, 0, 0 };

const int WED_EXPORT_FORMAT = 1000;
const int WED_EXPORT_RASTERS = 1001;
const int WED_EXPORT_OFFSET = 1002;
const int WED_EXPORT_SCALE = 1003;
const int WED_EXPORT_TRANSLATION = 1004;
const int WED_EXPORT_INVERT = 1005;
const int WED_EXPORT_ALPHA = 1006;
const int WED_EXPORT_COLOR = 1007;
const int WED_EXPORT_RESCALE = 1008;
const int WED_EXPORT_CALC = 1009;

enum {
	export_PNG=0,
	export_BMP,
	export_Div,
	export_Raw8,
	export_Raw16LE,
	export_Raw32LE,
	export_RawFloatLE,
	export_Raw16BE,
	export_Raw32BE,
	export_RawFloatBE
};
static const char * kExtensions[] = { ".png", ".bmp", "-", ".raw",".raw",".raw",".raw",".raw",".raw" };
static const char * kTitles = "PNG;BMP;-;8-Bit Raw;16-Bit Raw (IBM);32-Bit Raw (IBM);Floating Point Raw(IBM);16-Bit Raw (Mac);32-Bit Raw (Mac);Floating Point Raw(Mac)";


void	WED_ShowExportDialog(void)
{
	if (sExport != NULL)
	{
		ResyncExportDialog();
		if (!XPIsWidgetVisible(sExport))
			XPShowWidget(sExport);
		XPBringRootWidgetToFront(sExport);
	} else {
		WED_RegisterNotifyFunc(ExportNotifier);
		sExport = XPCreateWidgetLayout(
			0, XP_DIALOG_BOX, "Export", XP_DIALOG_CLOSEBOX, 1, 0, DoExport,
				XP_COLUMN,
					XP_ROW,
						XP_CAPTION, "Raster Layer:",
						XP_POPUP_MENU, "DEM A;DEM B;DEM C", &sExportState.dem, XP_TAG, WED_EXPORT_RASTERS,
					XP_END,
					XP_ROW,
						XP_CAPTION, "Format:",
						XP_POPUP_MENU, kTitles, &sExportState.format, XP_TAG, WED_EXPORT_FORMAT, XP_NOTIFY, SetEnables,
					XP_END,
					XP_ROW,
						XP_CHECKBOX, "Colorize", &sExportState.color, XP_TAG, WED_EXPORT_COLOR, XP_NOTIFY, SetEnables,
						XP_CHECKBOX, "Alpha", &sExportState.alpha, XP_TAG, WED_EXPORT_ALPHA,
					XP_END,
					XP_ROW,
						XP_BUTTON_ACTION, "Load Translation", LoadCLUT,
						XP_CAPTION, "(no translation)", XP_TAG, WED_EXPORT_TRANSLATION,
					XP_END,
					XP_ROW,
						XP_CHECKBOX, "Rescale", &sExportState.rescale, XP_TAG, WED_EXPORT_RESCALE, XP_NOTIFY, SetEnables,
						XP_CAPTION, "Offset:", XP_EDIT_FLOAT, 15, 6, 2, &sExportState.offset, XP_TAG, WED_EXPORT_OFFSET,
						XP_CAPTION, "Scale:", XP_EDIT_FLOAT, 15, 6, 2, &sExportState.scale, XP_TAG, WED_EXPORT_SCALE,
						XP_BUTTON_ACTION, "Calculate", CalculateRescaling, XP_TAG, WED_EXPORT_CALC,
					XP_END,
					XP_ROW,
						XP_CHECKBOX, "Flip X", &sExportState.flip_x,
						XP_CHECKBOX, "Flip Y", &sExportState.flip_y,
					XP_END,
					XP_ROW,
						XP_CHECKBOX, "Invert", &sExportState.invert, XP_TAG, WED_EXPORT_INVERT,
					XP_END,
					XP_ROW, XP_BUTTON_OK, "Export", XP_END,
				XP_END,
			XP_END);
		ResyncExportDialog();
		XPShowWidget(sExport);
	}
}

void	DoExport(XPWidgetID inWidget, int inResult)
{
	if (inResult != xpDialog_ResultOK) return;
	DEMGeoMap::iterator layer = gDem.begin();
	int n = sExportState.dem;
	while (n > 0 && layer != gDem.end())
		++layer, --n;
	if (layer == gDem.end()) return;

	bool	enum_layer = layer->first == dem_LandUse || layer->first == dem_Climate;	// || layer->first == dem_NudeColor;

	char	fileBuf[2048];
	strcpy(fileBuf, FetchTokenString(layer->first));
	strcat(fileBuf, kExtensions[sExportState.format]);
	if (GetFilePathFromUser(getFile_Save, "Please name your DEM", "Export", 2, fileBuf, sizeof(fileBuf)))
	{
		if (sExportState.format == export_PNG || sExportState.format == export_BMP)
		{
			ImageInfo	info;
			int channels = 1;
			if (sExportState.format == export_BMP)													channels = 3;
			if (sExportState.color)																	channels = 3;
			if (sExportState.format == export_PNG && sExportState.color && sExportState.alpha) 		channels = 4;
			if (CreateNewBitmap(layer->second.mWidth, layer->second.mHeight, channels, &info) == 0)
			{
				for (int x = 0; x < layer->second.mWidth; ++x)
				for (int y = 0; y < layer->second.mHeight; ++y)
				{
					int ix = x, iy = y;
					if (sExportState.flip_x) ix = layer->second.mWidth - 1 - ix;
					if (sExportState.flip_y) iy = layer->second.mHeight - 1 - iy;
					float e = layer->second.get(ix,iy);

					if (!sExportReverseMap.empty())
						e = sExportReverseMap[e];

					unsigned char * p = info.data + y * (info.channels * info.width + info.pad) + x * info.channels;
					if (sExportState.color)
					{
						ColorForValue(layer->first, e, p);
						swap(p[0], p[2]);
						p[3] = 255;
						if (channels == 4)
						{
							if (enum_layer)
							if (e == NO_VALUE) 	p[3] = 0;
							if (e == DEM_NO_DATA) 	p[3] = 0;
						}
					} else {
						if (sExportState.rescale) {
							e *= sExportState.scale;
							e += sExportState.offset;
						}
						for (int n = 0; n < info.channels; ++n)
							*p++ = e;
					}
					p = info.data + y * (info.channels * info.width + info.pad) + x * info.channels;
					if (sExportState.invert)
						for (int n = 0; n < info.channels; ++n, ++p)
							*p = 255 - *p;
				}
				if (sExportState.format==export_PNG)	WriteBitmapToPNG (&info, fileBuf, sExportCLUT.empty() ? NULL : &*sExportCLUT.begin(), sExportCLUT.size() / 3);
				if (sExportState.format==export_BMP)	WriteBitmapToFile(&info, fileBuf);
				DestroyBitmap(&info);
			}
		}
		if (sExportState.format == export_Raw8 ||
			sExportState.format == export_Raw16LE || sExportState.format == export_Raw16BE ||
			sExportState.format == export_Raw32LE || sExportState.format == export_Raw32BE ||
			sExportState.format == export_RawFloatLE || sExportState.format == export_RawFloatBE)
		{
			bool	range_err = false;
			FileWriter	writer(fileBuf, (
						sExportState.format == export_Raw16LE ||
						sExportState.format == export_Raw32LE ||
						sExportState.format == export_RawFloatLE) ? platform_LittleEndian : platform_BigEndian);
			{
				for (int y = 0; y < layer->second.mHeight; ++y)
				for (int x = 0; x < layer->second.mWidth; ++x)
				{
					int ix = x, iy = y;
					if (sExportState.flip_x) ix = layer->second.mWidth - 1 - ix;
					if (sExportState.flip_y) iy = layer->second.mHeight - 1 - iy;
					float e = layer->second.get(ix,iy);
					if (!sExportReverseMap.empty())
						e = sExportReverseMap[e];

					if (sExportState.rescale) {
						e *= sExportState.scale;
						e += sExportState.offset;
					}
					unsigned char as_us;
					switch(sExportState.format) {
					case export_Raw8:
						if (!range_err && (e < 0 || e >= 256.0))	range_err = true;
						as_us = e;
						writer.WriteBulk((char*) &as_us, 1, false);
						break;
					case export_Raw16LE:
					case export_Raw16BE:
						if (!range_err && (e < SHRT_MIN || e > SHRT_MAX))	range_err = true;
						writer.WriteShort(e);
						break;
					case export_Raw32LE:
					case export_Raw32BE:
						if (!range_err && (e < LONG_MIN || e > LONG_MAX))	range_err = true;
						writer.WriteInt(e);
						break;
					case export_RawFloatLE:
					case export_RawFloatBE:
						writer.WriteFloat(e);
						break;
					}
				}
			}
			if (range_err)
				DoUserAlert("Warning: the raw export format you chose could not represent all of the data in this raster layer.  Some values may have been truncated or altered.");
		}
	}
}

void	ResyncExportDialog(void)
{
	XPWidgetID demPopup = XPFindWidgetByTag(sExport, WED_EXPORT_RASTERS);
	string	names;
	for (DEMGeoMap::iterator layer = gDem.begin(); layer != gDem.end(); ++layer)
	{
		if (layer != gDem.begin()) names += ';';
		names += FetchTokenString(layer->first);
	}
	if (names.empty()) names = "-No Raster Layers to Export-";
	XPSetWidgetDescriptor(demPopup, names.c_str());
	SetEnables(NULL);
}

void	ExportNotifier(int catagory, int message, void * param)
{
	if (catagory == wed_Cat_File && sExport != NULL && XPIsWidgetVisible(sExport))
		ResyncExportDialog();
}

void	CalculateRescaling(XPWidgetID)
{
	if (sExport == NULL) return;
	XPDataFromItem(sExport, WED_EXPORT_RASTERS);

	DEMGeoMap::iterator layer = gDem.begin();
	int n = sExportState.dem;
	while (n > 0 && layer != gDem.end())
		++layer, --n;
	sExportState.offset = 0.0;
	sExportState.scale = 1.0;
	if (layer != gDem.end())
	{
		float	minv, maxv;
		minv = maxv = DEM_NO_DATA;
		for (int x = 0; x < layer->second.mWidth; ++x)
		for (int y = 0; y < layer->second.mHeight; ++y)
		{
			float e = layer->second.get(x,y);
			minv = MIN_NODATA(minv, e);
			maxv = MAX_NODATA(maxv, e);
		}
		if (minv != DEM_NO_DATA && maxv != DEM_NO_DATA)
		{
			sExportState.offset = -minv;
			sExportState.scale = (maxv == minv)  ? 0.0 : (255.0 / (maxv - minv));
		}
	}

	XPDataToItem(sExport, WED_EXPORT_OFFSET);
	XPDataToItem(sExport, WED_EXPORT_SCALE);
}

static	void	LoadCLUT(XPWidgetID inID)
{
	XPWidgetID caption = XPFindWidgetByTag(sExport, WED_EXPORT_TRANSLATION);
	if (sExportForwardMap.empty())
	{
		char fileBuf[2048];
		fileBuf[0] = 0;
		if (GetFilePathFromUser(getFile_Open, "Please pick a mapping file", "Open", 3, fileBuf,sizeof(fileBuf)))
		{

			if (LoadTranslationFile(fileBuf, sExportForwardMap, &sExportReverseMap, &sExportCLUT))
			{
				char * s = fileBuf + strlen(fileBuf) - 1;
				while (s > fileBuf && *s != ':' && *s != '\\' && *s != '/')
					--s;
				XPSetWidgetDescriptor(caption, s);
				XPSetWidgetDescriptor(inID, "Clear Translation");
			} else {
				sExportForwardMap.clear();
				sExportReverseMap.clear();
				sExportCLUT.clear();
				XPSetWidgetDescriptor(caption, "(no translation)");
			}
		}
	} else {
		sExportForwardMap.clear();
		sExportReverseMap.clear();
		sExportCLUT.clear();
		XPSetWidgetDescriptor(caption, "(no translation)");
		XPSetWidgetDescriptor(inID, "Load Translation");
	}
}

void SetEnables(XPWidgetID)
{
	XPDataFromItem(sExport, WED_EXPORT_FORMAT);
	XPDataFromItem(sExport, WED_EXPORT_COLOR);
	XPDataFromItem(sExport, WED_EXPORT_RESCALE);
	bool	is_png = sExportState.format == export_PNG;
	bool	is_bmp = sExportState.format == export_BMP;
	bool	is_image = is_png || is_bmp;

	// Rescaling of values - available for all raws and non-color image formats.
	XPEnableByTag(sExport, WED_EXPORT_RESCALE, 	!is_image || !sExportState.color);
	XPEnableByTag(sExport, WED_EXPORT_OFFSET, 	(!is_image || !sExportState.color) && sExportState.rescale);
	XPEnableByTag(sExport, WED_EXPORT_SCALE, 	(!is_image || !sExportState.color) && sExportState.rescale);
	XPEnableByTag(sExport, WED_EXPORT_CALC, 	(!is_image || !sExportState.color) && sExportState.rescale);

	// Color inversion - image formats only.
	XPEnableByTag(sExport, WED_EXPORT_INVERT, is_image);

	// Alpha mask - only available for colorized PNGs.
	XPEnableByTag(sExport, WED_EXPORT_ALPHA, is_png && sExportState.color);

	// Colorization - only available for image formats.
	XPEnableByTag(sExport, WED_EXPORT_COLOR, is_image);
}
