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

#ifndef WED_RUNWAY_H
#define WED_RUNWAY_H

#include "WED_GISLine_Width.h"

struct	AptRunway_t;

class	WED_Runway : public WED_GISLine_Width {

DECLARE_PERSISTENT(WED_Runway)

public:

	virtual	bool	Cull(const Bbox2& b) const;

	// These routines return a rectangle for the given sub-rect of the runway.  Like all rects
	// they are clockwise, with the first point on the left side of the runway (looking from the low
	// to high end) at the low end.

	pair<int,int>	GetRunwayEnumsOneway() const;		// returns atc_Runway_None/atc_Runway_None if unparseable
													// returns valid enum/atc_Runway_None if in the form 36L/XXX
	int				GetRunwayEnumsTwoway() const;		// returns atc_rwy_None if unparsable

	// These routines return false if these elements aren't there.
	bool		GetCornersBlas1(Point2 corners[4]) const;
	bool		GetCornersBlas2(Point2 corners[4]) const;
	bool		GetCornersDisp1(Point2 corners[4]) const;
	bool		GetCornersDisp2(Point2 corners[4]) const;
	bool		GetCornersShoulders(Point2 corners[8]) const;
	double		GetRoughness(void) const;

	void		SetSurface(int);
	void		SetShoulder(int);
	void		SetRoughness(double);
	void		SetCenterLights(int);
	void		SetEdgeLights(int);
	void		SetRemainingSigns(int);
	void		SetMarkings1(int);
	void		SetAppLights1(int);
	void		SetTDZL1(int);
	void		SetREIL1(int);
	void		SetMarkings2(int);
	void		SetAppLights2(int);
	void		SetTDZL2(int);
	void		SetREIL2(int);

	int			GetSurface(void) const;
	int			GetShoulder(void) const;

	double		GetDisp1(void) const;
	double		GetDisp2(void) const;
	double		GetBlas1(void) const;
	double		GetBlas2(void) const;

	void		SetDisp1(double disp1);
	void		SetDisp2(double disp2);
	void		SetBlas1(double blas1);
	void		SetBlas2(double blas2);

	void		Import(const AptRunway_t& x, void (* print_func)(void *, const char *, ...), void * ref);
	void		Export(		 AptRunway_t& x) const;

	virtual const char * HumanReadableType(void) const { return "Runway"; }

	virtual	void         GetNthPropertyDict(int n, PropertyDict_t& dict) const;
	virtual	void         PropEditCallback(int before);

private:

	WED_PropIntEnum			surface;
	WED_PropIntEnum			shoulder;
	WED_PropDoubleText		roughness;
	WED_PropBoolText		center_lites;
	WED_PropIntEnum			edge_lites;
	WED_PropBoolText		remaining_signs;

//	WED_PropStringText			id1;
	WED_PropDoubleTextMeters	disp1;
	WED_PropDoubleTextMeters	blas1;
	WED_PropIntEnum				mark1;
	WED_PropIntEnum				appl1;
	WED_PropBoolText			tdzl1;
	WED_PropIntEnum				reil1;

//	WED_PropStringText			id2;
	WED_PropDoubleTextMeters	disp2;
	WED_PropDoubleTextMeters	blas2;
	WED_PropIntEnum				mark2;
	WED_PropIntEnum				appl2;
	WED_PropBoolText			tdzl2;
	WED_PropIntEnum				reil2;

};

#endif /* WED_RUNWAY_H */