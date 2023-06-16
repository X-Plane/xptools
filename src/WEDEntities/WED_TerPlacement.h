/*
 * Copyright (c) 2023, Laminar Research.
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

#ifndef WED_TerPlacement_H
#define WED_TerPlacement_H

#include "IHasResource.h"
#include "WED_GISPolygon.h"

class	WED_TerPlacement : public WED_GISPolygon, public IHasResource {

	DECLARE_PERSISTENT(WED_TerPlacement)

public:
	virtual void		GetResource(string& r) const;
	virtual void		SetResource(const string& r);
	double 				GetSamplingFactor(void) const;
	double 				GetSkirtDepth(void) const;
	// about the .obj to be generated and written to the DSF file
	int					GetShowLevel(void) const;
	int					GetMSLType(void) const;
	double				GetCustomMSL(void) const;

	virtual const char* HumanReadableType(void) const { return "Terrain Object"; }

protected:

	bool		IsInteriorFilled(void) const { return true; }

private:

	WED_PropStringText			resource;       // really want WED_PropFileText, but something is broken with it now
	WED_PropIntEnum				has_msl;
	WED_PropDoubleTextMeters	msl;
	WED_PropDoubleText			derez;
	WED_PropDoubleTextMeters	skirt;
	WED_PropIntEnum				show_level;
};

#endif /* WED_TerPlacement_H */
