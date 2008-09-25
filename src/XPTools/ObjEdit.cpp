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
/*

NOTES:

1. MAIN SCREENS:

ÃSEL		selection
ÃPRV		preview
ÃHID		hidden surfaces
2SED		surface editing
3TAP		texture apply
ÃTXL		texture list
ÃTWE		texture WYSIWYG editing
ÃTNE		texture numeric editing

Config 1: hidden surface removal
SEL	PRV
SED	HID
Config 2: texture definition
TWE	PRV
TNE TXL
Config 3: texture application
SEL PRV
TAP	TXL

TODO:
1. generalize preview into one of:
	previewer
	hidden face viewer
	selection viewer
	(texture placer eventually)
and deploy in main program.
2. add menus
3. add some kind of support for the main window resizing, etc.
4. add scaling, night lighting, etc. to the previewer
5. add save and load commands and a dirty flag
6. then go on to the management functions.
7. add controls for: toggling day and night and which LOD we see
8. add the notion of front and back faces...we need to build this list on load.
	(need a fast search algorithm.)

NEED TO ADD:
 - Wire frame ability to drawing polygons.
 - Thicker wire frame for selection
 - Option to draw wire frame as well as textures
 - e.g. draw solid is an independent bool from draw frame (but neither would be BAD)
   (use poly offset when using both to show the wire frame!)
 (then consider how each frame should look...
 	- selection should probably be...um...wire frame + poly's
 	- selection - textures should be optional, as should solidness (prefs)
	- hidden surface could optionally have textures (prefs)

	Mouse wheel zooming
	Issue: selection arrows get stuck on attributes!


*/

#include "XWidgetApp.h"
#include "XPWidgetWin.h"
#include "XPWidgets.h"
#include "XPLMMenus.h"
#include "XPStandardWidgets.h"
#include "OE_Globals.h"
#include "OE_TexMgr.h"
#include "OE_Utils.h"
#include "trackball.h"
#include "XUtils.h"
#include "XObjReadWrite.h"

#include "OE_Notify.h"
#include "OE_Msgs.h"
#include "OE_TexCmds.h"

#include "OE_PatchList.h"
#include "OE_Preview.h"
#include "OE_TexEdWindow.h"
#include "OE_FileCmds.h"
#include "OE_SelCmds.h"
#include "OE_DataModel.h"

#include "OE_ProjectionMgr.h"

ObjectTable		gObjects;
LODTable		gObjectLOD;
TextureTable	gTextures;

set<int>		gSelection;
int				gCurTexture = -1;
int				gLevelOfDetail;	// Cur LOD

int				gRebuildStep = -1;


OE_Zoomer3d		gZoomer;

string			gFileName,gFilePath;

OE_ProjectionMgr *	gProjectionMgr = NULL;

#define 	kMargin	10

enum {
	view_HiddenSurfaces = 0,
	view_Patches,
	view_Projection,
	view_Count
};

const char * kViewCmds[view_Count] = {
	"&Hidden Surfaces",
	"&Patches",
	"Pr&ojection"
};

static	OE_Pane *	gHiddenSurfaces;
static	OE_Pane *	gPatches;
static	OE_Pane *	gPatchEd;
static	OE_Pane *	gTexEd;
static	OE_Pane *	gProjEd;
static	OE_Pane *	gProjSetup;
static	OE_Pane *	gProjPrev;
static	OE_Pane *	gPreview;

static	XPLMMenuID	gViewMenu;

static	void	UpdateViewCmds(void)
{
	XPLMCheckMenuItem(gViewMenu, view_HiddenSurfaces, gHiddenSurfaces->IsVisible() ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(gViewMenu, view_Patches, gPatches->IsVisible() ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(gViewMenu, view_Projection, gProjPrev->IsVisible() ? xplm_Menu_Checked : xplm_Menu_Unchecked);
}


void	ViewCmdHandler(void * inMenuRef, void * inItemRef)
{
	switch((int) inItemRef) {
	case view_HiddenSurfaces:
	case view_Patches:
	case view_Projection:
		gHiddenSurfaces->Show((int) inItemRef == view_HiddenSurfaces);
		gPatches->Show((int) inItemRef == view_Patches);
		gTexEd->Show((int) inItemRef == view_HiddenSurfaces);
		gPatchEd->Show((int) inItemRef == view_Patches);
		gProjEd->Show((int) inItemRef == view_Projection);
		gProjSetup->Show((int) inItemRef == view_Projection);
		gProjPrev->Show((int) inItemRef == view_Projection);
		gPreview->Show((int) inItemRef != view_Projection);
		UpdateViewCmds();
		break;
	}
}

void	XGrindFiles(const vector<string>& fileList, int x, int y)
{
	for (vector<string>::const_iterator i = fileList.begin(); i != fileList.end(); ++i)
	{
		if (HasExtNoCase(*i, ".obj")) {
			XObj	obj;
			if (XObjRead(i->c_str(), obj))
			{
				gFilePath = gFileName = *i;
				StripPath(gFileName);

				gObjects.clear();
				gObjectLOD.clear();
				OE_SplitObj(obj, gObjects, gObjectLOD);

				string foo(*i);
				StripPath(foo);
				double	rad = GetObjRadius(obj);
				if (rad > 0.0)
				{
					gZoomer.SetScale(50.0 / rad);
				}
				XGrinder_SetTitle(foo.c_str());

				gSelection.clear();
				gRebuildStep = -1;
				OE_Notifiable::Notify(catagory_Object, msg_ObjectSelectionChanged, NULL);
				OE_Notifiable::Notify(catagory_Object, msg_ObjectLoaded, NULL);
			}
		} else if (HasExtNoCase(*i, ".bmp") || HasExtNoCase(*i, ".png"))
		{
			AccumTexture(*i);
		}
	}
}

void XGrindDone(void)
{
}

void	XGrindInit(string& outName)
{
	gFileName.clear();
	gFilePath.clear();

	outName = "ObjEdit";

	int width, height;
	gWidgetWin->GetBounds(&width, &height);

	height -= 20;

	// Selection
	new OE_Preview(kMargin, height - kMargin, width / 2 - kMargin, height / 2 + kMargin, oe_PreviewType_Select);

	// Preview
	gPreview =  new OE_Preview(width / 2 + kMargin, height - kMargin, width - kMargin, height / 2 + kMargin, oe_PreviewType_Preview);
	// Preview with textures applied
	gProjPrev = new OE_Preview(width / 2 + kMargin, height - kMargin, width - kMargin, height / 2 + kMargin, oe_PreviewType_ProjectionPreview);

	// Hidden surfaces
	gHiddenSurfaces = 	new OE_Preview	(width / 2 + kMargin, height / 2 - kMargin, width - kMargin, kMargin + 20, oe_PreviewType_HiddenSurfaces);
	// Texture application
	gProjSetup = 		new OE_Preview	(width / 2 + kMargin, height / 2 - kMargin, width - kMargin, kMargin + 20, oe_PreviewType_Projection);
	// Patches
	gPatches = 			new OE_PatchList(width / 2 + kMargin, height / 2 - kMargin, width - kMargin, kMargin + 20);

	// Tex Editor
	gTexEd = new	OE_TexEdWindow(kMargin, height / 2 - kMargin, width / 2 - kMargin, kMargin + 20, oe_DirectEd);
	// Patch Editor
	gPatchEd = new	OE_TexEdWindow(kMargin, height / 2 - kMargin, width / 2 - kMargin, kMargin + 20, oe_PatchEd);
	// Projection Editor
	gProjEd = new	OE_TexEdWindow(kMargin, height / 2 - kMargin, width / 2 - kMargin, kMargin + 20, oe_ProjEd);

	gProjectionMgr = new OE_ProjectionMgr;

	gPatches->Show(false);
	gPatchEd->Show(false);
	gProjSetup->Show(false);
	gProjPrev->Show(false);
	gProjEd->Show(false);

	SetupFileCmds();

	OE_SetupUndoCmds();

	SetupSelCmds();

	gViewMenu = XPLMCreateMenu("&View", NULL, 0, ViewCmdHandler, NULL);
	for (int n = 0; n < view_Count; ++n)
	{
		XPLMAppendMenuItem(gViewMenu, kViewCmds[n], (void *) n, 1);
		XPLMSetMenuItemKey(gViewMenu, n, '1' + n, xplm_ControlFlag);
	}

	SetupTexCmds();

	UpdateViewCmds();

}

bool	XGrindCanQuit(void)
{
	return true;
}

void	XGrindDragStart(int x, int y)
{
}

void	XGrindDragOver(int x, int y)
{
}

void	XGrindDragLeave(void)
{
}

