#include "WED_StructureLayer.h"
#include "IGIS.h"
#include "GUI_GraphState.h"
#include "GISUtils.h"GetSel()
#include "WED_MapZoomerNew.h"
#include "WED_Runway.h"

#define	BEZ_COUNT 100

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

WED_StructureLayer::WED_StructureLayer(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(h, zoomer, resolver)
{
}

WED_StructureLayer::~WED_StructureLayer()
{
}

inline void glVertex2(const Point2& p) { glVertex2d(p.x,p.y); }

void		WED_StructureLayer::DrawEntityStructure		(int inCurrent, IGISEntity * entity, GUI_GraphState * g, int selected)
{
	g->SetState(false,0,false,   false,true,false,false);
	
	glColor4f(selected ? 1 : 0, 1, selected ? 1 : 0, selected ? 1.0 : 0.75);
	
	GISClass_t kind = entity->GetGISClass();

	IGISPoint *						pt;
	IGISPoint_Heading *				pth;
	IGISPoint_WidthLength *			ptwl;
	IGISPointSequence *				ps;
	IGISLine_Width *				lw;
	IGISPolygon *					poly;
	
	WED_Runway *					rwy;
	
	if ((rwy = SAFE_CAST(WED_Runway,entity)) != NULL)
	{
		Point2 corners[4];
		rwy->GetCorners(corners);
		corners[0] = GetZoomer()->LLToPixel(corners[0]);
		corners[1] = GetZoomer()->LLToPixel(corners[1]);
		corners[2] = GetZoomer()->LLToPixel(corners[2]);
		corners[3] = GetZoomer()->LLToPixel(corners[3]);
		
		glBegin(GL_LINE_LOOP);
		glVertex2(corners[0]);
		glVertex2(corners[1]);
		glVertex2(corners[2]);
		glVertex2(corners[3]);
		glEnd();
		
		glBegin(GL_LINES);
		glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
		glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
		glEnd();
	
		glColor4f(1,1,0,1);
		if (rwy->GetCornersBlas1(corners))
		{
			corners[0] = GetZoomer()->LLToPixel(corners[0]);
			corners[1] = GetZoomer()->LLToPixel(corners[1]);
			corners[2] = GetZoomer()->LLToPixel(corners[2]);
			corners[3] = GetZoomer()->LLToPixel(corners[3]);
			
			glBegin(GL_LINE_LOOP);
			glVertex2(corners[0]);
			glVertex2(corners[1]);
			glVertex2(corners[2]);
			glVertex2(corners[3]);
			glEnd();
		}
		if (rwy->GetCornersBlas2(corners))
		{
			corners[0] = GetZoomer()->LLToPixel(corners[0]);
			corners[1] = GetZoomer()->LLToPixel(corners[1]);
			corners[2] = GetZoomer()->LLToPixel(corners[2]);
			corners[3] = GetZoomer()->LLToPixel(corners[3]);
			
			glBegin(GL_LINE_LOOP);
			glVertex2(corners[0]);
			glVertex2(corners[1]);
			glVertex2(corners[2]);
			glVertex2(corners[3]);
			glEnd();
		}

		glColor4f(1,1,1,1);
		if (rwy->GetCornersDisp1(corners))
		{
			corners[0] = GetZoomer()->LLToPixel(corners[0]);
			corners[1] = GetZoomer()->LLToPixel(corners[1]);
			corners[2] = GetZoomer()->LLToPixel(corners[2]);
			corners[3] = GetZoomer()->LLToPixel(corners[3]);
			
			glBegin(GL_LINE_LOOP);
			glVertex2(corners[0]);
			glVertex2(corners[1]);
			glVertex2(corners[2]);
			glVertex2(corners[3]);
			glEnd();
		}
		if (rwy->GetCornersDisp2(corners))
		{
			corners[0] = GetZoomer()->LLToPixel(corners[0]);
			corners[1] = GetZoomer()->LLToPixel(corners[1]);
			corners[2] = GetZoomer()->LLToPixel(corners[2]);
			corners[3] = GetZoomer()->LLToPixel(corners[3]);
			
			glBegin(GL_LINE_LOOP);
			glVertex2(corners[0]);
			glVertex2(corners[1]);
			glVertex2(corners[2]);
			glVertex2(corners[3]);
			glEnd();
		}
	
	}
	else switch(kind) {
	case gis_Point:
	case gis_Point_Bezier:
		if ((pt = SAFE_CAST(IGISPoint,entity)) != NULL)
		{
			Point2 l;
			pt->GetLocation(l);
			l = GetZoomer()->LLToPixel(l);
			glBegin(GL_LINES);
			glVertex2f(l.x, l.y - 3);
			glVertex2f(l.x, l.y + 3);
			glVertex2f(l.x - 3, l.y);
			glVertex2f(l.x + 3, l.y);
			glEnd();
		}
		break;

	case gis_Point_Heading:
		if ((pth = SAFE_CAST(IGISPoint_Heading,entity)) != NULL)
		{
			Point2 l;
			pth->GetLocation(l);
			Vector2	dir;
			NorthHeading2VectorMeters(l,l,pth->GetHeading(),dir);
			Vector2 r(dir.perpendicular_cw());
			l = GetZoomer()->LLToPixel(l);
			
			glBegin(GL_LINES);
			glVertex2(l - dir * 3.0);			glVertex2(l + dir * 6.0);
			glVertex2(l - r   * 3.0);			glVertex2(l + r   * 3.0);
			glEnd();
		}
		break;
	case gis_Point_HeadingWidthLength:
		if ((ptwl = SAFE_CAST(IGISPoint_WidthLength,entity)) != NULL)
		{
			Point2 corners[4];
			ptwl->GetCorners(corners);
			corners[0] = GetZoomer()->LLToPixel(corners[0]);
			corners[1] = GetZoomer()->LLToPixel(corners[1]);
			corners[2] = GetZoomer()->LLToPixel(corners[2]);
			corners[3] = GetZoomer()->LLToPixel(corners[3]);
			
			glBegin(GL_LINE_LOOP);
			glVertex2(corners[0]);
			glVertex2(corners[1]);
			glVertex2(corners[2]);
			glVertex2(corners[3]);
			glEnd();
			
			glBegin(GL_LINES);
			glVertex2(Segment2(corners[0],corners[1]).midpoint(0.5));
			glVertex2(Segment2(corners[2],corners[3]).midpoint(0.5));
			glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
			glVertex2(Segment2(
						Segment2(corners[0],corners[1]).midpoint(1.5),
						Segment2(corners[3],corners[2]).midpoint(1.5)).midpoint());
			glEnd();
			
		}
		break;
	case gis_PointSequence:
	case gis_Line:
	case gis_Ring:
	case gis_Chain:
		if ((ps = SAFE_CAST(IGISPointSequence,entity)) != NULL)
		{
			int n = ps->GetNumSides();
			for (int i = 0; i < n; ++i)
			{
				Segment2	s;
				Bezier2		b;
				if (ps->GetSide(i,s,b))
				{
					b.p1 = GetZoomer()->LLToPixel(b.p1);
					b.p2 = GetZoomer()->LLToPixel(b.p2);
					b.c1 = GetZoomer()->LLToPixel(b.c1);
					b.c2 = GetZoomer()->LLToPixel(b.c2);
					glBegin(GL_LINE_STRIP);
					for (int n = 0; n <= BEZ_COUNT; ++n)
						glVertex2(b.midpoint((float) n / (float) BEZ_COUNT));
					glEnd();
				} 
				else
				{
					glBegin(GL_LINES);				
					s.p1 = GetZoomer()->LLToPixel(s.p1);
					s.p2 = GetZoomer()->LLToPixel(s.p2);
					glVertex2(s.p1);
					glVertex2(s.p2);
					glEnd();
				}
			}
		}
		break;

	case gis_Line_Width:
		if ((lw = SAFE_CAST(IGISLine_Width,entity)) != NULL)
		{
			Point2 corners[4];
			lw->GetCorners(corners);
			corners[0] = GetZoomer()->LLToPixel(corners[0]);
			corners[1] = GetZoomer()->LLToPixel(corners[1]);
			corners[2] = GetZoomer()->LLToPixel(corners[2]);
			corners[3] = GetZoomer()->LLToPixel(corners[3]);
			
			glBegin(GL_LINE_LOOP);
			glVertex2(corners[0]);
			glVertex2(corners[1]);
			glVertex2(corners[2]);
			glVertex2(corners[3]);
			glEnd();
			
			glBegin(GL_LINES);
			glVertex2(Segment2(corners[0],corners[3]).midpoint(0.5));
			glVertex2(Segment2(corners[1],corners[2]).midpoint(0.5));
			glEnd();
			
		}
		break;

	case gis_Polygon:
		if ((poly = SAFE_CAST(IGISPolygon,entity)) != NULL)
		{
			this->DrawEntityStructure(inCurrent,poly->GetOuterRing(),g,selected);
			int n = poly->GetNumHoles();
			for (int c = 0; c < n; ++c)
				this->DrawEntityStructure(inCurrent,poly->GetNthHole(c),g,selected);			
		}
		break;
	}
	
}
