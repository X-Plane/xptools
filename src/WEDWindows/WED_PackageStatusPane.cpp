#include "WED_PackageStatusPane.h"
#include "WED_Package.h"
#include "GUI_GraphState.h"
#include "WED_DocumentWindow.h"
#include "GUI_DrawUtils.h"
#include "PlatformUtils.h"
#include "WED_Menus.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include  <gl/gl.h>
#endif

WED_PackageStatusPane::WED_PackageStatusPane(WED_Package * inPackage,GUI_Commander * doc_super) :
	mPackage(inPackage),
	mSuper(doc_super)
{
}

WED_PackageStatusPane::~WED_PackageStatusPane()
{
}
	
void		WED_PackageStatusPane::Draw(GUI_GraphState * state)
{
	int bounds[4];
	this->GetBounds(bounds);
	float fbounds[4] = { bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1] };
	
	state->SetState(false, 0, false,   false, false,  false, false);
	
	glBegin(GL_QUADS);
	
	int x, y;
	float x1, x2, y1, y2;
	
	for (y =  -90; y <  90; ++y)
	for (x = -180; x < 180; ++x)
	{
		int status = mPackage->GetTileStatus(x,y);
		x1 = fbounds[0] + fbounds[2] * ((float) (x + 180) / 360.0);
		x2 = fbounds[0] + fbounds[2] * ((float) (x + 181) / 360.0);

		y1 = fbounds[1] + fbounds[3] * ((float) (y + 90) / 180.0);
		y2 = fbounds[1] + fbounds[3] * ((float) (y + 91) / 180.0);

		switch(status) {
		case status_None:		glColor3f(0.0,0.0,1.0); break;
		case status_XES:		glColor3f(1.0,0.0,0.0); break;
		case status_DSF:		glColor3f(0.5,0.5,0.5);	break;
		case status_Stale:		glColor3f(1.0,1.0,0.0); break;
		case status_UpToDate:	glColor3f(0.0,1.0,0.0); break;
		}
		
		glVertex2f(x1,y1);	
		glVertex2f(x1,y2);	
		glVertex2f(x2,y2);	
		glVertex2f(x2,y1);	
	}
	glEnd();

	glColor3f(0,0,0);
	glBegin(GL_LINES);
	for (y =  -90; y <=  90; ++y)
	{
		y1 = fbounds[1] + fbounds[3] * ((float) (y + 90) / 180.0);
		glVertex2f(fbounds[0], y1);
		glVertex2f(fbounds[0]+fbounds[2], y1);
	}

	for (x =  -180; x <=  180; ++x)
	{
		x1 = fbounds[0] + fbounds[2] * ((float) (x + 180) / 360.0);
		glVertex2f(x1, fbounds[1]);
		glVertex2f(x1, fbounds[1]+fbounds[3]);
	}
	glEnd();
}
	
int			WED_PackageStatusPane::MouseDown(int x, int y, int button)
{
	if (button == 1)
	{
		static int last = -1;
		GUI_MenuItem_t items[4] = { 
		"Item 1", 0, 0, 0,0,
		"Item 2", 0, 0, 0,0,
		"Item 3", 0, 0, 0,0,
		NULL, 0,0,0, 0 };
		last = PopupMenuDynamic(items,x,y,last);
		char buf[100];
		sprintf(buf,"Got %d\n",last);
		DoUserAlert(buf);
		return 1;
	}
	int bounds[4];
	this->GetBounds(bounds);
	float fbounds[4] = { bounds[0], 
						bounds[1], 
						bounds[2] - bounds[0], 
						bounds[3] - bounds[1] };
	int xp = ((float) (x - fbounds[0]) * 360.0 / fbounds[2]) - 180;
	int yp = ((float) (y - fbounds[1]) * 180.0 / fbounds[3]) -  90;
	
	if (xp >= -180 && xp < 180 &&
		yp >= -90 && yp < 90)
	{
		WED_Document * doc;
		int st = mPackage->GetTileStatus(xp, yp);
		if (st == status_None || st == status_DSF)
			doc = mPackage->OpenTile(xp, yp);
		else
			doc = mPackage->NewTile(xp, yp);
	
		WED_DocumentWindow * doc_win = new WED_DocumentWindow(
				"new document",
				mSuper,
				doc);
	}	
	
	return 1;
}

void		WED_PackageStatusPane::MouseDrag(int x, int y, int button)
{
}

void		WED_PackageStatusPane::MouseUp(int x, int y, int button)
{
}
