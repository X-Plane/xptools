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

#ifndef WED_OBJPLACEMENT_H
#define WED_OBJPLACEMENT_H

#include "IHasResource.h"
#include "WED_GISPoint_Heading.h"
#include "WED_PropertyHelper.h"


class	WED_ObjPlacement : public WED_GISPoint_Heading, public IHasResource {

DECLARE_PERSISTENT(WED_ObjPlacement)

public:

	virtual Bbox3		GetVisibleBounds() const;
	virtual	bool		Cull(const Bbox2& b) const;
	virtual void			GetResource(	  string& r) const;
			const string&	GetResource() const;
	virtual void			SetResource(const string& r);
	virtual void		SetHeading(double h);
	virtual	void		Rotate(GISLayer_t l,const Point2& center, double angle);

			int			HasCustomMSL(void) const;
			double		GetCustomMSL(void) const;
			
			void		SetCustomMSL(double msl,  bool is_AGL);
			void		SetDefaultMSL(void);
	virtual void		GetNthPropertyDict(int n, PropertyDict_t& dict) const;
	virtual	void		GetNthPropertyDictItem(int n, int e, string& item) const;
	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;

			void		SetShowLevel(int show_level);
			int			GetShowLevel(void) const;

			// Visible radius in degrees of latitude / longitude. Only takes horizontal extent, not height into account.
			double 		GetVisibleDeg(void) const;
			// Visible radius in meters. Takes extent in all three spatial directions into account.
			double		GetVisibleMeters(void) const;
			unsigned	ObjectGeometry(void) const;

	virtual const char *	HumanReadableType(void) const { return "Object"; }

private:

			unsigned	ObjectGeometryUncached(void) const;

	WED_PropIntEnum				has_msl;
	WED_PropDoubleTextMeters	msl;	
	WED_PropStringText			resource;
	WED_PropIntEnum				show_level;

	mutable float				visibleWithinDeg;     // for culling in the map_view
	mutable float				visibleWithinMeters;  // for culling in the 3D preview window
	mutable float				height;
	mutable bool				objectGeometrySet;
	mutable unsigned			objectGeometry;
};


#endif
