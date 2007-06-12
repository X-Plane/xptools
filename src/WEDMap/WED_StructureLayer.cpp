#include "WED_StructureLayer.h"
#include "IGIS.h"
#include "GUI_GraphState.h"
#include "WED_Colors.h"
#include "GISUtils.h"
#include "WED_UIDefs.h"
#include "WED_EnumSystem.h"
#include "WED_MapZoomerNew.h"
#include "WED_AirportNode.h"
#include "XESConstants.h"
#include "MathUtils.h"
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

#define	AIRPORT_MIN_PIX	20

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

			int pixels_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
								sqrt(Vector2(b.c1,b.c2).squared_length()) +
								sqrt(Vector2(b.c2,b.p2).squared_length());
			int point_count = intlim(pixels_approx / BEZ_PIX_PER_SEG, BEZ_MIN_SEGS, BEZ_MAX_SEGS);
			pts.reserve(pts.capacity() + point_count);
			contours.reserve(contours.capacity() + point_count);
			for (int k = 0; k < point_count; ++k)
			{
				pts.push_back(b.midpoint((float) k / (float) point_count));
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
	else for(set<int>::iterator a = attrs.begin(); a != attrs.end(); ++a)
	switch(*a) {	
	// ------------ STANDARD TAXIWAY LINES ------------
	case line_SolidYellow:
	
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_BrokenYellow:
	
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_DoubleSolidYellow:
	
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);			
		break;
	case line_RunwayHold:
	
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
		break;
	case line_OtherHold:
	
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x3333);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,1.5);		
		glDisable(GL_LINE_STIPPLE);
		glShapeOffset2v(GL_LINE_STRIP, pts, count,-1.5);		
		break;
	case line_ILSHold:
	
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
		break;
	case line_ILSCriticalCenter:
	
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
		break;
	case line_WideBrokenSingle:
	
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
		break;
	case line_WideBrokenDouble:
	
		glColor4f(1,1,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
		glColor4f(0,0,0,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
		break;

	// ------------ ROADWAY TAXIWAY LINES ------------
	case line_SolidWhite:
	
		glColor4f(1,1,1,1);
		glLineWidth(1);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);			
		break;
	case line_Chequered:
	
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
		break;
	case line_BrokenWhite:
	
		glColor4f(1,1,1,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);			
		break;
	
	// ------------ BLACK-BACKED TAXIWAY LINES ------------
	case line_BSolidYellow:
	
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_BBrokenYellow:
	
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x3333);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_BDoubleSolidYellow:
	
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
		break;
	case line_BRunwayHold:
	
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
		break;
	case line_BOtherHold:
	
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
		break;
	case line_BILSHold:
	
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
		break;
	case line_BILSCriticalCenter:
	
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
		break;
	case line_BWideBrokenSingle:
	
		glColor4f(0.3,0.3,0.3,1);
		glLineWidth(3);
		glDisable(GL_LINE_STIPPLE);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,1,0,1);
		glLineWidth(1);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0xF0F0);
		glShape2v(GL_LINE_STRIP, pts, count);			
		break;
	case line_BWideBrokenDouble:
	
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
		break;

	// ------------ LIGHTS ------------
	case line_TaxiCenter:
	
		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_TaxiEdge:
	
		glColor4f(0,0,1,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_HoldLights:
	
		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7070);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_HoldLightsPulse:
	
		glColor4f(1,0.5,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(0.3,0.1,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_HoldShortCenter:
	
		glColor4f(0.3,1,0.3,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
		glColor4f(1,0.5,0,1);
		glLineStipple(1,0x0070);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	case line_BoundaryEdge:
	
		glColor4f(1,0,0,1);
		glLineWidth(3);
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1,0x7000);
		glShape2v(GL_LINE_STRIP, pts, count);		
		break;
	}
	
	glLineWidth(1);
	glDisable(GL_LINE_STIPPLE);
}

bool		WED_StructureLayer::DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
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
	
	GISClass_t 		kind		= entity->GetGISClass();
	const char *	sub_class	= entity->GetGISSubtype();
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
	if (sub_class == WED_Airport::sClass && (airport = SAFE_CAST(WED_Airport, entity)) != NULL)
	{
		Bbox2	bounds;
		airport->GetBounds(bounds);		
		bounds.p1 = GetZoomer()->LLToPixel(bounds.p1);
		bounds.p2 = GetZoomer()->LLToPixel(bounds.p2);		
		if (bounds.xspan() < AIRPORT_MIN_PIX && bounds.yspan() < AIRPORT_MIN_PIX)
		{
			float * f1 = WED_Color_RGBA(struct_color);
			float * f2 = f1 + 4;
			Point2 loc = Segment2(bounds.p1,bounds.p2).midpoint();
			switch(airport->GetAirportType()) {
			case type_Airport:		mAirportIconsX.push_back(loc.x);	mAirportIconsY.push_back(loc.y);	mAirportIconsC.insert(mAirportIconsC.end(),f1,f2);
			case type_Seaport:		mSeaportIconsX.push_back(loc.x);	mSeaportIconsY.push_back(loc.y);	mSeaportIconsC.insert(mSeaportIconsC.end(),f1,f2);
			case type_Heliport:		mHeliportIconsX.push_back(loc.x);	mHeliportIconsY.push_back(loc.y);	mHeliportIconsC.insert(mHeliportIconsC.end(),f1,f2);
			}
			return false;
		}
	}
	else if (sub_class == WED_Runway::sClass && (rwy = SAFE_CAST(WED_Runway,entity)) != NULL)
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

		if (mPavementAlpha > 0.0f)
		{
			// "Solid" geometry.		
			glColor4fv(WED_Color_Surface(rwy->GetSurface(),mPavementAlpha, storage));
										glShape2v(GL_QUADS, corners, 4);
			if (has_blas1)				glShape2v(GL_QUADS, blas1,4);
			if (has_blas2)				glShape2v(GL_QUADS, blas2,4);
			glColor4fv(WED_Color_Surface(rwy->GetShoulder(),mPavementAlpha, storage));
			if (has_shoulders)			glShape2v(GL_QUADS, shoulders, 8);		
		}
		
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
		{
			Point2			l;
			const char *	icon = NULL;
				 if (sub_class == WED_TowerViewpoint::sClass&& (tower  = SAFE_CAST(WED_TowerViewpoint, entity)) != NULL) pt = tower , icon = "map_towerview.png";
			else if (sub_class == WED_Windsock::sClass		&& (sock   = SAFE_CAST(WED_Windsock		 , entity)) != NULL) pt = sock  , icon = "map_windsock.png" ;
			else if (sub_class == WED_AirportBeacon::sClass && (beacon = SAFE_CAST(WED_AirportBeacon , entity)) != NULL) pt = beacon, icon = "map_beacon.png"   ;
			else pt = SAFE_CAST(IGISPoint,entity);
			if (pt)			
			{
				pt->GetLocation(l);
				l = GetZoomer()->LLToPixel(l);
				if (icon) GUI_PlotIcon(g,icon, l.x,l.y,0,icon_scale);
				else {
					glBegin(GL_LINES);
					glVertex2f(l.x, l.y - 3);
					glVertex2f(l.x, l.y + 3);
					glVertex2f(l.x - 3, l.y);
					glVertex2f(l.x + 3, l.y);
					glEnd();
				}
			}
		}
		break;
	case gis_Point_Heading:
		/******************************************************************************************************************************************************
		 * DIRECTIONAL POINT ELEMENTS
		 ******************************************************************************************************************************************************/			
		{
			Point2			l;
			Vector2			dir;
			const char *	icon = NULL;
			
				 if (sub_class == WED_LightFixture::sClass && (lfix = SAFE_CAST(WED_LightFixture,entity)) != NULL)pth = lfix, icon = "map_light.png"	;
			else if (sub_class == WED_AirportSign::sClass  && (sign = SAFE_CAST(WED_AirportSign ,entity)) != NULL)pth = sign, icon = "map_taxisign.png"	;
			else if (sub_class == WED_RampPosition::sClass && (ramp = SAFE_CAST(WED_RampPosition,entity)) != NULL)pth = ramp, icon = "map_rampstart.png";
			else pth = SAFE_CAST(IGISPoint_Heading,entity);
			if (pth)
			{
				pth->GetLocation(l);
				NorthHeading2VectorMeters(l,l,pth->GetHeading(),dir);
				Vector2 r(dir.perpendicular_cw());
				l = GetZoomer()->LLToPixel(l);
				if (icon)	GUI_PlotIcon(g,icon, l.x,l.y,atan2(dir.dx,dir.dy) * RAD_TO_DEG,icon_scale);
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
			if (sub_class == WED_Helipad::sClass && (helipad = SAFE_CAST(WED_Helipad		 , entity)))	ptwl = helipad;
			else									 ptwl    = SAFE_CAST(IGISPoint_WidthLength,entity)					  ;
			
			if (ptwl)
			{		
				Point2 corners[4];
				ptwl->GetCorners(corners);
				GetZoomer()->LLToPixelv(corners, corners, 4);

				if (helipad && mPavementAlpha > 0.0f)
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
					
						
					int pixels_approx = sqrt(Vector2(b.p1,b.c1).squared_length()) +
										sqrt(Vector2(b.c1,b.c2).squared_length()) +
										sqrt(Vector2(b.c2,b.p2).squared_length());
					int point_count = intlim(pixels_approx / BEZ_PIX_PER_SEG, BEZ_MIN_SEGS, BEZ_MAX_SEGS);
					pts.reserve(point_count+1);					
					for (int n = 0; n <= point_count; ++n)
						pts.push_back(b.midpoint((float) n / (float) point_count));
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
		if (sub_class == WED_Sealane::sClass && (sea = SAFE_CAST(WED_Sealane, entity)) != NULL) lw = sea;
		else									 lw  = SAFE_CAST(IGISLine_Width,entity);
		if (lw)
		{
			Point2 corners[4];
			lw->GetCorners(corners);
			GetZoomer()->LLToPixelv(corners, corners, 4);
			
			if (sea && mPavementAlpha > 0.0f)
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
		if (sub_class == WED_Taxiway::sClass && (taxi = SAFE_CAST(WED_Taxiway, entity)) != NULL) poly = taxi;
		else								     poly = SAFE_CAST(IGISPolygon,entity);
		
		if (poly)
		{
			if (taxi && mPavementAlpha > 0.0f)
			{
				// I tried "LODing" out the solid pavement, but the margin between when the pavement can disappear and when the whole
				// airport can is tiny...most pavement is, while visually insignificant, still sprawling, so a bbox-sizes test is poor.
				// Any other test is too expensive, and for the small pavement squares that would get wiped out, the cost of drawing them
				// is negligable anyway.
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

void		WED_StructureLayer::SetPavementTransparency(float alpha)
{
	mPavementAlpha = alpha;
	GetHost()->Refresh();
}

float		WED_StructureLayer::GetPavementTransparency(void) const
{
	return mPavementAlpha;
}

void		WED_StructureLayer::GetCaps(int& draw_ent_v, int& draw_ent_s, int& cares_about_sel)
{
	draw_ent_v = 0;
	draw_ent_s = 1;
	cares_about_sel = 1;
}

#if DEV
doc this
#endif

void		WED_StructureLayer::DrawStructure(int inCurrent, GUI_GraphState * g)
{
	float scale = GetZoomer()->GetPPM() * 30.0;
	if (scale > 1.0) scale = 1.0;
	if (scale < 0.25) scale = 0.25;
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

