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
#include "WED_DemPlacement.h"
#include "WED_ObjPlacement.h"
#include "WED_GISUtils.h"
#include "WED_ToolUtils.h"
#include "WED_HierarchyUtils.h"
#include "WED_ResourceMgr.h"
#include "WED_PackageMgr.h"
#include "WED_Document.h"

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

	if(absPathDDS == absPathIMG)
	{
		DoUserAlert((msg + "Output DDS file would overwrite source file, aborting DSF Export. Change polygon name.").c_str());
		return -1;
	}

	Bbox2 UVbounds; orth->GetBounds(gis_UV, UVbounds);
	Bbox2 UVbounds_used(0,0,1,1);                            // we may end up not using all of the texture
	WED_ResourceMgr * rmgr = WED_GetResourceMgr(resolver);

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
			pol_info_t out_info = { FILE_get_file_name(relativePathDDS), false, tile_info(),
				/*SCALE*/ (float) LonLatDistMeters(b.p1,Point2(b.p2.x(), b.p1.y())), (float) LonLatDistMeters(b.p1,Point2(b.p1.x(), b.p2.y())),  // althought its irrelevant here
				false, false,
				/*LAYER_GROUP*/ "beaches", +1,
				/*LOAD_CENTER*/ (float) center.y(), (float) center.x(), (float) LonLatDistMeters(b.p1,b.p2), intmax2(DDSInfo.height,DDSInfo.width) };
			rmgr->WritePol(absPathPOL, out_info);
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

static WED_DrapedOrthophoto* find_ortho(Polygon2 area, WED_Thing* base)
{
	vector<WED_DrapedOrthophoto*> orthos;
	CollectRecursive(base, back_inserter(orthos));
	for (auto o : orthos)
		if (o->Overlaps(gis_Geo, area))
			return o;                       // might need to check which one is on the top, i..e visible in scenery (layering + hierachy)
	return nullptr;
}

static void mesh2obj(XObj8& obj, const vector<Point2>& pts, const vector<int>& extra_contours, float height)
{
}


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

	fill_pt({0,0}, ll2uv.Forward(ll2mtr.Reverse({0,0})));
	pt[1] = height;
	obj.geo_tri.append(pt);
	int n_pts = area.size();
	fill_pt(ll2mtr.Forward(area[n_pts-1]), ll2uv.Forward(area[n_pts-1]));
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

int WED_ExportTerrObj(WED_DemPlacement* dem, IResolver* resolver, const string& pkg, string& resource)
{
	Polygon2 area;
	IGISPointSequence* area_ps;
	auto dem_pol = dynamic_cast<IGISPolygon*>(dem);
	if (dem_pol)
	{
		auto wrl = WED_GetWorld(resolver);
		if (area_ps = dem_pol->GetOuterRing())
			WED_PolygonForPointSequence(area_ps, area, COUNTERCLOCKWISE);
		auto ortho = find_ortho(area, wrl);
		if (!ortho)
			return -1;
		auto ortho_pol = dynamic_cast<IGISPolygon*>(ortho);

		// figure uv locations within ortho
		CoordTranslator2 ll2uv;
		{
			Bbox2 geo_corners;
			ortho_pol->GetBounds(gis_Geo, geo_corners);
			Bbox2 uv_corners;
			ortho_pol->GetBounds(gis_UV, uv_corners);  // thats relating to the source image, NOT the exported .dds

			ll2uv.mSrcMin = geo_corners.bottom_left();
			ll2uv.mSrcMax = geo_corners.top_right();
//			ll2uv.mDstMin = uv_corners.top_left();
//			ll2uv.mDstMax = uv_corners.bottom_right();
			ll2uv.mDstMin = { 0, 0 };                  // assumes that WED will export .pol as one texture
			ll2uv.mDstMax = { 1, 1 };
		}
		// get dem heights
		// optionally change heights to be relative to terrain height
		// optionally change height so it fits
		// tessealte the obj

		Bbox2 dem_corners;
		dem_pol->GetBounds(gis_Geo, dem_corners);
		CoordTranslator2 ll2mtr;
		{
			CreateTranslatorForBounds(dem_corners, ll2mtr);
			auto ctr_mtr = Vector2(ll2mtr.mDstMin, ll2mtr.mDstMax);
			ll2mtr.mDstMin -= ctr_mtr * 0.5;
			ll2mtr.mDstMax -= ctr_mtr * 0.5;
			swap(ll2mtr.mDstMin.y_, ll2mtr.mDstMax.y_);
		}

		string orthoName;
		ortho->GetName(orthoName);
		string objName = FILE_get_file_name_wo_extensions(orthoName) + ".obj";
		string orthoResource;
		ortho->GetResource(orthoResource);
		string objVPath = FILE_get_dir_name(orthoResource) + objName;
		string objAbsPath = pkg + objVPath;

		XObj8 dem_obj;
		XObjCmd8 cmd;
		dem_obj.texture =  FILE_get_file_name_wo_extensions(orthoName) + ".dds";
		dem_obj.glass_blending = 0;

		// create & add mesh
		poly2obj(dem_obj, area, ll2mtr, ll2uv, ll2mtr.mDstMax.x_ * 0.3);
		// "ATTR_LOD"
		dem_obj.lods.push_back(XObjLOD8());
		dem_obj.lods.back().lod_near = 0;
		dem_obj.lods.back().lod_far = 3000;
		// "TRIS ";
		cmd.cmd = obj8_Tris;
		cmd.idx_offset = 0;
		cmd.idx_count = dem_obj.indices.size();
		dem_obj.lods.back().cmds.push_back(cmd);

		dem_obj.xyz_min[0] = ll2mtr.mDstMin.x();
		dem_obj.xyz_max[0] = ll2mtr.mDstMax.x();
		dem_obj.xyz_min[2] = ll2mtr.mDstMin.y();
		dem_obj.xyz_max[2] = ll2mtr.mDstMax.y();
		dem_obj.loadCenter_latlon[0] = ll2mtr.Reverse({ 0,0 }).y();
		dem_obj.loadCenter_latlon[1] = ll2mtr.Reverse({ 0,0 }).x();
		Bbox2 uv_corners(ll2uv.Forward(dem_corners.top_left()), ll2uv.Forward(dem_corners.bottom_right()));
		dem_obj.loadCenter_texSize = 2048 * uv_corners.xspan(); // assumes WED will create a 2k texture - may be wrong ?

		XObj8Write(objAbsPath.c_str(), dem_obj, "Created by WED " WED_VERSION_STRING );
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