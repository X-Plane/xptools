/*
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_DSFImport.h"
#include "DSFLib.h"
#include "WED_Group.h"
#include "WED_ExclusionZone.h"
#include "WED_EnumSystem.h"
#include "WED_ObjPlacement.h"
#include "WED_ToolUtils.h"
#include "PlatformUtils.h"
#include "WED_UIDefs.h"
#include "WED_SimpleBoundaryNode.h"

#include "WED_FacadePlacement.h"
#include "WED_ForestPlacement.h"
#include "WED_StringPlacement.h"
#include "WED_LinePlacement.h"
#include "WED_PolygonPlacement.h"
#include "WED_DrapedOrthophoto.h"
#include "WED_Ring.h"

#include "WED_TextureBezierNode.h"
#include "WED_TextureNode.h"
#include "WED_SimpleBezierBoundaryNode.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_ForestRing.h"
#include "WED_FacadeRing.h"
#include "WED_FacadeNode.h"
#include "DSF2Text.h"
#include "WED_GISUtils.h"
#include "STLUtils.h"
#include "WED_AptIE.h"
#include "WED_Airport.h"

static void debug_it(const vector<BezierPoint2>& pts)
{
	for(int n = 0; n < pts.size(); ++n)
		printf("%d) %lf,%lf (%lf,%lf/%lf,%lf)\n",
			n,
			pts[n].pt.x(),
			pts[n].pt.y(),
			pts[n].lo.x() - pts[n].pt.x(),
			pts[n].lo.y() - pts[n].pt.y(),
			pts[n].hi.x() - pts[n].pt.x(),
			pts[n].hi.y() - pts[n].pt.y());
}

inline bool end_match(const char * str, const char * suf)
{
	int ls = strlen(suf);
	int lstr = strlen(str);
	if(lstr > ls)
	{
		return strcmp(str+lstr-ls,suf) == 0;
	}
	else
	return false;
}

class	DSF_Importer {
public:

	DSF_Importer() { for(int n = 0; n < 7; ++n) req_level_obj[n] = req_level_agp[n] = req_level_fac[n] = -1; }

	int					req_level_obj[7];
	int					req_level_agp[7];
	int					req_level_fac[7];

	vector<string>		obj_table;
	vector<string>		pol_table;

	WED_Thing *			parent;
	WED_Archive *		archive;

	vector<BezierPoint2>pts,uvs;
	vector<int>			walls;
	WED_Thing *			ring;
	WED_Thing *			poly;
	bool				want_uv;
	bool				want_bezier;
	bool				want_wall;

	int GetShowForFacID(int id)
	{
		for(int l = 1; l <= 6; ++l)
		if(req_level_fac[l] != -1)
		if(req_level_fac[l] <= id)
			return l;
		return 6;
	}

	int GetShowForObjID(int id)
	{
		for(int l = 1; l <= 6; ++l)
		if(req_level_obj[l] != -1)
		if(req_level_obj[l] <= id)
			return l;
		return 6;
	}

	void handle_req_obj(const char * str)
	{
		int level, id;		
		if(sscanf(str,"%d/%d",&level,&id) == 2)
		for(int l = level; l <= 6; ++l)
		if(req_level_obj[l] == -1 || req_level_obj[l] > id)
			req_level_obj[l] = id;
	}

	void handle_req_agp(const char * str)
	{
		int level, id;		
		if(sscanf(str,"%d/%d",&level,&id) == 2)
		for(int l = level; l <= 6; ++l)
		if(req_level_agp[l] == -1 || req_level_agp[l] > id)
			req_level_agp[l] = id;
	}

	void handle_req_fac(const char * str)
	{
		int level, id;		
		if(sscanf(str,"%d/%d",&level,&id) == 2)
		for(int l = level; l <= 6; ++l)
		if(req_level_fac[l] == -1 || req_level_fac[l] > id)
			req_level_fac[l] = id;
	}

	void make_exclusion(const char * ex, int k)
	{
		float b[4];
		if(sscanf(ex,"%f/%f/%f/%f",b,b+1,b+2,b+3)==4)
		{
			WED_ExclusionZone * z = WED_ExclusionZone::CreateTyped(archive);
			z->SetName("Exclusion Zone");
			z->SetParent(parent,parent->CountChildren());
			set<int> s;
			s.insert(k);
			z->SetExclusions(s);

			WED_SimpleBoundaryNode * p1 =WED_SimpleBoundaryNode::CreateTyped(archive);
			WED_SimpleBoundaryNode * p2 =WED_SimpleBoundaryNode::CreateTyped(archive);
			p1->SetParent(z,0);
			p2->SetParent(z,1);
			p1->SetName("e1");
			p2->SetName("e2");
			p1->SetLocation(gis_Geo,Point2(b[0],b[1]));
			p2->SetLocation(gis_Geo,Point2(b[2],b[3]));
		}
	}

	static bool NextPass(int finished_pass_index, void * inRef)
	{
		return true;
	}

	static void	AcceptTerrainDef(const char * inPartialPath, void * inRef)
	{
	}

	static void	AcceptObjectDef(const char * inPartialPath, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->obj_table.push_back(inPartialPath);
	}

	static void	AcceptPolygonDef(const char * inPartialPath, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->pol_table.push_back(inPartialPath);
	}

	static void	AcceptNetworkDef(const char * inPartialPath, void * inRef)
	{
	}
	
	static void AcceptRasterDef(const char * inPartalPath, void * inRef)
	{
	}

	static void	AcceptProperty(const char * inProp, const char * inValue, void * inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;

		if(strcmp(inProp, "sim/exclude_obj") == 0)	me->make_exclusion(inValue, exclude_Obj);
		if(strcmp(inProp, "sim/exclude_fac") == 0)	me->make_exclusion(inValue, exclude_Fac);
		if(strcmp(inProp, "sim/exclude_for") == 0)	me->make_exclusion(inValue, exclude_For);
		if(strcmp(inProp, "sim/exclude_bch") == 0)	me->make_exclusion(inValue, exclude_Bch);
		if(strcmp(inProp, "sim/exclude_net") == 0)	me->make_exclusion(inValue, exclude_Net);
		if(strcmp(inProp, "sim/exclude_lin") == 0)	me->make_exclusion(inValue, exclude_Lin);
		if(strcmp(inProp, "sim/exclude_pol") == 0)	me->make_exclusion(inValue, exclude_Pol);
		if(strcmp(inProp, "sim/exclude_str") == 0)	me->make_exclusion(inValue, exclude_Str);
		
		if(strcmp(inProp, "sim/require_object") == 0)	me->handle_req_obj(inValue);
		if(strcmp(inProp, "sim/require_agpoint") == 0)	me->handle_req_agp(inValue);
		if(strcmp(inProp, "sim/require_facade") == 0)	me->handle_req_fac(inValue);
	}

	static void	BeginPatch(
					unsigned int	inTerrainType,
					double 			inNearLOD,
					double 			inFarLOD,
					unsigned char	inFlags,
					int				inCoordDepth,
					void *			inRef)
	{
	}

	static void	BeginPrimitive(
					int				inType,
					void *			inRef)
	{
	}

	static void	AddPatchVertex(
					double			inCoordinates[],
					void *			inRef)
	{
	}

	static void	EndPrimitive(
					void *			inRef)
	{
	}

	static void	EndPatch(
					void *			inRef)
	{
	}

	static void	AddObject(
					unsigned int	inObjectType,
					double			inCoordinates[2],
					double			inRotation,
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		WED_ObjPlacement * obj = WED_ObjPlacement::CreateTyped(me->archive);
		obj->SetResource(me->obj_table[inObjectType]);
		obj->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
		#if AIRPORT_ROUTING
		obj->SetDefaultMSL();
		#endif
		obj->SetHeading(inRotation);
		obj->SetName(me->obj_table[inObjectType]);
		obj->SetParent(me->parent,me->parent->CountChildren());
		obj->SetShowLevel(me->GetShowForObjID(inObjectType));
	}
	static void	AddObjectMSL(
					unsigned int	inObjectType,
					double			inCoordinates[3],
					double			inRotation,
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		WED_ObjPlacement * obj = WED_ObjPlacement::CreateTyped(me->archive);
		obj->SetResource(me->obj_table[inObjectType]);
		obj->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
		#if AIRPORT_ROUTING
		obj->SetCustomMSL(inCoordinates[2]);
		#endif
		obj->SetHeading(inRotation);
		obj->SetName(me->obj_table[inObjectType]);
		obj->SetParent(me->parent,me->parent->CountChildren());
		obj->SetShowLevel(me->GetShowForObjID(inObjectType));
	}

	static void	BeginSegment(
					unsigned int	inNetworkType,
					unsigned int	inNetworkSubtype,
					unsigned int	inStartNodeID,
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef)
	{
	}

	static void	AddSegmentShapePoint(
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef)
	{
	}

	static void	EndSegment(
					unsigned int	inEndNodeID,
					double			inCoordinates[],
					bool			inCurved,
					void *			inRef)
	{
	}

	static void	BeginPolygon(
					unsigned int	inPolygonType,
					unsigned short	inParam,
					int				inCoordDepth,
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		string r  = me->pol_table[inPolygonType];
		if(end_match(r.c_str(),".fac"))
		{
			// Ben says: .fac must be 2-coord for v9.  But...maybe for v10 we allow curved facades?
			me->want_uv=false;
			me->want_bezier=(inCoordDepth >= 4);
			me->want_wall = (inCoordDepth == 3 || inCoordDepth == 5);
			WED_FacadePlacement * fac = WED_FacadePlacement::CreateTyped(me->archive);
			#if AIRPORT_ROUTING
			fac->SetCustomWalls(me->want_wall);
			#endif
			me->poly = fac;
			me->ring = NULL;
			fac->SetHeight(inParam);
			fac->SetResource(r);
			fac->SetShowLevel(me->GetShowForFacID(inPolygonType));
			
		}

		if(end_match(r.c_str(),".for"))
		{
			me->want_uv=false;
			me->want_bezier=false;
			me->want_wall=false;
			WED_ForestPlacement * forst = WED_ForestPlacement::CreateTyped(me->archive);
			me->poly = forst;
			me->ring = NULL;
			forst->SetDensity((inParam % 256) / 255.0);
			#if AIRPORT_ROUTING
			forst->SetFillMode(inParam / 256);
			#endif
			forst->SetResource(r);
		}

		if(end_match(r.c_str(),".lin"))
		{
			me->want_uv=false;
			me->want_bezier=inCoordDepth == 4;
			me->want_wall=false;
			WED_LinePlacement * lin = WED_LinePlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = lin;
			lin->SetClosed(inParam);
			lin->SetResource(r);
		}

		if(end_match(r.c_str(),".str") || end_match(r.c_str(),".ags"))
		{
			me->want_uv=false;
			me->want_bezier=inCoordDepth == 4;
			me->want_wall=false;
			WED_StringPlacement * str = WED_StringPlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = str;
			str->SetSpacing(inParam);
			str->SetResource(r);
		}

		if(end_match(r.c_str(),".pol") || end_match(r.c_str(),".agb"))
		{
			me->want_uv=inParam == 65535;
			me->want_bezier=me->want_uv ? (inCoordDepth == 8) : (inCoordDepth == 4);
			me->want_wall=false;
			if(me->want_uv)
			{
				WED_DrapedOrthophoto * orth = WED_DrapedOrthophoto::CreateTyped(me->archive);
				me->poly = orth;
				orth->SetResource(r);
			}
			else
			{
				WED_PolygonPlacement * pol = WED_PolygonPlacement::CreateTyped(me->archive);
				me->poly = pol;
				pol->SetHeading(inParam);
				pol->SetResource(r);
			}
			me->ring = NULL;
		}

		if(me->poly)
		{
			me->poly->SetParent(me->parent,me->parent->CountChildren());
			me->poly->SetName(r);
		}
		if(me->ring)
		{
			me->ring->SetParent(me->parent,me->parent->CountChildren());
			me->ring->SetName(r);
		}
	}

	static void	BeginPolygonWinding(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->pts.clear();
		me->walls.clear();
		me->uvs.clear();
		if(me->poly != NULL)
		{
			if(me->poly->GetClass() == WED_ForestPlacement::sClass)
				me->ring = WED_ForestRing::CreateTyped(me->archive);
			else if(me->poly->GetClass() == WED_FacadePlacement::sClass)
				me->ring = WED_FacadeRing::CreateTyped(me->archive);
			else
				me->ring = WED_Ring::CreateTyped(me->archive);
			me->ring->SetParent(me->poly,me->poly->CountChildren());
			me->ring->SetName("Ring");
		}
	}

	static void	AddPolygonPoint(
					double *		inCoordinates,
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		WED_Thing * node;
		if(me->want_wall && me->want_bezier)
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.hi.x_ = inCoordinates[3];
			p.hi.y_ = inCoordinates[4];
			p.lo = p.pt + Vector2(p.hi, p.pt);
			me->pts.push_back(p);		
			me->walls.push_back(inCoordinates[2]);
		}
		else if(me->want_wall)
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.lo = p.hi = p.pt;
			me->pts.push_back(p);		
			me->walls.push_back(inCoordinates[2]);
		}
		else if(me->want_uv && me->want_bezier)
		{
			BezierPoint2	p, u;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.hi.x_ = inCoordinates[2];
			p.hi.y_ = inCoordinates[3];
			p.lo = p.pt + Vector2(p.hi, p.pt);

			u.pt.x_ = inCoordinates[4];
			u.pt.y_ = inCoordinates[5];
			u.hi.x_ = inCoordinates[6];
			u.hi.y_ = inCoordinates[7];
			u.lo = u.pt + Vector2(u.hi, u.pt);
			
			me->pts.push_back(p);
			me->uvs.push_back(u);
		}
		else if (me->want_uv)
		{
			BezierPoint2	p, u;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.lo = p.hi = p.pt;

			u.pt.x_ = inCoordinates[2];
			u.pt.y_ = inCoordinates[3];
			u.lo = u.hi = u.pt;
			me->pts.push_back(p);
			me->uvs.push_back(u);	
		}
		else if (me->want_bezier)
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.hi.x_ = inCoordinates[2];
			p.hi.y_ = inCoordinates[3];
			p.lo = p.pt + Vector2(p.hi, p.pt);
			me->pts.push_back(p);
		}
		else
		{
			BezierPoint2	p;
			p.pt.x_ = inCoordinates[0];
			p.pt.y_ = inCoordinates[1];
			p.lo = p.hi = p.pt;
			me->pts.push_back(p);
		}
	}

	static void	EndPolygonWinding(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		
		if(me->want_bezier)
		{
			vector<BezierPoint2>	pc, uc;

//			debug_it(me->pts);
//			debug_it(me->uvs);
			
			BezierPointSeqFromTriple(me->pts.begin(),me->pts.end(), back_inserter(pc));
			me->pts.swap(pc);
			if(me->want_uv)
			{
				BezierPointSeqFromTriple(me->uvs.begin(),me->uvs.end(), back_inserter(uc));
				me->uvs.swap(uc);
			}

			if(me->pts.front().pt == me->pts.back().pt)
			{
				me->pts.front().lo = me->pts.back().lo;
				me->pts.pop_back();
				if(me->want_uv)
				{
					me->uvs.front().lo = me->uvs.back().lo;
					me->uvs.pop_back();
				}
				
				WED_StringPlacement * str = dynamic_cast<WED_StringPlacement*>(me->ring);
				if(str)
					str->SetClosed(1);
			}
			
//			debug_it(me->pts);
//			debug_it(me->uvs);
			
		}
		
		for(int n = 0; n < me->pts.size(); ++n)
		{		
			WED_Thing * node;
			if(me->want_uv && me->want_bezier)
			{
				WED_TextureBezierNode * tb = WED_TextureBezierNode::CreateTyped(me->archive);
				node=tb;
				tb->SetBezierLocation(gis_Geo,me->pts[n]);
				tb->SetBezierLocation(gis_UV,me->uvs[n]);
			}
			else if (me->want_uv)
			{
				WED_TextureNode * t = WED_TextureNode::CreateTyped(me->archive);
				node=t;
				t->SetLocation(gis_Geo,me->pts[n].pt);
				t->SetLocation(gis_UV,me->uvs[n].pt);
			}
			else if (me->ring && me->ring->GetClass() == WED_FacadeRing::sClass)
			{
				WED_FacadeNode * b = WED_FacadeNode::CreateTyped(me->archive);
				node=b;
				b->SetBezierLocation(gis_Geo,me->pts[n]);
				#if AIRPORT_ROUTING
				if(me->want_wall)
					b->SetWallType(me->walls[n]);
				#endif	
			}
			else if (me->want_bezier)
			{
				WED_SimpleBezierBoundaryNode * b = WED_SimpleBezierBoundaryNode::CreateTyped(me->archive);
				node=b;
				b->SetBezierLocation(gis_Geo,me->pts[n]);
			}
			else
			{
				WED_SimpleBoundaryNode * nd = WED_SimpleBoundaryNode::CreateTyped(me->archive);
				node=nd;
				nd->SetLocation(gis_Geo,me->pts[n].pt);
			}
			node->SetParent(me->ring,me->ring->CountChildren());
			node->SetName("Point");
		}

		
		if (me->poly != NULL)
			me->ring = NULL;
	}

	static void	EndPolygon(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->poly = NULL;
	}
	
	static void AddRasterData(
					DSFRasterHeader_t *	header,
					void *				data,
					void *				inRef)
	{
	}


	void do_import_dsf(const char * file_name, WED_Thing * base)
	{
		parent = base;
		archive = parent->GetArchive();

		DSFCallbacks_t cb = {	NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptRasterDef, AcceptProperty,
								BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
								AddObject,AddObjectMSL,
								BeginSegment, AddSegmentShapePoint, EndSegment,
								BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon, AddRasterData };

		int res = DSFReadFile(file_name, &cb, NULL, this);
		if(res != 0)
			printf("DSF Error: %d\n", res);
	}

	void do_import_txt(const char * file_name, WED_Thing * base)
	{
		parent = base;
		archive = parent->GetArchive();

		DSFCallbacks_t cb = {	NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptRasterDef, AcceptProperty,
								BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
								AddObject,AddObjectMSL,
								BeginSegment, AddSegmentShapePoint, EndSegment,
								BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon, AddRasterData };

		int ok = Text2DSFWithWriter(file_name, &cb, this);

//		int res = DSFReadFile(file_name, &cb, NULL, this);
//		if(res != 0)
//			printf("DSF Error: %d\n", res);
	}


};


void DSF_Import(const char * path, WED_Group * base)
{
	DSF_Importer importer;
	importer.do_import_dsf(path, base);
}

int		WED_CanImportDSF(IResolver * resolver)
{
	return 1;
}

void	WED_DoImportDSF(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);
	char path[1024];
	strcpy(path,"");
	if (GetFilePathFromUser(getFile_Open,"Import DSF file...", "Import", FILE_DIALOG_IMPORT_DSF, path, sizeof(path)))
	{
		wrl->StartOperation("Import DSF");
		WED_Group * g = WED_Group::CreateTyped(wrl->GetArchive());
		g->SetName(path);
		g->SetParent(wrl,wrl->CountChildren());
		DSF_Import(path,g);
		wrl->CommitOperation();
	}

}

static WED_Thing * find_airport_by_icao_recursive(const string& icao, WED_Thing * who)
{
	if(WED_Airport::sClass == who->GetClass())
	{
		WED_Airport * apt = dynamic_cast<WED_Airport *>(who);
		DebugAssert(apt);
		string aicao;
		apt->GetICAO(aicao);
		
		if(aicao == icao)
			return apt;
		else
			return NULL;
	}
	else
	{
		int n, nn = who->CountChildren();
		for(n = 0; n < nn; ++n)
		{
			WED_Thing * found_it = find_airport_by_icao_recursive(icao, who->GetNthChild(n));
			if(found_it) return found_it;
		}
	}
	return NULL;
}

void WED_DoImportText(const char * path, WED_Thing * base)
{
	DSF_Importer importer;
	importer.do_import_txt(path, base);
}


#if GATEWAY_IMPORT_FEATURES
//This is from an older method of importing things which involved manually getting the files from the hard drive
void	WED_DoImportDSFText(IResolver * resolver)
{
	WED_Thing * wrl = WED_GetWorld(resolver);

	char * paths = GetMultiFilePathFromUser("Import DSF file...", "Import", FILE_DIALOG_IMPORT_DSF);
	if(paths)
	{
		char * free_me = paths;
		
		wrl->StartOperation("Import DSF");
		
		while(*paths)
		{
			if(strstr(paths,".dat"))
			{			
				WED_ImportOneAptFile(paths,wrl);
			}
			paths = paths + strlen(paths) + 1;
		}
		
		paths = free_me;

		while(*paths)
		{
			if(!strstr(paths,".dat"))
			{
				string tname(paths);
				string::size_type p = tname.find_last_of("\\/");
				if(p != tname.npos)
					tname = tname.substr(p+1);
				p = tname.find_last_of(".");
				if(p != tname.npos)
					tname = tname.substr(0,p);
				
				WED_Thing * g = find_airport_by_icao_recursive(tname,wrl);
				if(g == NULL)
				{
					g = WED_Group::CreateTyped(wrl->GetArchive());
					g->SetName(paths);
					g->SetParent(wrl,wrl->CountChildren());
				}
		//		DSF_Import(path,g);
				DSF_Importer importer;
				importer.do_import_txt(paths, g);
			}	
			paths = paths + strlen(paths) + 1;
		}
		
		wrl->CommitOperation();
		free(free_me);
	}
}
#endif
