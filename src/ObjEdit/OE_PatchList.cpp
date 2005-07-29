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
#include "OE_PatchList.h"
#include "OE_Scroller.h"
#include "OE_Globals.h"
#include "OE_Msgs.h"
#include "XPLMGraphics.h"
#include "OE_TexMgr.h"

enum {
	col_Pic = 0,
	col_Name,
	col_Left,
	col_Bottom,
	col_Right,
	col_Top,
	col_Count
};

const int kRowHeight = 32;

const int	kColRights[] = {
	32, 150, 180, 210, 240, 270 };

OE_PatchTable::OE_PatchTable(
                                   int                  inLeft,    
                                   int                  inTop,    
	                               OE_Pane *			inSuper) :
	OE_TablePane(inLeft, inTop, inLeft + kColRights[col_Count - 1],inTop - kRowHeight, 1, inSuper)
{
}
	
OE_PatchTable::~OE_PatchTable()
{
}

void	OE_PatchTable::DrawCell(int row, int col, int l, int t, int r, int b)
{
	static	GLfloat	black[3] = { 0.0, 0.0, 0.0 };
	
	if (row < 0 || row >= gTextures.size())	return;
	if (col < 0 || col >= col_Count) return;
	
	XPLMSetGraphicsState(0, 0, 0,    0, 0,  0, 0);
	if (row == gCurTexture)
		glColor3f(1.0, 1.0, 0.0);
	else
		glColor3f(1.0, 1.0, 1.0);	
	glBegin(GL_QUADS);
	glVertex2i(l,t);
	glVertex2i(r,t);
	glVertex2i(r,b);
	glVertex2i(l,b);
	glEnd();
	glColor3f(0.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	glVertex2i(l,t);
	glVertex2i(r,t);
	glVertex2i(r,b);
	glVertex2i(l,b);
	glEnd();
	
	if (col == col_Pic)
	{
		if (!gObjects.empty())
		{
			string tex = gObjects[gLevelOfDetail].texture;
			if (!tex.empty())
			{
				GLenum texid = FindTexture(tex, false, NULL, NULL);
				if (texid)
				{
					XPLMBindTexture2d(texid, 0);
					XPLMSetGraphicsState(0, 1, 0,  1, 1, 0, 0);
					glColor3f(1.0, 1.0, 1.0);
					
					glBegin(GL_QUADS);
					glTexCoord2f(gTextures[row].s1, gTextures[row].t2);
					glVertex2i(l+1,t-1);
					glTexCoord2f(gTextures[row].s2, gTextures[row].t2);
					glVertex2i(r-1,t-1);
					glTexCoord2f(gTextures[row].s2, gTextures[row].t1);
					glVertex2i(r-1,b+1);
					glTexCoord2f(gTextures[row].s1, gTextures[row].t1);
					glVertex2i(l+1,b+1);
					glEnd();					
				}					
			}
		}

		
	} else {
		char	buf[256];
		switch(col) {
		case col_Name:
			sprintf(buf,"%s", gTextures[row].name.c_str());
			break;
		case col_Left:
			sprintf(buf,"%f", gTextures[row].s1);
			break;
		case col_Right:
			sprintf(buf,"%f", gTextures[row].s2);
			break;
		case col_Bottom:
			sprintf(buf,"%f", gTextures[row].t1);
			break;
		case col_Top:
			sprintf(buf,"%f", gTextures[row].t2);
			break;
		}
		
		XPLMDrawString(black, l + 3, b + 5, buf, NULL, xplmFont_Basic);
	}
}

void	OE_PatchTable::ClickCell(int row, int col, int l, int t, int r, int b)
{
	int old = gCurTexture;
	if (row >= 0 && row < gTextures.size())
		gCurTexture = row;
	else
		gCurTexture = -1;
	if (old != gCurTexture)
		OE_Notifiable::Notify(catagory_Texture, msg_TexSelectionChanged, NULL);
}

void	OE_PatchTable::HandleNotification(int catagory, int message, void * param)
{
	if (catagory == catagory_Texture &&
		(message == msg_TexAdded || message == msg_TexDeleted || message == msg_TexLoaded))
	{
		SnapToCols();
	}
}

int		OE_PatchTable::GetColCount(void)
{
	return	col_Count;
}

int		OE_PatchTable::GetRowCount(void)
{
	return gTextures.size();
}

int		OE_PatchTable::GetColRight(int col)
{
	return kColRights[col];
}

int		OE_PatchTable::GetRowBottom(int col)
{
	return col * kRowHeight + kRowHeight;
}

#pragma mark -

OE_PatchList::OE_PatchList(
                                   	int                  inLeft,    
                                   	int                  inTop,    
                                   	int                  inRight,    
                                   	int                  inBottom) :
	OE_Pane(inLeft,inTop, inRight,inBottom, 1, "TexWindow", NULL)
{
	mScroller = new OE_Scroller(inLeft,inTop,inRight, inBottom,
					1,	this,	false,true);

	mPane = new OE_PatchTable(inLeft, inTop, mScroller);

	mScroller->SetContents(mPane);
	mPane->SnapToCols();
}

	
OE_PatchList::~OE_PatchList()
{
	delete mPane;
	delete mScroller;
}
