#ifndef GUI_DRAWUTILS_H
#define GUI_DRAWUTILS_H

class	GUI_GraphState;
struct	GUI_TexPosition_t;

void	GUI_PositionRect(
				int							in_bounds[4],
				int							io_rect[4],		// Move this within in_buobnds
				int							just_h,
				int							just_v);

void	GUI_TileToST(
				int							tile_sel[4],	// tile x, tile y, total tiles x, total tiles y
				float						tex_st[4],		// s1, t1, s2, t2
				GUI_TexPosition_t *			metrics);		// can be null

void	GUI_DrawCentered(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							just_h,
				int							just_v,
				int							tile_sel[4],	// tile x, tile y, total tiles x, total tiles y, can be null
				int *						out_width,		// can be null
				int *						out_height);

void	GUI_DrawStretched(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							tile_sel[4]);	// can be null

#endif