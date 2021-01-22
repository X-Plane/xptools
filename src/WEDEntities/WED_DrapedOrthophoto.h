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

#ifndef WED_DrapedOrthophoto_H
#define WED_DrapedOrthophoto_H

#include "IHasResource.h"
#include "WED_GISPolygon.h"

/*
	WED_DrapedOrthophoto - THEORY OF OPERATION

	WED_DrapedOrthophoto is just a simple polygon - it represents the polygon component of a draped ortho photo which is essentially:

		WED_DrapedOrthoPhoto (is a polygon)			this defines its area
			WED_Ring (is a chain)							this is the outer contour
				WED_TextureBezierNode (is a bezier point)				these define the coordinates of the polygon boundary and map textures
				WED_TextureBezierNode (is a bezier point)				these define the coordinates of the polygon boundary and map textures
			WED_Ring (is a chain)							this is the optional first hole
				WED_TextureBezierNode (is a bezier point)				these define the coordinates of the polygon boundary and map textures
				WED_TextureBezierNode (is a bezier point)				these define the coordinates of the polygon boundary and map textures

*/

class	WED_DrapedOrthophoto : public WED_GISPolygon, public IHasResource {

DECLARE_PERSISTENT(WED_DrapedOrthophoto)

public:
			double		GetHeading(void) const;
			void		SetHeading(double h);

	virtual void			GetResource(	  string& r) const;
			const string&	GetResource() const;
	virtual void			SetResource(const string& r);

			//Checks if the draped orthophoto being used is the old .pol system or the new .someimagetype
			//True if new, false if old, optional way to get the suffix
			bool		IsNew( string* out_string=NULL);
	virtual const char *HumanReadableType(void) const { return "Draped Orthophoto"; }
	
			// Re-calculate UV mapping to stretch out texture over full extend of polygon
			void 		Redrape(bool updProp = 1);
			void		GetSubTexture(Bbox2& b);
			void		SetSubTexture(const Bbox2& b);

		virtual void	PropEditCallback(int before);

protected:

	virtual	bool		IsInteriorFilled(void) const { return true; }

private:

	WED_PropDoubleText        heading;
	WED_PropStringText        resource;

	// This specifies the part of a texture to be used for textureing.
	WED_PropDoubleText        top;
	WED_PropDoubleText        bottom;
	WED_PropDoubleText        left;
	WED_PropDoubleText        right;

};

#endif /* WED_DrapedOrthophoto_H */
