#include "WED_MapBkgnd.h"
#include "WED_MapZoomerNew.h"
#include "GUI_GraphState.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl/gl.h>
#endif

WED_MapBkgnd::WED_MapBkgnd(GUI_Pane * h, WED_MapZoomerNew * zoomer, IResolver * i) : WED_MapLayer(h, zoomer, i)
{
}

WED_MapBkgnd::~WED_MapBkgnd()
{
}

void		WED_MapBkgnd::DrawVisualization(int inCurrent, GUI_GraphState * g)
{
	double pl,pr,pb,pt;
	double ll,lr,lb,lt;

	g->SetState(false,false,false, false,false, false,false);
	
	glColor3f(0.2f,0.2f,0.2f);
	glBegin(GL_QUADS);

	GetZoomer()->GetPixelBounds(pl,pb,pr,pt);
	glVertex2d(pl,pb);
	glVertex2d(pl,pt);
	glVertex2d(pr,pt);
	glVertex2d(pr,pb);

	GetZoomer()->GetMapLogicalBounds(ll,lb,lr,lt);
	ll = GetZoomer()->LonToXPixel(ll);
	lr = GetZoomer()->LonToXPixel(lr);
	lb = GetZoomer()->LatToYPixel(lb);
	lt = GetZoomer()->LatToYPixel(lt);

	glColor3f(0.0f,0.0f,0.0f);
	glVertex2d(ll,lb);
	glVertex2d(ll,lt);
	glVertex2d(lr,lt);
	glVertex2d(lr,lb);
	glEnd();
	
	glColor4f(1.0, 1.0, 1.0, 0.1);
	g->SetState(false,false,false, false,true, false,false);
	
	pl = min(lr,max(ll,pl));
	pr = min(lr,max(ll,pr));
	pt = min(lt,max(lb,pt));
	pb = min(lt,max(lb,pb));
	
	glBegin(GL_LINES);
	for(double t = 0.0; t <= 1.0; t += 0.25)
	{
		glVertex2d(pl,lb+(lt-lb)*t);
		glVertex2d(pr,lb+(lt-lb)*t);
		glVertex2d(ll+(lr-ll)*t,pb);
		glVertex2d(ll+(lr-ll)*t,pt);
	}
	glEnd();
	
}
