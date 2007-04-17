#if 0
#include "WED_ObjectLayers.h"
#include "WED_ObjectPlacements.h"
#include "GUI_GraphState.h"
#include "WED_MapZoomer.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <gl.h>
#endif


WED_ObjectLayers::WED_ObjectLayers(WED_ObjectRoot * root) : mRoot(root)
{
}

WED_ObjectLayers::~WED_ObjectLayers()
{
}

int					WED_ObjectLayers::CountLayers(void)
{
	return mRoot->CountLayers();
}

int					WED_ObjectLayers::GetIndent(int n)
{
	return 0;
}

int					WED_ObjectLayers::GetLayerCapabilities(int n)
{
	return wed_Layer_Rename;
}

int					WED_ObjectLayers::GetLayerAllowedTools(int n)
{
	return 0;
}

int					WED_ObjectLayers::GetFlags(int n) const
{
	return wed_Flag_Visible;
}

string				WED_ObjectLayers::GetName(int n) const
{
	return mRoot->GetNthLayer(n)->GetName();
}

float				WED_ObjectLayers::GetOpacity(int n) const
{
	return 1.0;
}

void				WED_ObjectLayers::ToggleFlag(int n, int flag)
{
}

void				WED_ObjectLayers::Rename(int n, const string& name)
{
	mRoot->GetNthLayer(n)->SetName(name);
}

void				WED_ObjectLayers::SetOpacity(int n, float opacity)
{
}

void				WED_ObjectLayers::DrawLayer(
						int						n,
						GUI_GraphState *		state,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						overlay)
{
	state->SetState(false,0,false,  false, false,   false, false);
	WED_ObjectLayer * l = mRoot->GetNthLayer(n);
	
	glPointSize(4);
	glColor3f(1,1,1);
	glBegin(GL_POINTS);
	for (int o = 0; o < l->CountObjects(); ++o)
	{
		WED_CustomObject * oo = l->GetNthObject(o);
		Point2	loc = oo->GetLocation();
		glVertex2f( zoomer->LonToXPixel(loc.x),
					zoomer->LatToYPixel(loc.y));
	}
	glEnd();
	glPointSize(1);
}

int					WED_ObjectLayers::HandleMouseDown(
						int						n,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						x,
						int						y,
						int						button)
{
	return 0;
}

void				WED_ObjectLayers::HandleMouseDrag(
						int						n,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						x,
						int						y,
						int						button)
{
}

void				WED_ObjectLayers::HandleMouseUp(
						int						n,
						WED_MapZoomer *			zoomer,
						int						tool,
						int						selected,
						int						x,
						int						y,
						int						button)
{
}
#endif