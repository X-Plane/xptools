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
#include <sstream>

#define KPIXELS 2     // maximum texture size per side in kibi-pixels before splitting up orthos at import into smaller chunks

WED_Ring * WED_RingfromImage(char * path, WED_Archive * arch, WED_MapZoomerNew * zoomer, bool use_bezier, vector<Point2> * gcp)
{
	Point2	coords[4];
	int has_geo = 0;

	if(GetSupportedType(path) == WED_TIF)  // suffix based decision
	{
		double c[8];
		int pos =  dem_want_Area;
		if (FetchTIFFCorners(path, c, pos, gcp))
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
	}
	
	if (!has_geo)
	{
		ImageInfo	inf;
		double pix_w = 1.0;
		double pix_h = 1.0;
		if (!LoadBitmapFromAnyFile(path, &inf))  // just to get width + height ...
		{
			pix_w = inf.width;
			pix_h = inf.height;
			DestroyBitmap(&inf);
		}

		double	nn, ss, ee, ww;
		zoomer->GetPixelBounds(ww, ss, ee, nn);
		Point2 center((ee + ww)*0.5, (nn + ss)*0.5);

		double grow_x = 0.5*(ee - ww) / pix_w;
		double grow_y = 0.5*(nn - ss) / pix_h;

		if (grow_x < grow_y) { pix_w *= grow_x; pix_h *= grow_x; }
		else                 { pix_w *= grow_y; pix_h *= grow_y; }

		coords[0] = zoomer->PixelToLL(center + Vector2(-pix_w, -pix_h));
		coords[1] = zoomer->PixelToLL(center + Vector2(pix_w, -pix_h));
		coords[2] = zoomer->PixelToLL(center + Vector2(pix_w, +pix_h));
		coords[3] = zoomer->PixelToLL(center + Vector2(-pix_w, +pix_h));
	}

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

static Point2 interpol_LonLat(double UV_x, double UV_y, double dUV, const Point2 * LonLat, int y_stride)
{
	int A, B, C, D;

	if(y_stride > 1)
	{
		int xi0 = floor(UV_x / dUV);
		int xi1 = ceil (UV_x / dUV);
		int yi0 = floor(UV_y / dUV);
		int yi1 = ceil (UV_y / dUV);
		
		UV_x = UV_x / dUV - xi0;
		UV_y = UV_y / dUV - yi0;
		
		A = xi0 + yi1 * y_stride;
		B = xi1 + yi1 * y_stride;
		C = xi0 + yi0 * y_stride;
		D = xi1 + yi0 * y_stride;
	}
	else
	{
		A = 3;
		B = 2;
		C = 0;
		D = 1;
	}

	return Point2(
		BilinearInterpolate2d( LonLat[A].x(), LonLat[B].x(), LonLat[C].x(), LonLat[D].x(),
						UV_x, 1.0 - UV_y),
		BilinearInterpolate2d( LonLat[A].y(), LonLat[B].y(), LonLat[C].y(), LonLat[D].y(),
						UV_x, 1.0 - UV_y)
				 );
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

		int last_answer = -1;  // on multi-file import, only ask _once_ to use full or reduced size import
		while(*path)
		{
			string base_tex(FILE_get_file_name(path));
			string img_path(path);
			lib->ReducePath(img_path);
				
			if (base_tex.find(" ") == base_tex.npos && img_path[0] != '.' && img_path[0] != DIR_CHAR && img_path[1] != ':')
			{
				vector<Point2> gcp;
				WED_Ring * rng0 = WED_RingfromImage(path, arch, zoomer, true, &gcp);
				int gcp_divs = intround(sqrt(gcp.size()));
				if (rng0)
				{
					ITexMgr *	tman = WED_GetTexMgr(inResolver);
					TexRef tref = tman->LookupTexture(img_path.c_str(), false, tex_Compress_Ok|tex_Linear);

					if(tref != NULL)
					{
						int orig_x, orig_y;
						tman->GetTexInfo(tref, NULL, NULL, NULL, NULL, &orig_x, &orig_y);
						int kpix_x = ceil(orig_x / 1024.0);    // round up, never loose resolution
						int kpix_y = ceil(orig_y / 1024.0);
						
						Point2 corner[4];
						for(int i = 0; i < 4; ++i)
							dynamic_cast<WED_TextureBezierNode *>(rng0->GetNthPoint(i))->GetLocation(gis_Geo,corner[i]);
							
						int kopt_x = ceil(orig_x / 1600.0);    // typ 70%, min 64% of original resolution
						int kopt_y = ceil(orig_y / 1600.0);
						if(kopt_x < kpix_x || kopt_y < kpix_y)
						{
							if(last_answer < 0)
							{
								stringstream msg;
								auto w = (kpix_x * 1024 == orig_x && kpix_y * 1024 == orig_y) ? "Exact size" : "Upscaled size";
								msg << w << " " << kpix_x << " x " << kpix_x << " Kpix, ";
								msg.precision(1);
								msg << std::fixed;
								msg << LonLatDistMeters(corner[0], corner[1]) / kpix_x / 1024 << " x " << LonLatDistMeters(corner[0], corner[3]) / kpix_y / 1024<< " m/pix\n";
								msg << "Total DDS texture size ~" << intround(kpix_x * kpix_y * 0.7) << " MB\n";
								msg << "\n";
								msg << "Optimized size " << kopt_x << " x " << kopt_y << " Kpix, ";
								msg << LonLatDistMeters(corner[0], corner[1]) / kopt_x / 1024 << " x " << LonLatDistMeters(corner[0], corner[3]) / kopt_y / 1024 << " m/pix\n";
								msg << "Total DDS texture size ~" << intround(kopt_x * kopt_y * 0.7) << " MB\n";
								
								last_answer = ConfirmMessage(msg.str().c_str(), "Optimized size", w);   // returns 0 or 1
							}
							if(last_answer > 0)
							{
								kpix_x = kopt_x;
								kpix_y = kopt_y;
							}
						}
						
						int x0 = 0, x1 = largest_pow2(kpix_x, KPIXELS);
						while(x0 < kpix_x)
						{
							int y0 = 0, y1 = largest_pow2(kpix_y, KPIXELS);
							while(y0 < kpix_y)
							{
								auto rng = dynamic_cast<WED_Ring *>(rng0->Clone());
								auto igp = dynamic_cast<IGISPointSequence *>(rng);
								
								Bbox2 b;
								b.p1 = Point2(((double) x0)/kpix_x, ((double) y0)/kpix_y );
								b.p2 = Point2(((double) x1)/kpix_x, ((double) y1)/kpix_y );
								rng->Rescale(gis_UV, Bbox2(0,0,1,1), b);
								
								double df = 0.0;
								Point2 * pts = corner;
								if(gcp_divs > 1)                  // if map projection info is available for the underlying source image 
								{
									df = 1.0 / (gcp_divs - 1);
									pts = gcp.data();             // use ground control points, i.e. place the subtextures to propper projected/warped locations
								}

								Point2 p;
								// lower left
								p = interpol_LonLat(b.p1.x(), b.p1.y(), df, pts, gcp_divs);
								igp->GetNthPoint(0)->SetLocation(gis_Geo,p);
								// lower right
								p = interpol_LonLat(b.p2.x(), b.p1.y(), df, pts, gcp_divs);
								igp->GetNthPoint(1)->SetLocation(gis_Geo,p);
								// upper right
								p = interpol_LonLat(b.p2.x(), b.p2.y(), df, pts, gcp_divs);
								igp->GetNthPoint(2)->SetLocation(gis_Geo,p);
								// upper left
								p = interpol_LonLat(b.p1.x(), b.p2.y(), df, pts, gcp_divs);
								igp->GetNthPoint(3)->SetLocation(gis_Geo,p);

								WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(arch);
								rng->SetParent(dpol,0);
								dpol->SetParent(wrl,0);
								sel->Insert(dpol);
								dpol->SetResource(img_path);
								dpol->SetSubTexture(b);             // Turn on auto-redraping. So one can punch holes into them w/o distortion.

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
					    "Spaces are not allowed and location must be inside this sceneries directory.", img_path.c_str());
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
