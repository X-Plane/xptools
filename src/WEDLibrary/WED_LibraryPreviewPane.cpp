/* 
 * Copyright (c) 2012, Laminar Research.
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

#include "WED_LibraryPreviewPane.h"
#include "GUI_DrawUtils.h"

#include "WED_ResourceMgr.h"
#include "ITexMgr.h"
#include "WED_LibraryMgr.h"
#include "MathUtils.h"
#include "XObjDefs.h"
#include "TexUtils.h"
#include "ObjDraw.h"
#include "GUI_GraphState.h"
#include "WED_PreviewLayer.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

WED_LibraryPreviewPane::WED_LibraryPreviewPane(WED_ResourceMgr * res_mgr, ITexMgr * tex_mgr) : mResMgr(res_mgr), mTexMgr(tex_mgr),mZoom(1.0),mPsi(0.0f),mThe(0.0f)
{
}

void WED_LibraryPreviewPane::SetResource(const string& r, int res_type)
{
	mRes = r;
	mType = res_type;
}

void WED_LibraryPreviewPane::ClearResource(void)
{
	mRes.clear();
}

int		WED_LibraryPreviewPane::ScrollWheel(int x, int y, int dist, int axis)
{
	while(dist > 0)
	{
		mZoom /= 1.2;
		--dist;
	}
	while(dist < 0)
	{
		mZoom *= 1.2;
		++dist;
	}
	mZoom=fltlim(mZoom,0.1,3.0);
	Refresh();
	return 1;
}

int			WED_LibraryPreviewPane::MouseDown(int x, int y, int button)
{
	mX = x;
	mY = y;
	mPsiOrig=mPsi;
	mTheOrig=mThe;
	return 1;
}
void		WED_LibraryPreviewPane::MouseDrag(int x, int y, int button)
{
	float dx = x - mX;
	float dy = y - mY;
	mPsi = mPsiOrig + dx * 0.5;
	mThe = mTheOrig - dy * 0.5;
	
	mThe = fltlim(mThe,-90,90);
	mPsi = fltwrap(mPsi,-180,180);
	Refresh();
}
void		WED_LibraryPreviewPane::MouseUp  (int x, int y, int button)
{
}


void	WED_LibraryPreviewPane::Draw(GUI_GraphState * g)
{
	int b[4];
	GetBounds(b);

	XObj8 * o = NULL;
	float dx = b[2] - b[0];
	float dy = b[3] - b[1];
	float sx = ((dx > dy) ? (dx / dy) : 1.0)/2;
	float sy = ((dx > dy) ? 1.0 : (dy / dx))/2;
	#if AIRPORT_ROUTING
	agp_t agp;
	#endif
	pol_info_t pol;

	if(!mRes.empty())
	switch(mType) {
	case res_Polygon:
		if(mResMgr->GetPol(mRes,pol))
		{
			TexRef	ref = mTexMgr->LookupTexture(pol.base_tex.c_str(),true, pol.wrap ? (tex_Compress_Ok|tex_Wrap) : tex_Compress_Ok);
			if(ref != NULL)
			{
				int tex_id = mTexMgr->GetTexID(ref);

				if (tex_id != 0)
				{
					g->SetState(false,1,false,!pol.kill_alpha,!pol.kill_alpha,false,false);
					glColor3f(1,1,1);
					g->BindTex(tex_id,0);
					
					float prev_space = min(b[2]-b[0],b[3]-b[1]);
					float ds = prev_space / mZoom * ((pol.proj_s > pol.proj_t) ? 1.0 : (pol.proj_s / pol.proj_t));
					float dt = prev_space / mZoom * ((pol.proj_s > pol.proj_t) ? (pol.proj_t / pol.proj_s) : 1.0);
					
					float x1 = (dx - ds) /2;
					float x2 = (dx + ds) /2;
					float y1 = (dy - dt) /2;
					float y2 = (dy + dt) /2;
					
					glBegin(GL_QUADS);
					if(pol.wrap)
					{
						glTexCoord2f(extrap(x1,0,x2,1,b[0]),	extrap(y1,0,y2,1,b[1]));			glVertex2f(b[0],b[1]);
						glTexCoord2f(extrap(x1,0,x2,1,b[0]),	extrap(y1,0,y2,1,b[3]));			glVertex2f(b[0],b[3]);
						glTexCoord2f(extrap(x1,0,x2,1,b[2]),	extrap(y1,0,y2,1,b[3]));			glVertex2f(b[2],b[3]);
						glTexCoord2f(extrap(x1,0,x2,1,b[2]),	extrap(y1,0,y2,1,b[1]));			glVertex2f(b[2],b[1]);
					}
					else
					{
						glTexCoord2f(0,0); glVertex2f(x1,y1);
						glTexCoord2f(0,1); glVertex2f(x1,y2);
						glTexCoord2f(1,1); glVertex2f(x2,y2);
						glTexCoord2f(1,0); glVertex2f(x2,y1);
					}
					glEnd();
				}	
			}
		}
		break;
	case res_Forest:
		if(!mResMgr->GetFor(mRes,o))
			break;
	case res_Object:
		if (o || mResMgr->GetObj(mRes,o))
		{
			float real_radius=pythag(
								o->xyz_max[0]-o->xyz_min[0],
								o->xyz_max[1]-o->xyz_min[1],
								o->xyz_max[2]-o->xyz_min[2]);
			float approx_radius = real_radius * mZoom;

			glPushAttrib(GL_VIEWPORT_BIT);
			glViewport(b[0],b[1],b[2]-b[0],b[3]-b[1]);
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(sx * -approx_radius,sx * approx_radius,sy * -approx_radius,sy * approx_radius,-real_radius,real_radius);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();			
			glRotatef(mThe,1,0,0);
			glRotatef(mPsi,0,1,0);
			g->EnableDepth(true,true);
			glClear(GL_DEPTH_BUFFER_BIT);
			draw_obj_at_xyz(mTexMgr, o, 
								-(o->xyz_max[0]+o->xyz_min[0])*0.5f,
								-(o->xyz_max[1]+o->xyz_min[1])*0.5f,
								-(o->xyz_max[2]+o->xyz_min[2])*0.5f,
				0, g);
			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glPopAttrib();
		}
		#if AIRPORT_ROUTING
		else if (mResMgr->GetAGP(mRes,agp))
		{
			double min_xy[2] = { 0, 0 };
			double max_xy[2] = { 0, 0 };
			for(int n = 0; n < agp.tile.size(); n += 4)
			{
				min_xy[0] = min(min_xy[0],agp.tile[n]);
				max_xy[0] = max(max_xy[0],agp.tile[n]);
				min_xy[1] = min(min_xy[1],agp.tile[n+1]);
				max_xy[1] = max(max_xy[1],agp.tile[n+1]);
			}

			float real_radius=pythag(
								max_xy[0] - min_xy[0],
								max_xy[1] - min_xy[1]);
								
			float approx_radius = real_radius * mZoom;

			glPushAttrib(GL_VIEWPORT_BIT);
			glViewport(b[0],b[1],b[2]-b[0],b[3]-b[1]);
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glOrtho(sx * -approx_radius,sx * approx_radius,sy * -approx_radius,sy * approx_radius,-real_radius,real_radius);
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();			
			glRotatef(mThe,1,0,0);
			glRotatef(mPsi,0,1,0);
			g->EnableDepth(true,true);
			glClear(GL_DEPTH_BUFFER_BIT);
			
			glTranslatef((max_xy[0]+min_xy[0]) * -0.5,
						  0.0,
						(max_xy[1]+min_xy[1]) * 0.5);

			g->SetState(false,1,false,true,true,false,false);
			TexRef	ref = mTexMgr->LookupTexture(agp.base_tex.c_str() ,true, tex_Linear|tex_Mipmap|tex_Compress_Ok);			
			int id1 = ref  ? mTexMgr->GetTexID(ref ) : 0;
			if(id1)g->BindTex(id1,0);

			glColor3f(1,1,1);
			if(!agp.tile.empty() && !agp.hide_tiles)
			{
				glDisable(GL_CULL_FACE);
				glBegin(GL_TRIANGLE_FAN);
				for(int n = 0; n < agp.tile.size(); n += 4)
				{
					glTexCoord2f(agp.tile[n+2],agp.tile[n+3]);
					glVertex3f(agp.tile[n],0,-agp.tile[n+1]);
				}
				glEnd();
				glEnable(GL_CULL_FACE);
			}	
			for(vector<agp_t::obj>::iterator o = agp.objs.begin(); o != agp.objs.end(); ++o)
			{
				XObj8 * oo;
				if(mResMgr->GetObjRelative(o->name,mRes,oo))
				{
					draw_obj_at_xyz(mTexMgr, oo, o->x,0,-o->y,o->r, g);			
				} 
			}

			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();
			glPopAttrib();			
		}
		#endif
		break;
	}
}
