//
//  WED_ATCLayer.cpp
//  SceneryTools_xcode6
//
//  Created by Ben Supnik on 1/25/16.
//
//

#include "WED_Airport.h"
#include "WED_ATCLayer.h"
#include "WED_RampPosition.h"
#include "WED_RoadEdge.h"
#include "WED_TaxiRoute.h"
#include "WED_TruckDestination.h"

#include "AssertUtils.h"
#include "GISUtils.h"
#include "MathUtils.h"
#include "TexUtils.h"
#include "XESConstants.h"
#include "GUI_DrawUtils.h"
#include "GUI_Fonts.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"
#include "WED_DrawUtils.h"
#include "WED_EnumSystem.h"
#include "WED_HierarchyUtils.h"
#include "WED_MapZoomerNew.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif


WED_ATCLayer::WED_ATCLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host, zoomer, resolver)
{
}

WED_ATCLayer::~WED_ATCLayer()
{
}

void		WED_ATCLayer::DrawVisualization(bool inCurrent, GUI_GraphState * g)
{
	mServices.clear();
	mGTEdges.clear();
	mStarts.clear();
	mATCEdges.clear();
}

static double box_edge_distance(Point2 p, const Bbox2 b)   // returns positive if inside box, negative if outside
{
	Vector2 from_p1(b.p1, p);
	Vector2 from_p2(p, b.p2);
	return min(min(from_p1.dx, from_p2.dx), min(from_p1.dy, from_p2.dy));
}

static void lines_to_nearest(const vector<Segment2>& starts, const vector<Segment2>& edges, const Bbox2& screen_bounds, double err_2nd, GUI_GraphState * g)
{
	if (starts.size() * edges.size() > 10000) return;        // skip drawing too complex scenarios - takes too long

	for (auto pix : starts)
	{
		double nearest_dist(1e8), next_dist(1e8);
		Point2 nearest_pix, next_pix;
		for (auto pix_seg : edges)
		{
			double dist;
			if (pix_seg.collinear_has_on(pix.p1))
			{
				Point2 p = pix_seg.projection(pix.p1);
				dist = pix.p1.squared_distance(p);
				if (dist < next_dist)
				{
					if (dist < nearest_dist)
					{
						next_dist = nearest_dist; nearest_dist = dist;
						next_pix = nearest_pix;   nearest_pix = p;
					}
					else
					{
						next_dist = dist;
						next_pix = p;
					}
				}
				continue;
			}
			dist = pix.p1.squared_distance(pix_seg.p1);
			if (dist < next_dist)
			{
				if (dist < nearest_dist)
				{
					next_dist = nearest_dist; nearest_dist = dist;
					next_pix = nearest_pix;   nearest_pix = pix_seg.p1;
				}
				else if(Vector2(nearest_pix, pix_seg.p1).squared_length() > 0.1 * dist)  // only take 2nd nearest if it goes to a noticeably different location
				{                                                                        // that helps not only avoiding bogous hits on segment endpoints, but
					next_dist = dist;                                                    // also keeping track of 'real' locations that would otherwise rank 3rd
					next_pix = pix_seg.p1;
				}
			}
			dist = pix.p1.squared_distance(pix_seg.p2);
			if (dist < next_dist)
			{
				if (dist < nearest_dist)
				{
					next_dist = nearest_dist; nearest_dist = dist;
					next_pix = nearest_pix;   nearest_pix = pix_seg.p2;
				}
				else if (Vector2(nearest_pix, pix_seg.p2).squared_length() > 0.1 * dist)
				{
					next_dist = dist;
					next_pix = pix_seg.p2;
				}
			}
		}
		double edge_dist = box_edge_distance(pix.p1, screen_bounds);
		// avoid showing lines if its not certain that all other edges that could be closer are on-screen, i.e. tested for
		// the alternative would be to always test against ALL edges at that airport - which takes a lot of time at large apts
		// e.g. KATL with ~900 taxi and truck edges and 120 starts

		if (nearest_dist < edge_dist * edge_dist)
		{
			glLineStipple(2, 0x24FF);
			glBegin(GL_LINE_STRIP);
			glVertex2(pix.p2);
			glVertex2(pix.p1);
			glVertex2(nearest_pix);
			glEnd();
			GUI_PlotIcon(g, "handle_closeloop.png", pix.p1.x(), pix.p1.y(), 0.0, 0.7);
			g->SetTexUnits(0);
			if (sqrt(next_dist) < sqrt(nearest_dist) + err_2nd)
			if (Vector2(next_pix, nearest_pix).squared_length() > 0.1 * next_dist)  // dont draw second if its going to almost same location.
			{
				glLineStipple(2, 0x10FF);
				glBegin(GL_LINES);
				glVertex2(pix.p1);
				glVertex2(next_pix);
				glEnd();
			}
		}
	}
}

void		WED_ATCLayer::DrawSelected(bool inCurrent, GUI_GraphState * g)
{

	Bbox2 bnds;
	GetZoomer()->GetPixelBounds(bnds.p1.x_, bnds.p1.y_, bnds.p2.x_, bnds.p2.y_);

//	printf("GT %ld, ATC %ld\n", mGTEdges.size(), mATCEdges.size());

	g->SetState(0, 0, 0, 0, 0, 0, 0);
	glEnable(GL_LINE_STIPPLE);
	glLineWidth(2.0);

	glColor4f(1, 1, 1, 1); // white for roads
	lines_to_nearest(mServices, mGTEdges, bnds, GetZoomer()->GetPPM() * 6.0, g);

	glColor4f(1, 1, 0, 1); // yellow for ATC routes
	lines_to_nearest(mStarts, mATCEdges, bnds, GetZoomer()->GetPPM() * 3.0, g);

	glDisable(GL_LINE_STIPPLE);
	glLineWidth(1.0);
}


static void make_arrow_line(Point2 p[5])
{
	// incoming:
	// 0------------1
	// S            D
	// 3------------2

	// outgoing:
	// 0-----------1
	// S            2
	// 4-----------3

	p[4] = p[3];
	p[3] = p[2];

	Vector2 v1to3(p[1],p[3]);

	v1to3 *= 0.5;
	p[2] = p[1] + v1to3;

	v1to3 = v1to3.perpendicular_cw();
	p[1] += v1to3;
	p[3] += v1to3;
}

void WED_ATCLayer_DrawAircraft(WED_RampPosition * pos, GUI_GraphState * g, WED_MapZoomerNew * z)
{
		int id = 0;

		switch(pos->GetWidth()) {
		case width_A:	id = GUI_GetTextureResource("ClassA.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_B:	id = GUI_GetTextureResource("ClassB.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_C:	id = GUI_GetTextureResource("ClassC.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_D:	id = GUI_GetTextureResource("ClassD.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_E:	id = GUI_GetTextureResource("ClassE.png",tex_Linear|tex_Mipmap,NULL);	break;
		case width_F:	id = GUI_GetTextureResource("ClassF.png",tex_Linear|tex_Mipmap,NULL);	break;
		}

		if (id)
		{
			g->BindTex(id, 0);
			g->SetTexUnits(1);

			Point2 tips[4];
			pos->GetTips(tips);
			Point2	c[4];
			Vector2 span(tips[1], tips[3]);
			c[1] = tips[0] + span * 0.5;
			c[2] = tips[0] - span * 0.5;
			c[3] = tips[2] - span * 0.5;
			c[0] = tips[2] + span * 0.5;

			z->LLToPixelv(c,c,4);

			glBegin(GL_QUADS);
			glTexCoord2f(0,0);	glVertex2(c[0]);
			glTexCoord2f(0,1);	glVertex2(c[1]);
			glTexCoord2f(1,1);	glVertex2(c[2]);
			glTexCoord2f(1,0);	glVertex2(c[3]);
			glEnd();
			g->SetTexUnits(0);
		}
}

static bool is_ground_traffic_route(WED_Thing * r)
{
	return static_cast<WED_TaxiRoute*>(r)->AllowTrucks();
}

static bool is_aircraft_taxi_route(WED_Thing * r)
{
	return static_cast<WED_TaxiRoute*>(r)->AllowAircraft();
}

static void	Quad_2to4pix(const Point2 ends[2], double width_pix, Point2 corners[4])
{
	Vector2 perp(Vector2(ends[0], ends[1]).perpendicular_cw());
	perp.normalize();
	perp *= width_pix * 0.5;

	// corners:
	// 0------------1
	// S            D
	// 3------------2

	corners[0] = ends[0] - perp;
	corners[3] = ends[0] + perp;
	corners[1] = ends[1] - perp;
	corners[2] = ends[1] + perp;
}


bool	WED_ATCLayer::DrawEntityStructure		(bool inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
	if(entity->GetGISSubtype() == WED_RampPosition::sClass)
	{
		WED_RampPosition * pos = dynamic_cast<WED_RampPosition *>(entity);
		DebugAssert(pos);
		WED_MapZoomerNew * z = GetZoomer();
		if(z->GetPPM() > 5)
			glColor4f(0, 1, 0, 0.2); // avoid getting more opaque when StructureLayer preview kicks in as well
		else
			glColor4f(0, 1, 0, 0.4);
		WED_ATCLayer_DrawAircraft(pos, g, z);
		}
	if(entity->GetGISSubtype() == WED_TaxiRoute::sClass)
	{
		WED_TaxiRoute * seg = dynamic_cast<WED_TaxiRoute *>(entity);
		DebugAssert(seg);

		Point2 ends[2];
		seg->GetNthPoint(0)->GetLocation(gis_Geo, ends[0]);
		seg->GetNthPoint(1)->GetLocation(gis_Geo, ends[1]);

		int icao_width = seg->GetWidth();
		bool hot = seg->HasHotArrival() || seg->HasHotDepart();
		bool ils = seg->HasHotILS();
		bool rwy = seg->IsRunway();
		bool road = seg->AllowTrucks() && !seg->AllowAircraft();
		bool one_way = seg->IsOneway();

		int mtr1,mtr2;
		if(road)
			mtr1 = mtr2 = one_way ? 4.0 : 8.0;
		else
			switch(icao_width)
			{
			default:
			case width_A:	mtr1 = 4.5;		mtr2 = 15.0;	break;
			case width_B:	mtr1 = 6.0;		mtr2 = 24.0;	break;
			case width_C:	mtr1 = 9.0;		mtr2 = 36.0;	break;
			case width_D:	mtr1 = 14.0;	mtr2 = 52.0;	break;
			case width_E:	mtr1 = 14.0;	mtr2 = 65.0;	break;
			case width_F:	mtr1 = 16.0;	mtr2 = 80.0;	break;
			}

		Point2	c[5], d[5];
		Quad_2to4(ends, mtr1, c);
		Quad_2to4(ends, mtr2, d);

		g->SetState(0, 0, 0, 0, 1, 0, 0);
		if(rwy && hot)
			glColor4f(0.9,0.1,0.7,0.4);
		else if(hot)
			glColor4f(1,0,0,0.4);
		else if(ils)
			glColor4f(0.8,0.5,0,0.4);
		else if(road) //Warning! Because a ground route can also have IsRunway() == true, this check must come first
			glColor4f(1, 1, 1, 0.2);
		else if(rwy)
			glColor4f(0.0,0.2,0.6,0.4);
		else
			glColor4f(1,1,0,0.4);

		GetZoomer()->LLToPixelv(c,c,4);
		GetZoomer()->LLToPixelv(d,d,4);

		int np = 4;
		if(one_way)
		{
			make_arrow_line(c);
			make_arrow_line(d);
			np = 5;
		}
		else
		{   // help visibility of vertices by creating little gaps in most opaque part of routeS
			Vector2 dir(c[0],c[1]);
			dir.normalize();
			dir *= 2.0;
			c[0] += dir;
			c[1] -= dir;
			c[2] -= dir;
			c[3] += dir;
		}
		if (rwy && hot)
			glColor4f(0.9, 0.1, 0.7, 0.5);  // purple
		else if (hot)
			glColor4f(1, 0, 0, 0.5);        // red
		else if (ils)
			glColor4f(0.8, 0.5, 0, 0.5);    // orange
		else if (road) //Warning! Because a ground route can also have IsRunway() == true, this check must come first
			glColor4f(1, 1, 1, 0.4);        // white
		else if (rwy)
			glColor4f(0.0, 0.2, 0.6, 0.4);  // blue
		else
			glColor4f(1, 1, 0, 0.6);        // yellow
		glBegin(GL_TRIANGLE_FAN);
		glVertex2v(c,np);
		glEnd();

		if (!road)
		{
			GLfloat currentColor[4];
			glGetFloatv(GL_CURRENT_COLOR, currentColor);
			currentColor[3] = 0.25;
			glColor4fv(currentColor);
			glBegin(GL_TRIANGLE_FAN);
			glVertex2v(d, np);
			glEnd();
		}
		if (seg->AllowAircraft())              // display name of taxi route
		{
			string nam;
			if(seg->GetRunway() != atc_rwy_None)   // ideally this would not be needed, but cant figure a way to fix up name upon earth.wed.xml import
				nam = ENUM_Desc(seg->GetRunway());
			else
				seg->GetName(nam);

			if(!nam.empty())
			{
				if (d[1].squared_distance(d[2]) > 20*20 &&         // draw labels only if segment wide enough
					d[0].squared_distance(d[1]) > sqr(20+6.0*nam.size()) )      // and long enough
				{
					glPushMatrix();
					Point2 label_xy = GetZoomer()->LLToPixel(Midpoint2(ends[0],ends[1]));
					glTranslatef(label_xy.x(), label_xy.y(), 0);
					double hdg = VectorDegs2NorthHeading(ends[0],ends[1], Vector2(ends[0],ends[1]));
					if (hdg > 180.0) hdg -=180; // dont print label upside down
					glRotated(90.0-hdg,0,0,1);
					float white[4] = { 1.0, 1.0, 1.0, 1.0 };
					GUI_FontDraw(g, font_UI_Basic, white, 0, -4, nam.c_str(), align_Center);
					glPopMatrix();
					g->SetTexUnits(0);
				}
			}
			mATCEdges.push_back(Segment2(ends[0], ends[1]));
		}
	}
#if 1 // ROAD_EDITING
	else if(entity->GetGISSubtype() == WED_RoadEdge::sClass)
	{
		WED_RoadEdge * seg = dynamic_cast<WED_RoadEdge *>(entity);
		DebugAssert(seg);

		WED_MapZoomerNew * z = GetZoomer();
		double mtr = seg->GetWidth();

		Bezier2 b;
		bool isBez = seg->GetSide(gis_Geo, 0, b);

		if(mtr > 0.0)
		{
			g->SetState(0, 0, 0, 0, 1, 0, 0);
			glColor4f(0.4, 1, 1, 0.4);         // desaturated cyan

			if(isBez)
			{
				vector<Point2> pts;
				vector<Point2> strip;

				float offs_pix = 0.5 * mtr * z->GetPPM();

				b.p1 = z->LLToPixel(b.p1);
				b.p2 = z->LLToPixel(b.p2);
				b.c1 = z->LLToPixel(b.c1);
				b.c2 = z->LLToPixel(b.c2);

				int point_count = BezierPtsCount(b,z);
				pts.reserve(point_count+1);

				for (int n = 0; n <= point_count; ++n)
					pts.push_back(b.midpoint((float) n / (float) point_count));

				strip.reserve(2*pts.size());

				Vector2	dir;
				for (int i = 0; i <= point_count; ++i)
				{
					Vector2	dir1,dir2;
					if (i > 0  )           dir1 = Vector2(pts[i-1],pts[i  ]);
					if (i < point_count-1) dir2 = Vector2(pts[i  ],pts[i+1]);
					dir = dir1+dir2;
					dir = dir.perpendicular_ccw();
					dir.normalize();
					dir *= offs_pix;
					strip.push_back(pts[i] - dir);
					strip.push_back(pts[i] + dir);
				}
				if(seg->IsOneway())
				{
					dir = dir.perpendicular_ccw();
					while(Vector2(strip.back(), b.p2).squared_length() < sqr(offs_pix) * 1.6 && strip.size() > 6)
					{
						strip.pop_back();
						strip.pop_back();
					}
					strip.push_back(b.p2);
				}

				glBegin(GL_TRIANGLE_STRIP);
				glVertex2v(strip.data(), strip.size());
				glEnd();

				int layers = min(max(seg->GetStartLayer(), seg->GetEndLayer()), 3);
				if(layers)
				{
					glColor4f(0.4, 1, 1, 1.0);         // bright cyan
					glLineWidth(2);
					for(int l = 1; l <= layers; l++)
					{
						int skip_pts = seg->GetStartLayer() >= l ? 0 :  pts.size()/4;
						Point2 * first = pts.data() + skip_pts;
						int num_pts = pts.size() - skip_pts - (seg->GetEndLayer() >= l ? 0 : pts.size()/4);

						glShapeOffset2v(GL_LINE_STRIP, first, num_pts,  offs_pix + 3 * (l-1));
						glShapeOffset2v(GL_LINE_STRIP, first, num_pts, -offs_pix - 3 * (l-1));
					}
				}
			}
			else
			{
				Point2 ends[2]; ends[0] = b.p2; ends[1] = b.p1;
				Point2 c[5];
				int np = 4;
				if(seg->IsOneway())
				{
					np = 5;
					c[0] = ends[0];
					Vector2 dir(ends[0], ends[1]);
					dir.normalize();
					dir *= 0.5e-4;
					ends[0] += dir;
					Quad_2to4(ends, mtr, c+1);
				}
				else
					Quad_2to4(ends, mtr, c);
				z->LLToPixelv(c,c,np);

				glBegin(GL_TRIANGLE_FAN);
					glVertex2v(c,np);
				glEnd();

				int layers = min(max(seg->GetStartLayer(), seg->GetEndLayer()), 3);
				if(layers)
				{
					glColor4f(0.4, 1, 1, 1.0);         // bright cyan
					glLineWidth(2);
					for(int l = 1; l <= layers; l++)
					{
						Point2 sides[4] = { c[0], c[1], c[2], c[3] };
						Vector2 dir(c[0], c[1]);
						if(seg->GetStartLayer() < l) { sides[0] += dir * 0.25; sides[3] += dir * 0.25; }
						if(seg->GetEndLayer() < l)   { sides[1] -= dir * 0.25; sides[2] -= dir * 0.25; }

						if(l > 1)
						{
							Vector2 perp(c[1],c[2]);
							perp.normalize();
							perp *= 3 * (l - 1);
							sides[0] -= perp; sides[1] -= perp; sides[2] += perp; sides[3] += perp;
						}

						glBegin(GL_LINES);
						glVertex2v(sides,4);
						glEnd();
					}
				}
			}
		}
	}
#endif
	return true;
}

void		WED_ATCLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = false;
	cares_about_sel = false;
	draw_ent_s = true;
	wants_clicks = false;
}
