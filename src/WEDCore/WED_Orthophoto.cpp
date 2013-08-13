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

#include "WED_Thing.h"
#include "WED_PropertyHelper.h"
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

void	WED_MakeOrthos(IResolver * in_resolver, string resource, WED_Archive * archive)
{		
/*
	WED_Thing *			outer_ring;*/
	//ISelection *	sel = WED_GetSelect(GetResolver());
	//outer_ring			  = WED_Ring::CreateTyped(outer_ring->GetArchive());
	//host->get
	//= GetHost(idx);
		/* really old code, important to think about here
	WED_Thing	*	wrl = WED_GetWorld(in_resolver);
	ISelection * sel = WED_GetSelect(in_resolver);*/

	char buf[256];

	int idx;
	WED_Thing * host = WED_GetCreateHost(in_resolver, 0, idx);
	if (host == NULL) return;

	string cname = string("Create Torthophoto");

	//danger breaks it all! 
	archive->StartCommand(cname.c_str());

	ISelection * sel = WED_GetSelect(in_resolver);
	sel->Clear();

	int is_poly = 1;
	int is_texed = 1;
	
	WED_Thing *	outer_ring;

	outer_ring = WED_Ring::CreateTyped(archive);

	static int n = 0;
	++n;

	WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(archive);
	
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
	//             --------------------------

	dpol->SetName(stripped);
	//sprintf(buf,"Polygon %d outer ring",n);
	outer_ring->SetName(buf);
	sel->Select(dpol);
	dpol->SetResource(stripped);
	dpol->IsOldOrNew();
	
	//--V NOT NEEDED? V--
	/*	double	lonmax=-9.9e9;
	double	latmax=-9.9e9;
	double	lonmin= 9.9e9;
	double	latmin= 9.9e9;
	bool	has_any_dirs = false;
	for(int n = 0; n < pts.size(); ++n)
	{
		lonmax=max(lonmax,pts[n].x());
		lonmin=min(lonmin,pts[n].x());
		latmax=max(latmax,pts[n].y());
		latmin=min(latmin,pts[n].y());
		if(has_dirs[n])
		{
			has_any_dirs=true;

			lonmax=max(lonmax,dirs_hi[n].x());
			lonmin=min(lonmin,dirs_hi[n].x());
			latmax=max(latmax,dirs_hi[n].y());
			latmin=min(latmin,dirs_hi[n].y());

			lonmax=max(lonmax,dirs_lo[n].x());
			lonmin=min(lonmin,dirs_lo[n].x());
			latmax=max(latmax,dirs_lo[n].y());
			latmin=min(latmin,dirs_lo[n].y());
		}
	}

	const float hard_coded_s[4] = { 0.0, 1.0, 1.0, 0.0 };
	const float hard_coded_t[4] = { 0.0, 0.0, 1.0, 1.0 };

	for(int n = 0; n < pts.size(); ++n)
	{
		int idx = is_ccw ? n : pts.size()-n-1;
		
		WED_AirportNode *		anode=NULL;
		WED_GISPoint_Bezier *	bnode=NULL;
		WED_GISPoint *			node=NULL;
		WED_TextureNode *		tnode=NULL;
		WED_TextureBezierNode *	tbnode=NULL;

		else if (is_texed)					node = tnode = WED_TextureNode::CreateTyped(GetArchive());
		else								node = WED_SimpleBoundaryNode::CreateTyped(GetArchive());

		node->SetLocation(gis_Geo,pts[idx]);
		Point2 st(			interp(lonmin,0.0,lonmax,1.0,pts[idx].x()),
							interp(latmin,0.0,latmax,1.0,pts[idx].y()));
		if(pts.size() == 4 && !has_any_dirs)	{st.x_ = hard_coded_s[n];
												 st.y_ = hard_coded_t[n];}
							
		if(tnode)			tnode->SetLocation(gis_UV,st);
		if(tbnode)			tbnode->SetLocation(gis_UV,st);

		if(bnode)
		{
			if (!has_dirs[idx])
			{
				bnode->DeleteHandleHi();
				bnode->DeleteHandleLo();
				if(tbnode)
				{
					tbnode->SetControlHandleLo(gis_UV,st);
					tbnode->SetControlHandleHi(gis_UV,st);
				}
			}
			else
			{
				bnode->SetSplit(has_split[idx]);
				if (is_ccw)
				{
					bnode->SetControlHandleHi(gis_Geo,dirs_hi[idx]);
					bnode->SetControlHandleLo(gis_Geo,dirs_lo[idx]);
					if(tbnode)
					{
						tbnode->SetControlHandleHi(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_hi[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_hi[idx].y())));
						tbnode->SetControlHandleLo(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_lo[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_lo[idx].y())));
					}
				} else {
					bnode->SetControlHandleHi(gis_Geo,dirs_lo[idx]);
					bnode->SetControlHandleLo(gis_Geo,dirs_hi[idx]);
					if(tbnode)
					{
						tbnode->SetControlHandleHi(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_lo[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_lo[idx].y())));
						tbnode->SetControlHandleLo(gis_UV,Point2(
							interp(lonmin,0.0,lonmax,1.0,dirs_hi[idx].x()),
							interp(latmin,0.0,latmax,1.0,dirs_hi[idx].y())));
					}
				}
			}
		}
		node->SetParent(outer_ring, n);
		if(anode)
			anode->SetAttributes(mMarkings.value);
		sprintf(buf,"Node %d",n+1);
		node->SetName(buf);
	}

	*/
	//--^NOT NEEDED?^----

	archive->CommitCommand();
/*
	int idx;
	WED_Thing * host = WED_GetCreateHost(GetResolver(), kIsAirport[mType], idx);;
	if (host == NULL) return;

	string cname = string("Create ") + kCreateCmds[mType];

	GetArchive()->StartCommand(cname.c_str());

	ISelection *	sel = WED_GetSelect(GetResolver());
	if (mType != create_Hole)
	sel->Clear();

	int is_poly = mType != create_Hole && mType != create_String && mType != create_Line;
	int is_texed = 1;
	
	WED_Thing *	outer_ring = WED_Ring::CreateTyped(GetArchive());

	static int n = 0;
	++n;

		if(is_texed)
		{
			WED_DrapedOrthophoto * dpol = WED_DrapedOrthophoto::CreateTyped(GetArchive());
			outer_ring->SetParent(dpol,0);
			dpol->SetParent(host,idx);
			sprintf(buf,"Orthophoto %d",n);
			dpol->SetName(stripped_resource(mResource.value));
			sprintf(buf,"Polygon %d outer ring",n);
			outer_ring->SetName(buf);
			sel->Select(dpol);
			dpol->SetResource(mResource.value);
		}

	/* really old code
	WED_Thing	*	wrl = WED_GetWorld(in_resolver);
	ISelection * sel = WED_GetSelect(in_resolver);
	
	vector<IGISEntity *>	entities;	
	sel->IterateSelectionOr(Iterate_CollectEntities,&entities);
	if(entities.empty()) return;

	bool	skip = false;
	bool	any = false;
//	Polygon_set_2 all;
	//Polygon_set_2 ent;
	for(vector<IGISEntity *>::iterator e = entities.begin(); e != entities.end(); ++e)
	{
		/*if (!WED_PolygonSetForEntity(*e, ent))
			skip = true;
		else
			any = true;
		all.join(ent);
	}
	
	if (/*!all.is_empty()true)
	{
		wrl->StartCommand("Make orthophotos");
		// Ben says: if the user has selected an overlay image itself, we are going to start our hierarchy traversal at THAT IMAGE,
		// ensuring that we only spit out one draped polygon for that overlay.  
		// This prevents the problem where, when two overlays are overlapped, WED creates the unnecessary overlap fragments of both.
		// In the overlap case we have to assume that if the user is selecting images one at a time then he or she wants overlapping square
		// draped polygons.
		// If the selection isn't exactly one overlay image, then start the traversal at the entire world, so that we pick up ANY overlay
		// that coincides with the area the user has selected.
		WED_OverlayImage * sel_over = NULL;
		if(entities.size() == 1)
			sel_over = SAFE_CAST(WED_OverlayImage,entities[0]);
		//int num_made = cut_for_image(sel_over ? sel_over : wrl, 100, wrl, WED_GetResourceMgr(in_resolver));
		if(num_made == 0)
		{
			wrl->AbortCommand();
			DoUserAlert("None of the selected polygons overlap orthophotos.");
		}
		else
		{
			wrl->CommitCommand();
			//if(skip) 
/*				DoUserAlert("Some selected polygons were ignored because they contain bezier curves - you can only cut non-curved orthophotos.");
		//}
	} else 
		DoUserAlert("No orthophotos were created because the selection contains no non-curved polygons.");		*/
	
}