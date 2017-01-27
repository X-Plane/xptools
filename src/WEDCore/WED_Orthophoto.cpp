/*
 *  WED_Orthophoto.cpp
 *  SceneryTools
 *
 *  Created by bsupnik on 5/26/09.
 *  Copyright 2009 Laminar Research. All rights reserved.
 *
 */

#include "WED_Orthophoto.h"
#include "IGIS.h"
#include "WED_ToolUtils.h"
#include "WED_Ring.h"
#include "WED_TextureNode.h"
#include "WED_TextureBezierNode.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_ResourceMgr.h"
#include "PlatformUtils.h"
#include "BitmapUtils.h"
#include "WED_MapZoomerNew.h"
#include "WED_UIDefs.h"
#include "ILibrarian.h"
#include "GISUtils.h"
#include "DEMIO.h"

WED_Ring * WED_RingfromImage(char * path, WED_Archive * arch, WED_MapZoomerNew * zoomer, bool use_bezier)
{
	Point2	coords[4];
	double c[8];
		
	ImageInfo	inf;
	int has_geo = 0;
	int align = dem_want_Area;
		
	int res = MakeSupportedType(path, &inf);
	if(res != 0)
	{
		DoUserAlert("Unable to open image file.");
		return NULL;
	}

	switch(GetSupportedType(path))
	{
	#if USE_GEOJPEG2K
	case WED_JP2K:
		if(FetchTIFFCornersWithJP2K(path,c,align))
		{
			coords[0] = Point2(c[0],c[1]);
			coords[1] = Point2(c[2],c[3]);
			coords[3] = Point2(c[4],c[5]);
			coords[2] = Point2(c[6],c[7]);
			has_geo = 1;
		}
		break;
	#endif
	case WED_TIF:
		if (FetchTIFFCorners(path, c, align))
		{
			if (c[1] < c[5])
			{
				// SW, SE, NW, NE from tiff, but we are making a CCW wound polygon, i.e. SW SE NE NW
				coords[0] = Point2(c[0],c[1]);
				coords[1] = Point2(c[2],c[3]);
				coords[3] = Point2(c[4],c[5]);
				coords[2] = Point2(c[6],c[7]);
			}
			else
			{
				// jpg compressed files have the origin at the bottom, not the top. So their encoding is
				// NW, NE, SW, SE, but we are making a CCW wound polygon, i.e. need it to be SW SE NE NW
				coords[0] = Point2(c[4],c[5]);
				coords[1] = Point2(c[6],c[7]);
				coords[3] = Point2(c[0],c[1]);
				coords[2] = Point2(c[2],c[3]);
			}
			has_geo = 1;
		}
		break;
	}

	if (!has_geo)
	{
		double	nn,ss,ee,ww;
		zoomer->GetPixelBounds(ww,ss,ee,nn);

		Point2 center((ee+ww)*0.5,(nn+ss)*0.5);
		
		double grow_x = 0.5*(ee-ww)/((double) inf.width);
		double grow_y = 0.5*(nn-ss)/((double) inf.height);

		double pix_w, pix_h;

		if (grow_x < grow_y) { pix_w = grow_x * (double) inf.width;	pix_h = grow_x * (double) inf.height; }
		else				 { pix_w = grow_y * (double) inf.width;	pix_h = grow_y * (double) inf.height; }

		coords[0] = zoomer->PixelToLL(center + Vector2(-pix_w,-pix_h));
		coords[1] = zoomer->PixelToLL(center + Vector2( pix_w,-pix_h));
		coords[2] = zoomer->PixelToLL(center + Vector2( pix_w,+pix_h));
		coords[3] = zoomer->PixelToLL(center + Vector2(-pix_w,+pix_h));
	}
	DestroyBitmap(&inf);

	WED_Ring * rng = WED_Ring::CreateTyped(arch);
	rng->SetName("Image Boundary");
	for (int i=0; i<4; ++i)
	{
		WED_GISPoint * tbn;
		char s[12]; 
		if (use_bezier)
			tbn = WED_TextureBezierNode::CreateTyped(arch);
		else
			tbn = WED_TextureNode::CreateTyped(arch);
		tbn->SetParent(rng,i);
		sprintf(s,"Corner %d",i+1);
		tbn->SetName(s);
		tbn->SetLocation(gis_Geo,coords[i]);
		tbn->SetLocation(gis_UV,Point2(i==1||i==2,i>1));     // Redrape() will also do that
	}
	return rng;
}

void	WED_MakeOrthos(IResolver * inResolver, WED_MapZoomerNew * zoomer)
{		
	char * path = GetMultiFilePathFromUser("Please pick an image file", "Open", FILE_DIALOG_PICK_IMAGE_OVERLAY);
	if(path)
	{
		WED_Thing *    wrl = WED_GetWorld(inResolver);
		WED_Archive * arch = wrl->GetArchive();
		ISelection *   sel = WED_GetSelect(inResolver);
		ILibrarian *   lib = WED_GetLibrarian(inResolver);
		char * free_me = path;

		wrl->StartOperation("Create Ortho Image");
		sel->Clear();
	
		while(*path)
		{
			string base_tex(path);
			string::size_type pp = base_tex.find_last_of("/:\\");
			if(pp != base_tex.npos)	
				base_tex.erase(0,pp+1);
			if (base_tex.find(" ") == base_tex.npos)
			{
				WED_Ring * rng = WED_RingfromImage(path, arch, zoomer, true);
				if (rng)
				{
					WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(arch);
					rng->SetParent(dpol,0);
					dpol->SetParent(wrl,0);
					sel->Select(dpol);

					string img_path(path);
					lib->ReducePath(img_path);
					dpol->SetResource(img_path);
					dpol->SetName(base_tex);
				}
			}
			else
			{
				DoUserAlert("XP does not support spaces in texture names.");
			}
			path += strlen(path) + 1;
		}
		
		if(sel->GetSelectionCount() == 0)
			wrl->AbortOperation();
		else
			wrl->CommitOperation();
		free(free_me);
	}
}