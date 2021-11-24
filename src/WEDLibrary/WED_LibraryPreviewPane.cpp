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

#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
	#include "ObjCUtils.h"
#else
	#include "glew.h"
	#include <GL/glu.h>
#endif

#include "ITexMgr.h"
#include "TexUtils.h"
#include "MathUtils.h"

#include "GUI_DrawUtils.h"
#include "GUI_Broadcaster.h"
#include "GUI_GraphState.h"
#include "GUI_Fonts.h"
#include "GUI_Messages.h"
#include "GUI_Resources.h"
#include "WED_Menus.h"

#include "WED_Colors.h"
#include "WED_LibraryMgr.h"
#include "WED_Globals.h"
#include "WED_ResourceMgr.h"
#include "WED_PreviewLayer.h"
#include "WED_Messages.h"

#include "XObjDefs.h"
#include "ObjDraw.h"
#include "XESConstants.h"

#define length_with_units(x) (x)*(gIsFeet ? MTR_TO_FT : 1), gIsFeet ? "ft" : "m"

enum {
	next_variant = GUI_APP_MESSAGES
};

static int best_num_rows(int items, int num_cols)
{
	return ceil(items / float(max(num_cols, 1)));
}

static int best_num_cols(int items, int w_pixels, int h_pixels)
{
	int cols = max(1,intround(sqrt(items * w_pixels / float(h_pixels))));
	int rows = best_num_rows(items, cols);
	int last_row = items % cols;
	while (last_row > 0 && cols > 2 && rows > 1 && cols - 1 - last_row >= rows -1)
	{
	  cols--;
	  last_row += rows - 1;
	}
	return cols;
}


WED_LibraryPreviewPane::WED_LibraryPreviewPane(GUI_Commander * cmdr, WED_ResourceMgr * res_mgr, ITexMgr * tex_mgr) : GUI_Commander(cmdr), mResMgr(res_mgr), mTexMgr(tex_mgr),mZoom(1.0),
                               mPsi(10),mThe(10), mHgt(10), mWid(20), mWalls(4)
{
		int k_reg[4] = { 0, 0, 1, 3 };
		int k_hil[4] = { 0, 1, 1, 3 };

//		int b[4]; GetBounds(b);  // No good, bounds not established at this point

		mNextButton = new GUI_Button("push_buttons.png",btn_Push,k_reg, k_hil,k_reg,k_hil);
		mNextButton->SetBounds(5,45,55,45+GUI_GetImageResourceHeight("push_buttons.png") / 3);
//		mNextButton->SetBounds(b[2]-55,b[3]-5-GUI_GetImageResourceHeight("push_buttons.png") / 3,b[2]-5,b[4]-5);
		mNextButton->SetSticky(1,1,0,0); // follow left bottom corner
		mNextButton->SetParent(this);
		mNextButton->SetMsg(next_variant,0);
		mNextButton->AddListener(this);
		mNextButton->Hide();

		mMSAA = 1;
		GLint tmp;
		glGetIntegerv(GL_SAMPLES, &tmp);
		if(tmp > 1)
		{
			mMSAA = 0;
			LOG_MSG("I/Lpp MSAA externally overridden already\n");
		}
#if APL
		const char * ext_str = (const char *)glGetString(GL_EXTENSIONS);
		if(strstr(ext_str, "GL_ARB_framebuffer_object") == nullptr)
		{
			mMSAA = 0;
			LOG_MSG("I/Lpp no FBO's - MSAA disabled\n");
		}

		float f = get_retina_backing();
		if (f != 1.0f)
		{
			mMSAA = 0;
			LOG_MSG("I/Lpp backingScaleFactor %.3f - MSAA disabled\n", f);
		}
#endif
		LOG_FLUSH();
}

void		WED_LibraryPreviewPane::ReceiveMessage(GUI_Broadcaster * inSrc, intptr_t inMsg, intptr_t inParam)
{
	if(inMsg == next_variant)
	{
		if (mVariant < mNumVariants-1)
			mVariant++;
		else
			mVariant = 0;

		char s[16]; sprintf(s,"%d/%d",mVariant+1,mNumVariants);
		mNextButton->SetDescriptor(s);
	}
}

void WED_LibraryPreviewPane::SetResource(const string& r, int res_type)
{
	mRes = r;
	mType = res_type;
	mVariant = 0;
//	mHgt = 0.0;

	if(res_type == res_Object || res_type == res_Facade)
	{
		mNumVariants = mResMgr->GetNumVariants(r);
		if(mNumVariants >1)
		{
			char s[16]; sprintf(s,"%d/%d",mVariant+1,mNumVariants);
			mNextButton->SetDescriptor(s);
			mNextButton->Show();
		}
		else
			mNextButton->Hide();

		if(res_type == res_Facade)
		{
			const fac_info_t * fac;
			if (mResMgr->GetFac(mRes, fac))
			{
				mWalls = intlim(fac->wallName.size(), 4, 16);
				if (!fac->is_new) mHgt = intlim(mHgt, fac->min_floors, fac->max_floors);
				//			mHgt = fac->is_new ? 10 : fac->min_floors;
				//			mWid = 20.0;
			}
			mWid = fltlim(mWid,10,150);
		}
		else
		{
			const agp_t * agp;
			if (mResMgr->GetAGP(mRes, agp))
				mHgt = 0.0;
		}
	}
	else
	{
		mNumVariants = 1;     // we haven't yet implemented variant display for anything else
		mNextButton->Hide();
		if(res_type == res_Directory)
		{
			mWalls = 1;
			mRess.clear();
			mResMgr->GetSimilar(mRes, mRess);
		}
		else if(res_type == res_Autogen)
			mWid = 0.0;
	}
}

void WED_LibraryPreviewPane::ClearResource(void)
{
	mRes.clear();
}

int	WED_LibraryPreviewPane::ScrollWheel(int x, int y, int dist, int axis)
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

void	WED_LibraryPreviewPane::MouseUp(int x, int y, int button)
{

	if(mX == x && mY == y && mType == res_Directory && mRess.size() && button == 0)
	{
		int b[4]; GetBounds(b);

		int w = b[2] - b[0];
		int h = b[3] - b[1];

		int num_x = best_num_cols(mRess.size(), w, h);
		int num_y = best_num_rows(mRess.size(), num_x);

		int pos_x = (x - b[0]) / float (w) * num_x;
		int pos_y = (y - b[1]) / float (h) * num_y;

//		for(auto& mr : mRess)
//			LOG_MSG("%s, %d\n", mr.first.c_str(), mr.second);
//		LOG_MSG("pos_x=%d pos_y=%d %d\n", pos_x, pos_y, pos_x + num_x * (num_y - pos_y - 1));
//		LOG_FLUSH();

		//SetResource(mRess[pos_x + num_x * (num_y - pos_y - 1)].first, mRess[pos_x + num_x * (num_y - pos_y - 1)].second);
		BroadcastMessage(WED_PRIVATE_MSG_BASE, (intptr_t) mRess[pos_x + num_x * (num_y - pos_y - 1)].first.c_str());
		Refresh();
	}
}

int	WED_LibraryPreviewPane::MouseDown(int x, int y, int button)
{
	mX = x;
	mY = y;
	mPsiOrig=mPsi;
	mTheOrig=mThe;
	mHgtOrig=mHgt;
	mWidOrig=mWid;

    if (mType == res_Polygon)
    {
		const pol_info_t * pol;
		mResMgr->GetPol(mRes,pol);

		if (pol->mSubBoxes.size())
		{
			int b[4]; GetBounds(b);
			float prev_space = min(b[2]-b[0],b[3]-b[1]);

			TexRef	tref = mTexMgr->LookupTexture(pol->base_tex.c_str(),true, pol->wrap ? (tex_Compress_Ok|tex_Wrap) : tex_Compress_Ok);
			int tex_x, tex_y;
			mTexMgr->GetTexInfo(tref, &tex_x, &tex_y, NULL, NULL, NULL, NULL);
			float tex_aspect = float(pol->proj_s * tex_x) / float(pol->proj_t * tex_y);
			float ds = prev_space / mZoom * (tex_aspect > 1.0 ? 1.0 : tex_aspect);
			float dt = prev_space / mZoom * (tex_aspect > 1.0 ? 1.0/tex_aspect : 1.0);

			float x1 = 0.5 *(b[2] + b[0] - ds);         // texture left bottom corner
			float y1 = 0.5* (b[3] + b[1] - dt);

			Point2 st = Point2((x-x1)/ds, (y-y1)/dt);   // texture coodinates where we clicked at

			// go through list of subtexture boxes and find if we clicked inside one
			static int lastBox = -1;                // the box we clicked on the last time. Helps to cycle trough overlapping boxes
			int        firstBox = 999;              // the first box that fits this click location
			int n;
			for (n=0; n < pol->mSubBoxes.size(); ++n)
			{
				if (pol->mSubBoxes[n].contains(st))
				{
					if (n < firstBox) firstBox = n; // memorize the first of all boxes that fits the click
					if (n > lastBox)                // is it a new-to-us box ?
					{
						mResMgr->SetPolUV(mRes,pol->mSubBoxes[n]);
						lastBox=n;
						break;
					}
				}
			}

			if (n >= pol->mSubBoxes.size())         // apparently there is no new-to-us box here
			{
				if (firstBox < 999)
				{
					mResMgr->SetPolUV(mRes,pol->mSubBoxes[firstBox]);    // so we go with the first best box we found
					lastBox=firstBox;
				}
				else
				{
					mResMgr->SetPolUV(mRes,Bbox2(0,0,1,1));   // there is no box where we clicked -> select whole texture
					lastBox = -1;
				}
			}
		}
		else
			mResMgr->SetPolUV(mRes,Bbox2());                 // there are no subboxes defined at all

		Refresh();
	}
	return 1;
}

void	WED_LibraryPreviewPane::MouseDrag(int x, int y, int button)
{
	float dx = x - mX;
	float dy = y - mY;
	if((mType == res_Facade || mType == res_Object || mType == res_Road || mType == res_Autogen) && button == 1)
	{
		mHgt = mHgtOrig + (fabs(dy) < 100.0 ? dy * 0.1 : sign(dy)*(fabs(dy)-80) * 0.5);
		mHgt = intlim(mHgt,0,250);

		if	(mType == res_Autogen)
		{
			mWid = mWidOrig + dx * 0.2;
			mWid = fltlim(mWid,0,250);
		}
		else
		{
			mWid = mWidOrig + (fabs(dx) < 100.0 ? dx * 0.2 : sign(dx)*(fabs(dx)-60.0) * 0.5);
			mWid = fltlim(mWid,1,150);
		}
	}
	else
	{
		mPsi = mPsiOrig + dx * 0.5;
		mThe = mTheOrig - dy * 0.5;

		mThe = fltlim(mThe,-85,85);                 // prevent some vertical objects fading completely out of view
		if (mType == res_Facade)
			mPsi = fltwrap(mPsi, 0, 90*mWalls);      // adjusted for propper annotations and showing mmore than 4 walls
		else
			mPsi = fltwrap(mPsi,-180,180);
	}
	Refresh();
}

int		WED_LibraryPreviewPane::MouseMove(int x, int y)
{
	int b[4]; GetBounds(b);
	if(b[2] - b[0] > 0 && b[2] - b[0] < 100)
	{
		DispatchHandleCommand(wed_autoOpenLibPane);
	}
	return 1;
}

GUI_DragOperation   WED_LibraryPreviewPane::DragEnter(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	return allowed;
}

GUI_DragOperation   WED_LibraryPreviewPane::DragOver(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	return allowed;
}

GUI_DragOperation   WED_LibraryPreviewPane::Drop(int x, int y, GUI_DragData * drag, GUI_DragOperation allowed, GUI_DragOperation recommended)
{
	#if DEV
	printf(" WED_LibraryPreviewPane::Drop drag:%p allowed:%d recommend:%d\n",drag,allowed,recommended);
	#endif
	return gui_Drag_None;
}


#define VIEW_DISTANCE 2.5   // set to 0 for isometric, non-perspective view used before WED 2.1
#define USE_2X2MSAA 1

void	WED_LibraryPreviewPane::begin3d(const int *b, double radius_m)
{
	int dx = b[2] - b[0];
	int dy = b[3] - b[1];

	double sx = ((dx > dy) ? ((double) dx / (double) dy) : 1.0) * 0.5;
	double sy = ((dx > dy) ? 1.0 : ((double) dy / (double) dx)) * 0.5;

	double act_radius = radius_m * mZoom;

	glPushAttrib(GL_VIEWPORT_BIT);

	GLfloat light_pos[4] = { -1, 1, 1, 0};          // x right, y up, z front
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glEnable(GL_LIGHT0);

	GLfloat ambient_color[4] = { 2, 2, 2, 2 };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_color);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, false);
	glEnable(GL_LIGHTING);
#if USE_2X2MSAA
//	if (GetModifiersNow() & gui_ShiftFlag) mMSAA = 0; else mMSAA = 1;
	if(mMSAA)
	{
		glGenFramebuffers(1, &mFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, mFBO);          CHECK_GL_ERR

		glGenRenderbuffers(1, &mColBuf);                  CHECK_GL_ERR
		glBindRenderbuffer(GL_RENDERBUFFER, mColBuf);     CHECK_GL_ERR
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB, dx, dy); CHECK_GL_ERR
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mColBuf); CHECK_GL_ERR

		glGenRenderbuffers(1, &mDepthBuf);                CHECK_GL_ERR
		glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuf);   CHECK_GL_ERR
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, dx, dy); CHECK_GL_ERR
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuf); CHECK_GL_ERR

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
		{
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFBO); CHECK_GL_ERR // copy the background - since we dont use any
	                                                                   // blend mode when Bliting buffer back at the end
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBlitFramebuffer(b[0], b[1], b[2], b[3], 0, 0, dx, dy, GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST); CHECK_GL_ERR
			glBindFramebuffer(GL_FRAMEBUFFER, mFBO);      CHECK_GL_ERR
			glViewport(0, 0, dx, dy);                     CHECK_GL_ERR

		}
		else
		{
			LOG_MSG("E/Lpp FBO incomplete %d %d %s %s\n", mColBuf, mDepthBuf, gluErrorString(glGetError()), gluErrorString(glGetError()));
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &mFBO);
			glDeleteRenderbuffers(1, &mColBuf);
			glDeleteRenderbuffers(1, &mDepthBuf);
			mMSAA = 0;
			glViewport(b[0], b[1], dx, dy);
		}
	}
	else
#endif
	glViewport(b[0], b[1], dx, dy);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
#ifdef VIEW_DISTANCE
	glFrustum(sx * -act_radius, sx * act_radius, sy * -act_radius, sy * act_radius,
					(VIEW_DISTANCE - 1.0) * radius_m, 2*(VIEW_DISTANCE + 1.0) * radius_m);
#else
	glOrtho(sx * -act_radius, sx * act_radius, sy * -act_radius, sy * act_radius, -radius_m, radius_m);
#endif
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
#ifdef VIEW_DISTANCE
	glTranslatef(0,0.0, -VIEW_DISTANCE * radius_m);
#endif
	glRotatef(mThe,1,0,0);
	glRotatef(mPsi,0,1,0);
}

void	WED_LibraryPreviewPane::end3d(const int *b)
{
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

#if USE_2X2MSAA
	glPopAttrib();
	if(mMSAA)
	{
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);      CHECK_GL_ERR
		glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);   CHECK_GL_ERR
		glDrawBuffer(GL_BACK);                          CHECK_GL_ERR
		int dx = b[2] - b[0];
		int dy = b[3] - b[1];
		glBlitFramebuffer(0, 0, dx, dy, b[0], b[1], b[2], b[3], GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT, GL_NEAREST);  CHECK_GL_ERR
		glBindFramebuffer(GL_FRAMEBUFFER, 0);  			CHECK_GL_ERR
		glDeleteFramebuffers(1, &mFBO);
		glDeleteRenderbuffers(1, &mColBuf);
		glDeleteRenderbuffers(1, &mDepthBuf);
	}
	glDisable(GL_LIGHTING);
#endif

}

void	WED_LibraryPreviewPane::Draw(GUI_GraphState * g)
{
		int b[4]; GetBounds(b);
		if(mType == res_Directory && mRess.size())
		{
			int w = b[2] - b[0];
			int h = b[3] - b[1];

			int num_x = best_num_cols(mRess.size(), w, h);
			int num_y = best_num_rows(mRess.size(), num_x);

			int x = 0;
			int y = num_y - 1;
			for(auto& r : mRess)
			{
				int bb[4];
				bb[0] = b[0] + w / num_x * x;
				bb[2] = b[0] + w / num_x * (x+1);
				bb[1] = b[1] + h / num_y * y;
				bb[3] = b[1] + h / num_y * (y+1);

				DrawOneItem(r.second, r.first, bb, g, r.first.c_str()+r.first.find_last_of('/') + 1);
				x++;
				if(x >= num_x)
				{
					x = 0;
					y--;
				}
			}
		}
		else
			DrawOneItem(mType, mRes, b, g);
}

void	WED_LibraryPreviewPane::DrawOneItem(int type, const string& res, const int b[4], GUI_GraphState * g, const char * label)
{
	const XObj8 * o = nullptr;
	const agp_t * agp = nullptr;
	const pol_info_t * pol = nullptr;
	const lin_info_t * lin = nullptr;
	const fac_info_t * fac = nullptr;
	const str_info_t * str = nullptr;
	const road_info_t * rd = nullptr;
	map<int,road_info_t::vroad_t>::const_iterator vr_it;

	if(!res.empty())
	{	switch(type) {
		case res_Polygon:
			if(mResMgr->GetPol(res,pol))
			{
				TexRef	tref = mTexMgr->LookupTexture(pol->base_tex.c_str(),true, pol->wrap ? (tex_Compress_Ok|tex_Wrap) : tex_Compress_Ok);
				if(tref != NULL)
				{
					int tex_x, tex_y;
					mTexMgr->GetTexInfo(tref, &tex_x, &tex_y, NULL, NULL, NULL, NULL);
					float tex_aspect = float(pol->proj_s * tex_x) / float(pol->proj_t * tex_y);
					float Ds = tex_aspect > 1.0 ? 1.0 : tex_aspect;
					float Dt = tex_aspect > 1.0 ? 1.0/tex_aspect : 1.0;

					int tex_id = mTexMgr->GetTexID(tref);

					if (tex_id != 0)
					{
						g->SetState(false,1,false,!pol->kill_alpha,!pol->kill_alpha,false,false);
						g->BindTex(tex_id,0);

						float prev_space = min(b[2]-b[0],b[3]-b[1]);
						float ds = prev_space / mZoom * Ds;
						float dt = prev_space / mZoom * Dt;
						float dx = b[2] - b[0];
						float dy = b[3] - b[1];
						float x1 = (dx - ds) /2;
						float x2 = (dx + ds) /2;
						float y1 = (dy - dt) /2;
						float y2 = (dy + dt) /2;

						glBegin(GL_QUADS);
						if(pol->wrap)
						{
							glTexCoord2f(extrap(x1,0,x2,1,b[0]), extrap(y1,0,y2,1,b[1]));	glVertex2f(b[0],b[1]);
							glTexCoord2f(extrap(x1,0,x2,1,b[0]), extrap(y1,0,y2,1,b[3]));	glVertex2f(b[0],b[3]);
							glTexCoord2f(extrap(x1,0,x2,1,b[2]), extrap(y1,0,y2,1,b[3]));	glVertex2f(b[2],b[3]);
							glTexCoord2f(extrap(x1,0,x2,1,b[2]), extrap(y1,0,y2,1,b[1]));	glVertex2f(b[2],b[1]);
						}
						else
						{
							x1 += b[0]; x2 += b[0];
							y1 += b[1]; y2 += b[1];
							glTexCoord2f(0,0); glVertex2f(x1,y1);
							glTexCoord2f(0,1); glVertex2f(x1,y2);
							glTexCoord2f(1,1); glVertex2f(x2,y2);
							glTexCoord2f(1,0); glVertex2f(x2,y1);
						}
						glEnd();

						if (!pol->mUVBox.is_null())                   // draw a box around the selected texture area
						{
							g->Reset();
							glColor4fv(WED_Color_RGBA(wed_StructureSelected));
							glBegin(GL_LINE_LOOP);
							glVertex2f(x1 + ds * pol->mUVBox.p1.x(), y1 + dt * pol->mUVBox.p1.y());
							glVertex2f(x1 + ds * pol->mUVBox.p2.x(), y1 + dt * pol->mUVBox.p1.y());
							glVertex2f(x1 + ds * pol->mUVBox.p2.x(), y1 + dt * pol->mUVBox.p2.y());
							glVertex2f(x1 + ds * pol->mUVBox.p1.x(), y1 + dt * pol->mUVBox.p2.y());
							glEnd();
						}
					}
				}
			}
			break;
		case res_Line:
			if(mResMgr->GetLin(res,lin))
			{
				TexRef	tref = mTexMgr->LookupTexture(lin->base_tex.c_str(),true, tex_Wrap|tex_Compress_Ok);
				if(tref != NULL)
				{
					int tex_id = mTexMgr->GetTexID(tref);
					if (tex_id != 0)
					{
						g->SetState(false,1,false,true,true,false,false);
						g->BindTex(tex_id,0);

						// always fit into vertical size of window
						float win_scale = (b[3] - b[1]) / mZoom;
						float center_x = (b[2] + b[0]) * 0.5f;
						float y1 = b[1] + (label != nullptr ? 15 : 0);

						glBegin(GL_QUADS);
						for (int n = 0; n < lin->s1.size(); n++)
//						for (int n = lin->s1.size() - 1; n >= 0; n--)
						{
							float left  = (lin->s1[n] - lin->sm[n]) * lin->scale_s / lin->scale_t;
							float right = (lin->s2[n] - lin->sm[n]) * lin->scale_s / lin->scale_t;

							float x1 = center_x + left * win_scale;
							float x2 = center_x + right * win_scale;

							glTexCoord2f(lin->s1[n], 0);     glVertex2f(x1, y1);
							glTexCoord2f(lin->s1[n], mZoom); glVertex2f(x1, b[3]);
							glTexCoord2f(lin->s2[n], mZoom); glVertex2f(x2, b[3]);
							glTexCoord2f(lin->s2[n], 0);     glVertex2f(x2, y1);
						}
						glEnd();
					}
				}
			}
			break;
		case res_Facade:
			if (mResMgr->GetFac(res, fac, mVariant))
			{
				Polygon2 footprint;
				vector<int> choices;
				Vector2 dir(mWid,0);
				Point2 corner(-mWid*0.5, mWid*0.5);

				int front_side = intround(mPsi/90) % mWalls;

				int n_max = 4;
				if(!fac->is_ring && fac->wallName.size() >= 4) n_max=5;
				for (int n = 0; n < n_max; ++n)
				{
					float thisWall_hdg              = fltwrap(mPsi + n*90,-180,180);
					int 	thisWall_idx_rel_to_front = -intround(thisWall_hdg/90);
					int   thisWall_idx = (front_side + thisWall_idx_rel_to_front + mWalls) % mWalls;
					if (thisWall_idx < 0 || thisWall_idx >= fac->wallName.size())
						thisWall_idx = 0;
					choices.push_back(thisWall_idx);
					footprint.push_back(corner);
					corner += n<3 ? dir : dir *0.8;  // open type facade need last point duplicated, even if drawn as closed loop
					dir = dir.perpendicular_cw();
				}
				double real_radius = 1.2 * fltmax3(30.0, mWid, 1.2*mHgt);
				begin3d(b, real_radius);

				glTranslatef(0.0, -mHgt*0.4, 0.0);
				g->EnableAlpha(true, true);
				g->EnableDepth(true, true);
				glClear(GL_DEPTH_BUFFER_BIT);

				draw_facade(mTexMgr, mResMgr, mRes, *fac, footprint, choices, mHgt, g, true);

				// draw "ground" plane
				g->SetTexUnits(0);
				if(mRes.find("piers") != mRes.npos)
					glColor4f(0.0, 0.2, 0.4, 0.7);   // darkish blue water
				else
					glColor4f(0.2, 0.4, 0.2, 0.7);   // green lawn, almost opaque

				g->EnableLighting(false);
				glDisable(GL_CULL_FACE);
				glBegin(GL_POLYGON);
				for (auto f : footprint)
					glVertex3f(f.x() + 2.0 * (f.x() > 0.0 ? mWid : -mWid), -0.01,
								  f.y() + 2.0 * (f.y() > 0.0 ? mWid : -mWid) );
				glEnd();
				glColor4f(1,1,1,1);

				end3d(b);
			}
			break;
		case res_Forest:
			if(!mResMgr->GetFor(res,o))
				break;
		case res_String:
			if(!o)
			{
				if(mResMgr->GetStr(res,str))
					if(str->objs.size())
						mResMgr->GetObjRelative(str->objs.front(), res, o);    // do the cheap thing: show only the first object. Could show a whole line ...
			}
		case res_Autogen:
		case res_Object:
			g->SetState(false,1,false,true,true,true,true);
			glClear(GL_DEPTH_BUFFER_BIT);

			if (o || mResMgr->GetObj(res,o,mVariant))
			{
				double real_radius = fltmax3(
									o->xyz_max[0]-o->xyz_min[0],
									o->xyz_max[1]-o->xyz_min[1],
									o->xyz_max[2]-o->xyz_min[2]);
				double xyz_off[3] = { -(o->xyz_max[0]+o->xyz_min[0])*0.5,
				                      -(o->xyz_max[1]+o->xyz_min[1])*0.5,
				                      -(o->xyz_max[2]+o->xyz_min[2])*0.5 };

				begin3d(b, real_radius);
				draw_obj_at_xyz(mTexMgr, o, xyz_off[0], xyz_off[1], xyz_off[2],	0, g);
				end3d(b);
			}
			else if (mResMgr->GetAGP(res,agp))
			{
				double real_radius=pythag(                 // not ideal - only scaling from first tile, Tiles down throw might be much larger
									agp->tiles[0].xyz_max[0] - agp->tiles[0].xyz_min[0],
									agp->tiles[0].xyz_max[1] - agp->tiles[0].xyz_min[1],
									agp->tiles[0].xyz_max[2] - agp->tiles[0].xyz_min[2]);
				double xyz_off[3];
				xyz_off[1] = -(agp->tiles[0].xyz_max[1] + agp->tiles[0].xyz_min[1]) * 0.5;

				for(int i = 0; i < intmin2(5, agp->tiles.size()); i++)
				{
					int tile_idx = intlim(i + mWid * 0.2, 0, agp->tiles.size()-1);
					auto ti = agp->tiles[tile_idx];
					xyz_off[0] = -(ti.xyz_max[0] + ti.xyz_min[0]) * 0.5;
				//	xyz_off[1] = -(ti.xyz_max[1] + ti.xyz_min[1]) * 0.5;
					xyz_off[2] =  (ti.xyz_max[2] + ti.xyz_min[2]) * 0.5;
					double row_offs = agp->tiles.size() > 1 ? (ti.xyz_max[0] - ti.xyz_min[0]) * (i-0.8) : 0.0;

					begin3d(b, real_radius);
					draw_agp_at_xyz(mTexMgr, agp, xyz_off[0] + row_offs, xyz_off[1], xyz_off[2], mHgt, 0, g, tile_idx);
					end3d(b);
				}
			}
			break;

		case res_Road:
			if(mResMgr->GetRoad(res,rd))
			{
				int i = intlim(mWid/2,0,rd->vroad_types.size()-1);
				//i=0;
				vr_it = rd->vroad_types.begin();
				for(int j = 0; j < i; j++) vr_it++;
				int rd_idx = vr_it->second.rd_type;

				if(!rd->road_types.count(rd_idx)) break;
				auto& t = rd->road_types.at(rd_idx);

				const float length = 50.0;
				begin3d(b, length);
				g->SetState(false, 1, false, true, true, true, true);
				glClear(GL_DEPTH_BUFFER_BIT);
				if(rd->textures.size() > t.tex_idx)
				if(auto tref = mTexMgr->LookupTexture(rd->textures[t.tex_idx].c_str(), true, tex_Wrap+tex_Mipmap+tex_Linear))
				{
					if(auto tex_id = mTexMgr->GetTexID(tref))
					{
						g->BindTex(tex_id,0);
						glDisable(GL_CULL_FACE);
						float v = 4.0 * length / t.length;
						for(auto s : t.segs)
						{
							glBegin(GL_POLYGON);
								glTexCoord2f(s.s_left,  0); glVertex3f(s.left,  0,  length * 2.0f);
								glTexCoord2f(s.s_left,  v); glVertex3f(s.left,  0, -length * 2.0f);
								glTexCoord2f(s.s_right, v); glVertex3f(s.right, 0, -length * 2.0f);
								glTexCoord2f(s.s_right, 0); glVertex3f(s.right, 0,  length * 2.0f);
							glEnd();
						}
						glEnable(GL_CULL_FACE);
					}
				}
				if(t.vert_objs.size())
				{
					const float front_twr = length * 0.7;
					const float back_twr = -length * 0.7;
					if(mResMgr->GetObjRelative(t.vert_objs.back().path, res, o))
					{
						draw_obj_at_xyz(mTexMgr, o, t.vert_objs.back().lat_offs, 0, front_twr, t.vert_objs.back().rotation, g);
						draw_obj_at_xyz(mTexMgr, o, t.vert_objs.back().lat_offs, 0, back_twr,  t.vert_objs.back().rotation, g);
					}
					for(auto w : t.wires)
					{
						g->SetState(false,0,false,true,true,false,false);
						glColor3f(0.1,0.1,0.1);
						glBegin(GL_LINE_STRIP);
							const float span = front_twr - back_twr;
							const int steps = 20;
							float pos = front_twr;
							float rel_d = 1.0;
							for(int i = 0; i <= steps; i++)
							{
								glVertex3f(w.lat_offs, w.end_height * (1.0f - (1.0f - rel_d*rel_d) * w.droop), pos);
								rel_d -= 2.0 / steps;
								pos -= span / steps;
							}
						glEnd();
					}
				}
				if(t.dist_objs.size())
				{
					float long_pos = length * 0.7;   // there isn't much info on the spacing and frequency parameters for newer commands.
					for(auto d : t.dist_objs)        // so we print each object once with uniform spacing.
					{
						if(mResMgr->GetObjRelative(d.path, res, o))
						{
							draw_obj_at_xyz(mTexMgr, o, d.lat_offs, 0, long_pos, d.rotation, g);
							long_pos -= length * 1.4 / t.dist_objs.size();
						}
					}
				}
				end3d(b);
			}
			break;
		}

		if (label)
			GUI_FontDraw(g, font_UI_Small, WED_Color_RGBA(wed_pure_white), (b[2]+b[0])/2,b[1]+5, label, align_Center);
		else
		{
			// plot some additional information about the previewed object
			char buf1[120] = "", buf2[120] = "";
			switch(type)
			{
			case res_Facade:
				if(fac && fac->wallName.size())
				{
					int n_wall = fac->wallName.size();
					int raw_side = intround(mPsi/90) % mWalls;
					int front_side = raw_side;
					if(front_side < 0 || front_side >= n_wall) front_side = 0;

					snprintf(buf1, sizeof(buf1), "Wall \'%s\' intended for %s @ w=%.1lf%s", fac->wallName[front_side].c_str(), fac->wallUse[front_side].c_str(),
						length_with_units(mWid));
					snprintf(buf2, sizeof(buf2), "Type %d, %d wall%s for %s @ h=%dm", fac->is_new ? 2 : 1, n_wall, n_wall > 1 ? "s" : "", fac->h_range.c_str(), mHgt);
				}
				else
					sprintf(buf2, "No preview for this facade available");
				break;
			case res_Polygon:
				if(pol)
					snprintf(buf1, sizeof(buf1), "%s %s", pol->description.c_str(), pol->hasDecal ? "(decal not shown)" : "");
				if (pol && pol->mSubBoxes.size())
					sprintf(buf2, "Select desired part of texture by clicking on it");
				break;
			case res_Line:
				if(lin)
					snprintf(buf1, sizeof(buf1), "%s %s", lin->description.c_str(), lin->hasDecal ? "(decal not shown)" : "");
				if (lin && lin->s1.size() && lin->s2.size())
					snprintf(buf2, sizeof(buf2), "w~%.0f%s",lin->eff_width * (gIsFeet ? 100.0/2.54 : 100.0), gIsFeet ? "in" : "cm" );
				break;
			case res_Autogen:
			case res_Object:
			case res_Forest:
			case res_String:
				if (o)
				{
					snprintf(buf1, sizeof(buf1), "%s", o->description.c_str());
					int n = sprintf(buf2, "max h=%.1f%s", length_with_units(o->xyz_max[1]));
					if (o->xyz_min[1] < -0.07)
						n += sprintf(buf2 + n, ", below ground to %.1f%s", length_with_units(o->xyz_min[1]));
				}
				else if(agp)
				{
					int n = 0;
					int tile_idx = intlim(mWid * 0.2, 0, agp->tiles.size()-1);
					if(agp->tiles.size() > 1)
					{
						if(agp->tiles[tile_idx].id >= 0)
							n = sprintf(buf1, "TILE_ID \'%d\' ", agp->tiles[tile_idx].id);
						sprintf(buf1 + n, "%sshowing tiles #%d+ @ h=%dm", agp->tiles[tile_idx].has_scp ? "has scrapers, " : "", tile_idx, mHgt);
						n = sprintf(buf2, "%d tiles, ", (int) agp->tiles.size());
					}
					else
						snprintf(buf1, sizeof(buf1), "%s", agp->description.c_str());

					n += sprintf(buf2 + n, "max h=%.1f%s", length_with_units(agp->tiles[tile_idx].xyz_max[1]));
					if(agp->has_scp)
					{
						double min_scp(999), max_scp(0);
						for (auto& t : agp->tiles)
							if(t.has_scp)
								for (auto& a : t.objs)
									if(a.scp_step > 0.0)
									{
										if(a.scp_min < min_scp) min_scp = a.scp_min;
										if(a.scp_max > max_scp) max_scp = a.scp_max;
									}
						sprintf(buf2 + n, ", varies for heights %.1f - %.1f%s", min_scp / (gIsFeet ? 0.3048 : 1.0), length_with_units(max_scp));
					}
				}
				else if (str)
					snprintf(buf1, sizeof(buf1), "%s", str->description.c_str());
				break;
			case res_Road:
				if(rd)
				{
					int n = snprintf(buf1, sizeof(buf1), "Road #%d \'%s\'", vr_it->second.rd_type, vr_it->second.description.c_str());
					if(rd->road_types.count(vr_it->second.rd_type))
					{
						auto& r = rd->road_types.at(vr_it->second.rd_type);
						if(r.wires.size())
							snprintf(buf1+n, sizeof(buf1)-n, " h=%.0f%s", length_with_units(r.wires.front().end_height)); // assuming first wire is near top
						else
							snprintf(buf1+n, sizeof(buf1)-n, " w=%.0f%s", length_with_units(r.width));
					}
					snprintf(buf2, sizeof(buf2), "Total %d vroad, %d road types", (int) rd->vroad_types.size(), (int) rd->road_types.size());
				}
				break;
			}

			if (buf1[0])
				GUI_FontDraw(g, font_UI_Basic, WED_Color_RGBA(wed_pure_white), b[0]+5,b[1]+20, buf1);
			if (buf2[0])
				GUI_FontDraw(g, font_UI_Basic, WED_Color_RGBA(wed_pure_white), b[0]+5,b[1]+5, buf2);
		}
	}
}
