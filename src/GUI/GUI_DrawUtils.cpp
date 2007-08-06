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

#include "GUI_DrawUtils.h"
#include "GUI_GraphState.h"
#include "TexUtils.h"
#include "GUI_Resources.h"
#if APL
	#include <OpenGL/gl.h>
	#include <OpenGL/glu.h>
#else
	#include <GL/gl.h>
	#include <GL/glu.h>
#endif

// for ui - no mipmap, no linear interp, no magenta filter, no wrapping
#define	UI_TEX_FLAGS	0

void	GUI_PositionRect(
				int							in_bounds[4],
				int							io_rect[4],
				int							just_h,
				int							just_v)
{
	int rect_h = io_rect[2] - io_rect[0];
	int rect_v = io_rect[3] - io_rect[1];
	
	switch(just_h) {
	case -1:
		io_rect[0] = in_bounds[0];
		io_rect[2] = io_rect[0] + rect_h;
		break;
	case 0:
		io_rect[0] = (in_bounds[0] + in_bounds[2] - rect_h) / 2;
		io_rect[2] = io_rect[0] + rect_h;		
		break;
	case 1:
		io_rect[2] = in_bounds[2];
		io_rect[0] = io_rect[2] - rect_h;
		break;
	}
	
	switch(just_v) {
	case -1:
		io_rect[1] = in_bounds[1];
		io_rect[3] = io_rect[1] + rect_v;
		break;
	case 0:
		io_rect[1] = (in_bounds[1] + in_bounds[3] - rect_v) / 2;
		io_rect[3] = io_rect[1] + rect_v;		
		break;
	case 1:
		io_rect[3] = in_bounds[3];
		io_rect[1] = io_rect[3] - rect_v;
		break;
	}
}

void	GUI_TileToST(
				int							tile_sel[4],
				float						tex_st[4],
				GUI_TexPosition_t *			metrics)	

{
	tex_st[0] = (float) (tile_sel[0]  ) / (float) tile_sel[2];
	tex_st[2] = (float) (tile_sel[0]+1) / (float) tile_sel[2];
	tex_st[1] = (float) (tile_sel[1]  ) / (float) tile_sel[3];
	tex_st[3] = (float) (tile_sel[1]+1) / (float) tile_sel[3];
	if (metrics)	tex_st[0] *= metrics->s_rescale;
	if (metrics)	tex_st[1] *= metrics->t_rescale;
	if (metrics)	tex_st[2] *= metrics->s_rescale;
	if (metrics)	tex_st[3] *= metrics->t_rescale;
}				

inline void gl_quad(int l, int b, int r, int t, float s1, float t1, float s2, float t2)
{
	glTexCoord2f(s1,t1);	glVertex2i(l,b);
	glTexCoord2f(s1,t2);	glVertex2i(l,t);
	glTexCoord2f(s2,t2);	glVertex2i(r,t);
	glTexCoord2f(s2,t1);	glVertex2i(r,b);
}

void	GUI_DrawCentered(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							just_h,
				int							just_v,
				int							tile_sel[4],
				int *						out_width,
				int *						out_height)
{
	GUI_TexPosition_t metrics;	
	int tex_id = GUI_GetTextureResource(in_resource, UI_TEX_FLAGS, &metrics);
	
	int	image_size[4] = { 
		(float) tile_sel[0]    * (float)metrics.real_width  / (float)tile_sel[2],
		(float) tile_sel[1]    * (float)metrics.real_height / (float)tile_sel[3],
		(float)(tile_sel[0]+1) * (float)metrics.real_width  / (float)tile_sel[2],
		(float)(tile_sel[1]+1) * (float)metrics.real_height / (float)tile_sel[3] };

	GUI_PositionRect(bounds, image_size, just_h, just_v);
	
	if ((metrics.real_width  / tile_sel[2]) % 2)		image_size[0]++, image_size[2]++;
	if ((metrics.real_height / tile_sel[3]) % 2)		image_size[1]++, image_size[3]++;
	
	state->SetState(0, 1, 0, 1, 1, 0, 0);
	state->BindTex(tex_id, 0);
	
	float	sts[4] = { 0.0, 0.0, metrics.s_rescale, metrics.t_rescale };
	if (tile_sel) GUI_TileToST(tile_sel, sts, &metrics);
	
	glBegin(GL_QUADS);	
	gl_quad(image_size[0],image_size[1],image_size[2],image_size[3],
					sts[0],sts[1],sts[2],sts[3]);
	glEnd();
	
	if (out_width)	*out_width = metrics.real_width;
	if (out_height)	*out_height = metrics.real_height;
}
				
void	GUI_DrawStretched(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							tile_sel[4])
{
	GUI_TexPosition_t metrics;
	
	int tex_id = GUI_GetTextureResource(in_resource, UI_TEX_FLAGS, &metrics);
	
	state->SetState(0, 1, 0, 1, 1, 0, 0);
	state->BindTex(tex_id, 0);
	
	float	sts[4] = { 0.0, 0.0, metrics.s_rescale, metrics.t_rescale };
	if (tile_sel) GUI_TileToST(tile_sel, sts, &metrics);
	
	glBegin(GL_QUADS);
	
	gl_quad(bounds[0],bounds[1],bounds[2],bounds[3],
					sts[0],sts[1],sts[2],sts[3]);
	glEnd();
	
}

void	GUI_DrawHorizontalStretch(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							tile_sel[4])
{
	GUI_TexPosition_t metrics;
	int tex_id = GUI_GetTextureResource(in_resource, UI_TEX_FLAGS, &metrics);
	
	int centered[4] = { bounds[0], 0, bounds[2], metrics.real_height / tile_sel[3]};
	GUI_PositionRect(bounds, centered, 0, 0);
	
	if ((centered[3] - centered[1]) % 2)		centered[1]++, centered[3]++;	
	
	int p1 = bounds[0] + metrics.real_width * 0.33 / tile_sel[2];
	int p2 = bounds[2] - metrics.real_width * 0.33 / tile_sel[2];
	
	int b1[4] = { bounds[0], centered[1], p1,		 centered[3] };
	int b2[4] = { p1,		 centered[1], p2,		 centered[3] };
	int b3[4] = { p2,		 centered[1], bounds[2], centered[3] };
	
	int t1[4] = { tile_sel[0] * 3    , tile_sel[1], tile_sel[2] * 3, tile_sel[3] };
	int t2[4] = { tile_sel[0] * 3 + 1, tile_sel[1], tile_sel[2] * 3, tile_sel[3] };
	int t3[4] = { tile_sel[0] * 3 + 2, tile_sel[1], tile_sel[2] * 3, tile_sel[3] };
	
	GUI_DrawStretched(state, in_resource, b1, t1);
	GUI_DrawStretched(state, in_resource, b2, t2);
	GUI_DrawStretched(state, in_resource, b3, t3);	
}

void	GUI_DrawVerticalStretch(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							tile_sel[4])
{
	GUI_TexPosition_t metrics;
	int tex_id = GUI_GetTextureResource(in_resource, UI_TEX_FLAGS, &metrics);
	
	int centered[4] = { 0, bounds[1], metrics.real_width / tile_sel[2], bounds[3] };
	GUI_PositionRect(bounds, centered, 0, 0);
	
	if ((centered[2] - centered[0]) % 2)		centered[0]++, centered[2]++;	
	
	int p1 = bounds[1] + metrics.real_height * 0.33 / tile_sel[3];
	int p2 = bounds[3] - metrics.real_height * 0.33 / tile_sel[3];
	
	int b1[4] = { centered[0], bounds[1],	centered[2], p1		   };
	int b2[4] = { centered[0], p1,			centered[2], p2		   };
	int b3[4] = { centered[0], p2,			centered[2], bounds[3] };
	
	int t1[4] = { tile_sel[0], tile_sel[1] * 3   , tile_sel[2], tile_sel[3] * 3 };
	int t2[4] = { tile_sel[0], tile_sel[1] * 3 +1, tile_sel[2], tile_sel[3] * 3 };
	int t3[4] = { tile_sel[0], tile_sel[1] * 3 +2, tile_sel[2], tile_sel[3] * 3 };

	GUI_DrawStretched(state, in_resource, b1, t1);
	GUI_DrawStretched(state, in_resource, b2, t2);
	GUI_DrawStretched(state, in_resource, b3, t3);	
}




void	GUI_PlotIcon(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							x,
				int							y,
				float						angle,
				float						scale)
{
	int bounds[4] = { x - 10, y - 10, x + 10, y + 10 };
	
	int tile_sel[4] = { 0, 0, 1, 1 };
	
	if (angle != 0.0 || scale != 1.0)
	{
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(x,y,0);
		if (angle != 0.0)		glRotatef	(angle,0,0,-1);	// cw rotate
		if (scale != 1.0) 		glScalef(scale,scale,scale);
		glTranslatef(-x,-y,0);
	}
	
	GUI_DrawCentered(state, in_resource, bounds, 0, 0,  tile_sel, NULL, NULL);
	if (angle != 0.0 || scale != 1.0)
		glPopMatrix();
}

void	GUI_PlotIconBulk(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							count,
				int *						x,
				int	*						y,
				float *						c,
				float						scale)
{
	GUI_TexPosition_t metrics;	
	int tex_id = GUI_GetTextureResource(in_resource, UI_TEX_FLAGS, &metrics);
	
	int width = (scale == 1.0f) ? metrics.real_width : scale * (float) metrics.real_width;
	int height = (scale == 1.0f) ? metrics.real_height : scale * (float) metrics.real_height;
	
	int width1 = width / 2;
	int width2 = width - width1;
	
	int height1 = height / 2;
	int height2 = height - height1;
	
	state->SetState(0, 1, 0, 1, 1, 0, 0);
	state->BindTex(tex_id, 0);

	glBegin(GL_QUADS);	
	while(count--)
	{
		register int xx = *x++;
		register int yy = *y++;
		glColor4fv(c);
		c += 4;
		glTexCoord2f(0.0f,0.0f);				glVertex2i(xx - width1, yy - height1);
		glTexCoord2f(0.0f,1.0f);				glVertex2i(xx - width1, yy + height2);
		glTexCoord2f(1.0f,1.0f);				glVertex2i(xx + width2, yy + height2);
		glTexCoord2f(1.0f,0.0f);				glVertex2i(xx + width2, yy - height1);
	}
	glEnd();	
}

