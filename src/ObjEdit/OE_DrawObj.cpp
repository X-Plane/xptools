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
#include "OE_DrawObj.h"
#include "OE_TexMgr.h"
#include "XObjDefs.h"
#include "GeoUtils.h"
#include "XPLMProcessing.h"
#if APL
#include <glext.h>
#endif
#include "XWinGL.h"
#include "XPLMGraphics.h"
#include "XPWidgetWin.h"
static	void	MultiTexCoord(float s, float t)
{
	glMultiTexCoord2fARB(GL_TEXTURE1_ARB, s, t);
	glTexCoord2f(s, t);
}

enum {

	pass_Back,
	pass_FrontNormal,
	pass_FrontOverlay,
	pass_WireNormal,
	pass_WireOverlay

};

void	OE_DrawObj(
				const XObj&		inObj,
				const set<int>&	inSel,
				int				inPolySolid,
				int				inCullMode,
				int				inTexMode)
{
	GLenum dayTex 	= FindTexture(inObj.texture, false, NULL, NULL);
	GLenum nightTex	= FindTexture(inObj.texture, true, NULL, NULL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	if (dayTex)
		XPLMBindTexture2d(dayTex, 0);
	if (nightTex)
		XPLMBindTexture2d(nightTex, 1);

	if (nightTex == 0 && inTexMode == tex_Lit)	inTexMode = tex_Day;
	if (dayTex == 0)	inTexMode = tex_None;

	vector<int>	passes;

	switch(inPolySolid){
	case poly_Wire:
		passes.push_back(pass_WireNormal);
		if (inCullMode == cull_ShowAll)
			glDisable(GL_CULL_FACE);
		break;
	case poly_Solid:
		if (inCullMode == cull_ShowVisible)
			passes.push_back(pass_FrontNormal);
		else {
			passes.push_back(pass_Back);
			passes.push_back(pass_FrontOverlay);
		}
		break;
	case poly_WireAndSolid:
		if (inCullMode == cull_ShowVisible)
			passes.push_back(pass_FrontNormal);
		else {
			passes.push_back(pass_Back);
			passes.push_back(pass_FrontOverlay);
		}
		passes.push_back(pass_WireOverlay);
		break;
	}

	for (vector<int>::iterator pass = passes.begin(); pass != passes.end(); ++pass)
	{
		glPolygonOffset(-5.0, 0.0);
		bool	writeDepth = false;
		switch(*pass) {
		case	pass_Back:
			glPolygonMode(GL_FRONT, GL_FILL);
			glCullFace(GL_FRONT);
			writeDepth = true;
			glDisable(GL_POLYGON_OFFSET_LINE);
			glDisable(GL_POLYGON_OFFSET_FILL);
			break;
		case	pass_FrontNormal:
			glPolygonMode(GL_FRONT, GL_FILL);
			glCullFace(GL_BACK);
			writeDepth = true;
			glDisable(GL_POLYGON_OFFSET_LINE);
			glDisable(GL_POLYGON_OFFSET_FILL);
			break;
		case	pass_FrontOverlay:
			glPolygonMode(GL_FRONT, GL_FILL);
			glCullFace(GL_BACK);
			writeDepth = false;
			glDisable(GL_POLYGON_OFFSET_LINE);
			glEnable(GL_POLYGON_OFFSET_FILL);
			break;
		case	pass_WireNormal:
			glPolygonMode(GL_FRONT, GL_LINE);
			glPolygonMode(GL_BACK, GL_LINE);
			glCullFace(GL_BACK);
			writeDepth = true;
			glDisable(GL_POLYGON_OFFSET_LINE);
			glDisable(GL_POLYGON_OFFSET_FILL);
			break;
		case	pass_WireOverlay:
			glPolygonMode(GL_FRONT, GL_LINE);
			glCullFace(GL_BACK);
			writeDepth = false;
			glEnable(GL_POLYGON_OFFSET_LINE);
			glDisable(GL_POLYGON_OFFSET_FILL);
			break;
		}

		int	cmdIndex = 0;
		for (vector<XObjCmd>::const_iterator cmdIter = inObj.cmds.begin();
			cmdIter != inObj.cmds.end(); ++cmdIter, ++cmdIndex)
		{
			switch(cmdIter->cmdType) {
			case type_PtLine:
				if (writeDepth)
				{
					XPLMSetGraphicsState(0, 0, 0,   0, 0,    1, 1);

					switch(cmdIter->cmdID) {
					case obj_Light:
						glPointSize(inSel.count(cmdIndex) ? 3 : 1);
						glBegin(GL_POINTS);
						break;
					case obj_Line:
						glLineWidth(inSel.count(cmdIndex) ? 3 : 1);
						glBegin(GL_LINES);
						break;
					}
					for(vector<vec_rgb>::const_iterator liter = cmdIter->rgb.begin();
						liter != cmdIter->rgb.end(); ++liter)
					{
						glColor3f(liter->rgb[0] / 10.0, liter->rgb[1] / 10.0, liter->rgb[2] / 10.0);
						glVertex3fv(liter->v);
					}
					glEnd();
				}
				break;

			case type_Poly:

				XPLMSetGraphicsState(0, inTexMode, 0, inTexMode ? 1 : 0, inTexMode ? 1 : 0, 1, writeDepth);

				switch(*pass) {
				case	pass_Back:
					if (inCullMode == cull_ShowAll)
					{
						if (inSel.count(cmdIndex))
							glColor3f(1.0, 0.5, 0.5);
						else
							glColor3f(1.0, 1.0, 1.0);
					} else {
						if (inSel.count(cmdIndex))
							glColor3f(1.0, 0.4, 0.4);
						else
							glColor3f(1.0, 0.2, 0.2);
					}
					break;
				case	pass_FrontNormal:
				case	pass_FrontOverlay:
					if (inSel.count(cmdIndex))
						glColor3f(1.0, 0.5, 0.5);
					else
						glColor3f(1.0, 1.0, 1.0);
					break;
				case	pass_WireNormal:
					if (inSel.count(cmdIndex))
						glColor3f(1.0, 0.5, 0.5);
					else
						glColor3f(1.0, 1.0, 1.0);
					break;
				case	pass_WireOverlay:
					glColor3f(0.0, 0.0, 0.0);
					break;
				}

				switch(cmdIter->cmdID) {
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

				for (vector<vec_tex>::const_iterator piter = cmdIter->st.begin();
					piter != cmdIter->st.end(); ++piter)
				{
					MultiTexCoord(piter->st[0],piter->st[1]);
					glVertex3fv(piter->v);
				}

				glEnd();
				break;
			}

		}	// Cmds
		glLineWidth(1);
		glPointSize(1);
		glDisable(GL_POLYGON_OFFSET_LINE);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonMode(GL_FRONT, GL_FILL);
		glPolygonMode(GL_BACK, GL_FILL);
		XPLMSetGraphicsState(0,0,0, 0,0,  1,1);
	}	// Pass
	glDisable(GL_CULL_FACE);
}

void OE_LabelVertices(const XObj& inObj, const set<int>& sel, int cur)
{
	XPLMSetGraphicsState(0, 0, 0,   0,0,   0,0);
	int counter = 0;
	vector<double>	x;
	vector<double>	y;
	for (set<int>::const_iterator s = sel.begin(); s != sel.end(); ++s)
	{
		for (int n = 0; n < inObj.cmds[*s].st.size(); ++n)
		{
			double	mtpt[3];
			mtpt[0] = inObj.cmds[*s].st[n].v[0];
			mtpt[1] = inObj.cmds[*s].st[n].v[1];
			mtpt[2] = inObj.cmds[*s].st[n].v[2];
			double	scpt[2];
			ModelToScreenPt(mtpt, scpt);
			x.push_back(scpt[0]);
			y.push_back(scpt[1]);
		}
	}

	glPushAttrib(GL_VIEWPORT_BIT);

	int	w, h;
	gWidgetWin->GetBounds(&w, &h);
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	float	hilite[3] = { 1.0, 1.0, 0.5 };
	float	normal[3] = { 0.5, 0.5, 0.5 };
	char	buf[50];


	float	now = XPLMGetElapsedTime();
	now = (now - ((float) ((int) now)));
	hilite[0] = hilite[1] = now;
	hilite[2] = 0.5 * now;

	for (int n = 0; n < x.size(); ++n)
	{
		sprintf(buf, "%d", n);
		if (n != cur)
		XPLMDrawString(normal, x[n], y[n], buf, NULL, xplmFont_Basic);
	}

	sprintf(buf, "%d", cur);
	XPLMDrawString(hilite, x[cur], y[cur], buf, NULL, xplmFont_Basic);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glPopAttrib();
}
