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

void	WED_MakeOrthos(IResolver * in_resolver)
{
/*
	WED_Thing	*	wrl = WED_GetWorld(in_resolver);
	ISelection * sel = WED_GetSelect(in_resolver);
	
	vector<IGISEntity *>	entities;	
	sel->IterateSelectionOr(Iterate_CollectEntities,&entities);
	if(entities.empty()) return;

	bool	skip = false;
	bool	any = false;
	Polygon_set_2 all, ent;
	for(vector<IGISEntity *>::iterator e = entities.begin(); e != entities.end(); ++e)
	{
		if (!WED_PolygonSetForEntity(*e, ent))
			skip = true;
		else
			any = true;
		all.join(ent);
	}
	
	if (!all.is_empty())
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
		int num_made = cut_for_image(sel_over ? sel_over : wrl, all, wrl, WED_GetResourceMgr(in_resolver));
		if(num_made == 0)
		{
			wrl->AbortCommand();
			DoUserAlert("None of the selected polygons overlap orthophotos.");
		}
		else
		{
			wrl->CommitCommand();
			if(skip) 
				DoUserAlert("Some selected polygons were ignored because they contain bezier curves - you can only cut non-curved orthophotos.");
		}
	} else 
		DoUserAlert("No orthophotos were created because the selection contains no non-curved polygons.");
*/		
}
