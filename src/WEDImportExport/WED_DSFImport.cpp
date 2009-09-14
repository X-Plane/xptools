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

	vector<string>		obj_table;
	vector<string>		pol_table;

	WED_Group *			parent;
	WED_Archive *		archive;

	WED_Thing *			ring;
	WED_Thing *			poly;
	bool				want_uv;
	bool				want_bezier;

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
		obj->SetHeading(inRotation);
		obj->SetName(me->obj_table[inObjectType]);
		obj->SetParent(me->parent,me->parent->CountChildren());
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
			me->want_uv=false;
			me->want_bezier=false;
			WED_FacadePlacement * fac = WED_FacadePlacement::CreateTyped(me->archive);
			me->poly = fac;
			me->ring = NULL;
			fac->SetHeight(inParam);
			fac->SetResource(r);
		}

		if(end_match(r.c_str(),".for"))
		{
			me->want_uv=false;
			me->want_bezier=false;
			WED_ForestPlacement * forst = WED_ForestPlacement::CreateTyped(me->archive);
			me->poly = forst;
			me->ring = NULL;
			forst->SetDensity(inParam / 255.0);
			forst->SetResource(r);
		}

		if(end_match(r.c_str(),".lin"))
		{
			me->want_uv=false;
			me->want_bezier=inCoordDepth == 4;
			WED_LinePlacement * lin = WED_LinePlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = lin;
			lin->SetClosed(inParam);
			lin->SetResource(r);
		}

		if(end_match(r.c_str(),".str"))
		{
			me->want_uv=false;
			me->want_bezier=inCoordDepth == 4;
			WED_StringPlacement * str = WED_StringPlacement::CreateTyped(me->archive);
			me->poly = NULL;
			me->ring = str;
			str->SetSpacing(inParam);
			str->SetResource(r);
		}

		if(end_match(r.c_str(),".pol"))
		{
			me->want_uv=inParam == 65535;
			me->want_bezier=me->want_uv ? (inCoordDepth == 8) : (inCoordDepth == 4);
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
		if(me->poly != NULL)
		{
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
		if(me->want_uv && me->want_bezier)
		{
			WED_TextureBezierNode * tb = WED_TextureBezierNode::CreateTyped(me->archive);
			node=tb;
			tb->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
			tb->SetControlHandleHi(gis_Geo,Point2(inCoordinates[2],inCoordinates[3]));
			tb->SetLocation(gis_UV,Point2(inCoordinates[4],inCoordinates[5]));
			tb->SetControlHandleHi(gis_UV,Point2(inCoordinates[6],inCoordinates[7]));
			tb->SetControlHandleLo(gis_UV,Point2(
							inCoordinates[4] * 2.0 - inCoordinates[6],
							inCoordinates[5] * 2.0 - inCoordinates[7]));
		}
		else if (me->want_uv)
		{
			WED_TextureNode * t = WED_TextureNode::CreateTyped(me->archive);
			node=t;
			t->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
			t->SetLocation(gis_UV,Point2(inCoordinates[2],inCoordinates[3]));
		}
		else if (me->want_bezier)
		{
			WED_SimpleBezierBoundaryNode * b = WED_SimpleBezierBoundaryNode::CreateTyped(me->archive);
			node=b;
			b->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
			b->SetControlHandleHi(gis_Geo,Point2(inCoordinates[2],inCoordinates[3]));
		}
		else
		{
			WED_SimpleBoundaryNode * n = WED_SimpleBoundaryNode::CreateTyped(me->archive);
			node=n;
			n->SetLocation(gis_Geo,Point2(inCoordinates[0],inCoordinates[1]));
		}
		node->SetParent(me->ring,me->ring->CountChildren());
		node->SetName("Point");
	}

	static void	EndPolygonWinding(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		if (me->poly != NULL)
			me->ring = NULL;
	}

	static void	EndPolygon(
					void *			inRef)
	{
		DSF_Importer * me = (DSF_Importer *) inRef;
		me->poly = NULL;
	}


	void do_import(const char * file_name, WED_Group * base)
	{
		parent = base;
		archive = parent->GetArchive();

		DSFCallbacks_t cb = {	NextPass, AcceptTerrainDef, AcceptObjectDef, AcceptPolygonDef, AcceptNetworkDef, AcceptProperty,
								BeginPatch, BeginPrimitive, AddPatchVertex, EndPrimitive, EndPatch,
								AddObject,
								BeginSegment, AddSegmentShapePoint, EndSegment,
								BeginPolygon, BeginPolygonWinding, AddPolygonPoint,EndPolygonWinding, EndPolygon };

		int res = DSFReadFile(file_name, &cb, NULL, this);
		if(res != 0)
			printf("DSF Error: %d\n", res);
	}
};


void DSF_Import(const char * path, WED_Group * base)
{
	DSF_Importer importer;
	importer.do_import(path, base);
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
