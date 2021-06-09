/*
 * Copyright (c) 2020, Laminar Research.
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

#ifndef WED_AutogenPlacement_H
#define WED_AutogenPlacement_H

#include "WED_GISPolygon.h"
#include "IHasResource.h"

class WED_AutogenPlacement : public WED_GISPolygon, public IHasResource {

DECLARE_PERSISTENT(WED_AutogenPlacement)

public:

	virtual	bool			IsClosed	(void	) const	{ return true; }
			bool			IsAGBlock     (void) const;

			double			GetHeight	(void) const;
			void			SetHeight   (double h);
			int				GetSpelling	(void) const;
			void			SetSpelling (int s);

	virtual void			GetResource(	  string& r) const;
	virtual void			SetResource(const string& r);

	virtual void			GetNthPropertyInfo(int n, PropertyInfo_t& info) const;

	virtual const char *	HumanReadableType(void) const { return "Autogen"; }

protected:

	virtual	bool			IsInteriorFilled(void) const { return true; }

private:

	WED_PropStringText			resource;
	WED_PropDoubleTextMeters	height;
	WED_PropIntText				spelling;
};

#endif /* WED_AutogenPlacement_H */
