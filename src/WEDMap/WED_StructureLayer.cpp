#include "WED_StructureLayer.h"
#include "IGIS.h"
#include "GUI_GraphState.h"
#include "WED_Colors.h"
#include "GISUtils.h"
#include "WED_EnumSystem.h"
#include "WED_MapZoomerNew.h"
#include "WED_AirportNode.h"
#include "XESConstants.h"
#include "WED_Runway.h"
#include "WED_Helipad.h"
#include "WED_LightFixture.h"
#include "WED_Taxiway.h"
#include "WED_Sealane.h"
#include "WED_AirportSign.h"
#include "WED_TowerViewpoint.h"
#include "WED_Airport.h"
#include "WED_RampPosition.h"
#include "WED_Windsock.h"
#include "WED_AirportBeacon.h"

#include "GUI_DrawUtils.h"

#define	BEZ_COUNT 100

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <gl/gl.h>
	#include <gl/glu.h>
#endif

WED_StructureLayer::WED_StructureLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(h, zoomer, resolver)
{
	mRealLines = true;
	mPavementAlpha = 0.5;
}

WED_StructureLayer::~WED_StructureLayer()
{
}

inline void glVertex2(const Point2& p) { glVertex2d(p.x,p.y); }

inline void	glVertex2v(const Point2 * p, int n) { while(n--) { glVertex2d(p->x,p->y); ++p; } }
inline void glShape2v(GLenum mode,  const Point2 * p, int n) { glBegin(mode); glVertex2v(p,n); glEnd(); }

inline void glShapeOffset2v(GLenum mode,  const Point2 * pts, int n, double offset)
{
	glBegin(mode); 
	for (int i = 0; i < n; ++i)
	{
		Vector2	dir1,dir2;
		if (i > 0  ) dir1 = Vector2(pts[i-1],pts[i  ]);
		if (i < n-1) dir2 = Vector2(pts[i  ],pts[i+1]);
		dir1.normalize();
		dir2.normalize();
		Vector2	dir = dir1+dir2;
		dir = dir.perpendicular_ccw();
		dir.normalize();
		dir *= offset;
		glVertex2d(pts[i].x + dir.dx, pts[i].y + dir.dy);		
	}	
	glEnd(); 
}



static void PointSequenceToVector(IGISPointSequence * ps, WED_MapZoomerNew * z, vector<Point2>& pts, vector<int>& contours, int is_hole)
{
	int n = ps->GetNumSides();
	for (int i = 0; i < n; ++i)
	{
		Segment2	s;
		Bezier2		b;
		if (ps->GetSide(i,s,b))
		{
			b.p1 = z->LLToPixel(b.p1);
			b.p2 = z->LLToPixel(b.p2);
			b.c1 = z->LLToPixel(b.c1);
			b.c2 = z->LLToPixel(b.c2);
			pts.reserve(pts.capacity() + BEZ_COUNT);
			contours.reserve(contours.capacity() + BEZ_COUNT);
			for (int k = 0; k < BEZ_COUNT; ++k)
			{
				pts.push_back(b.midpoint((float) k / (float) BEZ_COUNT));
				contours.push_back((k == 0 && i == 0) ? is_hole : 0);
			}
			
			if (i == n-1 && !ps->IsClosed())
			{
				pts.push_back(b.p2);
				contours.push_back(0);
			}
		} 
		else
		{
			pts.push_back(z->LLToPixel(s.p1));
			contours.push_back(i == 0 ? is_hole : 0);
			if (i == n-1 && !ps->IsClosed())
			{
				pts.push_back(s.p2);
				contours.push_back(0);
			}
		}
	}
}

#if !IBM
#define CALLBACK
#endif

static void CALLBACK TessBegin(GLenum mode)		{ glBegin(mode);		 }
static void CALLBACK TessEnd(void)				{ glEnd();				 }
static void CALLBACK TessVertex(const Point2 * p){ glVertex2d(p->x,p->y); }

static void glPolygon2(const Point2 * pts, const int * contours, int n)
{
	GLUtesselator * tess = gluNewTess();

	gluTessCallback(tess, GLU_TESS_BEGIN,	(void (CALLBACK *)(void))TessBegin);
	gluTessCallback(tess, GLU_TESS_END,		(void (CALLBACK *)(void))TessEnd);
	gluTessCallback(tess, GLU_TESS_VERTEX,	(void (CALLBACK *)(void))TessVertex);

	gluBeginPolygon(tess);
	
	while(n--)
	{
		if (contours && *contours++)	gluNextContour(tess, GLU_INTERIOR);
	
		double	xyz[3] = { pts->x, pts->y, 0 };
		gluTessVertex(tess, xyz, (void*) pts++);
	}

	gluEndPolygon (tess);
	gluDeleteTess(tess);
}

static void DrawLineAttrs(GUI_GraphState * state, const Point2 * pts, int count, const set<int>& attrs, WED_Color c)
{
	if (attrs.empty())
	{
		glColor4fv(WED_Color_RGBA(c));
		glShape2v(GL_LINE_STRIP, pts, count);
		return;
	} 
	
	// ------------ STANDARD TAXIWAY LINES ------------
	if (attrs.count(line_SolidYellow))
	{
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_BrokenYellow))
	{
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_DoubleSolidYellow))
	{
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}
	if (attrs.count(line_RunwayHold))
	{
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);		
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);		
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);		
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);		
	}
	if (attrs.count(line_OtherHold))
	{
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,1.5);		
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-1.5);		
	}
	if (attrs.count(line_ILSHold))
	{
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(0,0,0,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(1,1,0,1);		
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x1111);
		glShape2v(GL_LINE_STRIP, pts, count);					
	}
	if (attrs.count(line_ILSCriticalCenter))
	{
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(0,0,0,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);				
	}
	if (attrs.count(line_WideBrokenSingle))
	{
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}
	if (attrs.count(line_WideBrokenDouble))
	{
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}

	// ------------ ROADWAY TAXIWAY LINES ------------
	if (attrs.count(line_SolidWhite))
	{
		glColor4f(1,1,1,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}
	if (attrs.count(line_Chequered))
	{
		glColor4f(1,1,1,1);
		glLineWidth(6);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(0,0,0,1);
		glLineWidth(2);		
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(1,1,1,1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0xCCCCC);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}
	if (attrs.count(line_BrokenWhite))
	{
		glColor4f(1,1,1,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}
	
	// ------------ BLACK-BACKED TAXIWAY LINES ------------
	if (attrs.count(line_BSolidYellow))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_BBrokenYellow))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_BDoubleSolidYellow))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}
	if (attrs.count(line_BRunwayHold))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(12);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);	
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);		
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);		
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,3);		
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-3);		
	}
	if (attrs.count(line_BOtherHold))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(6);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,1.5);		
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-1.5);		
	}
	if (attrs.count(line_BILSHold))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(7);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(1,1,0,1);		
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x1111);
		glShape2v(GL_LINE_STRIP, pts, count);					
	}
	if (attrs.count(line_BILSCriticalCenter))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(7);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(5);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);				
	}
	if (attrs.count(line_BWideBrokenSingle))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}
	if (attrs.count(line_BWideBrokenDouble))
	{
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(5);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
	}

	// ------------ LIGHTS ------------
	if (attrs.count(line_TaxiCenter))
	{
		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_TaxiEdge))
	{
		glColor4f(0,0,1,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_HoldLights))
	{
		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7070);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_HoldLightsPulse))
	{
		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(0.3,0.1,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_HoldShortCenter))
	{
		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,0.5,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	if (attrs.count(line_BoundaryEdge))
	{
		glColor4f(1,0,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
	}
	
	glLineWidth(1);
	glDisable(GL_LINE_STIPPLE);
}

void		WED_StructureLayer::DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
	g->SetState(false,0,false,   false,true,false,false);
	
	int locked = 0;
	WED_Entity * thing = dynamic_cast<WED_Entity *>(entity);
	while(thing)
	{
		if (thing->GetLocked()) { locked=1;break;}
		thing = dynamic_cast<WED_Entity *>(thing->GetParent());
	}
	
	WED_Color struct_color = selected ? (locked ? wed_StructureLockedSelected : wed_StructureSelected) :
										(locked ? wed_StructureLocked		 : wed_Structure);
	
	GISClass_t kind = entity->GetGISClass();

	IGISPoint *						pt;
	IGISPoint_Heading *				pth;
	IGISPoint_WidthLength *			ptwl;
	IGISPointSequence *				ps;
	IGISLine_Width *				lw;
	IGISPolygon *					poly;
	
	WED_Taxiway *					taxi;
	WED_Sealane *					sea;
	
	WED_Runway *					rwy;
	WED_Helipad *					helipad;

	WED_LightFixture *				lfix;
	WED_AirportSign *				sign;
	WED_RampPosition *				ramp;

	WED_TowerViewpoint *			tower;
	WED_Windsock *					sock;
	WED_AirportBeacon *				beacon;
	
	WED_Airport *					airport;

	float							storage[4];

	glColor4fv(WED_Color_RGBA(struct_color));

	float icon_scale = GetZoomer()->GetPPM() * 2.0;
	if (icon_scale > 1.0) icon_scale = 1.0;
	
	/******************************************************************************************************************************************************
	 * RUNWAY DRAWING
	 ******************************************************************************************************************************************************/	
	if ((airport = SAFE_CAST(WED_Airport, entity)) != NULL)
	{
		if (GetZoomer()->GetPPM() < 0.005)
		{
			Bbox2	bounds;
			airport->GetBounds(bounds);
			Point2 loc = GetZoomer()->LLToPixel(Segment2(bounds.p1,bounds.p2).midpoint());
			switch(airport->GetAirportType()) {
			case type_Airport:		GUI_PlotIcon(g,"map_airport.png", loc.x,loc.y,0,1.0);	break;
			case type_Seaport:		GUI_PlotIcon(g,"map_seaport.png", loc.x,loc.y,0,1.0);	break;
			case type_Heliport:		GUI_PlotIcon(g,"map_heliport.png", loc.x,loc.y,0,1.0);	break;
			}
		}
	}
	else if ((rwy = SAFE_CAST(WED_Runway,entity)) != NULL)
	{
		Point2 	corners[4], shoulders[8], disp1[4], disp2[4], blas1[4], blas2[4];
		bool	has_shoulders, has_disp1, has_disp2, has_blas1, has_blas2;
		
		// First, transform our geometry.		
						rwy->GetCorners(corners);					GetZoomer()->LLToPixelv(corners, corners, 4);
		if (has_blas1 = rwy->GetCornersBlas1(blas1))				GetZoomer()->LLToPixelv(blas1, blas1, 4);
		if (has_blas2 = rwy->GetCornersBlas2(blas2))				GetZoomer()->LLToPixelv(blas2, blas2, 4);
		if (has_disp1 = rwy->GetCornersDisp1(disp1))				GetZoomer()->LLToPixelv(disp1, disp1, 4);
		if (has_disp2 = rwy->GetCornersDisp2(disp2))				GetZoomer()->LLToPixelv(disp2, disp2, 4);		
		if (has_shoulders = rwy->GetCornersShoulders(shoulders))	GetZoomer()->LLToPixelv(shoulders, shoulders, 8);

		// "Solid" geometry.		
		glColor4fv(WED_Color_Surface(rwy->GetSurface(),mPavementAlpha, storage));
									glShape2v(GL_QUADS, corners, 4);
		if (has_blas1)				glShape2v(GL_QUADS, blas1,4);
		if (has_blas2)				glShape2v(GL_QUADS, blas2,4);
		glColor4fv(WED_Color_Surface(rwy->GetShoulder(),mPavementAlpha, storage));
		if (has_shoulders)			glShape2v(GL_QUADS, shoulders, 8);		

		//  "Outline" geometry	
		glColor4fv(WED_Color_RGBA(struct_color));
		glShape2v(GL_LINE_LOOP, corners, 4);
		glBegin(GL_LINES);
		glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
		glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
		glEnd();

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

		if (mRealLines) glColor4f(1,1,1,1);
		if (has_disp1)				glShape2v(GL_LINE_LOOP, disp1,4);
		if (has_disp2)				glShape2v(GL_LINE_LOOP, disp2,4);
	
	}
	else switch(kind) {
	case gis_Point:
	case gis_Point_Bezier:
		/******************************************************************************************************************************************************
		 * NON-DIRECTIONAL POINT ELEMENTS
		 ******************************************************************************************************************************************************/		
		if ((pt = SAFE_CAST(IGISPoint,entity)) != NULL)
		{
			Point2 l;
			pt->GetLocation(l);
			l = GetZoomer()->LLToPixel(l);
			
			if ((tower = SAFE_CAST(WED_TowerViewpoint, pt)) != NULL)
			{
				GUI_PlotIcon(g,"map_towerview.png", l.x,l.y,0,icon_scale);
			}
			else if ((sock = SAFE_CAST(WED_Windsock, pt)) != NULL)
			{
				GUI_PlotIcon(g,"map_windsock.png", l.x,l.y,0,icon_scale);
			}
			else if ((beacon = SAFE_CAST(WED_AirportBeacon, pt)) != NULL)
			{
				GUI_PlotIcon(g,"map_beacon.png", l.x,l.y,0,icon_scale);
			}
			else
			{
				glBegin(GL_LINES);
				glVertex2f(l.x, l.y - 3);
				glVertex2f(l.x, l.y + 3);
				glVertex2f(l.x - 3, l.y);
				glVertex2f(l.x + 3, l.y);
				glEnd();
			}
		}
		break;

	case gis_Point_Heading:
		/******************************************************************************************************************************************************
		 * DIRECTIONAL POINT ELEMENTS
		 ******************************************************************************************************************************************************/			
		if ((pth = SAFE_CAST(IGISPoint_Heading,entity)) != NULL)
		{
			Point2 l;
			pth->GetLocation(l);
			Vector2	dir;
			NorthHeading2VectorMeters(l,l,pth->GetHeading(),dir);
			Vector2 r(dir.perpendicular_cw());
			l = GetZoomer()->LLToPixel(l);
			
			if ((lfix = SAFE_CAST(WED_LightFixture, pth)) != NULL)
			{
				GUI_PlotIcon(g,"map_light.png", l.x,l.y,atan2(dir.dx,dir.dy) * RAD_TO_DEG,icon_scale);
			}
			else if ((sign = SAFE_CAST(WED_AirportSign,pth)) != NULL)
			{
				GUI_PlotIcon(g,"map_taxisign.png", l.x,l.y,atan2(dir.dx,dir.dy) * RAD_TO_DEG,icon_scale);
			}
			else if ((ramp = SAFE_CAST(WED_RampPosition,pth)) != NULL)
			{
				GUI_PlotIcon(g,"map_rampstart.png", l.x,l.y,atan2(dir.dx,dir.dy) * RAD_TO_DEG,icon_scale);
			}
			else
			{
				glBegin(GL_LINES);
				glVertex2(l - dir * 3.0);			glVertex2(l + dir * 6.0);
				glVertex2(l - r   * 3.0);			glVertex2(l + r   * 3.0);
				glEnd();
			}
		}
		break;
	case gis_Point_HeadingWidthLength:
		/******************************************************************************************************************************************************
		 * HELIPADS AND OTHER GIS-SIMILAR
		 ******************************************************************************************************************************************************/			
		if ((ptwl = SAFE_CAST(IGISPoint_WidthLength,entity)) != NULL)
		{
			helipad = SAFE_CAST(WED_Helipad, ptwl);
		
			Point2 corners[4];
			ptwl->GetCorners(corners);
			GetZoomer()->LLToPixelv(corners, corners, 4);

			if (helipad)
			{	
				glColor4fv(WED_Color_Surface(helipad->GetSurface(), mPavementAlpha, storage));
				glShape2v(GL_QUADS, corners, 4);
				glColor4fv(WED_Color_RGBA(struct_color));
			}

			glShape2v(GL_LINE_LOOP, corners, 4);
			
			glBegin(GL_LINES);
			glVertex2(Segment2(corners[0],corners[1]).midpoint(0.5));
			glVertex2(Segment2(corners[2],corners[3]).midpoint(0.5));
			glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
			glVertex2(Segment2(
						Segment2(corners[0],corners[1]).midpoint(1.5),
						Segment2(corners[3],corners[2]).midpoint(1.5)).midpoint());
			glEnd();
			
			if (helipad)
			{
				Point2	p;
				helipad->GetLocation(p);
				p = GetZoomer()->LLToPixel(p);
				GUI_PlotIcon(g, "map_helipad.png", p.x, p.y, 0,icon_scale);
			}
		}
		break;
	case gis_PointSequence:
	case gis_Line:
	case gis_Ring:
	case gis_Chain:
		/******************************************************************************************************************************************************
		 * CHAINS
		 ******************************************************************************************************************************************************/		
		if ((ps = SAFE_CAST(IGISPointSequence,entity)) != NULL)
		{
			int n = ps->GetNumSides();
			for (int i = 0; i < n; ++i)
			{
				set<int>		attrs;
				if (mRealLines)
				{
					WED_AirportNode * apt_node = dynamic_cast<WED_AirportNode*>(ps->GetNthPoint(i));
					if (apt_node) apt_node->GetAttributes(attrs);
				}
				vector<Point2>	pts;
				
				Segment2	s;
				Bezier2		b;
				if (ps->GetSide(i,s,b))
				{
					b.p1 = GetZoomer()->LLToPixel(b.p1);
					b.p2 = GetZoomer()->LLToPixel(b.p2);
					b.c1 = GetZoomer()->LLToPixel(b.c1);
					b.c2 = GetZoomer()->LLToPixel(b.c2);
					for (int n = 0; n <= BEZ_COUNT; ++n)
						pts.push_back(b.midpoint((float) n / (float) BEZ_COUNT));
				} 
				else
				{
					pts.push_back(GetZoomer()->LLToPixel(s.p1));
					pts.push_back(GetZoomer()->LLToPixel(s.p2));
				}
				
				DrawLineAttrs(g, &*pts.begin(), pts.size(), attrs, struct_color);				
			}
		}
		break;

	case gis_Line_Width:
		/******************************************************************************************************************************************************
		 * NON-RUNWAY LINES
		 ******************************************************************************************************************************************************/		
		if ((lw = SAFE_CAST(IGISLine_Width,entity)) != NULL)
		{
			Point2 corners[4];
			lw->GetCorners(corners);
			GetZoomer()->LLToPixelv(corners, corners, 4);
			
			if ((sea = SAFE_CAST(WED_Sealane, entity)) != NULL)
			{
				glColor4fv(WED_Color_RGBA_Alpha(wed_Surface_Water,mPavementAlpha, storage));
				glShape2v(GL_QUADS, corners, 4);
				glColor4fv(WED_Color_RGBA(struct_color));
			}
									
			glShape2v(GL_LINE_LOOP, corners, 4);
			
			glBegin(GL_LINES);
			glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
			glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
			glEnd();
			
		}
		break;

	case gis_Polygon:
		/******************************************************************************************************************************************************
		 * POLYGONS (TAXIWAYAS, ETC.)
		 ******************************************************************************************************************************************************/		
		if ((poly = SAFE_CAST(IGISPolygon,entity)) != NULL)
		{
			if ((taxi = SAFE_CAST(WED_Taxiway, poly)) != NULL)
			{
				vector<Point2>	pts;
				vector<int>		is_hole_start;
				
				PointSequenceToVector(poly->GetOuterRing(), GetZoomer(), pts, is_hole_start, 0);
				int n = poly->GetNumHoles();
				for (int i = 0; i < n; ++i)
					PointSequenceToVector(poly->GetNthHole(i), GetZoomer(), pts, is_hole_start, 1);
					
				if (!pts.empty())
				{
					glColor4fv(WED_Color_Surface(taxi->GetSurface(), mPavementAlpha, storage));
					glDisable(GL_CULL_FACE);
					glPolygon2(&*pts.begin(), &*is_hole_start.begin(), pts.size());
					glEnable(GL_CULL_FACE);
				}
			}
		
			this->DrawEntityStructure(inCurrent,poly->GetOuterRing(),g,selected);
			int n = poly->GetNumHoles();
			for (int c = 0; c < n; ++c)
				this->DrawEntityStructure(inCurrent,poly->GetNthHole(c),g,selected);			
		}
		break;
	}
	
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

void		WED_StructureLayer::SetPavementTransparency(float alpha)
{
	mPavementAlpha = alpha;
	GetHost()->Refresh();
}

float		WED_StructureLayer::GetPavementTransparency(void) const
{
	return mPavementAlpha;
}
