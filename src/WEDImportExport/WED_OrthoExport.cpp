/*
 * Copyright (c) 2023, Laminar Research.
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

#include "WED_OrthoExport.h"

#include <geotiffio.h>
#include <geo_normalize.h>
#include <xtiffio.h>
#define PVALUE LIBPROJ_PVALUE
#include <proj_api.h>
#include <cpl_serv.h>

#if IBM
#include "GUI_Unicode.h"
#endif

#include "FileUtils.h"
#include "PlatformUtils.h"
#include "BitmapUtils.h"
#include "CompGeomUtils.h"
#include "GISUtils.h"
#include "STLUtils.h"
#include "XObjReadWrite.h"

#include "IGIS.h"
#include "IResolver.h"
#include "ITexMgr.h"

#include "WED_Version.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_TerPlacement.h"
#include "WED_ObjPlacement.h"
#include "WED_GISUtils.h"
#include "WED_ToolUtils.h"
#include "WED_HierarchyUtils.h"
#include "WED_ResourceMgr.h"
#include "WED_LibraryMgr.h"
#include "WED_PackageMgr.h"
#include "WED_Document.h"

#include "DEMDefs.h"

#include "tesselator.h"
#include <time.h>

#if DEV
#include "PerfUtils.h"
#endif

extern int gOrthoExport;

DSF_export_info_t::DSF_export_info_t(IResolver* resolver) : DockingJetways(true), resourcesAdded(false)

{
	orthoImg.data = NULL;

	if (resolver)
	{
		inDoc = dynamic_cast<WED_Document*>(resolver);
		auto dsf = inDoc->ReadStringPref("export/last", "", IDocPrefs::pref_type_doc);

		int last_pos = 0;
		for (int pos = 0; pos < dsf.size(); pos++)
		{
			if (dsf[pos] == ' ' || pos == dsf.size() - 1)
			{
				pos++;
				previous_dsfs.insert(dsf.substr(last_pos, pos - last_pos));
				last_pos = pos;
			}
		}
	}
	else
		inDoc = nullptr;
}

DSF_export_info_t::~DSF_export_info_t(void)
{
	if (orthoImg.data)
		free(orthoImg.data);

	if(resourcesAdded)
		gPackageMgr->Rescan(true);  // a full rescan of LibraryMgr can take a LOT of time on large systems. Only update local resources ?

	if (inDoc)
	{
		string path = "Earth nav data" DIR_STR;
		inDoc->LookupPath(path);
		for (auto& d : previous_dsfs)
			FILE_delete_file((path + d).c_str(), false);

		if (inDoc->ReadStringPref("export/last", "", IDocPrefs::pref_type_doc) != new_dsfs)
		{
			inDoc->WriteStringPref("export/last", new_dsfs, IDocPrefs::pref_type_doc);
			inDoc->SetDirty();
		}
	}
}

void DSF_export_info_t::mark_written(const string& file)
{
	previous_dsfs.erase(file);
	if (new_dsfs.length() < 200)   // 10 dsf files max remembered. Don't let a GW export blow this up ..
	{
		if (!new_dsfs.empty())
			new_dsfs += " ";
		new_dsfs += file;
	}
}


static bool hasPartialTransparency(ImageInfo * info)
{
	if(info->channels < 4) return false;
	int semiTransPixels = 0;

	unsigned char * src = info->data + 3;
	for(int y = info->height; y > 0; y--)
	{
		for(int x = info->width; x > 0; x--)
		{
			if(*src < 250 && *src > 0) semiTransPixels++; // deliberately ignore almost opaque pixels. Some tools create such
			src += 4;
		}
		src += 4 * info->pad;
	}
	return semiTransPixels > 10; // even ignore if there are just a very few stray semi-transparent pixels
}

static bool is_dir_sep(char c) { return c == '/' || c == ':' || c == '\\'; }

static bool is_backout_path(const string& p)
{
	vector<string> comps;
	tokenize_string_func(p.begin(), p.end(), back_inserter(comps), is_dir_sep);

	comps.erase(remove(comps.begin(), comps.end(), string(".")), comps.end());

	bool did_work = false;
	do {
		did_work = false;
		for (int i = 1; i < comps.size(); ++i)
			if (comps[i] == string(".."))
				if (comps[i - 1] != string(".."))
				{
					comps.erase(comps.begin() + i - 1, comps.begin() + i + 1);
					did_work = true;
					break;
				}
	} while (did_work);

	for (int i = 0; i < comps.size(); ++i)
	{
		if (comps[i] == string(".."))
			return true;
	}
	return false;
}


int WED_ExportOrtho(WED_DrapedOrthophoto* orth, IResolver* resolver, const string& pkg, DSF_export_info_t* export_info, string& r)
{
	string msg;
	orth->GetName(msg);

	// can't use the image name any more to determine the .pol/.dds names, as the same image could be used for multiple Orthos.
	// So we assume the 'Name" contains the image name plus some suffix to make it unique

	string relativePath(FILE_get_dir_name(r) + FILE_get_file_name_wo_extensions(msg));

	if (relativePath.compare(0, 3, "DEV") == 0 && (relativePath[3] == '/' || relativePath[3] == '\\') )  // source img is in DEV folder (LR convention for non-installer source art)
		relativePath.erase(0, 4);

	string relativePathDDS = relativePath + ( gOrthoExport ? ".dds" : ".png");
	string relativePathPOL = relativePath + ".pol";

	msg = string("The polygon '") + msg + "' cannot be converted to an orthophoto: ";

	if(is_backout_path(relativePath) || is_dir_sep(relativePath[0]) || relativePath[1] == ':')
	{
		DoUserAlert((msg + "The image resource must be a relative path to a location inside the sceneries directory, aborting DSF Export.").c_str());
		return -1;
	}

	string absPathIMG = pkg + r;
	string absPathDDS = pkg + relativePathDDS;
	string absPathPOL = pkg + relativePathPOL;
	FILE_make_dir_exist(FILE_get_dir_name(absPathDDS).c_str()); // if using the dev folder, non-DEV equivalent folder may not yet exist

	if(absPathDDS == absPathIMG)
	{
		DoUserAlert((msg + "Output DDS file would overwrite source file, aborting DSF Export. Change polygon name.").c_str());
		return -1;
	}

	Bbox2 UVbounds; orth->GetBounds(gis_UV, UVbounds);
	Bbox2 UVbounds_used(0,0,1,1);                            // we may end up not using all of the texture

	date_cmpr_result_t date_cmpr_res = FILE_date_cmpr(absPathIMG.c_str(),absPathDDS.c_str());
	//-----------------
	/* How to export a orthophoto
	* If it is a orthophoto and the image is newer than the DDS (avoid unnecissary DDS creation),
	* Create a Bitmap from whatever file format is being used.
	* Use the number of channels to decide the compression level
	* Create a DDS from that file format
	* Create the .pol with the file format in mind
	* Enjoy your new orthophoto
	*/

	if(date_cmpr_res == dcr_firstIsNew || date_cmpr_res == dcr_same)
	{
#if DEV
		StElapsedTime	etime("DDS export time");
#endif
		if(export_info->orthoFile != absPathIMG)
		{
			if(!export_info->orthoFile.empty())
			{
				Assert(export_info->orthoImg.data);
				free(export_info->orthoImg.data);
				export_info->orthoImg.data = NULL;
				export_info->orthoFile = "";
			}
			if(LoadBitmapFromAnyFile(absPathIMG.c_str(),&export_info->orthoImg)) // to cut into pieces, only. Make sure its not forcibly rescaled
			{
				DoUserAlert((msg + "Unable to convert the image file '" + absPathIMG + "'to a DDS file, aborting DSF Export.").c_str());
				return -1;
			}
			else
			{
				export_info->orthoFile = absPathIMG;

				// force reload of texture from disk - for visual confirmation that WED realized the image had changed
				ITexMgr * tman = WED_GetTexMgr(resolver);
				string relImgPath;
				orth->GetResource(relImgPath);
				tman->DropTexture(relImgPath.c_str());
			}
		}
		ImageInfo imgInfo(export_info->orthoImg);
		ImageInfo DDSInfo;

		int UVMleft   = intround(imgInfo.width * UVbounds.xmin());
		int UVMright  = intround(imgInfo.width * UVbounds.xmax());
		int UVMtop    = intround(imgInfo.height * UVbounds.ymax());
		int UVMbottom = intround(imgInfo.height * UVbounds.ymin());

		/* If the source image is a multiple of 1k pix/side - we want to avoid scaling the subtextures as much as possible.
			So in case the UV coords are a tiny bit off - rather round towards a size that allows keeping 1:1 pixel ratio.
		*/
		bool is1Ksource = imgInfo.width % 1024 == 0 && imgInfo.height % 1024 == 0;

		if(is1Ksource)
		{
			if(UVMleft   % 512 == 1) UVMleft -= 1;   else if(UVMleft   % 512 == 511) UVMleft += 1;
			if(UVMright  % 512 == 1) UVMright -= 1;  else if(UVMright  % 512 == 511) UVMright += 1;
			if(UVMtop    % 512 == 1) UVMtop -= 1;    else if(UVMtop    % 512 == 511) UVMtop += 1;
			if(UVMbottom % 512 == 1) UVMbottom -= 1; else if(UVMbottom % 512 == 511) UVMbottom += 1;
		}

		int UVMwidth  = UVMright - UVMleft;
		int UVMheight = UVMtop - UVMbottom;

		int DDSwidth = 4;
		int DDSheight = 4;

		while(DDSwidth < UVMwidth && DDSwidth < 2048) DDSwidth <<= 1;      // round up dimensions under 2k to a power of 2 AND limit to 2k
		while(DDSheight < UVMheight && DDSheight < 2048) DDSheight <<= 1;

		/* we may end up with a 'partial' tile - i.e. the polygon was reshaped and now the UVbounds don't cover a full tile
			any more. Normally - we would upscale the exact part of the source image needed to make it a power of 2.
			But - we may not have to: *if* the source image is large enough - we just grab a 1:1 copy of the next larger pow2 size
			and the only use a part of it.
		*/
		if(is1Ksource)
		{
			if(DDSwidth > UVMwidth)
			{
				if(UVbounds.ymin() > 0.0 && UVMright % 512 == 0)
				{
					double desired_left = UVMright - DDSwidth;
					if(desired_left >= 0)
					{
						UVMleft = desired_left;
						UVbounds_used.p1.x_ = 1.0 - ((double) UVMwidth) / DDSwidth;
						LOG_MSG("I/DSF save a scale: using w=%d/%d pix, leaving some unused on left\n", UVMwidth, DDSwidth);
						UVMwidth = DDSwidth;
					}
				}
				else
				{
					double desired_right = UVMleft + DDSwidth;
					if(desired_right <= imgInfo.width)
					{
						UVMright = desired_right;
						UVbounds_used.p2.x_ = ((double) UVMwidth) / DDSwidth;
						LOG_MSG("I/DSF save a scale: using w=%d/%d pix, leaving some unused on right\n", UVMwidth, DDSwidth);
						UVMwidth = DDSwidth;
					}
				}
			}
			if(DDSheight > UVMheight)
			{
				if(UVbounds.xmin() > 0.0 && UVMtop % 512 == 0)
				{
					double desired_bottom = UVMtop - DDSheight;
					if(desired_bottom >= 0)
					{
						UVMbottom = desired_bottom;
						UVbounds_used.p1.y_ = 1.0 - ((double) UVMheight) / DDSheight;
						LOG_MSG("I/DSF save a scale: using h=%d/%d pix, leaving some unused on bottom\n", UVMheight, DDSheight);
						UVMheight = DDSheight;
					}
				}
				else
				{
					double desired_top = UVMbottom + DDSheight;
					if(desired_top <= imgInfo.height)
					{
						UVMtop = desired_top;
						UVbounds_used.p2.y_ = ((double) UVMheight) / DDSheight;
						LOG_MSG("I/DSF save a scale: using h=%d/%d pix, leaving some unused on top\n", UVMheight, DDSheight);
						UVMheight = DDSheight;
					}
				}
			}
		}
		else
		{
			if (UVMwidth < UVMheight * 0.7)        // avoid up-rezzing too much, 1025x2047 texture would otherwise grow to 2048x2048
				if (DDSwidth >= DDSheight) DDSwidth = DDSheight / 2;
			if (UVMheight < UVMwidth * 0.7)
				if (DDSheight >= DDSwidth) DDSheight = DDSwidth / 2;
		}

		if (CreateNewBitmap(DDSwidth, DDSheight, imgInfo.channels, &DDSInfo) == 0)       // create array to hold upsized image
		{
			if(UVMwidth == DDSwidth && UVMheight == DDSheight)
			{
				CopyBitmapSectionDirect(imgInfo, DDSInfo, UVMleft, UVMbottom, 0, 0, DDSwidth, DDSheight);
				LOG_MSG("I/DSF exporting ortho tile %s at 1:1 scale\n", absPathDDS.c_str());
			}
			else
			{
				CopyBitmapSectionSharp(imgInfo, DDSInfo, UVMleft, UVMbottom, UVMright, UVMtop,
																	0, 0, DDSwidth, DDSheight);
				LOG_MSG("I/DSF exporting ortho tile %s scaled\n", absPathDDS.c_str());
			}
			if(gOrthoExport)
			{
				if(DDSInfo.channels == 3)
					ConvertBitmapToAlpha(&DDSInfo,false);
				int BCMethod = hasPartialTransparency(&DDSInfo) ? 3 : 1;
				WriteBitmapToDDS_MT(DDSInfo, BCMethod, absPathDDS.c_str(), mip_filter_box);
			}
			else
				WriteBitmapToPNG(&DDSInfo, absPathDDS.c_str(), NULL, 0, 2.2);
		}
	}
	else if(date_cmpr_res == dcr_error)
	{
		string msg = string("The file '") + absPathIMG + string("' is missing, aborting DSF Export.");
		DoUserAlert(msg.c_str());
		return -1;
	}

	if(!FILE_exists(absPathPOL.c_str()))
	{
		ImageInfo DDSInfo;
		if(CreateBitmapFromDDS(absPathDDS.c_str(), &DDSInfo) == 0)
		{
			Bbox2 b;
			orth->GetBounds(gis_Geo, b);
			Point2 center = b.centroid();
			//-------------------------------------------
			pol_info_t out_info = { FILE_get_file_name(relativePathDDS),  orth->GetDecal(), tile_info(),
				/*SCALE*/ (float) LonLatDistMeters(b.p1,Point2(b.p2.x(), b.p1.y())), (float) LonLatDistMeters(b.p1,Point2(b.p1.x(), b.p2.y())),  // althought its irrelevant here
				false, false,
				/*LAYER_GROUP*/ "beaches", +1,
				/*LOAD_CENTER*/ (float) center.y(), (float) center.x(), (float) LonLatDistMeters(b.p1,b.p2), intmax2(DDSInfo.height,DDSInfo.width) };
			WED_GetResourceMgr(resolver)->WritePol(absPathPOL, out_info);
			DestroyBitmap(&DDSInfo);
		}
	}

	orth->StartOperation("Norm Ortho");
	orth->Rescale(gis_UV, UVbounds, UVbounds_used);
	r = relativePathPOL;		// Resource name comes from the pol no matter what we compress to disk.
#if IBM
	std::replace(r.begin(), r.end(), '\\', '/');  // improve backward comp. with older WED versions that don't (yet) convert these to '/' at import. XP is fine with either.
#endif
	return 0;
}

static WED_DrapedOrthophoto* find_ortho(Polygon2 area, Bbox2 area_box, WED_Thing* base)
{
	string res;
	auto lmgr = WED_GetLibraryMgr(base->GetArchive()->GetResolver());
	vector<WED_DrapedOrthophoto*> orthos;
	CollectRecursive(base, back_inserter(orthos));
	for (auto o : orthos)
	{
		Bbox2 b;
		o->GetBounds(gis_Geo, b);               // fast cull - ortho must fully enclose .ter object as drawn
		if (b.contains(area_box))               // todo - allow go across a multiple orthos, create all .ter and merge in .agp
		{										// do check area is truly fully enclosed by ortho ?
			o->GetResource(res);
			if (!lmgr->IsResourceLibrary(res))  // not a library means its gotta be local. 
				                                // can't use IsResourceLocal() because if its a true WED orthophoto patch, its not a .pol
				                                // but the .tif/.jpg thats is going to be used to make the .pol/.dds based on the name of the ortho
			{
				return o;                          
			}
		}
	}
	return nullptr;
}

enum {
	dem_want_Post,	// Use pixel=post sampling
	dem_want_Area,	// Use area-pixel sampling!
	dem_want_File	// Use whatever the file has.
};

template<typename T>
void copy_scanline(const T* v, int y, dem_info_t& dem)
{
	for (int x = 0; x < dem.mWidth; ++x, ++v)
	{
		float e = *v;
		dem(x, dem.mHeight - y - 1) = e;
	}
}

template<typename T>
void copy_tile(const T* v, int x, int y, int w, int h, dem_info_t& dem)
{
	for (int cy = 0; cy < h; ++cy)
		for (int cx = 0; cx < w; ++cx)
		{
			int dem_x = x + cx;
			int dem_y = dem.mHeight - (y + cy) - 1;
			float e = *v;
			dem(dem_x, dem_y) = e;
			++v;
		}
}

// adapted version of equivalent function in DEMIO.h

 bool	WED_ExtractGeoTiff(dem_info_t& inMap, const char* inFileName, int post_style)
{
	TIFF * tif;
#if SUPPORT_UNICODE
	XTIFFInitialize();
	tif = TIFFOpenW(convert_str_to_utf16(inFileName).c_str(), "r");
#else
	tif = XTIFFOpen(inFileName, "r");
#endif
	if (tif)
	{
		double	corners[8];
		if (FetchTIFFCornersWithTIFF(tif, corners, post_style))
	{

		// this assumes geopgrahic, not projected coordinates ...
		inMap.mWest = corners[0];
		inMap.mSouth = corners[1];
		inMap.mEast = corners[6];
		inMap.mNorth = corners[7];
		inMap.mPost = (post_style == dem_want_Post);

		uint32 w, h;
		uint16 cc;
		uint16 d;
		uint16 format = SAMPLEFORMAT_UINT;	// sample format is NOT mandatory - unsigned int is the default if not present!

		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &cc);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &d);
		TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &format);
//		printf("Image is: %dx%d, samples: %d, depth: %d, format: %d\n", w, h, cc, d, format);

		inMap.resize(w, h);

		if (TIFFIsTiled(tif))
		{
			uint32	tw, th;
			TIFFGetField(tif, TIFFTAG_TILEWIDTH, &tw);
			TIFFGetField(tif, TIFFTAG_TILELENGTH, &th);
			vector<char> buf;
			buf.resize(TIFFTileSize(tif));
			for (int y = 0; y < h; y += th)
				for (int x = 0; x < w; x += tw)
				{
					if (TIFFReadTile(tif, buf.data(), x, y, 0, 0) == -1)
						goto bail;

					int ux = min(tw, w - x);
					int uy = min(th, h - y);

					switch (format) 
					{
					case SAMPLEFORMAT_UINT:
						switch (d) 
						{
						case 8:  copy_tile<unsigned char >((unsigned char*)  buf.data(), x, y, ux, uy, inMap); break;
						case 16: copy_tile<unsigned short>((unsigned short*) buf.data(), x, y, ux, uy, inMap); break;
						case 32: copy_tile<unsigned int  >((unsigned int*)   buf.data(), x, y, ux, uy, inMap); break;
						default: goto bail;
						}
						break;
					case SAMPLEFORMAT_INT:
						switch (d) 
						{
						case 8:  copy_tile<char >((char* ) buf.data(), x, y, ux, uy, inMap); break;
						case 16: copy_tile<short>((short*) buf.data(), x, y, ux, uy, inMap); break;
						case 32: copy_tile<int  >((int*  ) buf.data(), x, y, ux, uy, inMap); break;
						default: goto bail;
						}
						break;
					case SAMPLEFORMAT_IEEEFP:
						switch (d) 
						{
						case 32: copy_tile<float >((float* ) buf.data(), x, y, ux, uy, inMap); break;
						case 64: copy_tile<double>((double*) buf.data(), x, y, ux, uy, inMap); break;
						default: goto bail;
						}
						break;
					default: goto bail;
					}
				}
			XTIFFClose(tif);
			return true;
		}
		else
		{
			tsize_t line_size = TIFFScanlineSize(tif);
			vector<char> aline;
			aline.resize(line_size);

			int cs = TIFFCurrentStrip(tif);
			int nos = TIFFNumberOfStrips(tif);
			int cr = TIFFCurrentRow(tif);

			for (int y = 0; y < h; ++y)
			{
				if (TIFFReadScanline(tif, aline.data(), y, 0) == -1)
					goto bail;

				switch (format) 
				{
				case SAMPLEFORMAT_UINT:
					switch (d) 
					{
					case 8:  copy_scanline<unsigned char >((unsigned char* )aline.data(), y, inMap); break;
					case 16: copy_scanline<unsigned short>((unsigned short*)aline.data(), y, inMap); break;
					case 32: copy_scanline<unsigned int  >((unsigned int*  )aline.data(), y, inMap); break;
					default: goto bail;
					}
					break;
				case SAMPLEFORMAT_INT:
					switch (d) 
					{
					case 8:	 copy_scanline<char >((char* )aline.data(), y, inMap); break;
					case 16: copy_scanline<short>((short*)aline.data(), y, inMap); break;
					case 32: copy_scanline<int  >((int*  )aline.data(), y, inMap); break;
					default: goto bail;
					}
					break;
				case SAMPLEFORMAT_IEEEFP:
					switch (d) 
					{
					case 32: copy_scanline<float >((float* )aline.data(), y, inMap); break;
					case 64: copy_scanline<double>((double*)aline.data(), y, inMap); break;
					default: goto bail;
					}
					break;
				default: break;
				}
			}
			XTIFFClose(tif);
			return true;
		}
	
	}
	bail:
		XTIFFClose(tif);
		LOG_MSG("E/Dem Error reading DEM %s\n", inFileName);
	}
	else
		LOG_MSG("E/Dem Error opening DEM %s\n", inFileName);
	return false;
}

 static float dist_to_edge(const dem_info_t& dem, int loc_x, int loc_y, int max_dist)
 {
     float min_dist = 99*99;
	 for (int y = loc_y - max_dist; y <= loc_y +  max_dist; y++)
		 for(int x = loc_x - max_dist; x <= loc_x + max_dist; x++)
			 if (dem.get(x, y) == DEM_NO_DATA)
			 {
				 float dist = pythag_sqr(x - loc_x, y - loc_y);
				 if (dist < min_dist) min_dist = dist;
			 }
	 return sqrtf(min_dist);
 }

static int mesh2obj(XObj8& obj, const Polygon2& area, const CoordTranslator2& ll2mtr, const CoordTranslator2& ll2uv,
                     const DEMGeo& dem, float deres_factor, float skirt_depth, const Polygon2& area_dem, float clip_elev)
{
	Bbox2 bounds = area.bounds();
	DEMGeo ldem;
	dem.subset(ldem, dem.x_upper(bounds.xmin()), dem.y_upper(bounds.ymin()), dem.x_lower(bounds.xmax()), dem.y_lower(bounds.ymax()));
	if (deres_factor != 1.0f)
	{
		DEMGeo smaller;
		smaller.copy_geo_from(ldem);
		smaller.resize(ldem.mPost + intround((ldem.mWidth - ldem.mPost) / deres_factor), 
					   ldem.mPost + intround((ldem.mWidth - ldem.mPost) / deres_factor));

		for (int y = 0; y < smaller.mHeight; ++y)
			for (int x = 0; x < smaller.mWidth; ++x)
			{
				double lon = smaller.x_to_lon(x);
				double lat = smaller.y_to_lat(y);
				smaller(x, y) = max(ldem.value_linear(lon,lat), clip_elev);
			}
		ldem.swap(smaller);
	}

	//	local_dem.deres_nearest() or make a derez_average() derez_cubic()

	// make normals before creating the skirt - so normals stay unaffected, i.e. skirt area is less notable due to shading
	DEMGeo NX, NY, NZ;
	ldem.calc_normal(NX, NY, NZ, nullptr);

	// nuke all data outside of desired object area
	for (int y = 0; y < ldem.mHeight; y++)
		for (int x = 0; x < ldem.mWidth; x++)
		{
			auto pt = Point2(ldem.x_to_lon(x), ldem.y_to_lat(y));
			if (!area.inside(pt))
				ldem.zap(x, y);
		}

	// make perimeter points droop
	DEMMask skirt(ldem);

	for (int y = 0; y < ldem.mHeight; y++)
		for (int x = 0; x < ldem.mWidth; x++)
			if (skirt.get(x, y))
			{
				if (ldem.get(x + 1, y) == DEM_NO_DATA || ldem.get(x - 1, y) == DEM_NO_DATA ||
					ldem.get(x, y + 1) == DEM_NO_DATA || ldem.get(x, y - 1) == DEM_NO_DATA)
				{
					ldem(x, y) = ldem(x, y) - skirt_depth;
				}
				else
				{
					auto pt = Point2(ldem.x_to_lon(x), ldem.y_to_lat(y));
					if (area_dem.empty() || area_dem.inside(pt))
						skirt.set(x, y, false);
					else
					{
						double dist_outside = 999;
						for (int i = 0; i < area.size(); i++)
						{
							double d = area.side(i).squared_distance({ ldem.x_to_lon(x), ldem.y_to_lat(y) });
							if (d < dist_outside) dist_outside = d;
						}
						double dist_inside = 999;
						for (int i = 0; i < area_dem.size(); i++)
						{
							double d = area_dem.side(i).squared_distance({ ldem.x_to_lon(x), ldem.y_to_lat(y) });
							if (d < dist_inside) dist_inside = d;
						}
						dist_inside = sqrt(dist_inside);
						dist_outside = sqrt(dist_outside);
						ldem(x, y) = ldem(x, y) - skirt_depth * dist_inside / (dist_inside + dist_outside);
					}
				}
			}

	float pt[8];
	vector<int> skirt_idx;

	// make a quadrilateral tesselated mesh
	for (int y = 0; y < ldem.mHeight - 1; y++)
		for (int x = 0; x < ldem.mWidth - 1; x++)
			if (ldem.get(x, y) != DEM_NO_DATA)
			{

				auto fill_pt = [&](int x, int y) -> int
				{
					auto p = Point2(ldem.x_to_lon(x), ldem.y_to_lat(y));
					pt[0] = ll2mtr.Forward(p).x(); // xyz
					pt[1] = ldem(x, y);
					pt[2] = ll2mtr.Forward(p).y();
					pt[3] = NX(x,y);              // nml
					pt[4] = NZ(x,y);
					pt[5] = -NY(x,y);
					pt[6] = ll2uv.Forward(p).x();  // uv
					pt[7] = ll2uv.Forward(p).y();

					return obj.geo_tri.accumulate(pt);
				};

				bool s = skirt.get(x, y) || skirt.get(x + 1, y) || skirt.get(x, y + 1) || skirt.get(x + 1, y + 1);

				auto push_idx = [&](int i, int j, int k)
				{
					if (s)
					{
						skirt_idx.push_back(i);
						skirt_idx.push_back(j);
						skirt_idx.push_back(k);
					}
					else
					{
						obj.indices.push_back(i);
						obj.indices.push_back(j);
						obj.indices.push_back(k);
					}
				};

				bool has_next_E  = ldem.get(x + 1, y)     != DEM_NO_DATA;
				bool has_next_N  = ldem.get(x, y + 1)     != DEM_NO_DATA;
				bool has_next_NE = ldem.get(x + 1, y + 1) != DEM_NO_DATA;
				bool has_next_S  = ldem.get(x, y - 1)     != DEM_NO_DATA;
				bool has_next_SE = ldem.get(x + 1, y - 1) != DEM_NO_DATA;

				if (has_next_E && has_next_N && has_next_NE)   // full quad
				{
					int i0 = fill_pt(x,y);
					int i1 = fill_pt(x+1,y);
					int i2 = fill_pt(x,y+1);
					int i3 = fill_pt(x+1,y+1);
					push_idx(i0, i2, i1);
					push_idx(i1, i2, i3);
				}
				else if(has_next_E && has_next_N)
				{
					int i0 = fill_pt(x,y);
					int i1 = fill_pt(x+1,y);
					int i2 = fill_pt(x,y+1);
					push_idx(i0, i2, i1);
				}
				else if (has_next_E && has_next_NE)
				{
					int i0 = fill_pt(x,y);
					int i1 = fill_pt(x+1,y);
					int i2 = fill_pt(x+1,y+1);
					push_idx(i0, i2, i1);
				}
				else if (has_next_N && has_next_NE)
				{
					int i0 = fill_pt(x, y);
					int i1 = fill_pt(x+1,y+1);
					int i2 = fill_pt(x,y+1);
					push_idx(i0, i2, i1);
				}
				if (has_next_E && has_next_SE && !has_next_S)
				{
					int i0 = fill_pt(x,y);
					int i1 = fill_pt(x+1,y-1);
					int i2 = fill_pt(x+1,y);
					push_idx(i0, i2, i1);
				}
			}
	int skirt_start = obj.indices.size();
	for (auto i : skirt_idx)
		obj.indices.push_back(i);

	// create "skirt". Make a polygon encircling the outermost of these points,
	// tesselate a polygon using the area as outer ring and the above as inner ring/hole
	// append that donut shaped mesh to the regular one
	//

	return skirt_start;
}

// Suuuper trivial 3D object for testing or debugging. Literally a MineralsPile.obj lookalike pyramid.
static void poly2obj(XObj8& obj, const Polygon2& area, const CoordTranslator2& ll2mtr, const CoordTranslator2& ll2uv, float height)
{
	int i_base = obj.geo_tri.count();
	float pt[8];

	auto fill_pt = [&](const Point2& loc, const Point2& uv)
	{
		pt[0] = loc.x(); // xyz
		pt[1] = 0.0;
		pt[2] = loc.y();
		pt[3] = 0;      // nml
		pt[4] = 1;
		pt[5] = 0;
		pt[6] = uv.x();  // uv
		pt[7] = uv.y();
	};

	fill_pt({ 0,0 }, ll2uv.Forward(ll2mtr.Reverse({ 0,0 })));
	pt[1] = height;
	obj.geo_tri.append(pt);
	int n_pts = area.size();
	fill_pt(ll2mtr.Forward(area[n_pts - 1]), ll2uv.Forward(area[n_pts - 1]));
	obj.geo_tri.append(pt);
	for (int n = 0; n < n_pts; n++)
	{
		fill_pt(ll2mtr.Forward(area[n]), ll2uv.Forward(area[n]));
		if (n < n_pts - 1)
			obj.geo_tri.append(pt);
		obj.indices.push_back(i_base);
		obj.indices.push_back(i_base + 2 + ((n < n_pts - 1) ? n : -1));
		obj.indices.push_back(i_base + 2 + n - 1);
	}
}


int WED_ExportTerrObj(WED_TerPlacement* ter, IResolver* resolver, const string& pkg, string& resource)
{
	if(auto ter_pol = dynamic_cast<IGISPolygon*>(ter))
	{
		Polygon2 ter_poly, ter_skirt;
		IGISPointSequence* ter_ps;
		auto wrl = WED_GetWorld(resolver);
		if (ter_ps = ter_pol->GetOuterRing())
			WED_PolygonForPointSequence(ter_ps, ter_poly, COUNTERCLOCKWISE);

		if (ter_pol->GetNumHoles() > 0)
		{
			if (ter_ps = ter_pol->GetNthHole(0))
				WED_PolygonForPointSequence(ter_ps, ter_skirt, COUNTERCLOCKWISE);
		}
		else
			ter_skirt = ter_poly;   // or undersize ter_poly ???

		Bbox2 ter_box;
		ter_pol->GetBounds(gis_Geo, ter_box);
		auto ortho = find_ortho(ter_poly, ter_box, wrl);
		if (!ortho)
			return -1;
		auto ortho_pol = dynamic_cast<IGISPolygon*>(ortho);

		// figure uv locations within ortho
		string orthoResource;
		ortho->GetResource(orthoResource);

		Bbox2 ortho_corners;
		ortho_pol->GetBounds(gis_Geo, ortho_corners);

		CoordTranslator2 ll2uv;
		{
			if(ortho->IsNew())
			{
				ll2uv.mDstMin = { 0, 0 };                  // assumes that WED will export .pol as one texture
				ll2uv.mDstMax = { 1, 1 };
			}
			else
			{
				Bbox2 ortho_uv;
				ortho_pol->GetBounds(gis_UV, ortho_uv);
				ll2uv.mDstMin = ortho_uv.bottom_left();
				ll2uv.mDstMax = ortho_uv.top_right();
			}

			ll2uv.mSrcMin = ortho_corners.bottom_left();
			ll2uv.mSrcMax = ortho_corners.top_right();
		}
		// get dem heights
		string dem_file;
		ter->GetResource(dem_file);
		dem_file = pkg + dem_file;
		const dem_info_t* ter_dem;

		if (!(WED_GetResourceMgr(ter->GetArchive()->GetResolver())->GetDem(dem_file, ter_dem)))
			return -1;

		// optionally change heights to be relative to terrain height
		// optionally change height so it fits

		CoordTranslator2 ll2mtr;
		{
			CreateTranslatorForBounds(ter_box, ll2mtr);
			auto ctr_mtr = Vector2(ll2mtr.mDstMin, ll2mtr.mDstMax);
			ll2mtr.mDstMin -= ctr_mtr * 0.5;
			ll2mtr.mDstMax -= ctr_mtr * 0.5;
			swap(ll2mtr.mDstMin.y_, ll2mtr.mDstMax.y_);
		}

		string orthoName;
		ortho->GetName(orthoName);
		string terName;
		ter->GetName(terName);
		string objName = FILE_get_file_name_wo_extensions(orthoName) + "_" + FILE_get_file_name_wo_extensions(terName) + ".obj";
		string objVPath = FILE_get_dir_name(orthoResource) + objName;

		if (objVPath.compare(0, 3, "DEV") == 0 && (objVPath[3] == '/' || objVPath[3] == '\\'))  // source img is in DEV folder (LR convention for non-installer source art)
			objVPath.erase(0, 4);

		string objAbsPath = pkg + objVPath;
		FILE_make_dir_exist(FILE_get_dir_name(objAbsPath).c_str());

		XObj8 ter_obj;
		XObjCmd8 cmd;
		if (ortho->IsNew())
		{
			ter_obj.texture = FILE_get_file_name_wo_extensions(orthoName) + (gOrthoExport ? ".dds" : ".png");
			ter_obj.decal_lib = ortho->GetDecal();
		}
		else
		{
			const pol_info_t* pol;
			if (WED_GetResourceMgr(resolver)->GetPol(orthoResource, pol))
			{
				ter_obj.decal_lib = pol->decal;
				if (pol->base_tex.compare(0, pkg.length(), pkg) == 0)
					ter_obj.texture = FILE_get_file_name(pol->base_tex);
				else
					DoUserAlert((string("Terrain '") + terName + "' is covered by '" + orthoResource + "' which uses a non-local texture '" 
						          + pol->base_tex + "'\nBut objects only have access to local textures.").c_str());
			}
		}
		ter_obj.glass_blending = 0;
		float clip_elev = ter->IsClip() ? -ter->GetCustomMSL() /* (ter->GetMSLType() != 0) */ : -999.0;

		// create & add mesh
		// the super-sily proof-of-concept function
//		poly2obj(ter_obj, area, ll2mtr, ll2uv, ter_dem.value_linear(ter_corners.centroid()));
		int skirt_idx = mesh2obj(ter_obj, ter_poly, ll2mtr, ll2uv, *ter_dem, ter->GetSamplingFactor(), ter->GetSkirtDepth(), ter_skirt, clip_elev);

		float lod_dist = 20.0 * ceil(LonLatDistMeters(ter_box.p1, ter_box.p2));

		// ATTR_LOD
		ter_obj.lods.push_back(XObjLOD8());
		ter_obj.lods.back().lod_near = 0;
		ter_obj.lods.back().lod_far = fltlim(lod_dist, 1000, 20000);
		// ATTR_HARD
		if (ter->IsHardSurface())
		{
			cmd.cmd = attr_Hard;
			ter_obj.lods.back().cmds.push_back(cmd);
		}
		// TRIS
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = 0;
		cmd.idx_count = skirt_idx;
		ter_obj.lods.back().cmds.push_back(cmd);
		// ATTR_no_shadow;
		cmd.cmd = attr_No_Shadow;
		ter_obj.lods.back().cmds.push_back(cmd);
		// TRIS
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = skirt_idx;
		cmd.idx_count = ter_obj.indices.size() - skirt_idx;
		ter_obj.lods.back().cmds.push_back(cmd);
		// LOAD_CENTER
#if 0
		// center of this object
		ter_obj.loadCenter_latlon[0] = ll2mtr.Reverse({ 0,0 }).y();
		ter_obj.loadCenter_latlon[1] = ll2mtr.Reverse({ 0,0 }).x();
		Bbox2 uv_corners(ll2uv.Forward(ter_box.top_left()), ll2uv.Forward(ter_box.bottom_right()));
		ter_obj.loadCenter_texSize = 2048 * uv_corners.xspan(); // assumes WED will create a 2k texture - may be wrong ?
		ter_obj.loadCenter_size = ter_obj.xyz_max[0] - ter_obj.xyz_min[0];
#else
		// center of underlying draped polygon
		ter_obj.loadCenter_latlon[0] = ortho_corners.centroid().y();
		ter_obj.loadCenter_latlon[1] = ortho_corners.centroid().x();
		ter_obj.loadCenter_texSize = 2048; // assumes WED will create a 2k texture - may be wrong ?
		ter_obj.loadCenter_size = LonLatDistMeters(ortho_corners.p1, ortho_corners.p2);
#endif
		char msg[100];
		snprintf(msg, 99, "Created by WED " WED_VERSION_STRING " for %.8lf %.8lf", ter_box.centroid().x(), ter_box.centroid().y());

		XObj8Write(objAbsPath.c_str(), ter_obj, msg);
		resource = objVPath;
#if IBM
		std::replace(resource.begin(), resource.end(), '\\', '/');
#endif
		if (auto resMgr = WED_GetResourceMgr(resolver))
			resMgr->Purge(resource);

		return 0;
	}
	else
		return -1;
}
