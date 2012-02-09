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

#include "WED_GISPoint_Heading.h"
#include "WED_PropertyHelper.h"

class	WED_ObjPlacement : public WED_GISPoint_Heading {

DECLARE_PERSISTENT(WED_ObjPlacement)

public:

	virtual	bool		Cull(const Bbox2& b) const;
			void		GetResource(	  string& r) const;
			void		SetResource(const string& r);

			bool		HasCustomMSL(void) const;
			double		GetCustomMSL(void) const;
			
			void		SetCustomMSL(double msl);
			void		SetDefaultMSL(void);
			
			void		SetShowLevel(int show_level);
			int			GetShowLevel(void) const;

private:

#if AIRPORT_ROUTING
	WED_PropBoolText		has_msl;
	WED_PropDoubleText		msl;	
#endif
	WED_PropStringText		resource;
	WED_PropIntEnum			show_level;

};


#endif
