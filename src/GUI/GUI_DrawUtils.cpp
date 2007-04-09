#include "GUI_DrawUtils.h"
#include "GUI_GraphState.h"
#include "TexUtils.h"
#include "GUI_Resources.h"
#if APL
	#include <OpenGL/gl.h>
#else
	#include <GL/gl.h>
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
	
	int	image_size[4] = { 0, 0, metrics.real_width, metrics.real_height };

	GUI_PositionRect(bounds, image_size, just_h, just_v);
	
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