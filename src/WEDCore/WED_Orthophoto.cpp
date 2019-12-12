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
#include "ITexMgr.h"
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
#include "FileUtils.h"
#include "TexUtils.h"
#include "Interpolation.h"

#define KPIXELS 2     // maximum texture size per side in kibi-pixels before splitting up orthos at import into smaller chunks

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


static int largest_pow2(int x, int pow2)  // return largest power-of-2 upto pow2 that is smaller or equal to x
{
    while (pow2 >= 1)
    {
        if(x >= pow2) return pow2;
        pow2 /= 2;
    }
    return 0;
}


void	WED_MakeOrthos(IResolver * inResolver, WED_MapZoomerNew * zoomer)
{
	char * path = GetMultiFilePathFromUser("Please pick image files", "Open", FILE_DIALOG_PICK_IMAGE_OVERLAY);
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
			string base_tex(FILE_get_file_name(path));
			string img_path(path);
			lib->ReducePath(img_path);
				
			if (base_tex.find(" ") == base_tex.npos && img_path[0] != '.' && img_path[0] != DIR_CHAR && img_path[1] != ':')
			{
				WED_Ring * rng0 = WED_RingfromImage(path, arch, zoomer, true);
				if (rng0)
				{
					ITexMgr *	tman = WED_GetTexMgr(inResolver);
					TexRef tref = tman->LookupTexture(img_path.c_str(), false, tex_Compress_Ok|tex_Linear);

					if(tref != NULL)
					{
						int org_x, org_y;

						tman->GetTexInfo(tref, NULL, NULL, NULL, NULL, &org_x, &org_y);
						int kpix_x = ceil(org_x / 1024.0);  // rounded up result
						int kpix_y = ceil(org_y / 1024.0);

						Point2 corner[4];
						for(int i = 0; i < 4; ++i)
							dynamic_cast<WED_TextureBezierNode *>(rng0->GetNthPoint(i))->GetLocation(gis_Geo,corner[i]);

						int x0 = 0, x1 = largest_pow2(kpix_x, KPIXELS);
						while(x0 < kpix_x)
						{
							int y0 = 0, y1 = largest_pow2(kpix_y, KPIXELS);
							while(y0 < kpix_y)
							{
								WED_Ring * rng = dynamic_cast<WED_Ring *>(rng0->Clone());
								
								Bbox2 b;
								b.p1 = Point2(((double) x0)/kpix_x, ((double) y0)/kpix_y );
								b.p2 = Point2(((double) x1)/kpix_x, ((double) y1)/kpix_y );
								
								rng->Rescale(gis_UV, Bbox2(0,0,1,1), b);
								
								Point2 p;
								// lower left
								dynamic_cast<WED_TextureBezierNode *>(rng0->GetNthPoint(0))->GetLocation(gis_Geo,p);
								p.x_ = BilinearInterpolate2d( corner[3].x(), corner[2].x(), corner[0].x(), corner[1].x(),
												b.p1.x(), 1.0-b.p1.y());
								p.y_ = BilinearInterpolate2d( corner[3].y(), corner[2].y(), corner[0].y(), corner[1].y(),
												b.p1.x(), 1.0-b.p1.y());
								dynamic_cast<WED_TextureBezierNode *>(rng->GetNthPoint(0))->SetLocation(gis_Geo,p);
								// lower right
								dynamic_cast<WED_TextureBezierNode *>(rng0->GetNthPoint(1))->GetLocation(gis_Geo,p);
								p.x_ = BilinearInterpolate2d( corner[3].x(), corner[2].x(), corner[0].x(), corner[1].x(),
												b.p2.x(), 1.0-b.p1.y());
								p.y_ = BilinearInterpolate2d( corner[3].y(), corner[2].y(), corner[0].y(), corner[1].y(),
												b.p2.x(), 1.0-b.p1.y());
								dynamic_cast<WED_TextureBezierNode *>(rng->GetNthPoint(1))->SetLocation(gis_Geo,p);
								// upper right
								dynamic_cast<WED_TextureBezierNode *>(rng0->GetNthPoint(2))->GetLocation(gis_Geo,p);
								p.x_ = BilinearInterpolate2d( corner[3].x(), corner[2].x(), corner[0].x(), corner[1].x(),
												b.p2.x(), 1.0-b.p2.y());
								p.y_ = BilinearInterpolate2d( corner[3].y(), corner[2].y(), corner[0].y(), corner[1].y(),
												b.p2.x(), 1.0-b.p2.y());
								dynamic_cast<WED_TextureBezierNode *>(rng->GetNthPoint(2))->SetLocation(gis_Geo,p);
								// upper left
								dynamic_cast<WED_TextureBezierNode *>(rng0->GetNthPoint(3))->GetLocation(gis_Geo,p);
								p.x_ = BilinearInterpolate2d( corner[3].x(), corner[2].x(), corner[0].x(), corner[1].x(),
												b.p1.x(), 1.0-b.p2.y());
								p.y_ = BilinearInterpolate2d( corner[3].y(), corner[2].y(), corner[0].y(), corner[1].y(),
												b.p1.x(), 1.0-b.p2.y());
								dynamic_cast<WED_TextureBezierNode *>(rng->GetNthPoint(3))->SetLocation(gis_Geo,p);

								WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(arch);
								rng->SetParent(dpol,0);
								dpol->SetParent(wrl,0);
								sel->Insert(dpol);
								dpol->SetResource(img_path);
								dpol->SetSubTexture(b);             // do we want that ? Yes, so one can at least punch holes into them gracefully.

								string::size_type pos = base_tex.find_last_of('.');
								char s[10] = ""; if(x0 > 0 || y0 > 0) snprintf(s,10,"+%dk+%dk", x0, y0);
								dpol->SetName(base_tex.substr(0,pos) + s + base_tex.substr(pos));

								y0 = y1; y1 += largest_pow2(kpix_y-y1, KPIXELS);
							}
							x0 = x1; x1 += largest_pow2(kpix_x-x1, KPIXELS);
						}
					}
					// we left rng0 untouched, so all Rescale() can refer to the unmodified original
					for(int i = 0; i < 4; ++i)
						rng0->GetNthChild(i)->Delete();
					rng0->Delete();
				}
			}
			else
			{
				char msg[200]; snprintf(msg,200,"Orthoimage name/path not acceptable:\n\n%s\n\n"
					    "Spaces are not allowed and location must be inside the scenery directory.", img_path.c_str());
				DoUserAlert(msg);
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
