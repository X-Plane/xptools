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
#ifndef OE_DRAWOBJ_H
#define OE_DRAWOBJ_H

struct	XObj;

enum	{

	cull_ShowVisible = 0,	// Show only polygons that are visible in x-plane
	cull_ShowAll,			// Show everything
	cull_ShowAllWarn,		// Show everything but draw hidden surfaces in red!

	poly_Wire = 0,			// Draw as a wire frame
	poly_Solid,				// Draw as a solid object
	poly_WireAndSolid,		// Draw solid but with wire frame

	tex_None = 0,			// No texturing
	tex_Day = 1,			// Show day textures
	tex_Lit = 2				// Show night texturse

};

void	OE_DrawObj(
				const XObj&		inObj,
				const set<int>&	inSel,
				int				inPolySolid,
				int				inCullMode,
				int				inTexMode);

void 	OE_LabelVertices(
				const XObj& 	inObj,
				const set<int>& inSelection,
				int				inCurrentVertex);

#endif
