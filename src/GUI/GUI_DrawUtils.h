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

void	GUI_DrawHorizontalStretch(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							tile_sel[4]);

void	GUI_DrawVerticalStretch(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							tile_sel[4]);

void	GUI_DrawStretched(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							bounds[4],
				int							tile_sel[4]);	// can be null

void	GUI_PlotIcon(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							x,
				int							y,
				float						angle,
				float						scale);

void	GUI_PlotIconBulk(
				GUI_GraphState *			state,
				const char *				in_resource,
				int							count,
				int *						x,
				int	*						y,
				float *						c,	//	4 floats RGBA per icon
				float						scale);


#endif