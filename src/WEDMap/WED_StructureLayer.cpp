/*
 * Copyright (c) 2007, Laminar Research.
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

#include "WED_StructureLayer.h"
#include "IGIS.h"
#include "GUI_GraphState.h"
#include "WED_Colors.h"
#include "GISUtils.h"
#include "WED_UIDefs.h"
#include "WED_ToolUtils.h"
#include "WED_TextureNode.h"
#include "ITexMgr.h"
#include "TexUtils.h"
#include "WED_EnumSystem.h"
#include "WED_MapZoomerNew.h"
#include "WED_AirportNode.h"
#include "XESConstants.h"
#include "MathUtils.h"
#include "GUI_Resources.h"
#include "GUI_Fonts.h"
#include "WED_Runway.h"
#include "MatrixUtils.h"
#include "WED_Helipad.h"
#include "WED_LightFixture.h"
#include "WED_Taxiway.h"
#include "WED_OverlayImage.h"
#include "WED_Sealane.h"
#include "WED_AirportSign.h"
#include "WED_TowerViewpoint.h"
#include "WED_TruckDestination.h"
#include "WED_Airport.h"
#include "WED_UIMeasurements.h"
#include "WED_RampPosition.h"
#include "WED_Windsock.h"
#include "WED_AirportBeacon.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_DrawUtils.h"
#include "GUI_DrawUtils.h"
#include "WED_TaxiRoute.h"
#include "WED_TaxiRouteNode.h"
#include "WED_RoadNode.h"
#include "WED_AirportBoundary.h"

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

#define HILIGHT_ALPHA 0.33           // transparency of highlighting for selected itemss area


WED_StructureLayer::WED_StructureLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(h, zoomer, resolver)
{
	mRealLines = true;
	mVertices = true;
//	mPavementAlpha = 0.5;
}

WED_StructureLayer::~WED_StructureLayer()
{
}


bool		WED_StructureLayer::DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
	//	g->SetState(false,0,false,   false,true,false,false);
	//  very carefully check that ALL operations that change the state to textured its re-set again,
	//  so we don't have to reset state for *evrvy* entity.

	int locked = IsLockedNow(entity);
	WED_Color struct_color = selected ? (locked ? wed_StructureLockedSelected : wed_StructureSelected) :
										(locked ? wed_StructureLocked		 : wed_Structure);
	glColor4fv(WED_Color_RGBA(struct_color));

	GISClass_t 		kind		= entity->GetGISClass();
	const char *	sub_class	= entity->GetGISSubtype();

	float							storage[4];

	if (sub_class == WED_Airport::sClass)
	{
		WED_Airport * airport = SAFE_CAST(WED_Airport, entity);
		if(airport)
		{
			Bbox2	bounds;
			airport->GetBounds(gis_Geo,bounds);
			bounds.p1 = GetZoomer()->LLToPixel(bounds.p1);
			bounds.p2 = GetZoomer()->LLToPixel(bounds.p2);
			if (bounds.xspan() < GetAirportTransWidth() && bounds.yspan() < GetAirportTransWidth())
			{
				float * f1 = WED_Color_RGBA(struct_color);
				float * f2 = f1 + 4;
				Point2 loc = Segment2(bounds.p1,bounds.p2).midpoint();
				switch(airport->GetAirportType()) {
				case type_Airport:		mAirportIconsX.push_back(loc.x());	mAirportIconsY.push_back(loc.y());	mAirportIconsC.insert(mAirportIconsC.end(),f1,f2);		break;
				case type_Seaport:		mSeaportIconsX.push_back(loc.x());	mSeaportIconsY.push_back(loc.y());	mSeaportIconsC.insert(mSeaportIconsC.end(),f1,f2);		break;
				case type_Heliport:		mHeliportIconsX.push_back(loc.x());	mHeliportIconsY.push_back(loc.y());	mHeliportIconsC.insert(mHeliportIconsC.end(),f1,f2);	break;
				}
				return false;
			}
		}
	}
	else if (sub_class == WED_Runway::sClass)
	{
		WED_Runway * rwy = SAFE_CAST(WED_Runway,entity);
		if(rwy)
		{
			Point2 	corners[4], shoulders[8], disp1[4], disp2[4], blas1[4], blas2[4];
			bool	has_shoulders, has_disp1, has_disp2, has_blas1, has_blas2;

			// First, transform our geometry.
							rwy->GetCorners(gis_Geo,corners);			GetZoomer()->LLToPixelv(corners, corners, 4);
			if (has_blas1 = rwy->GetCornersBlas1(blas1))				GetZoomer()->LLToPixelv(blas1, blas1, 4);
			if (has_blas2 = rwy->GetCornersBlas2(blas2))				GetZoomer()->LLToPixelv(blas2, blas2, 4);
			if (has_disp1 = rwy->GetCornersDisp1(disp1))				GetZoomer()->LLToPixelv(disp1, disp1, 4);
			if (has_disp2 = rwy->GetCornersDisp2(disp2))				GetZoomer()->LLToPixelv(disp2, disp2, 4);
			if (has_shoulders = rwy->GetCornersShoulders(shoulders))	GetZoomer()->LLToPixelv(shoulders, shoulders, 8);

			//  "Outline" geometry
			glShape2v(GL_LINE_LOOP, corners, 4);
			glBegin(GL_LINES);
			glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
			glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
			glEnd();

			if(selected)
			{
				glColor4fv(WED_Color_RGBA_Alpha(struct_color, HILIGHT_ALPHA, storage));
				glBegin(GL_QUADS);
				glVertex2v(corners,4);
				glEnd();
			}

			if (has_shoulders)
			{
				glColor4fv(WED_Color_RGBA(struct_color));
				glBegin(GL_LINES);
				glVertex2(shoulders[3]);		glVertex2(shoulders[0]);
				glVertex2(shoulders[0]);		glVertex2(shoulders[1]);
				glVertex2(shoulders[1]);		glVertex2(shoulders[2]);
				glVertex2(shoulders[4]);		glVertex2(shoulders[7]);
				glVertex2(shoulders[7]);		glVertex2(shoulders[6]);
				glVertex2(shoulders[6]);		glVertex2(shoulders[5]);
				glEnd();
			}

			if (mRealLines) glColor4f(1,1,0,1);
			if (has_blas1)				glShape2v(GL_LINE_LOOP, blas1,4);
			if (has_blas2)				glShape2v(GL_LINE_LOOP, blas2, 4);

			float white[4] = { 1,1,1,1 };
			
			if (mRealLines) glColor4fv(locked ? WED_Color_RGBA(wed_StructureLocked): white);
			if (has_disp1)				glShape2v(GL_LINE_LOOP, disp1,4);
			if (has_disp2)				glShape2v(GL_LINE_LOOP, disp2,4);

			if (Vector2(corners[3],corners[0]).normalize() > 20.0)  // paint the numbers on the runway, if scale large enough
			{
				Point2 Th1, Th2;    // locations of runway thresholds
				if (has_disp1)
					Th1 = Midpoint2(disp1[1],disp1[2]);
				else
					Th1 = Midpoint2(corners[3],corners[0]);

				if (has_disp2)
					Th2 = Midpoint2(disp2[3],disp2[0]);
				else
					Th2 = Midpoint2(corners[1],corners[2]);

				pair<int,int> e = rwy->GetRunwayEnumsOneway();
				float hdg = rwy->GetHeading();
				
				if(e.first != atc_Runway_None)
				{
					glPushMatrix();                  // rotate the numbers properly
					glTranslatef(Th1.x(), Th1.y(), 0);
					glRotatef(-hdg,0,0,1);
					GUI_FontDraw(g, font_UI_Basic, locked ? WED_Color_RGBA(wed_StructureLocked) : white, -8, 10, ENUM_Desc(e.first));
					glPopMatrix();
				}
				if(e.second != atc_Runway_None) 
				{
					glPushMatrix();
					glTranslatef(Th2.x(), Th2.y(), 0);
					glRotatef(180-hdg,0,0,1);
					GUI_FontDraw(g, font_UI_Basic, locked ? WED_Color_RGBA(wed_StructureLocked) : white, -8, 10, ENUM_Desc(e.second));
					glPopMatrix();
				}
				g->SetState(false,0,false,false,true,false,false);
			}
		}
	}
	else switch(kind) {
	case gis_Point:
	case gis_Point_Bezier:
		/******************************************************************************************************************************************************
		 * NON-DIRECTIONAL POINT ELEMENTS
		 ******************************************************************************************************************************************************/
		{
			WED_TowerViewpoint *	tower;
			WED_Windsock *			sock;
			WED_AirportBeacon *		beacon;
			IGISPoint *				pt;
			const char *			icon = NULL;
			
				 if (sub_class == WED_TowerViewpoint::sClass&& (tower  = SAFE_CAST(WED_TowerViewpoint, entity)) != NULL) pt = tower , icon = "map_towerview.png";
			else if (sub_class == WED_Windsock::sClass		&& (sock   = SAFE_CAST(WED_Windsock		 , entity)) != NULL) pt = sock  , icon = "map_windsock.png" ;
			else if (sub_class == WED_AirportBeacon::sClass && (beacon = SAFE_CAST(WED_AirportBeacon , entity)) != NULL) pt = beacon, icon = "map_beacon.png"   ;
			else if (sub_class == WED_SimpleBoundaryNode::sClass && (pt = SAFE_CAST(IGISPoint, entity)) != NULL) icon = "map_tree.png";
			else pt = SAFE_CAST(IGISPoint,entity);
			if (pt)
			{
				Point2			l;
				pt->GetLocation(gis_Geo,l);
				l = GetZoomer()->LLToPixel(l);
				if (icon) 
				{
					// Pretty much all non-heading single point entities should have SOME kind of icon!!
					// Off-hand I think windsocks, tower viewpoints and airport-beacons are the only ones 
					// we have right now.
					GUI_PlotIcon(g,icon, l.x(),l.y(),0,GetFurnitureIconScale());
					g->SetState(false,0,false,   false,true,false,false);
				}
				else {
								
					// Special case: for taxi routes, we are going to draw them here IF
					// vertex-preview is on.  This way we can use the selection state of the 
					// node itself to color the node.
					//
					// Note that for "line" types like taxi lines if vertex preview is on and we
					// are using a random tool (not the vertex tool) this code does not kick in -
					// instead we sub-iterate our chain.  The result is a bug we'll ship with for 
					// now: incorrect node hilighting.  For line types, I think this is pretty 
					// unimportant: a user has pretty much no reason to hilight a line vertex
					// with a tool other than the vertex tool for editing, which is why no one
					// reported the bug in the several years of WED's history.  
					//
					// So we will special-case taxi routes for now.  When we get road grids in 
					// we may need a better heuristic for this case than the actual obj type.
								
					if (sub_class == WED_TaxiRouteNode::sClass
#if ROAD_EDITING 
						|| sub_class == WED_RoadNode::sClass
#endif
						)
					{
						if(mVertices)
						{
							glPointSize(5);
							glBegin(GL_POINTS);
							Point2 p;
							pt->GetLocation(gis_Geo,p);
							glVertex2(GetZoomer()->LLToPixel(p));
							glEnd();
							glPointSize(1);
						}
					}
					else 
					{
						// Ideally if anything falls into here we'd like to know and fixit.  It probably
						// means we need an icon but we are missing one.
						//printf("Skipped preview of %s\n", sub_class);
						/*
							glBegin(GL_LINES);
							glVertex2f(l.x(), l.y() - 3);
							glVertex2f(l.x(), l.y() + 3);
							glVertex2f(l.x() - 3, l.y());
							glVertex2f(l.x() + 3, l.y());
							glEnd();
						*/
					}
				}
			}
		}
		break;
	case gis_Point_Heading:
		/******************************************************************************************************************************************************
		 * DIRECTIONAL POINT ELEMENTS
		 ******************************************************************************************************************************************************/
		{
			IGISPoint_Heading *		pth;
			WED_LightFixture *		lfix;
			WED_AirportSign *		sign;
			WED_RampPosition *		ramp;
			const char *			icon = NULL;

				 if (sub_class == WED_LightFixture::sClass && (lfix = SAFE_CAST(WED_LightFixture,entity)) != NULL) pth = lfix, icon = "map_light.png"	;
			else if (sub_class == WED_AirportSign::sClass  && (sign = SAFE_CAST(WED_AirportSign ,entity)) != NULL) pth = sign, icon = "map_taxisign.png"	;
			else if (sub_class == WED_RampPosition::sClass && (ramp = SAFE_CAST(WED_RampPosition,entity)) != NULL) pth = ramp, icon = "map_rampstart.png";
			else if (sub_class == WED_TruckDestination::sClass && (pth = SAFE_CAST(IGISPoint_Heading,entity)) != NULL) icon = "map_servicedest.png";
			else pth = SAFE_CAST(IGISPoint_Heading,entity);
			if (pth)
			{
				Point2		l;
				Vector2		dir;
				pth->GetLocation(gis_Geo,l);
				NorthHeading2VectorMeters(l,l,pth->GetHeading(),dir);
				Vector2 r(dir.perpendicular_cw());
				l = GetZoomer()->LLToPixel(l);
				if (icon)	
				{
					GUI_PlotIcon(g,icon, l.x(),l.y(),atan2(dir.dx,dir.dy) * RAD_TO_DEG,GetFurnitureIconScale());
					g->SetState(false,0,false,   false,true,false,false);
				}
				else {
					glBegin(GL_LINES);
					glVertex2(l - dir * 3.0);			glVertex2(l + dir * 6.0);
					glVertex2(l - r   * 3.0);			glVertex2(l + r   * 3.0);
					glEnd();
				}
			}
		}
		break;
	case gis_Point_HeadingWidthLength:
		/******************************************************************************************************************************************************
		 * HELIPADS AND OTHER GIS-SIMILAR
		 ******************************************************************************************************************************************************/
		{
			IGISPoint_WidthLength *			ptwl;
			WED_Helipad * 					helipad;
			if (sub_class == WED_Helipad::sClass && (helipad = SAFE_CAST(WED_Helipad		 , entity)))	ptwl = helipad;
			else									 ptwl    = SAFE_CAST(IGISPoint_WidthLength,entity)					  ;

			if (ptwl)
			{
				Point2 corners[4];
				ptwl->GetCorners(gis_Geo,corners);
				GetZoomer()->LLToPixelv(corners, corners, 4);

				glShape2v(GL_LINE_LOOP, corners, 4);

				glBegin(GL_LINES);
				glVertex2(Segment2(corners[0],corners[1]).midpoint(0.5));
				glVertex2(Segment2(corners[2],corners[3]).midpoint(0.5));
				glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
				glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
				glEnd();

				if (helipad)
				{
					Point2	p;
					helipad->GetLocation(gis_Geo,p);
					p = GetZoomer()->LLToPixel(p);
					GUI_PlotIcon(g, "map_helipad.png", p.x(), p.y(), ptwl->GetHeading(),GetFurnitureIconScale());
					g->SetState(false,0,false,   false,true,false,false);
				}
			}
		}
		break;
	case gis_PointSequence:
	case gis_Line:
	case gis_Ring:
	case gis_Chain:
	case gis_Edge:
		{
			IGISPointSequence *				ps;
			if ((ps = SAFE_CAST(IGISPointSequence,entity)) != NULL)
			{
				#if AIRPORT_ROUTING
				if (kind == gis_Edge)
				{
					WED_TaxiRoute * tr = SAFE_CAST(WED_TaxiRoute, entity);
					if(tr && !locked)
					{
						bool hot = tr->HasHotDepart() || tr->HasHotArrival();
						bool rwy = tr->IsRunway();
						bool ils = tr->HasHotILS();
						
						if(hot)                 struct_color = selected ? wed_Hotzone_Selected : wed_Hotzone;
						else if(ils)            struct_color = selected ? wed_ILSzone_Selected : wed_ILSzone;
						else if (rwy)           struct_color = selected ? wed_Runway_Selected : wed_Runway;

						glColor4fv(WED_Color_RGBA(struct_color));
					}
				}
				#endif
				int i, n = ps->GetNumSides();
				WED_MapZoomerNew * z = GetZoomer();

				bool showRealLines = mRealLines > 0 && z->GetPPM() * 0.3 <= MIN_PIXELS_LINE_PREVIEW;
				
				for (i = 0; i < n; ++i)
				{
					set<int>		attrs;
					if (showRealLines)
					{
						WED_AirportNode * apt_node = dynamic_cast<WED_AirportNode*>(ps->GetNthPoint(i));
						if (apt_node) apt_node->GetAttributes(attrs);
					}
					vector<Point2> pts;
					
					Bezier2		b;
					if (ps->GetSide(gis_Geo,i,b))
					{
						b.p1 = z->LLToPixel(b.p1);
						b.p2 = z->LLToPixel(b.p2);
						b.c1 = z->LLToPixel(b.c1);
						b.c2 = z->LLToPixel(b.c2);

						int point_count = BezierPtsCount(b, z);
						
						pts.reserve(point_count+1);
						for (int n = 0; n <= point_count; ++n)
							pts.push_back(b.midpoint((float) n / (float) point_count));

					}
					else
					{
						pts.push_back(z->LLToPixel(b.p1));
						pts.push_back(z->LLToPixel(b.p2));
					}
					DrawLineAttrs(&*pts.begin(), pts.size(), attrs);
					if(!attrs.empty()) glColor4fv(WED_Color_RGBA(struct_color));
					
					if(kind == gis_Edge && pts.size() >= 2)
					{
						IGISEdge * gisedge = SAFE_CAST(IGISEdge, ps);
						if(gisedge->IsOneway())
						{
							Vector2 orient(pts[pts.size()-2],pts[pts.size()-1]);
							GUI_PlotIcon(g,"handle_arrowhead.png", pts.back().x(), pts.back().y(),atan2(orient.dx,orient.dy) * RAD_TO_DEG,1.0);
							g->SetState(false,0,false,   false,true,false,false);
						}
					}
				}
				if (mVertices && kind != gis_Edge)	// Gis EDGE points will be picked up separately!  That way we can get their hilite right.
				{
					n = ps->GetNumPoints();
					glPointSize(5);
					glBegin(GL_POINTS);
					for (i = 0; i < n; ++i)
					{
						IGISPoint * pt = ps->GetNthPoint(i);
						WED_Entity * e = dynamic_cast<WED_Entity *>(pt);
						if(!e || !e->GetHidden())
						{
							Point2 p;
							pt->GetLocation(gis_Geo,p);
							glVertex2(GetZoomer()->LLToPixel(p));
						}
					}
					glEnd();
					glPointSize(1);
				}
			}
		}
		break;

	case gis_Line_Width:  //  NON-RUNWAY LINES
		{
			IGISLine_Width *	lw;
			WED_Sealane *		sea;
			if (sub_class == WED_Sealane::sClass && (sea = SAFE_CAST(WED_Sealane, entity)) != NULL) lw = sea;
			else									 lw  = SAFE_CAST(IGISLine_Width,entity);
			if (lw)
			{
				Point2 corners[4];
				lw->GetCorners(gis_Geo,corners);
				GetZoomer()->LLToPixelv(corners, corners, 4);

				glShape2v(GL_LINE_LOOP, corners, 4);

				glBegin(GL_LINES);
				glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
				glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
				glEnd();
			}
		}
		break;

	case gis_BoundingBox:
		{
			IGISBoundingBox *				box;
			box = SAFE_CAST(IGISBoundingBox, entity);
			if(box)
			{
				Point2	pts[2];
				box->GetMin()->GetLocation(gis_Geo,pts[0]);
				box->GetMax()->GetLocation(gis_Geo,pts[1]);
				GetZoomer()->LLToPixelv(pts,pts,2);
				if(locked || selected)
					glColor4fv(WED_Color_RGBA_Alpha(struct_color, 1.0/*mPavementAlpha*/, storage));
				else
					glColor4fv(WED_Color_RGBA_Alpha(wed_Link, 1.0/*mPavementAlpha*/, storage));
				glBegin(GL_LINE_LOOP);
				glVertex2f(pts[0].x(), pts[0].y());
				glVertex2f(pts[0].x(), pts[1].y());
				glVertex2f(pts[1].x(), pts[1].y());
				glVertex2f(pts[1].x(), pts[0].y());
				glEnd();

				if(selected)
				{
					glColor4fv(WED_Color_RGBA_Alpha(struct_color, HILIGHT_ALPHA, storage));
					glBegin(GL_QUADS);
					glVertex2f(pts[0].x(), pts[0].y());
					glVertex2f(pts[0].x(), pts[1].y());
					glVertex2f(pts[1].x(), pts[1].y());
					glVertex2f(pts[1].x(), pts[0].y());
					glEnd();
				}
			}
		}
		break;

	case gis_Composite:
		if(sub_class != WED_AirportBoundary::sClass)      // boundaries are not down-clickable in the interior, but still get a highlighted interior 
			break;
	case gis_Polygon:
		/******************************************************************************************************************************************************
		 * POLYGONS (TAXIWAYAS, ETC.)
		 ******************************************************************************************************************************************************/
		{
			IGISPolygon * poly = SAFE_CAST(IGISPolygon,entity);
			if (poly)
			{
				this->DrawEntityStructure(inCurrent,poly->GetOuterRing(),g,selected);
				int n = poly->GetNumHoles();
				for (int c = 0; c < n; ++c)
					this->DrawEntityStructure(inCurrent,poly->GetNthHole(c),g,selected);

				if(selected)
				{
					vector<Point2>	pts;
					vector<int>		is_hole_start;

					PointSequenceToVector(poly->GetOuterRing(), GetZoomer(), pts, false, is_hole_start, 0);
					int n = poly->GetNumHoles();
					for (int i = 0; i < n; ++i)
						PointSequenceToVector(poly->GetNthHole(i), GetZoomer(), pts, false, is_hole_start, 1);

					glColor4fv(WED_Color_RGBA_Alpha(struct_color, HILIGHT_ALPHA, storage));
					glFrontFace(GL_CCW);
					glPolygon2(&*pts.begin(), false, &*is_hole_start.begin(), pts.size());
					glFrontFace(GL_CW);
				}
			}
		}
		break;
	}
	return true;
}

bool		WED_StructureLayer::DrawEntityVisualization		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
//	g->SetState(false,0,false,   false,true,false,false);

//	int locked = IsLockedNow(entity);
//	WED_Color struct_color = selected ? (locked ? wed_StructureLockedSelected : wed_StructureSelected) :
//										(locked ? wed_StructureLocked		 : wed_Structure);

	GISClass_t 		kind		= entity->GetGISClass();
//	const char *	sub_class	= entity->GetGISSubtype();
//	IGISPolygon *					poly;

	WED_OverlayImage *				overlay;

//	glColor4fv(WED_Color_RGBA(struct_color));

	/******************************************************************************************************************************************************
	 * RUNWAY DRAWING
	 ******************************************************************************************************************************************************/
	switch(kind) {
	case gis_Polygon:
		if(entity->GetGISSubtype() == WED_OverlayImage::sClass)
		{
			if((overlay = dynamic_cast<WED_OverlayImage *> (entity)) != NULL)
			{
				IGISPointSequence * oring = overlay->GetOuterRing();
				if(oring->GetNumPoints() > 3)
				{
					WED_TextureNode * tn1 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(0));
					WED_TextureNode * tn2 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(1));
					WED_TextureNode * tn3 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(2));
					WED_TextureNode * tn4 = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(3));
					Point2 st1,st2,st3,st4, v1,v2,v3,v4;
					tn1->GetLocation(gis_UV,st1);	tn1->GetLocation(gis_Geo,v1);
					tn2->GetLocation(gis_UV,st2);	tn2->GetLocation(gis_Geo,v2);
					tn3->GetLocation(gis_UV,st3);	tn3->GetLocation(gis_Geo,v3);
					tn4->GetLocation(gis_UV,st4);	tn4->GetLocation(gis_Geo,v4);

					string img_file;
					overlay->GetImage(img_file);

					ITexMgr * mgr = WED_GetTexMgr(GetResolver());
					TexRef ref = mgr->LookupTexture(img_file.c_str(),false,tex_Compress_Ok);
					g->SetState(0,ref ? 1 : 0,0, 1, 1, 0, 0);
					if (ref) 
					{ 
						g->BindTex(mgr->GetTexID(ref),0);
						int vis_x, vis_y, tot_x, tot_y;
						mgr->GetTexInfo(ref,&vis_x,&vis_y,&tot_x,&tot_y, NULL, NULL);
						double sx = (double) vis_x / (double) tot_x;
						double sy = (double) vis_y / (double) tot_y;
						st1.x_ *= sx; st1.y_ *= sy;
						st2.x_ *= sx; st2.y_ *= sy;
						st3.x_ *= sx; st3.y_ *= sy;
						st4.x_ *= sx; st4.y_ *= sy;
					}
					glDisable(GL_CULL_FACE);
					glColor4f(1,1,1,overlay->GetAlpha());
					glBegin(GL_QUADS);
					glTexCoord2(st4);	glVertex2(GetZoomer()->LLToPixel(v4));
					glTexCoord2(st3);	glVertex2(GetZoomer()->LLToPixel(v3));
					glTexCoord2(st2);	glVertex2(GetZoomer()->LLToPixel(v2));
					glTexCoord2(st1);	glVertex2(GetZoomer()->LLToPixel(v1));
					glEnd();
					glEnable(GL_CULL_FACE);
				}
			}
		}
		break;
	}
	// This one is important - we want a clean state for the per-entity structure drawing. 
	// This code is executed ONCE prior to all pre-entity drawing, so even the first entity is visible.
	g->SetState(false,0,false,false,true,false,false);
	return true;
}


bool		WED_StructureLayer::GetRealLinesShowing(void) const
{
	return mRealLines;
}

void		WED_StructureLayer::SetRealLinesShowing(bool show)
{
	mRealLines = show;
	GetHost()->Refresh();
}

bool		WED_StructureLayer::GetVerticesShowing(void) const
{
	return mVertices;
}

void		WED_StructureLayer::SetVerticesShowing(bool show)
{
	mVertices = show;
	GetHost()->Refresh();
}

void		WED_StructureLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = 1;
	draw_ent_s = 1;
	cares_about_sel = 1;
	wants_clicks = 0;
}

void		WED_StructureLayer::DrawStructure(bool inCurrent, GUI_GraphState * g)
{
	// Drawing each icon during iteration is slow because:
	// - We have a ton of OGL state thrash.
	// - We have to make individual calls to PlotIcon, which does overhead work per icon.
	// So instead, we accumulate all of the icons into vectors and then blit them out all
	// at once.  This gives us one bind, one set state and one glBegin.  We're still transforming
	// vertices per frame, but that's okay -- the OGL state was the single really big cost.
	//
	// Note that we clear the vectors to keep them from building up forever, but their memory
	// is not dealloated, so this works up a high-water-mark of icons.  This is good, as it means
	// that in the long term our memory usage will stabilize.
	float scale = GetAirportIconScale();
	if (!mAirportIconsX.empty())
	{
		GUI_PlotIconBulk(g,"map_airport.png", mAirportIconsX.size(), &*mAirportIconsX.begin(), &*mAirportIconsY.begin(), &*mAirportIconsC.begin(), scale);
		mAirportIconsX.clear();
		mAirportIconsY.clear();
		mAirportIconsC.clear();
	}
	if (!mSeaportIconsX.empty())
	{
		GUI_PlotIconBulk(g,"map_seaport.png", mSeaportIconsX.size(), &*mSeaportIconsX.begin(), &*mSeaportIconsY.begin(), &*mSeaportIconsC.begin(), scale);
		mSeaportIconsX.clear();
		mSeaportIconsY.clear();
		mSeaportIconsC.clear();
	}
	if (!mHeliportIconsX.empty())
	{
		GUI_PlotIconBulk(g,"map_heliport.png", mHeliportIconsX.size(), &*mHeliportIconsX.begin(), &*mHeliportIconsY.begin(), &*mHeliportIconsC.begin(), scale);
		mHeliportIconsX.clear();
		mHeliportIconsY.clear();
		mHeliportIconsC.clear();
	}
}

