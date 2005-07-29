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
#include "OE_TexEdWindow.h"
#include "XPWidgets.h"
#include "OE_Globals.h"
#include "OE_TexMgr.h"
#include "XPLMGraphics.h"
#include "OE_Msgs.h"
#include "XPUIGraphics.h"
#include "OE_Scroller.h"
#include "OE_Utils.h"
#include "OE_DataModel.h"
#include "OE_ProjectionMgr.h"

const float	kMinScale = 1.0 / 4.0;
const float	kMaxScale = 16.0;

const	float	kMargin = 20;

OE_TexEdWindow::OE_TexEdWindow(
									int			inLeft,
									int			inTop,
									int			inRight,
									int			inBottom,
									int			inKind) :
	OE_Pane(inLeft,inTop, inRight,inBottom, 1, "TexWindow", NULL)
{
	mScroller = new OE_Scroller(inLeft,inTop,inRight, inBottom,
					1,	this,	true,true);

	switch(inKind) {
	case oe_DirectEd:
		mPane = new OE_DirectEdPane(inLeft, inTop,inRight,inBottom, mScroller);
		break;
	case oe_PatchEd:
	case oe_ProjEd:
		mPane = new OE_PatchEdPane(inLeft, inTop,inRight,inBottom, mScroller, inKind == oe_ProjEd);
		break;
	}
	mScroller->SetContents(mPane);
}

	
OE_TexEdWindow::~OE_TexEdWindow()
{
	delete mPane;
	delete mScroller;
}

OE_TexEdPane::OE_TexEdPane(
									int			inLeft,
									int			inTop,
									int			inRight,
									int			inBottom,
									OE_Pane *	inParent)
	: OE_Pane(inLeft, inTop, inRight, inBottom, 1, "TexPreview",
			inParent),//, xpWidgetClass_MainWindow)
	mScale(1.0)
	
{
	XPSetWidgetProperty(mWidget,xpProperty_Clip,1);
}				

OE_TexEdPane::~OE_TexEdPane()
{
}
	
void		OE_TexEdPane::DrawSelf(void)
{
	int	l, r, t, b;
	float	vp[4];

	XPGetWidgetGeometry(mWidget, &l,&t,&r,&b);



	if ((l == r) || (t == b)) 
		return;
	
//	XPDrawWindow(l, b, r, t, xpWindow_Screen);	

	if (gObjects.empty())	return;
	
	string	texName= gObjects[gLevelOfDetail].texture;
	
	GLenum	texNum = FindTexture(texName, false, NULL, NULL);
	if (texNum)
	{
		XPLMBindTexture2d(texNum, 0);
		XPLMSetGraphicsState(0, 1, 0, 1, 1, 0, 0);
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0);		glVertex2i(l+kMargin, t-kMargin);
		glTexCoord2f(1.0, 1.0);		glVertex2i(r-kMargin, t-kMargin);
		glTexCoord2f(1.0, 0.0);		glVertex2i(r-kMargin, b+kMargin);
		glTexCoord2f(0.0, 0.0);		glVertex2i(l+kMargin, b+kMargin);
		glEnd();
	}
}

int			OE_TexEdPane::HandleClick(XPLMMouseStatus status, int x, int y, int button)
{
	if (button == 0)
		return 0;
	static	int	mouseX, mouseY;
	
	int l,t,r,b;
	if (status == xplm_MouseDown)
	{
		mouseX = x;
		mouseY = y;
	} else {
		int hDelta = x - mouseX;
		int vDelta = y - mouseY;
		mouseX = x;
		mouseY = y;
		if (hDelta || vDelta)
		{
			XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
			XPSetWidgetGeometry(mWidget,l+hDelta,t+vDelta,r+hDelta,b+vDelta);
		}		
	}
	return 1;
}

int		OE_TexEdPane::HandleMouseWheel(int x, int y, int direction)
{
	int	oldScale = mScale;
	while (direction > 0)
	{
		if (mScale < kMaxScale)
		{
			if (mScale < 1.0)
				mScale *= 2.0;
			else
				mScale += 1.0;
		}
		direction--;
	}
	while (direction < 0)
	{
		if (mScale > kMinScale)
		{
			if (mScale > 1.0)
				mScale -= 1.0;
			else
				mScale *= 0.5;
		}
		direction++;
	}

	if (mScale != oldScale)
	{
		float mouseS = XCoordToS(x);
		float mouseT = YCoordToT(y);
		RecalcSize();
		float newX = SCoordToX(mouseS);
		float newY = TCoordToY(mouseT);
		
		int	deltaX = newX - x;
		int deltaY = newY - y;
		
		int	l,b,r,t;
		XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
		XPSetWidgetGeometry(mWidget,
			l-deltaX, 
			t-deltaY,
			r-deltaX,
			b-deltaY);
		RecalcSize();
	}	
	return 1;
}

float		OE_TexEdPane::GetHandleRadius(bool	inSquared)
{
	// Note: we pad 1 pixel of radius before squaring - this is used for figuring
	// out if a mouse click is on a handle...if we go by the pixels of the handle, it's
	// way too picky.  Best to pad it a bit.
	if (mScale > 2.0)	return inSquared ? 25 : 3.0;
	if (mScale > 1.0)	return inSquared ? 16 : 2.0;
						return inSquared ? 12.0 : 1.5;
}

float		OE_TexEdPane::XCoordToS(float inX)
{
	int	l,b,t,r;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	return (inX - (float) l - kMargin) / (float) (r - l - kMargin - kMargin);
}

float		OE_TexEdPane::YCoordToT(float inY)
{
	int	l,b,t,r;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	return (inY - (float) b - kMargin) / (float) (t - b - kMargin - kMargin);
}

float		OE_TexEdPane::SCoordToX(float inX)
{
	int	l,b,t,r;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	return l + kMargin + inX * (float) (r - l - kMargin - kMargin);	
}

float		OE_TexEdPane::TCoordToY(float inY)
{
	int	l,b,t,r;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	return b + kMargin + inY * (float) (t - b - kMargin - kMargin);	
}


void OE_TexEdPane::HandleNotification(int catagory, int message, void * param)
{
	switch(catagory) {
	case catagory_Texture:
		switch(message) {
		case msg_TexLoaded:
			RecalcSize();
			break;
		}
		break;
	case catagory_Object:
		switch(message) {
		case msg_ObjectLoaded:
			RecalcSize();
			break;
		}
		break;
	}
}

void	OE_TexEdPane::RecalcSize(void)
{
	if (gObjects.empty())	return;
	
	string	texName= gObjects[gLevelOfDetail].texture;
	int	width,height;
	GLenum	texNum = FindTexture(texName, false, &width, &height);
	width *= mScale;
	height *= mScale;
	width += (kMargin*2);
	height += (kMargin*2);
	if (texNum)
	{
		int	l, t, r, b;
		XPGetWidgetGeometry(mWidget, &l, &t,&r,&b);
		XPSetWidgetGeometry(mWidget, l, t, l + width, t - height);
	}
}

#pragma mark -

OE_DirectEdPane::OE_DirectEdPane(
									int			inLeft,
									int			inTop,
									int			inRight,
									int			inBottom,
									OE_Pane *	inParent) :
	OE_TexEdPane(inLeft, inTop, inRight, inBottom, inParent),
	mDragCmd(-1)
{
}	

OE_DirectEdPane::~OE_DirectEdPane()
{
}

void		OE_DirectEdPane::DrawSelf(void)
{
	OE_TexEdPane::DrawSelf();
	if (gObjects.empty())
		return;
	
	XPLMSetGraphicsState(0,0,0,  0,0, 0,0);
	glColor3f(1.0, 1.0, 0.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	int	l,t,r,b;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	
	vector<float>	ptsx, ptsy;
	
	int n = 0;
	for (vector<XObjCmd>::iterator cmd = gObjects[gLevelOfDetail].cmds.begin();
		cmd != gObjects[gLevelOfDetail].cmds.end(); ++cmd, ++n)
	{
		if (gSelection.find(n) != gSelection.end())
		{
			if (cmd->cmdType == type_Poly)
			{
				switch(cmd->cmdID) {
				case	obj_Tri:
					glBegin(GL_TRIANGLES);
					break;
				case	obj_Quad:
					glBegin(GL_QUADS);
					break;
				case	obj_Quad_Hard:
					glBegin(GL_QUADS);
					break;
				case	obj_Smoke_Black:
					glBegin(GL_QUADS);
					break;
				case	obj_Smoke_White:
					glBegin(GL_QUADS);
					break;
				case	obj_Movie:
					glBegin(GL_QUADS);
					break;
				case	obj_Polygon:
					glBegin(GL_POLYGON);
					break;
				case	obj_Quad_Strip:
					glBegin(GL_QUAD_STRIP);
					break;
				case	obj_Tri_Strip:
					glBegin(GL_TRIANGLE_STRIP);
					break;
				case	obj_Tri_Fan:
					glBegin(GL_TRIANGLE_FAN);
					break;
				}
				
				for (vector<vec_tex>::const_iterator piter = cmd->st.begin();
					piter != cmd->st.end(); ++piter)
				{
					ptsx.push_back(SCoordToX(piter->st[0]));
					ptsy.push_back(TCoordToY(piter->st[1]));
					glVertex2f(SCoordToX(piter->st[0]), TCoordToY(piter->st[1]));
				}

				glEnd();
			}
		}
	}
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glPointSize(GetHandleRadius(false) * 2);
	glColor3f(1.0, 0.8, 0.8);
	glBegin(GL_POINTS);
	for (int n = 0; n <ptsx.size(); ++n)
	{
		glVertex2f(ptsx[n],ptsy[n]);		
	}
	glEnd();
	glPointSize(1);
}

void OE_DirectEdPane::HandleNotification(int catagory, int message, void * param)
{
	switch(catagory) {
	case catagory_Texture:
		switch(message) {
		case msg_DoRebuild:
			gRebuildStep = 0;
			return;
		}
		break;
	}
	OE_TexEdPane::HandleNotification(catagory,message, param);
}


int			OE_DirectEdPane::HandleClick(XPLMMouseStatus status, int x, int y, int button)
{
	if (OE_TexEdPane::HandleClick(status, x,y,button))	return 1;
	int l,t,r,b;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	if (l == r || t == b) return 0;
	if (gObjects.empty()) return 0;
	
	if (status != xplm_MouseDown && (XPLMGetModifiers() & xplm_ShiftFlag))
	{
		OE_ConstrainDrag(mMouseX, mMouseY, x, y);
	}
	
	if (status == xplm_MouseDown)
	{
		if (gRebuildStep >= 0 && !gObjects.empty())
		{
			int	counter = gRebuildStep;
			mDragCmd = -1;
			// Go through the selection and calc our drag cmd and vertex.
			for (set<int>::iterator s = gSelection.begin(); s != gSelection.end(); ++s)			
			{
				for (int v = 0; v < gObjects[gLevelOfDetail].cmds[*s].st.size(); ++v)
				{
					if (counter == 0)
					{
						mDragCmd = *s;
						mDragVertex = v;
						break;
					} else
						--counter;
				}
				if (mDragCmd != -1) 
					break;
			}
			if (mDragCmd != -1)
			{
				OE_BeginCommand("Specify Texture Coordinate");
				++gRebuildStep;
				gObjects[gLevelOfDetail].cmds[mDragCmd].st[mDragVertex].st[0] = XCoordToS(x);
				gObjects[gLevelOfDetail].cmds[mDragCmd].st[mDragVertex].st[1] = YCoordToT(y);
				mMouseX = x;
				mMouseY = y;
				return 1;
			} else
				gRebuildStep = -1;
		}
		
		int n = 0;
		mOriginal = gObjects[gLevelOfDetail];
		mDragCmd = -1;
		mMouseX = x;
		mMouseY = y;
		for (vector<XObjCmd>::iterator cmd = gObjects[gLevelOfDetail].cmds.begin();
			cmd != gObjects[gLevelOfDetail].cmds.end(); ++cmd, ++n)
		{
			if (gSelection.find(n) != gSelection.end())
			{
				if (cmd->cmdType == type_Poly)
				{
					int v = 0;
					for (vector<vec_tex>::const_iterator piter = cmd->st.begin();
						piter != cmd->st.end(); ++piter, ++v)
					{
						float	px = SCoordToX(piter->st[0]);
						float	py = TCoordToY(piter->st[1]);

						if ((((px - x) * (px - x) + (py - y) * (py - y))) < GetHandleRadius(true))
						{
							mDragCmd = n;
							mDragVertex = v;
							OE_BeginCommand("Edit Texture Coordinate");
							return 1;
						}
					}
				}
			}
		}
		OE_BeginCommand("Edit Texture Coordinates");
		return 1;
	
	} else {
		if (gRebuildStep > 0)
		{
			gObjects[gLevelOfDetail].cmds[mDragCmd].st[mDragVertex].st[0] = XCoordToS(x);
			gObjects[gLevelOfDetail].cmds[mDragCmd].st[mDragVertex].st[1] = YCoordToT(y);
			if (status == xplm_MouseUp)
			{
				mDragCmd = -1;
				
				int	count = 0;
				for (set<int>::iterator s = gSelection.begin(); s != gSelection.end(); ++s)
				{
					count += gObjects[gLevelOfDetail].cmds[*s].st.size();
				}
				if (gRebuildStep >= count)
					gRebuildStep = -1;

				OE_CompleteCommand();
			}
			return 0;
		}
			
		float	deltaH = XCoordToS(x) - XCoordToS(mMouseX);
		float	deltaV = YCoordToT(y) - YCoordToT(mMouseY);

		if (mDragCmd == -1)
		{
			for (set<int>::iterator s = gSelection.begin(); s != gSelection.end(); ++s)
			{
				for (int i = 0; i < mOriginal.cmds[*s].st.size(); ++i)
				{
					gObjects[gLevelOfDetail].cmds[*s].st[i].st[0] = mOriginal.cmds[*s].st[i].st[0] + deltaH;
					gObjects[gLevelOfDetail].cmds[*s].st[i].st[1] = mOriginal.cmds[*s].st[i].st[1] + deltaV;
				}
			}
		} else  {
			gObjects[gLevelOfDetail].cmds[mDragCmd].st[mDragVertex].st[0] = mOriginal.cmds[mDragCmd].st[mDragVertex].st[0] + deltaH;
			gObjects[gLevelOfDetail].cmds[mDragCmd].st[mDragVertex].st[1] = mOriginal.cmds[mDragCmd].st[mDragVertex].st[1] + deltaV;
		}
				
		if (status == xplm_MouseUp)
		{
			mDragCmd = -1;
			OE_CompleteCommand();
		}
		return 1;
	}
}

#pragma mark -

const int kHandleWriteFlags[9][4] = {
	//	s1	s2	t1	t2
	{ 	1,	0,	0,	1	},		// Top left
	{	0,	0,	0,	1	},		// Top
	{	0,	1,	0,	1	},		// Top right
	{	0,	1,	0,	0	},		// Right
	{	0,	1,	1,	0	},		// Bot right
	{	0,	0,	1,	0	},		// Bottom
	{	1,	0,	1,	0	},		// Bot left
	{	1,	0,	0,	0	},		// Left
	{	1,	1,	1,	1	} };	// All

const float	kHandlePosFractions[8][4] = {
	// s1   s2   t1   t2
	{  1.0, 0.0, 0.0, 1.0 },	// Top Left
	{  0.5, 0.5, 0.0, 1.0 },	// Top
	{  0.0, 1.0, 0.0, 1.0 },	// Top Right
	{  0.0, 1.0, 0.5, 0.5 },	// Right
	{  0.0, 1.0, 1.0, 0.0 },	// Bot Right
	{  0.5, 0.5, 1.0, 0.0 },	// Bot
	{  1.0, 0.0, 1.0, 0.0 },	// Bot Left
	{  1.0, 0.0, 0.5, 0.5 } };	// Left

static	void	CalcHandlePositions(
			float s1, float s2, float t1, float t2, 
			float handles[8][2])
{
	for (int n = 0; n < 8; ++n)
	{
		handles[n][0] = kHandlePosFractions[n][0] * s1 +
						kHandlePosFractions[n][1] * s2;
		handles[n][1] = kHandlePosFractions[n][2] * t1 +
						kHandlePosFractions[n][3] * t2;
	}
}

static	void	DrawQuad(float x, float y, float radius)
{
	glVertex2f(x-radius,y-radius);
	glVertex2f(x-radius,y+radius);
	glVertex2f(x+radius,y+radius);
	glVertex2f(x+radius,y-radius);
}

OE_PatchEdPane::OE_PatchEdPane(
			int			inLeft,
			int			inTop,
			int			inRight,
			int			inBottom,
			OE_Pane *	inParent,
			bool		inProjection)  
	: OE_TexEdPane(inLeft, inTop, inRight, inBottom, inParent),
	mProjection(inProjection)
{
}	

OE_PatchEdPane::~OE_PatchEdPane()
{
}

void		OE_PatchEdPane::DrawSelf(void)
{
	OE_TexEdPane::DrawSelf();
	if (!HasTexture()) return;
	
	float	handles[8][2];
	CalcHandlePositions(
			GetTexture().s1,
			GetTexture().s2,
			GetTexture().t1,
			GetTexture().t2,
			handles);
			
	XPLMSetGraphicsState(0,0,0,  0,0, 0,0);
	glColor3f(1.0, 1.0, 0.0);	// Yellow
	glBegin(GL_LINE_LOOP);
	for (int n = 0; n < 8; ++n)
		glVertex2f(SCoordToX(handles[n][0]), TCoordToY(handles[n][1]));
	glEnd();
	
	glBegin(GL_QUADS);
	for (int n = 0; n < 8; ++n)
		DrawQuad(SCoordToX(handles[n][0]), TCoordToY(handles[n][1]), GetHandleRadius(false));
	glEnd();
}

int			OE_PatchEdPane::HandleClick(XPLMMouseStatus status, int x, int y, int button)
{
	if (OE_TexEdPane::HandleClick(status, x,y,button))	return 1;
	int l,t,r,b;
	XPGetWidgetGeometry(mWidget,&l,&t,&r,&b);
	if (l == r || t == b) return 0;
	if (!HasTexture()) return 0;

	if (status == xplm_MouseDown)
	{
		float	handles[8][2];
		CalcHandlePositions(
				mDragS1 = GetTexture().s1,
				mDragS2 = GetTexture().s2,
				mDragT1 = GetTexture().t1,
				mDragT2 = GetTexture().t2,
				handles);
				
		mMouseX = x;
		mMouseY = y;		
		mHandle = 8;
		
		for (int n = 0; n < 8; ++n)
		{
			int px = SCoordToX(handles[n][0]);
			int py = TCoordToY(handles[n][1]);
			if ((((px - x) * (px - x) + (py - y) * (py - y))) < GetHandleRadius(true))
			{
				mHandle = n;
			}
		}
		
	} else {
	
		float	deltaH = XCoordToS(x) - XCoordToS(mMouseX);
		float	deltaV = YCoordToT(y) - YCoordToT(mMouseY);
		
		if (kHandleWriteFlags[mHandle][0])		GetTexture().s1 = mDragS1 + deltaH;
		if (kHandleWriteFlags[mHandle][1])		GetTexture().s2 = mDragS2 + deltaH;
		if (kHandleWriteFlags[mHandle][2])		GetTexture().t1 = mDragT1 + deltaV;
		if (kHandleWriteFlags[mHandle][3])		GetTexture().t2 = mDragT2 + deltaV;	
		
		if (mProjection)	
			OE_Notifiable::Notify(catagory_Projection, msg_ProjectionTexChanged, NULL);
	}
	
	return 1;
}

void		OE_PatchEdPane::HandleNotification(int catagory, int message, void * param)
{
	OE_TexEdPane::HandleNotification(catagory,message, param);
}

bool			OE_PatchEdPane::HasTexture(void)
{
	if (mProjection) return true;
	if (gCurTexture < 0 || gCurTexture >= gTextures.size()) return false;
	return true;
}

OE_Texture_t&	OE_PatchEdPane::GetTexture(void)
{
	if (mProjection) 	return gProjectionMgr->GetTexture();
	return gTextures[gCurTexture];
}
