/*
 * Copyright (c) 2007, Laminar Research.
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

#include "WED_WorldMapLayer.h"
#include "GUI_Pane.h"
#include "GUI_GraphState.h"
#include "GUI_Resources.h"
#include "WED_MapZoomerNew.h"
#include "WED_ToolUtils.h"
#include "WED_TexMgr.h"
#include "WED_PackageMgr.h"
#include "TexUtils.h"
#include "PlatformUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

WED_WorldMapLayer::WED_WorldMapLayer(GUI_Pane * host, WED_MapZoomerNew * zoomer, IResolver * resolver) :
	WED_MapLayer(host,zoomer,resolver)
{
	mTexMgr = WED_GetTexMgr(resolver);
	gPackageMgr->GetXPlaneFolder(mBitmapPath);
	mBitmapPath += DIR_STR "Resources" DIR_STR "bitmaps" DIR_STR "Earth Orbit Textures" DIR_STR;
}

WED_WorldMapLayer::~WED_WorldMapLayer()
{
}

void		WED_WorldMapLayer::DrawVisualization		(bool inCurrent, GUI_GraphState * g)
{
	double ll,lb,lr,lt;	// logical boundary
	double vl,vb,vr,vt;	// visible boundry

	GetZoomer()->GetMapLogicalBounds(ll,lb,lr,lt);
	GetZoomer()->GetMapVisibleBounds(vl,vb,vr,vt);

	vl = max(vl,ll);
	vb = max(vb,lb);
	vr = min(vr,lr);
	vt = min(vt,lt);

	int	tex_id = GUI_GetTextureResource("worldmap.jpg", 0, NULL);
	if (tex_id)
	{
		g->SetState(0, 1, 0,    0, 1,  0, 0);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		g->BindTex(tex_id, 0);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2f( GetZoomer()->LonToXPixel(-180 ),
					GetZoomer()->LatToYPixel( -90 ));
		glTexCoord2f(0.0, 1.0);
		glVertex2f( GetZoomer()->LonToXPixel(-180 ),
					GetZoomer()->LatToYPixel(  90 ));
		glTexCoord2f(1.0, 1.0);
		glVertex2f( GetZoomer()->LonToXPixel( 180 ),
					GetZoomer()->LatToYPixel(  90 ));
		glTexCoord2f(1.0, 0.0);
		glVertex2f( GetZoomer()->LonToXPixel( 180 ),
					GetZoomer()->LatToYPixel( -90 ));
		glEnd();
	}

	double lon_span = vr - vl;
	double lat_span = vt - vb;
	if (lon_span < 20.0 && lat_span < 10.0)  // loading 20 degrees wide requires up to 3 textures - depending of how they are aligned
	{
		int l = 10 * (int) floor(vl / 10.0);
		int b = 10 * (int) floor(vb / 10.0);
		int r = 10 * (int) ceil(vr / 10.0);
		int t = 10 * (int) ceil(vt / 10.0);
		
		for (int y = b; y < t; y += 10)
			for (int x = l; x < r; x += 10)
			{
				char	fname[200];
				snprintf(fname,200,"%s%+03d%+04d.dds", mBitmapPath.c_str(), y, x);
				TexRef tref = mTexMgr->LookupTexture(fname, true, tex_Compress_Ok);
				if(tref)
				{
					int tex_id = mTexMgr->GetTexID(tref);
					if (tex_id)
					{
						g->BindTex(tex_id, 0);
						glBegin(GL_QUADS);
						glTexCoord2f(0.0, 0.0);
						glVertex2f( GetZoomer()->LonToXPixel( x    ),
									GetZoomer()->LatToYPixel( y    ));
						glTexCoord2f(0.0, 1.0);
						glVertex2f( GetZoomer()->LonToXPixel( x    ),
									GetZoomer()->LatToYPixel( y+10 ));
						glTexCoord2f(1.0, 1.0);
						glVertex2f( GetZoomer()->LonToXPixel( x+10 ),
									GetZoomer()->LatToYPixel( y+10 ));
						glTexCoord2f(1.0, 0.0);
						glVertex2f( GetZoomer()->LonToXPixel( x+10 ),
									GetZoomer()->LatToYPixel( y    ));
						glEnd();
					}
				}
			}
	}
	
}

void		WED_WorldMapLayer::GetCaps(bool& draw_ent_v, bool& draw_ent_s, bool& cares_about_sel, bool& wants_clicks)
{
	draw_ent_v = draw_ent_s = cares_about_sel = wants_clicks = 0;
}
