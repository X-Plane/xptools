/*
 * Copyright (c) 2008, Laminar Research.
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

#ifndef WED_DrapedOrthophotoBoundary_H
#define WED_DrapedOrthophotoBoundary_H

/*
	WED_DrapedOrthophotoBoundary - THEORY OF OPERATION

	WED_DrapedOrthophotoBoundary is just a simple polygon - it represents the polygon component of a draped ortho photo which is essentially:

	WED_DrapedOrthophoto (is a composite)					this is the draped orthophoto
		WED_DrapedOrthoPhotoBoundary (is a polygon)			this defines its area
			WED_Ring (is a chain)							this is the outer contour
				WED_SimpleBoundaryNode (is a bezier point)	with 2 or more vertices
				WED_SimpleBoundaryNode (is a bezier point)	that are bezier points for a curved boundary
			WED_Ring (is a chain)							this is the optional first hole
				WED_SimpleBoundaryNode (is a bezier point)	with 2 or more vertices
				WED_SimpleBoundaryNode (is a bezier point)	that are bezier points for a curved boundary
		WED_OverlayImage (is a polygon, is a quad)			an overlay image inside defines the texture's extent
			WED_Ring (is a chain)							one ring defines the images bounds
				WED_TextureNode (is a point)				exactly four texture nodes specify the image bounds and contain ST coords.
				WED_TextureNode (is a point)
				WED_TextureNode (is a point)
				WED_TextureNode (is a point)
*/

#include "WED_GISComposite.h"

#include "WED_GISPolygon.h"

class	WED_DrapedOrthophotoBoundary : public WED_GISPolygon {

DECLARE_PERSISTENT(WED_DrapedOrthophotoBoundary)

};



#endif /* WED_DrapedOrthophotoBoundary_H */
