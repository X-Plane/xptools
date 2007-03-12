#include "WED_MapBkgnd.h"
#include "WED_MapZoomerNew.h"
#include "GUI_GraphState.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

WED_MapBkgnd::WED_MapBkgnd(WED_MapZoomerNew * zoomer, IResolver * i) : WED_MapLayer(zoomer, i)
{
}

WED_MapBkgnd::~WED_MapBkgnd()
{
}

void		WED_MapBkgnd::DrawVisualization(int inCurrent, GUI_GraphState * g)
{
	double l,r,b,t;

	g->SetState(false,false,false, false,false, false,false);
	
	glColor3f(0.2f,0.2f,0.2f);
	glBegin(GL_QUADS);

	GetZoomer()->GetPixelBounds(l,b,r,t);
	glVertex2d(l,b);
	glVertex2d(l,t);
	glVertex2d(r,t);
	glVertex2d(r,b);

	GetZoomer()->GetMapLogicalBounds(l,b,r,t);
	l = GetZoomer()->LonToXPixel(l);
	r = GetZoomer()->LonToXPixel(r);
	b = GetZoomer()->LatToYPixel(b);
	t = GetZoomer()->LatToYPixel(t);

	glColor3f(0.0f,0.0f,0.0f);
	glVertex2d(l,b);
	glVertex2d(l,t);
	glVertex2d(r,t);
	glVertex2d(r,b);
	glEnd();
	
}
