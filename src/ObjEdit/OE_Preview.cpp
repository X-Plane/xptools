/* 
 * Copyright (c) 2004, Laminar Research.
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
#include "OE_Utils.h"
#include "OE_Preview.h"
#include "OE_DrawObj.h"

#include "OE_Msgs.h"
#include "OE_Notify.h"

#include "OE_ProjectionMgr.h"

#include "XPWidgets.h"
#include "XPWidgetUtils.h"
#include "XPStandardWidgets.h"
#include "XPLMDisplay.h"
#include "XPUIGraphics.h"
#include "XPLMGraphics.h"
#include "XSBPopups.h"

#include "trackball.h"

const	float	gCamDist = 200;

static	const char *	kTypes [] = {	
	"End of object marker",
	"Light",
	"Line",				
	"Triangle",				
	"Quad",				
	"Hard quad",		
	"Black smoke pt",		
	"White smoke pt",		
	"Drive-in movie quad",		
	"Convex polygon",			
	"Quad strip",		
	"Triangle strip",		
	"Triangle fan",			
	"attr_Shade_Flat",
	"attr_Shade_Smooth",
	"attr_Ambient_RGB",
	"attr_Diffuse_RGB",
	"attr_Emission_RGB",
	"attr_Specular_RGB",
	"attr_Shiny_Rat",
	"attr_No_Depth",
	"attr_Depth",
	"attr_LOD",
	"attr_Reset",
	"attr_Cull",
	"attr_NoCull",
	"attr_Offset"
	
};		
		
		

static	char *	kTitles[] = { "Preview", "Selection", "Hidden Surfaces", "Projection Setup", "Projection Preview" };
static	char *	kDisplayStrings[] = { "No Texture;Day;Night", "Wire Frame;Solid;Both", "No Back Faces;Show All;Show Hidden" };

OE_Preview::OE_Preview(
							    	int                  inLeft,    
                                   	int                  inTop,    
                                   	int                  inRight,    
                                   	int                  inBottom,    
                                   	int					 inKind)
	: OE_Pane(inLeft, inTop, inRight, inBottom, 1, kTitles[inKind],
				NULL),//, xpWidgetClass_MainWindow),
	mKind(inKind)
{
	XPSetWidgetProperty(mWidget, xpProperty_MainWindowHasCloseBoxes, 0);

	for (int n = 0; n < 3; ++n)
	mDisplayPopup[n] = new OE_Pane(
		inLeft + 130 + 120 * n, inTop - 5, inLeft + 240 + 120 * n, inTop - 25,
		1, kDisplayStrings[n], this, XSBPopupButtonProc);
	
	switch(inKind) {
	case oe_PreviewType_Preview:
	case oe_PreviewType_ProjectionPreview:
		XPSetWidgetProperty(mDisplayPopup[0]->GetWidget(), xpProperty_PopupCurrentItem, tex_Day);
		XPSetWidgetProperty(mDisplayPopup[1]->GetWidget(), xpProperty_PopupCurrentItem, poly_Solid);
		XPSetWidgetProperty(mDisplayPopup[2]->GetWidget(), xpProperty_PopupCurrentItem, cull_ShowVisible);
		break;
	case oe_PreviewType_Select:
		XPSetWidgetProperty(mDisplayPopup[0]->GetWidget(), xpProperty_PopupCurrentItem, tex_None);
		XPSetWidgetProperty(mDisplayPopup[1]->GetWidget(), xpProperty_PopupCurrentItem, poly_WireAndSolid);
		XPSetWidgetProperty(mDisplayPopup[2]->GetWidget(), xpProperty_PopupCurrentItem, cull_ShowVisible);
		break;
	case oe_PreviewType_Projection:
		XPSetWidgetProperty(mDisplayPopup[0]->GetWidget(), xpProperty_PopupCurrentItem, tex_None);
		XPSetWidgetProperty(mDisplayPopup[1]->GetWidget(), xpProperty_PopupCurrentItem, poly_Wire);
		XPSetWidgetProperty(mDisplayPopup[2]->GetWidget(), xpProperty_PopupCurrentItem, cull_ShowAll);
		break;
	case oe_PreviewType_HiddenSurfaces:
		XPSetWidgetProperty(mDisplayPopup[0]->GetWidget(), xpProperty_PopupCurrentItem, tex_None);
		XPSetWidgetProperty(mDisplayPopup[1]->GetWidget(), xpProperty_PopupCurrentItem, poly_WireAndSolid);
		XPSetWidgetProperty(mDisplayPopup[2]->GetWidget(), xpProperty_PopupCurrentItem, cull_ShowAllWarn);
		break;
	}
	
}				

OE_Preview::~OE_Preview()
{
	delete mDisplayPopup[0];
	delete mDisplayPopup[1];
	delete mDisplayPopup[2];
}

void	OE_Preview::DrawSelf(void)
{
	int	l, r, t, b;
	int	bounds[4];
	float	vp[4];

	XPGetWidgetGeometry(mWidget, &l, &t, &r, &b);
	bounds[0] = l;
	bounds[1] = b;
	bounds[2] = r;
	bounds[3] = t;

	if ((l == r) || (t == b)) 
		return;
	
	XPDrawWindow(l, b, r, t, xpWindow_Screen);	
		
	if (!gObjects.empty())
	{
		gZoomer.SetupMatrices(bounds);

		int	texMode = XPGetWidgetProperty(mDisplayPopup[0]->GetWidget(), xpProperty_PopupCurrentItem, NULL);
		int	solidMode = XPGetWidgetProperty(mDisplayPopup[1]->GetWidget(), xpProperty_PopupCurrentItem, NULL);
		int	cullMode = XPGetWidgetProperty(mDisplayPopup[2]->GetWidget(), xpProperty_PopupCurrentItem, NULL);		

		switch(mKind) {
		case oe_PreviewType_Select:
		case oe_PreviewType_HiddenSurfaces:
		case oe_PreviewType_Preview:
		case oe_PreviewType_Projection:
		case oe_PreviewType_ProjectionPreview:

		OE_DrawObj(mKind == oe_PreviewType_ProjectionPreview ? gProjectionMgr->GetPreviewObj() : gObjects[gLevelOfDetail], 
		
					(mKind == oe_PreviewType_Select) ? gSelection : set<int>(),
					solidMode, cullMode, texMode);
		}
		
		if (mKind == oe_PreviewType_Select && gRebuildStep != -1)
		{
			OE_LabelVertices(gObjects[gLevelOfDetail], gSelection, gRebuildStep);
		}
		
		if (mKind == oe_PreviewType_Projection)
		{
			gProjectionMgr->DrawProjectionSetup();
		}
		
		gZoomer.ResetMatrices();
	}

	static	float	white[4] = { 1.0, 1.0, 1.0, 1.0 };
	
	XPLMDrawString(white,l+10,t-15, kTitles[mKind], NULL, xplmFont_Basic);

		char	buf[60];
	
	if (mKind == oe_PreviewType_Select)
	{
		int maxObjs = gObjects.empty() ? 0 : gObjects[gLevelOfDetail].cmds.size();
		if (gSelection.size() == 1)
			sprintf(buf,"Item %d of %d selected (%s)",1+(*gSelection.begin()), maxObjs, kTypes[gObjects[gLevelOfDetail].cmds[*gSelection.begin()].cmdID]);
		else
			sprintf(buf,"%d items selected of %d.",
			gSelection.size(), maxObjs);
		XPLMDrawString(white, l+10, b + 5, buf, NULL, xplmFont_Basic);
	} 
	
	if (mKind == oe_PreviewType_Preview && !gObjectLOD.empty() && gLevelOfDetail != -1)
	{
		if (gObjectLOD[gLevelOfDetail].first == -1 || gObjectLOD[gLevelOfDetail].second == -1)
			sprintf(buf, "LOD %d of %d (No range specified)",
				gLevelOfDetail + 1, gObjects.size());
		else		
			sprintf(buf, "LOD %d of %d (%d-%d meters)",
				gLevelOfDetail + 1, gObjects.size(), (int) gObjectLOD[gLevelOfDetail].first, (int) gObjectLOD[gLevelOfDetail].second);
		
		XPLMDrawString(white, l+10, b + 5, buf, NULL, xplmFont_Basic);
	}
	
	if (mKind == oe_PreviewType_Preview && !gFileName.empty())
	{
		XPLMDrawString(white, l+10, b + 20, gFileName.c_str(), NULL, xplmFont_Basic);
	}
	
	if (mDragging)
	{
		XPLMSetGraphicsState(0, 0, 0,  0, 1,  0, 0);	
		glColor4f(1.0, 1.0, 1.0, 0.5);
		glBegin(GL_LINE_LOOP);
		glVertex2i(mDragX1, mDragY1);
		glVertex2i(mDragX1, mDragY2);
		glVertex2i(mDragX2, mDragY2);
		glVertex2i(mDragX2, mDragY1);
		glEnd();
	}
	
	return;
}		

int	OE_Preview::HandleClick(XPLMMouseStatus status, int x, int y, int button)
{
	int	l, r, t, b;
	int	bounds[4];
	
	XPGetWidgetGeometry(mWidget, &l, &t, &r, &b);
	bounds[0] = l;
	bounds[1] = b;
	bounds[2] = r;
	bounds[3] = t;
	
	switch (status) {
	case xplm_MouseDown:
		if (mKind == oe_PreviewType_Projection)
		{
			gZoomer.SetupMatrices(bounds);
			if (gProjectionMgr->TrackClick(&gZoomer, bounds, status, x, y, button))
			{
				gZoomer.ResetMatrices();
				return 1;				
			} else {
				gZoomer.ResetMatrices();
			}			
		}
		if (mKind != oe_PreviewType_Select)
		{
			if (button)
				gZoomer.HandleRotationClick(bounds, status, x, y);
			else
				gZoomer.HandleTranslationClick(bounds, status, x, y);
		} else {
			if (button)
				gZoomer.HandleRotationClick(bounds, status, x, y);
			else {			
				mDragging = true;
				mDragX2 = mDragX1 = x;
				mDragY2 = mDragY1 = y;
				mMovedOff = false;

				int	cullMode = XPGetWidgetProperty(mDisplayPopup[2]->GetWidget(), xpProperty_PopupCurrentItem, NULL);		
				
				mVisible.clear();
				if (!gObjects.empty())
				{
					if (((XPLMGetModifiers() & xplm_ShiftFlag) == 0) && (!gSelection.empty()))
					{
						gSelection.clear();
						gRebuildStep = -1;
						OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
					}
					mOriginalSelection = gSelection;
					
					if (cullMode == cull_ShowVisible)
					{
						gZoomer.SetupMatrices(bounds);
						vector<Polygon3Vector>	polys;
						vector<NormalVector>	normals;
						OE_DerivePolygons(gObjects[gLevelOfDetail], polys);
						OE_DeriveNormals(polys,normals);
						OE_DeriveVisible(normals,mVisible);
						gZoomer.ResetMatrices();
					}
										
					int sel = OE_SelectByPoint(bounds,gObjects[gLevelOfDetail], mVisible,mDragX1, mDragY1);
					if (sel != -1)
					{
						if (gSelection.find(sel) != gSelection.end())
							gSelection.erase(sel);
						else
							gSelection.insert(sel);
						gRebuildStep = -1;
						OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
					}					
				}
			}
		}
		return 1;		
	case xplm_MouseDrag:
	case xplm_MouseUp:
		if (mKind == oe_PreviewType_Projection)
		{
			gZoomer.SetupMatrices(bounds);
			if (gProjectionMgr->TrackClick(&gZoomer, bounds, status, x, y, button))
			{
				gZoomer.ResetMatrices();
				return 1;				
			} else {
				gZoomer.ResetMatrices();
			}			
		}
		if (mKind != oe_PreviewType_Select)
		{
			if (button)
				gZoomer.HandleRotationClick(bounds, status, x, y);
			else
				gZoomer.HandleTranslationClick(bounds, status, x, y);
		} else {
			if (button)
				gZoomer.HandleRotationClick(bounds, status, x, y);
			else {		
				if (status == xplm_MouseUp)
					mDragging = false;
				mDragX2 = x;
				mDragY2 = y;
				
				if (mDragX1 != mDragX2 || mDragY1 != mDragY2)
					mMovedOff = true;
				
				if (!gObjects.empty())
				{
					if (mMovedOff)
					{
						gZoomer.SetupMatrices(bounds);
						int	x1 = mDragX1, y1 = mDragY1, x2 = mDragX2, y2 = mDragY2;
						if (x1 > x2) swap(x1,x2);
						if (y1 > y2) swap(y1,y2);
						OE_SelectByPixels(gObjects[gLevelOfDetail], mVisible,
							x1,y1,x2,y2, gSelection);
						gZoomer.ResetMatrices();
						for (set<int>::iterator i = mOriginalSelection.begin();
							i != mOriginalSelection.end(); ++i)
						{
							gSelection.insert(*i);
						}
						gRebuildStep = -1;
						OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);						
					}
				}
			}
		}
		return 1;		
	default:
		return 0;
	}
}                           

int		OE_Preview::HandleMouseWheel(int x, int y, int direction)
{
	int	bounds[4];
	XPGetWidgetGeometry(mWidget, &bounds[0], &bounds[3], &bounds[2], &bounds[1]);
	gZoomer.HandleZoomWheel(bounds, direction, x, y);
	return 1;
}



