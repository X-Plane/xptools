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

#ifndef WED_FacadePlacement_H
#define WED_FacadePlacement_H

#include "WED_GISPolygon.h"
#include "IHasResource.h"

struct fac_info_t;

class	WED_FacadePlacement : public WED_GISPolygon, public IHasResource {

DECLARE_PERSISTENT(WED_FacadePlacement)

public:

	enum TopoMode {
		topo_Area = 0,
		topo_Ring = 1,
		topo_Chain = 2
	};

	virtual	bool			HasLayer		(GISLayer_t layer							  ) const;
	virtual Bbox3			GetVisibleBounds() const;

			double		GetHeight(void) const;
			void		SetHeight(double h);

	virtual void			GetResource(	  string& r) const;
			const string&	GetResource() const;
	virtual void			SetResource(const string& r);

			TopoMode	GetTopoMode(void) const;

			int		GetNumWallChoices(void) const;
			int		GetType(void) const;
			bool		HasCustomWalls(void) const;
			void		SetCustomWalls(bool has);

			void		SetShowLevel(int show_level);
			int			GetShowLevel(void) const;

	virtual const char *	HumanReadableType(void) const { return "Facade"; }

protected:

	virtual	bool		IsInteriorFilled(void) const { return GetTopoMode() == 0; }

private:

	const fac_info_t * 		GetFacInfo(void) const;

	WED_PropDoubleText		height;
	WED_PropStringText		resource;
	WED_PropBoolText			pick_walls;
	WED_PropIntEnum			show_level;

};


#endif /* WED_FacadePlacement_H */
