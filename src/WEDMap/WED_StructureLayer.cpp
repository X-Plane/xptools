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
#include "WED_AirportChain.h"
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
#include "WED_RampPosition.h"
#include "WED_ExclusionPoly.h"
#include "WED_FacadePlacement.h"
#include "WED_FacadeRing.h"
#include "WED_Windsock.h"
#include "WED_AirportBeacon.h"
#include "WED_SimpleBoundaryNode.h"
#include "WED_DrawUtils.h"
#include "GUI_DrawUtils.h"
#include "WED_TaxiRoute.h"
#include "WED_RoadEdge.h"
#include "WED_RoadNode.h"
#include "WED_AirportBoundary.h"
#include "WED_ATCLayer.h"
#include "WED_LinePlacement.h"
#include "WED_ResourceMgr.h"

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


bool		WED_StructureLayer::DrawEntityStructure		(bool inCurrent, IGISEntity * entity,  GUI_GraphState * g, bool selected, bool locked)
{
	//	g->SetState(false,0,false,   false,true,false,false);
	//  very carefully check that ALL operations that change the state to textured its re-set again,
	//  so we don't have to reset state for *evrvy* entity.

	WED_Color struct_color = selected ? (locked ? wed_StructureLockedSelected : wed_StructureSelected) :
										(locked ? wed_StructureLocked		 : wed_Structure);
	
	float * colorf = WED_Color_RGBA(struct_color);
	glColor4fv(colorf);

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
				Point2 loc = Segment2(bounds.p1,bounds.p2).midpoint();
				union { unsigned u; unsigned char c[4]; } colorb;
				for (int i = 0; i < 4; ++i)
					colorb.c[i] = intlim(colorf[i]*255, 0, 255);
				switch(airport->GetAirportType()) {
				case type_Airport:		mAirportIconsXY.push_back(loc.x());	mAirportIconsXY.push_back(loc.y());	mAirportIconsC.push_back(colorb.u);	break;
				case type_Seaport:		mSeaportIconsXY.push_back(loc.x());	mSeaportIconsXY.push_back(loc.y());	mSeaportIconsC.push_back(colorb.u);	break;
				case type_Heliport:		mHeliportIconsXY.push_back(loc.x());mHeliportIconsXY.push_back(loc.y());mHeliportIconsC.push_back(colorb.u);	break;
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

			float * white = locked ? WED_Color_RGBA(wed_StructureLocked) : WED_Color_RGBA(wed_pure_white);

			if (mRealLines) glColor4fv(white);
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

				float hdg = rwy->GetHeading();
				string nam, nam_1, nam_2;
				rwy->GetName(nam);
				auto pos = nam.find('/');
				if (pos != string::npos)
				{
					nam_1 = nam.substr(0, pos);
					nam_2 = nam.substr(pos + 1, string::npos);
				}
				else
					nam_1 = nam;

				if(!nam_1.empty())
				{
					glPushMatrix();                  // rotate the numbers properly
					glTranslatef(Th1.x(), Th1.y(), 0);
					glRotatef(-hdg, 0, 0, 1);
					GUI_FontDraw(g, font_UI_Basic, white, 0, 10, nam_1.c_str(), align_Center);
					glPopMatrix();
				}
				if (!nam_2.empty())
				{
					glPushMatrix();
					glTranslatef(Th2.x(), Th2.y(), 0);
					glRotatef(180 - hdg, 0, 0, 1);
					GUI_FontDraw(g, font_UI_Basic, white, 0, 10, nam_2.c_str(), align_Center);
					glPopMatrix();
				}
				g->SetTexUnits(0);
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
			IGISPoint *				pt = NULL;
			const char *			icon = NULL;

				 if (sub_class == WED_TowerViewpoint::sClass&& (tower  = SAFE_CAST(WED_TowerViewpoint, entity)) != NULL) pt = tower , icon = "map_towerview.png";
			else if (sub_class == WED_Windsock::sClass		&& (sock   = SAFE_CAST(WED_Windsock		 , entity)) != NULL) pt = sock  , icon = "map_windsock.png" ;
			else if (sub_class == WED_AirportBeacon::sClass && (beacon = SAFE_CAST(WED_AirportBeacon , entity)) != NULL) pt = beacon, icon = "map_beacon.png"   ;
			else if (sub_class == WED_SimpleBoundaryNode::sClass && (pt = SAFE_CAST(IGISPoint, entity)) != NULL) icon = "map_tree.png";
//			else pt = SAFE_CAST(IGISPoint,entity);
			if (pt)
			{
				if (icon)
				{
					// Pretty much all non-heading single point entities should have SOME kind of icon!!
					// Off-hand I think windsocks, tower viewpoints and airport-beacons are the only ones
					// we have right now.
					Point2			l;
					pt->GetLocation(gis_Geo,l);
					l = GetZoomer()->LLToPixel(l);
					GUI_PlotIcon(g,icon, l.x(),l.y(),0,GetFurnitureIconScale());
					g->SetTexUnits(0);
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
			WED_RampPosition *		ramp = NULL;
			const char *			icon = NULL;

				 if (sub_class == WED_LightFixture::sClass && (lfix = SAFE_CAST(WED_LightFixture,entity)) != NULL) pth = lfix, icon = "map_light.png"	;
			else if (sub_class == WED_AirportSign::sClass  && (sign = SAFE_CAST(WED_AirportSign ,entity)) != NULL) pth = sign, icon = "map_taxisign.png"	;
			else if (sub_class == WED_RampPosition::sClass && (ramp = SAFE_CAST(WED_RampPosition,entity)) != NULL) pth = ramp, icon = "map_rampstart.png";
			else if (sub_class == WED_TruckDestination::sClass && (pth = SAFE_CAST(IGISPoint_Heading,entity)) != NULL) icon = "map_servicedest.png";
			else pth = SAFE_CAST(IGISPoint_Heading,entity);
			if (pth)
			{
				Point2		l;
				pth->GetLocation(gis_Geo,l);
				l = GetZoomer()->LLToPixel(l);
				if (icon)
				{
					if (sub_class == WED_RampPosition::sClass && GetZoomer()->GetPPM() > 5)
					{
						glColor4f(0, 1, 0, 0.2);
						WED_ATCLayer_DrawAircraft(ramp, g, GetZoomer());
						glColor4fv(WED_Color_RGBA(struct_color));
					}

					GUI_PlotIcon(g,icon, l.x(),l.y(),pth->GetHeading(),GetFurnitureIconScale());
					g->SetTexUnits(0);
				}
				else // if(!selected)         	// selection layer will draw a big cross with handles over it, anyways. Does not save anything measureable.
				{
					Vector2		dir(0.0,3.0);
					dir.rotate_by_degrees(-pth->GetHeading());
					Vector2 r(dir.perpendicular_cw());
					glBegin(GL_LINES);
					glVertex2(l - dir);			glVertex2(l + dir * 2.0);
					glVertex2(l - r);			glVertex2(l + r);
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
			WED_Helipad * 					helipad = NULL;
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
					g->SetTexUnits(0);
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
			if (auto ps = dynamic_cast<IGISPointSequence *>(entity))
			{
				if (sub_class == WED_TaxiRoute::sClass && !locked)
				{
					auto tr = dynamic_cast<WED_TaxiRoute*>(entity);
					bool hot = tr->HasHotDepart() || tr->HasHotArrival();
					bool rwy = tr->IsRunway();
					bool ils = tr->HasHotILS();

					if (hot)                struct_color = selected ? wed_Hotzone_Selected : wed_Hotzone;
					else if (ils)           struct_color = selected ? wed_ILSzone_Selected : wed_ILSzone;
					else if (rwy)           struct_color = selected ? wed_Runway_Selected : wed_Runway;

					glColor4fv(WED_Color_RGBA(struct_color));
				}
				else if (kind = gis_Ring)
				{
					auto parent = dynamic_cast<WED_Thing*>(entity)->GetParent();
					if(parent->GetClass() == WED_ExclusionPoly::sClass)
						glColor4fv(WED_Color_RGBA_Alpha(wed_Link, 1.0, storage));
				}
				WED_MapZoomerNew* z = GetZoomer();
				bool showRealLines = mRealLines && z->GetPPM() * 0.4 <= MIN_PIXELS_PREVIEW;

				if(sub_class == WED_LinePlacement::sClass && showRealLines)
				{
					auto lin = dynamic_cast<WED_LinePlacement*>(entity);

					WED_ResourceMgr * rmgr = WED_GetResourceMgr(GetResolver());
					string vpath;
					const lin_info_t * linfo;
					lin->GetResource(vpath);
					if (rmgr->GetLin(vpath,linfo))
						if(!(linfo->rgb[0] == 0.0 && linfo->rgb[1] == 0.0 && linfo->rgb[2] == 0.0))
						{
							if (locked)
								glColor3fv(linfo->rgb);
							else                           // do some color correction to account for the green vs grey line
								glColor3f(min(1.0,linfo->rgb[0]+0.2),max(0.0,linfo->rgb[1]-0.0),min(1.0,linfo->rgb[2]+0.2));

							for(int i = 0; i < lin->GetNumSides(); ++i)
							{
								vector<Point2>	pts;
								SideToPoints(ps, i, z, pts);
								glLineWidth(3);
								glShape2v(GL_LINES, &*pts.begin(), pts.size());
								glLineWidth(1);
							}
							glColor4fv(WED_Color_RGBA(struct_color));
						}
				}

				set<int>		attrs;
				vector<Point2> pts;
				int n = ps->GetNumSides();
				glPointSize(5);

				for (int i = 0; i < n; ++i)
				{
					pts.clear();

					Bezier2		b;
					Point2 		mp(0,0);
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

						if(i == 0 && sub_class == WED_FacadeRing::sClass)
						{
							mp = b.midpoint(0.5);     // facade ground contact / 1st segment marker
						}
					}
					else
					{
						b.p1 = z->LLToPixel(b.p1);
						b.p2 = z->LLToPixel(b.p2);
						pts.push_back(b.p1);
						pts.push_back(b.p2);
						if (i == 0 && sub_class == WED_FacadeRing::sClass)
						{
							mp = b.p1 + Vector2(b.p1, b.p2) * 0.5;  // facade ground contact / 1st segment marker
						}
					}
					if (i == 0 && sub_class == WED_FacadeRing::sClass && Vector2(b.p1,b.p2).squared_length() > 20 * 20)
					{                                     	// facade ground contact / 1st segment marker
						glColor4fv(WED_Color_RGBA(wed_pure_white));
						GUI_PlotIcon(g, "handle_closeloop.png", mp.x(), mp.y(), 0.0, 0.7);
						g->SetTexUnits(0);
						glColor4fv(WED_Color_RGBA(struct_color));
					}
					if(sub_class == WED_FacadeRing::sClass)
					{
						const float colors[18] = { 1, 0, 0,	 1, 1, 0,  0, 1, 0,    // red, yellow, green
												   0, 1, 1,  0, 0, 1,  1, 0, 1,};  // aqua, blue, cyan
						Point2		tmp;
						auto fac = dynamic_cast<WED_FacadePlacement*>(dynamic_cast<WED_Thing*>(entity)->GetParent());
						if(fac && fac->HasCustomWalls())
						{
							ps->GetNthPoint(i)->GetLocation(gis_Param,tmp);
						}
						glColor3fv(colors + (((int) tmp.x()) % 6) * 3);
						glShapeOffset2v(GL_LINE_STRIP, &*pts.begin(), pts.size(), -2);
						glColor4fv(WED_Color_RGBA(struct_color));
					}

					if (sub_class == WED_AirportChain::sClass && showRealLines)
					{
						if (auto apt_node = dynamic_cast<WED_AirportNode*>(ps->GetNthPoint(i)))
							apt_node->GetAttributes(attrs);
					}
					DrawLineAttrs(&*pts.begin(), pts.size(), attrs);
					if(!attrs.empty()) glColor4fv(WED_Color_RGBA(struct_color));

					if(kind == gis_Edge && pts.size() >= 2)
					{
						bool re = sub_class == WED_RoadEdge::sClass;
						if(re || dynamic_cast<IGISEdge*>(ps)->IsOneway())
						{
							Vector2 orient1(pts[0],pts[1]);
							Vector2 orient2(pts[pts.size()-2],pts[pts.size()-1]);
							double sq_len = Vector2(b.p1,b.p2).squared_length();

							if( mVertices && re && (sq_len > 25*25) )
							{
								if( i == 0 )
									GUI_PlotIcon(g,"ArrowHeadRoadS.png", pts.front().x(), pts.front().y(),atan2(orient1.dx,orient1.dy) * RAD_TO_DEG,1);
								if(i == n-1)
									GUI_PlotIcon(g,"ArrowHeadRoadE.png", pts.back().x() , pts.back().y() ,atan2(orient2.dx,orient2.dy) * RAD_TO_DEG,1);
							}

							if(!re)
								GUI_PlotIcon(g,"handle_arrowhead.png", pts.back().x(), pts.back().y(),atan2(orient2.dx,orient2.dy) * RAD_TO_DEG, 1);

							g->SetTexUnits(0);
						}
					}
					// if (mVertices && kind != gis_Edge)	  // Gis EDGE points will be picked up separately!  That way we can get their hilite right.
					if (mVertices)
					{
						//	glColor4fv(WED_Color_RGBA(struct_color));  // Do this if green EdgeNdodes when unselected are desired
						glBegin(GL_POINTS);
						glVertex2(b.p1);
						if(i == n - 1)
							glVertex2(b.p2);
						glEnd();
					}
				}
				glPointSize(1);
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
			if(auto box = dynamic_cast<IGISBoundingBox*>(entity))
			{
				Point2	pts[2];
				box->GetMin()->GetLocation(gis_Geo,pts[0]);
				box->GetMax()->GetLocation(gis_Geo,pts[1]);
				if(locked || selected)
					glColor4fv(WED_Color_RGBA_Alpha(struct_color, 1.0/*mPavementAlpha*/, storage));
				else
					glColor4fv(WED_Color_RGBA_Alpha(wed_Link, 1.0/*mPavementAlpha*/, storage));

				vector<Point2> pix;
				BoxToPoints(pts[0], pts[1], GetZoomer(), pix);

				glBegin(GL_LINE_LOOP);
				glVertex2v(pix.data(), pix.size());
				glEnd();

				if(selected)
				{
					glColor4fv(WED_Color_RGBA_Alpha(struct_color, HILIGHT_ALPHA, storage));
					glDisable(GL_CULL_FACE);
					glBegin(GL_POLYGON);
					glVertex2v(pix.data(), pix.size());
					glEnd();
					glEnable(GL_CULL_FACE);
				}
			}
		}
		break;

	case gis_Composite:
		if(sub_class != WED_AirportBoundary::sClass && sub_class != WED_ExclusionPoly::sClass)    // not down-clickable in interior, but still highlighted interior
			break;
	case gis_Polygon:
		/******************************************************************************************************************************************************
		 * POLYGONS (TAXIWAYAS, ETC.)
		 ******************************************************************************************************************************************************/
		{
			if(auto poly = dynamic_cast<IGISPolygon*>(entity))
			{
				this->DrawEntityStructure(inCurrent, poly->GetOuterRing(), g, selected, locked);
				int n = poly->GetNumHoles();
				for (int c = 0; c < n; ++c)
					this->DrawEntityStructure(inCurrent, poly->GetNthHole(c), g, selected, locked);

				if(selected)
				{
					vector<Point2>	pts;
					vector<int>		hole_starts;

					PointSequenceToVector(poly->GetOuterRing(), GetZoomer(), pts, false);
					int n = poly->GetNumHoles();
					for (int i = 0; i < n; ++i)
					{
						hole_starts.push_back(pts.size());
						PointSequenceToVector(poly->GetNthHole(i), GetZoomer(), pts, false);
					}
					glColor4fv(WED_Color_RGBA_Alpha(struct_color, HILIGHT_ALPHA, storage));
					glFrontFace(GL_CCW);
					glPolygon2(pts, false, hole_starts, false);
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
	switch(entity->GetGISClass()) {
	case gis_Polygon:
		if(entity->GetGISSubtype() == WED_OverlayImage::sClass)
		{
			if(auto overlay = dynamic_cast<WED_OverlayImage *> (entity))
			{
				IGISPointSequence * oring = overlay->GetOuterRing();
				if(oring->GetNumPoints() > 3)
				{
					Point2 texUV[4], texLL[4];
					for(int i = 0; i < 4; i++)
					{
						WED_TextureNode * tn = dynamic_cast<WED_TextureNode *>(oring->GetNthPoint(i));
						tn->GetLocation(gis_UV, texUV[i]);
						tn->GetLocation(gis_Geo, texLL[i]);
					}
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
						for(int i = 0; i < 4; i++)
						{
							texUV[i].x_ *= (double) vis_x / (double) tot_x;
							texUV[i].y_ *= (double) vis_y / (double) tot_y;
						}
					}
					glDisable(GL_CULL_FACE);
					glColor4f(1,1,1,overlay->GetAlpha());
					auto gcp = overlay->GetGcpMat();
					if(gcp->size() > 3)                                  // draw a propperly projected/warped image
					{
						const int divs = intround(sqrt(gcp->size())) - 1;
						for(int x = 0; x < divs; x++)
						{
							const float df = 1.0f / (float) divs;
							const float x0 = x * df;
							glBegin(GL_TRIANGLE_STRIP);
							for(int y = 0; y <= divs; y++)
							{
								int idx = x + (divs+1) * y;
								float y0 = y * df;
								glTexCoord2f(x0, y0);
								glVertex2(GetZoomer()->LLToPixel(gcp->at(idx)));
								glTexCoord2f(x0+df, y0);
								glVertex2(GetZoomer()->LLToPixel(gcp->at(idx+1)));
							}
							glEnd();
						}
					}
					else
					{
						glBegin(GL_QUADS);
						for(int i = 0; i < 4; i++)
						{
							glTexCoord2(texUV[i]);
							glVertex2(GetZoomer()->LLToPixel(texLL[i]));
						}
						glEnd();
					}
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
	if (!mAirportIconsXY.empty())
	{
		GUI_PlotIconBulk(g,"map_airport.png", mAirportIconsXY.size()/2, mAirportIconsXY.data(), nullptr, mAirportIconsC.data(), scale);
		mAirportIconsXY.clear();
		mAirportIconsC.clear();
	}
	if (!mSeaportIconsXY.empty())
	{
		GUI_PlotIconBulk(g,"map_seaport.png", mSeaportIconsXY.size()/2, mSeaportIconsXY.data(), nullptr, mSeaportIconsC.data(), scale);
		mSeaportIconsXY.clear();
		mSeaportIconsC.clear();
	}
	if (!mHeliportIconsXY.empty())
	{
		GUI_PlotIconBulk(g,"map_heliport.png", mHeliportIconsXY.size()/2, mHeliportIconsXY.data(), nullptr, mHeliportIconsC.data(), scale);
		mHeliportIconsXY.clear();
		mHeliportIconsC.clear();
	}
}

