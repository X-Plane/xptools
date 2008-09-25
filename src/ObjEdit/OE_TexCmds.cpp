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
#include "OE_TexCmds.h"
#include "OE_Notify.h"
#include "OE_Msgs.h"
#include "OE_Globals.h"
#include "XPLMMenus.h"
#include "OE_Utils.h"
#include "OE_ProjectionMgr.h"
#include "OE_DataModel.h"

enum {
	texCmd_ResetTexture = 0,
	texCmd_ClearTexture,
	texCmd_SpecifyTexture,
	texCmd_ApplyTexture,
	texCmd_Divider1,
	texCmd_RotateCW,
	texCmd_RotateCCW,
	texCmd_Flip,
	texCmd_Divider2,
	texCmd_ApplyPlane,
	texCmd_ApplyCylinder,
	texCmd_ApplySphere,
	texCmd_ApplyProjection,
	texCmd_Divider3,
	texCmd_NewTex,
	texCmd_DelTex
};

static const char *	sTexCmds [] = {
	"&Reset Texture",
	"C&lear Texture",
	"&Specify Texture",
	"&Apply Texture",
	"-",
	"Rotate C&W",
	"Rotate C&CW",
	"&Flip",
	"-",
	"Project Plane",
	"Project Cylinder",
	"Project Sphere",
	"Apply &Projection",
	"-",
	"&New Texture",
	"&Delete Texture",
	0
};

static	const char	sTexKeys [] = {
	'R',				xplm_ControlFlag,
	XPLM_KEY_DELETE,	xplm_ControlFlag,
	XPLM_KEY_RETURN,	0,
	XPLM_KEY_RETURN,	xplm_ControlFlag,
	0,					0,
	']',				xplm_ControlFlag,
	'[',				xplm_ControlFlag,
	'F',				xplm_ControlFlag,
	0,					0,
	'1',				xplm_ControlFlag + xplm_OptionAltFlag,
	'2',				xplm_ControlFlag + xplm_OptionAltFlag,
	'3',				xplm_ControlFlag + xplm_OptionAltFlag,
	'P',				xplm_ControlFlag,
	0,					0,
	'N',				xplm_ControlFlag,
	0,					0
};

static	XPLMMenuID sTexCmdMenu;

static	void	TexCmdHandler(void * inMenuRef, void * inItemRef);
static	void	TexCmdHandleNotification(int inCatagory, int inMsg, void * inParam);
static	void	TexCmdUpdateItems(void);

void	TexCmdUpdateItems(void)
{
	bool	hasSel = !gSelection.empty();
	bool	hasTex = gCurTexture != -1;

	XPLMEnableMenuItem(sTexCmdMenu, texCmd_ResetTexture, hasSel);
	XPLMEnableMenuItem(sTexCmdMenu, texCmd_ClearTexture, hasSel);
	XPLMEnableMenuItem(sTexCmdMenu, texCmd_SpecifyTexture, hasSel);
	XPLMEnableMenuItem(sTexCmdMenu, texCmd_ApplyTexture, hasSel && hasTex);

	XPLMEnableMenuItem(sTexCmdMenu, texCmd_RotateCW, hasSel);
	XPLMEnableMenuItem(sTexCmdMenu, texCmd_RotateCCW, hasSel);
	XPLMEnableMenuItem(sTexCmdMenu, texCmd_Flip, hasSel);

	XPLMEnableMenuItem(sTexCmdMenu, texCmd_ApplyProjection, hasSel);

	XPLMCheckMenuItem(sTexCmdMenu, texCmd_ApplyPlane, (gProjectionMgr->GetProjector() == projector_Plane) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sTexCmdMenu, texCmd_ApplyCylinder, (gProjectionMgr->GetProjector() == projector_Cylinder) ? xplm_Menu_Checked : xplm_Menu_Unchecked);
	XPLMCheckMenuItem(sTexCmdMenu, texCmd_ApplySphere, (gProjectionMgr->GetProjector() == projector_Sphere) ? xplm_Menu_Checked : xplm_Menu_Unchecked);

	XPLMEnableMenuItem(sTexCmdMenu, texCmd_DelTex, hasTex);
}

void	SetupTexCmds(void)
{
	OE_RegisterNotifyFunc(TexCmdHandleNotification);

	sTexCmdMenu = XPLMCreateMenu("&Textures", NULL, 0, TexCmdHandler, NULL);
	for (int n = 0; sTexCmds[n]; ++n)
	{
		XPLMAppendMenuItem(sTexCmdMenu, sTexCmds[n], (void *) n, 1);
		if (sTexKeys[n*2])
			XPLMSetMenuItemKey(sTexCmdMenu,n,sTexKeys[n*2],sTexKeys[n*2+1]);
	}

	TexCmdUpdateItems();
}

void	TexCmdHandleNotification(int inCatagory, int inMsg, void * inParam)
{
	switch(inCatagory) {
	case catagory_Texture:
		switch(inMsg) {
		case msg_TexSelectionChanged:
		case msg_TexAdded:
		case msg_TexDeleted:
			TexCmdUpdateItems();
			break;
		}
		break;
	case catagory_Object:
		switch(inMsg){
		case msg_ObjectSelectionChanged:
			TexCmdUpdateItems();
			break;
		}
		break;
	}
}

void	TexCmdHandler(void * inMenuRef, void * inItemRef)
{
	static	int	newTexNum = 1;
	char	buf[256];

	switch((int) inItemRef) {
	case texCmd_ResetTexture:
		gRebuildStep = -1;
		if (!gObjects.empty())
		{
			OECommand	cmd("Reset Texture");
			for (set<int>::iterator s = gSelection.begin(); s != gSelection.end(); ++s)
			{
				OE_ResetST(gObjects[gLevelOfDetail].cmds[*s], 0.0, 1.0, 0.0, 1.0);
			}
			cmd.Commit();
		}
		break;
	case texCmd_ClearTexture:
		gRebuildStep = -1;
		if (!gObjects.empty())
		{
			OECommand	cmd("Clear Texture");
			for (set<int>::iterator s = gSelection.begin(); s != gSelection.end(); ++s)
			{
				OE_ClearST(gObjects[gLevelOfDetail].cmds[*s]);
			}
			cmd.Commit();
		}
		break;
	case texCmd_SpecifyTexture:
		OE_Notifiable::Notify(catagory_Texture,	msg_DoRebuild, NULL);
		break;
	case texCmd_ApplyTexture:
		gRebuildStep = -1;
		if (!gObjects.empty())
		{
			OECommand	cmd("Apply Texture");
			float	s1 = 0.0, s2 = 1.0, t1 = 0.0, t2 = 1.0;
			if (gCurTexture != -1)
			{
				s1 = gTextures[gCurTexture].s1;
				t1 = gTextures[gCurTexture].t1;
				s2 = gTextures[gCurTexture].s2;
				t2 = gTextures[gCurTexture].t2;
			}
			for (set<int>::iterator s = gSelection.begin(); s != gSelection.end(); ++s)
			{
				OE_ResetST(gObjects[gLevelOfDetail].cmds[*s], s1, s2, t1, t2);
			}
			cmd.Commit();
		}
		break;
	case texCmd_RotateCW:
	case texCmd_RotateCCW:
	case texCmd_Flip:
		if (!gObjects.empty())
		{
			OECommand	cmd("Rotate/Flip Texture");
			for (set<int>::iterator s = gSelection.begin(); s != gSelection.end(); ++s)
			{
				if ((int) inItemRef == texCmd_Flip)
					OE_FlipST(gObjects[gLevelOfDetail].cmds[*s]);
				else
					OE_RotateST(gObjects[gLevelOfDetail].cmds[*s], (int) inItemRef == texCmd_RotateCCW);
			}
			cmd.Commit();
		}
		break;
	case texCmd_ApplyPlane:
		gProjectionMgr->SetProjector(projector_Plane);
		TexCmdUpdateItems();
		break;
	case texCmd_ApplyCylinder:
		gProjectionMgr->SetProjector(projector_Cylinder);
		TexCmdUpdateItems();
		break;
	case texCmd_ApplySphere:
		gProjectionMgr->SetProjector(projector_Sphere);
		TexCmdUpdateItems();
		break;
	case texCmd_ApplyProjection:
		{
			OECommand	cmd("Apply Projection");
			gProjectionMgr->ApplyTexture();
			cmd.Commit();
		}
		break;
	case texCmd_NewTex:
		{
			sprintf(buf, "New texture %d", newTexNum++);
			OE_Texture_t	newt;
			newt.s1 = newt.t1 = 1.0;
			newt.s2 = newt.t2 = 0.0;
			if (gSelection.empty())
			{
				newt.s1 = newt.t1 = 0.0;
				newt.s2 = newt.t2 = 1.0;
			}
			if (!gObjects.empty())
			for (set<int>::iterator i = gSelection.begin(); i != gSelection.end(); ++i)
			{
				for (vector<vec_tex>::iterator st = gObjects[gLevelOfDetail].cmds[*i].st.begin();
					st != gObjects[gLevelOfDetail].cmds[*i].st.end(); ++st)
				{
					newt.s1 = min(newt.s1, st->st[0]);
					newt.s2 = max(newt.s2, st->st[0]);
					newt.t1 = min(newt.t1, st->st[1]);
					newt.t2 = max(newt.t2, st->st[1]);
				}
			}

			newt.name = buf;
			if (gCurTexture == -1)
			{
				gTextures.push_back(newt);
				gCurTexture = gTextures.size() - 1;
				OE_Notifiable::Notify(catagory_Texture, msg_TexAdded, NULL);
				OE_Notifiable::Notify(catagory_Texture, msg_TexSelectionChanged, NULL);
			} else {
				TextureTable::iterator i = gTextures.begin();
				advance(i, gCurTexture);
				gTextures.insert(i, newt);
				OE_Notifiable::Notify(catagory_Texture, msg_TexAdded, NULL);
			}
		}
		break;
	case texCmd_DelTex:
		if (gCurTexture != -1)
		{
			TextureTable::iterator i = gTextures.begin();
			advance(i, gCurTexture);
			gTextures.erase(i);
			if (gCurTexture >= gTextures.size())
			{
				gCurTexture = gTextures.size() - 1;
				OE_Notifiable::Notify(catagory_Texture, msg_TexDeleted, NULL);
				OE_Notifiable::Notify(catagory_Texture, msg_TexSelectionChanged, NULL);
			} else
				OE_Notifiable::Notify(catagory_Texture, msg_TexDeleted, NULL);
		}
		break;
	}
}
