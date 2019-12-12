/*
 * Copyright (c) 2009, Laminar Research.
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

#include "WED_TCE.h"
#include "WED_TCEToolNew.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "WED_ToolUtils.h"
#include "WED_Messages.h"
#include "IGIS.h"
#include "WED_Thing.h"
#include "WED_Entity.h"
#include "WED_DrapedOrthophoto.h"
#include "TexUtils.h"
#include "WED_ResourceMgr.h"
#include "WED_DrawUtils.h"

#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
#endif

WED_TCE::WED_TCE(IResolver * in_resolver) :
	mResolver(in_resolver)
{
	mKillAlpha=true;
	mWrap=false;
	mTex = NULL;
	mTool = NULL;
	mIsToolClick = 0;
	mIsDownCount = 0;
	mIsDownExtraCount = 0;
}

WED_TCE::~WED_TCE()
{
}

void		WED_TCE::SetTool(WED_TCEToolNew * tool)
{
	if (mTool) mTool->KillOperation(mIsToolClick);
	mTool = tool;
}

void		WED_TCE::AddLayer(WED_TCELayer * layer)
{
	mLayers.push_back(layer);
}

void		WED_TCE::SetBounds(int x1, int y1, int x2, int y2)
{
	GUI_Pane::SetBounds(x1,y1,x2,y2);
	SetPixelBounds(x1,y1,x2,y2);
}

void		WED_TCE::SetBounds(int inBounds[4])
{
	GUI_Pane::SetBounds(inBounds);
	SetPixelBounds(inBounds[0],inBounds[1],inBounds[2],inBounds[3]);

}

void		WED_TCE::Draw(GUI_GraphState * state)
{
	CalcBgknd();
	WED_TCELayer * cur = mTool;

	Bbox2 bounds;

	bool draw_ent_v, draw_ent_s;

	GetMapVisibleBounds(bounds.p1.x_,bounds.p1.y_,bounds.p2.x_,bounds.p2.y_);
	ISelection * sel = GetSel();
//	IGISEntity * base = GetGISBase();

	vector<IGISEntity *> possibles;
	sel->IterateSelectionOr(Iterate_CollectEntities, &possibles);

//		ILibrarian * lmgr = WED_GetLibrarian(mResolver);
		ITexMgr *	tman = WED_GetTexMgr(mResolver);

	if(mTex)
	{
		int tex_id = tman->GetTexID(mTex);
		if (tex_id != 0)
		{
			state->SetState(false,1,false,	!mKillAlpha,!mKillAlpha,		false,false);
			state->BindTex(tex_id,0);
			glColor3f(1,1,1);
			WED_MapZoomerNew * z = this;
			glBegin(GL_QUADS);
			float minv = mWrap ? -32 :  0;
			float maxv = mWrap ?  32 :  1;
			glTexCoord2f(minv,maxv); glVertex2f(z->LonToXPixel(minv),z->LatToYPixel(minv));
			glTexCoord2f(minv,minv); glVertex2f(z->LonToXPixel(minv),z->LatToYPixel(maxv));
			glTexCoord2f(maxv,minv); glVertex2f(z->LonToXPixel(maxv),z->LatToYPixel(maxv));
			glTexCoord2f(maxv,maxv); glVertex2f(z->LonToXPixel(maxv),z->LatToYPixel(minv));
			glEnd();
			state->SetState(false,0,false,true,true,false,false);
			glColor4f(1,1,1,0.5);
			glBegin(GL_LINES);
			for(float f = minv; f <= maxv; ++f)
			{
				glVertex2(z->LLToPixel(Point2(minv,f)));
				glVertex2(z->LLToPixel(Point2(maxv,f)));
				glVertex2(z->LLToPixel(Point2(f,minv)));
				glVertex2(z->LLToPixel(Point2(f,maxv)));
			}
			glEnd();
		}
	}

	vector<WED_TCELayer *>::iterator l;

	// Ben says: big hack for now.  Tools have NO idea if we have a background.  But...no background means the user is working blind.
	// If we haven't dug out a texture, just don't let the tools and layers run.
	if(mTex)
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->GetCaps(draw_ent_v, draw_ent_s);
		if(draw_ent_v)
		for(vector<IGISEntity *>::iterator e = possibles.begin(); e != possibles.end(); ++e)
			(*l)->DrawEntityVisualization(cur == *l, *e, state);
		(*l)->DrawVisualization(cur == *l, state);
	}

	if(mTex)
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->GetCaps(draw_ent_v, draw_ent_s);
		if(draw_ent_s)
		for(vector<IGISEntity *>::iterator e = possibles.begin(); e != possibles.end(); ++e)
			(*l)->DrawEntityVisualization(cur == *l, *e, state);
		(*l)->DrawStructure(cur == *l, state);
	}

	if(mTex)
	for (l = mLayers.begin(); l != mLayers.end(); ++l)
	{
		(*l)->DrawSelected(cur == *l, state);
	}

	int x, y;
	GetMouseLocNow(&x,&y);


	if (mIsDownExtraCount)
	{
		state->SetState(0,0,0,1,1,0,0);
		glColor4f(1,1,1,0.4);
		glBegin(GL_LINES);
		glVertex2i(mX_Orig,mY_Orig);
		glVertex2i(x,y);
		glEnd();
	}

	int b[4];
	GetBounds(b);
	float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[3] - 15, mTool ? mTool->GetToolName() : "");

	const char * status = (mTool && mTex) ? mTool->GetStatusText() : NULL;
	if (status)
	GUI_FontDraw(state, font_UI_Basic, white, b[0]+5,b[1] + 15, status);

}

int			WED_TCE::MouseDown(int x, int y, int button)
{
	if (mIsDownCount++==0)
	{
		mX_Orig = x;
		mY_Orig = y;
	}
	if (button > 1) ++mIsDownExtraCount;

	if(button==0) mIsToolClick = 0;
	if(button==0 && mTool && mTex && mTool->HandleClickDown(x,y,button, GetModifiersNow())) { mIsToolClick=1; }
	if(button==1)
	{
		mX = x;
		mY = y;
	}
	// Refresh - map tools don't have access to a GUI_Pane and can't force refreshes.  But they are likely to do per-gesture drawing.
	Refresh();
	return 1;
}

void		WED_TCE::MouseDrag(int x, int y, int button)
{
	if (button==0 && mIsToolClick && mTex && mTool) mTool->HandleClickDrag(x,y,button, GetModifiersNow());
	if (button==1)
	{
		this->PanPixels(mX, mY, x, y);
		mX = x;
		mY = y;
	}
	Refresh();
}

void		WED_TCE::MouseUp  (int x, int y, int button)
{
	--mIsDownCount;
	if (button > 1) --mIsDownExtraCount;

	if (button==0&&mIsToolClick && mTex && mTool)	mTool->HandleClickUp(x,y,button, GetModifiersNow());
	if (button==1)				this->PanPixels(mX, mY, x, y);
	if(button==0)mIsToolClick = 0;
	Refresh();
}

int	WED_TCE::MouseMove(int x, int y)
{
	Refresh();
	return 1;
}

int			WED_TCE::HandleKeyPress(uint32_t inKey, int inVK, GUI_KeyFlags inFlags)
{
	if (mTool && mTex) 	return mTool->HandleToolKeyPress(inKey, inVK, inFlags);
	else				return 0;
}


int			WED_TCE::ScrollWheel(int x, int y, int dist, int axis)
{
	double	zoom = 1.0;
	while (dist > 0)
	{
		zoom *= 1.1;
		--dist;
	}
	while (dist < 0)
	{
		zoom /= 1.1;
		++dist;
	}

	if (zoom != 1.0)
		this->ZoomAround(zoom, x, y);
	Refresh();
	return 1;
}

void		WED_TCE::ReceiveMessage(
							GUI_Broadcaster *		inSrc,
							intptr_t				inMsg,
							intptr_t				inParam)
{
	if(inMsg == msg_ArchiveChanged)	Refresh();
}

IGISEntity *	WED_TCE::GetGISBase()
{
	return dynamic_cast<IGISEntity *>(WED_GetWorld(mResolver));
}

ISelection *	WED_TCE::GetSel()
{
	return WED_GetSelect(mResolver);
}

void WED_TCE::CalcBgknd(void)
{
	TexRef old = mTex;
	ISelection * sel = WED_GetSelect(mResolver);
	mTex = NULL;
	vector<IGISEntity *> possibles;
	sel->IterateSelectionOr(Iterate_CollectEntities, &possibles);

		ITexMgr *	tman = WED_GetTexMgr(mResolver);
		WED_ResourceMgr * rmgr = WED_GetResourceMgr(mResolver);
		
	Bbox2		needed_area;

	for(vector<IGISEntity*>::iterator e = possibles.begin(); e != possibles.end(); ++e)
	{
		WED_DrapedOrthophoto * ortho;
		if ((ortho = dynamic_cast<WED_DrapedOrthophoto*>(*e)) != NULL)
		{
			string vpath;
			const pol_info_t * pol_info;
			ortho->GetResource(vpath);
			if(rmgr->GetPol(vpath,pol_info))
			{
				mTex = tman->LookupTexture(pol_info->base_tex.c_str(),true, pol_info->wrap ? (tex_Wrap|tex_Compress_Ok) : tex_Compress_Ok);
				if(mTex)
				{
					mKillAlpha=pol_info->kill_alpha;
					mWrap=pol_info->wrap;
				}
			}
			Bbox2	this_ortho;
			ortho->GetBounds(gis_UV,this_ortho);
			needed_area += this_ortho;
		}
	}
	if(mTex != old && mTex)
	{
		int org_x,org_y;
		tman->GetTexInfo(mTex,NULL,NULL,NULL,NULL,&org_x,&org_y);
		SetMapLogicalBounds(mWrap ? -32 : 0, mWrap ? -32 : 0, mWrap ? 32 : 1, mWrap ? 32 : 1);
		ZoomShowArea(floor(needed_area.xmin()),floor(needed_area.ymin()),ceil(needed_area.xmax()),ceil(needed_area.ymax()));
	}
}