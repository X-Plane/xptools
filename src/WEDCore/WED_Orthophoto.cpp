/*
 *  WED_Orthophoto.cpp
 *  SceneryTools
 *
 *  Created by bsupnik on 5/26/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "WED_Orthophoto.h"
#include "IGIS.h"
#include "DEMTables.h"
#include "WED_ToolUtils.h"
#include "WED_OverlayImage.h"
#include "WED_Ring.h"
#include "WED_TextureNode.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_ResourceMgr.h"
#include "PlatformUtils.h"
#include "BitmapUtils.h"
#include "WED_Thing.h"
#include "WED_PropertyHelper.h"
#include "WED_MapZoomerNew.h"
#include "WED_UIDefs.h"
#include "ILibrarian.h"
#include "GISUtils.h"
#include "DEMIO.h"
#if 0

#define USE_CGAL_POLYGONS 1
#include "WED_GISUtils.h"

#include "PlatformUtils.h"
#include "WED_DrawUtils.h"
#include "WED_ObjPlacement.h"

static void	cgal2ben(const Polygon_2& cgal, Polygon2& ben)
{
	ben.resize(cgal.size());
	for(int n = 0; n < cgal.size(); ++n)
		ben[n] = cgal2ben(cgal.vertex(n));
}

static void	cgal2ben(const Polygon_with_holes_2& cgal, vector<Polygon2>& ben)
{
	ben.push_back(Polygon2());
	if(!cgal.is_unbounded())	cgal2ben(cgal.outer_boundary(),ben.back());
	for(Polygon_with_holes_2::Hole_const_iterator h = cgal.holes_begin(); h != cgal.holes_end(); ++h)
	{
		ben.push_back(Polygon2());
		cgal2ben(*h,ben.back());
	}
}

static void add_pol_ring(WED_OverlayImage * image, const Polygon2& loc, const Polygon2& uv, WED_Thing * parent, bool is_outer)
{
	WED_Ring * r = WED_Ring::CreateTyped(parent->GetArchive());
	r->SetParent(parent, parent->CountChildren());
	r->SetName("ring");
	for(int n = 0; n < loc.size(); ++n)
	{
		WED_TextureNode * tnode = WED_TextureNode::CreateTyped(r->GetArchive());
		tnode->SetParent(r, n);
		tnode->SetName("node");
		tnode->SetLocation(gis_Geo,loc[n]);
		tnode->SetLocation(gis_UV,uv[n]);
	}	
}

static void make_one_pol(WED_OverlayImage * image, const vector<Polygon2>& area, WED_Thing * wrl, WED_ResourceMgr * rmgr)
{
	vector<Polygon2>	uv_poly;	
	UVMap_t	uv_map;
	
	WED_MakeUVMap((IGISQuad *) image, uv_map);
	WED_MapPolygonWithHoles(uv_map, area, uv_poly);
	
	string img, res;
	image->GetImage(img);
	res = img;
	string::size_type pp = res.find_last_of(".");
	if(pp != res.npos) res.erase(pp);
	res += ".pol";

	pol_info_t	info;
	if(!rmgr->GetPol(res,info))
	{
		info.base_tex = img;
		pp = info.base_tex.find_last_of("/:\\");
		if(pp != info.base_tex.npos)
			info.base_tex.erase(0,pp+1);
		info.proj_s = info.proj_t = 25.0;
		info.kill_alpha = false;
		info.wrap = false;
		rmgr->MakePol(res,info);
//		#if !DEV
//			#error add load center
//		#endif
	}
	
	WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(wrl->GetArchive());
	dpol->SetParent(wrl,wrl->CountChildren());
	dpol->SetName(img);
	dpol->SetResource(res);

	DebugAssert(!area.empty() && !area[0].empty());

	for(vector<Polygon2>::const_iterator a = area.begin(), b = uv_poly.begin(); a != area.end(); ++a, ++b)
		add_pol_ring(image,*a,*b, dpol, false);
}

static int cut_for_image(WED_Thing * ent, const Polygon_set_2& area, WED_Thing * wrl, WED_ResourceMgr * rmgr)
{
	int c = 0;
	WED_OverlayImage * ref_image = SAFE_CAST(WED_OverlayImage,ent);
	if(ref_image)
	{
		Polygon_set_2	clip_ref_image;	
		WED_PolygonSetForEntity(ref_image,clip_ref_image);
		
		Polygon_set_2	real_area(area);
		
		real_area.intersection(clip_ref_image);

		vector<Polygon_with_holes_2>	draped_orthos;
		real_area.polygons_with_holes(back_insert_iterator<vector<Polygon_with_holes_2> >(draped_orthos));
		
		for(vector<Polygon_with_holes_2>::iterator d = draped_orthos.begin(); d != draped_orthos.end(); ++d)
		{
			vector<Polygon2>	pwh;
			cgal2ben(*d, pwh);
		
			make_one_pol(ref_image, pwh, wrl, rmgr);		
			++c;
		}		
	}
	for(int n = 0; n < ent->CountChildren(); ++n)
		c += cut_for_image(ent->GetNthChild(n), area,wrl, rmgr);
	return c;
}

#endif

void	WED_MakeOrthos(IResolver * in_Resolver, WED_MapZoomerNew * zoomer/*, WED_Archive * archive*/)
{		
	/* From CreatePolyGone
	char buf[256];

	int idx;
	WED_Thing * host = WED_GetCreateHost(in_resolver, 0, idx);
	if (host == NULL) return;

	string cname = string("Create Torthophoto");

	//danger breaks it all! 
	archive->StartCommand(cname.c_str());

	ISelection * sel = WED_GetSelect(in_resolver);
	sel->Clear();
	
	//WED_Thing *	outer_ring;

	//outer_ring = WED_Ring::CreateTyped(archive);

	//static int n = 0;
	//++n;

	//WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(archive);
	
	outer_ring->SetParent(dpol,0);
		
	dpol->SetParent(host,idx);
	
	sprintf(buf,"Orthophoto %d",n);
		
	//stripped_resource---------------------
	string stripped(resource);
	string::size_type p = stripped.find_last_of("/\\:");

	if(p != stripped.npos)
	{
		stripped.erase(0,p+1);
	}

	dpol->SetName(stripped);
	//sprintf(buf,"Polygon %d outer ring",n);
	outer_ring->SetName(buf);
	sel->Select(dpol);
	dpol->SetResource(stripped);
	dpol->IsNew();
	*/

	//From GroupCommands
	char buf[1024];
	if (GetFilePathFromUser(getFile_OpenImages, "Please pick an image file", "Open", FILE_DIALOG_PICK_IMAGE_OVERLAY, buf, sizeof(buf)))
	{

		Point2	coords[4];
		double c[8];

		{
			ImageInfo	inf;
			int tif_ok=-1;

			if (CreateBitmapFromDDS(buf,&inf) != 0)
			if (CreateBitmapFromPNG(buf,&inf,false, GAMMA_SRGB) != 0)
#if USE_JPEG
			if (CreateBitmapFromJPEG(buf,&inf) != 0)
#endif
#if USE_TIF
			if ((tif_ok=CreateBitmapFromTIF(buf,&inf)) != 0)
#endif
			if (CreateBitmapFromFile(buf,&inf) != 0)
			{
				#if ERROR_CHECK
				better reporting
				#endif
				DoUserAlert("Unable to open image file.");
				return;
			}

			double	nn,ss,ee,ww;
			zoomer->GetPixelBounds(ww,ss,ee,nn);

			Point2 center((ee+ww)*0.5,(nn+ss)*0.5);

			double grow_x = 0.5*(ee-ww)/((double) inf.width);
			double grow_y = 0.5*(nn-ss)/((double) inf.height);

			double pix_w, pix_h;

			if (grow_x < grow_y) { pix_w = grow_x * (double) inf.width;	pix_h = grow_x * (double) inf.height; }
			else				 { pix_w = grow_y * (double) inf.width;	pix_h = grow_y * (double) inf.height; }

			coords[0] = zoomer->PixelToLL(center + Vector2( pix_w,-pix_h));
			coords[1] = zoomer->PixelToLL(center + Vector2( pix_w,+pix_h));
			coords[2] = zoomer->PixelToLL(center + Vector2(-pix_w,+pix_h));
			coords[3] = zoomer->PixelToLL(center + Vector2(-pix_w,-pix_h));

			DestroyBitmap(&inf);
			//uncomment when dem_want_Area and FetchTIFF are fixing
			int align = dem_want_Area;
			if (tif_ok==0)
			if (FetchTIFFCorners(buf, c, align))
			{
			// SW, SE, NW, NE from tiff, but SE NE NW SW internally
				coords[3].x_ = c[0];
				coords[3].y_ = c[1];
				coords[0].x_ = c[2];
				coords[0].y_ = c[3];
				coords[2].x_ = c[4];
				coords[2].y_ = c[5];
				coords[1].x_ = c[6];
				coords[1].y_ = c[7];
			}
			
			WED_Thing * wrl = WED_GetWorld(in_Resolver);
			ISelection * sel = WED_GetSelect(in_Resolver);

			wrl->StartOperation("Create Tortho Image");
			sel->Clear();
			WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(wrl->GetArchive());

			WED_Ring * rng = WED_Ring::CreateTyped(wrl->GetArchive());
			WED_TextureNode *  p1 = WED_TextureNode::CreateTyped(wrl->GetArchive());
			WED_TextureNode *  p2 = WED_TextureNode::CreateTyped(wrl->GetArchive());
			WED_TextureNode *  p3 = WED_TextureNode::CreateTyped(wrl->GetArchive());
			WED_TextureNode *  p4 = WED_TextureNode::CreateTyped(wrl->GetArchive());
			
			p4->SetParent(rng,0);
			p3->SetParent(rng,1);
			p2->SetParent(rng,2);
			p1->SetParent(rng,3);
			
			rng->SetParent(dpol,0);
			dpol->SetParent(wrl,0);
			sel->Select(dpol);

			p1->SetLocation(gis_Geo,coords[3]);
			p2->SetLocation(gis_Geo,coords[2]);
			p3->SetLocation(gis_Geo,coords[1]);
			p4->SetLocation(gis_Geo,coords[0]);


			string img_path(buf);
			WED_GetLibrarian(in_Resolver)->ReducePath(img_path);

			dpol->SetResource(img_path);

			p1->SetName("Corner 1");
			p1->SetName("Corner 2");
			p1->SetName("Corner 3");
			p1->SetName("Corner 4");
			rng->SetName("Image Boundary");
			const char * p = buf;
			const char * n = buf;
			while(*p) { if (*p == '/' || *p == ':' || *p == '\\') n = p+1; ++p; }
			
			dpol->SetName(n);

			p1->SetLocation(gis_UV,Point2(0,0));
			p2->SetLocation(gis_UV,Point2(0,1));
			p3->SetLocation(gis_UV,Point2(1,1));
			p4->SetLocation(gis_UV,Point2(1,0));

			wrl->CommitOperation();
		}
	}
}