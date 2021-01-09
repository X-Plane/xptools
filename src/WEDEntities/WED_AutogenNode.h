/*
 * Copyright (c) 2011, Laminar Research.
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

#ifndef WED_AutogenNode_H
#define WED_AutogenNode_H

#include "WED_GISPoint.h"

class WED_AutogenNode : public WED_GISPoint {

DECLARE_PERSISTENT(WED_AutogenNode)

public:

	virtual	bool	HasLayer           (GISLayer_t layer            ) const;

	virtual	void	GetLocation		   (GISLayer_t l,      Point2& p) const;

			bool	GetSpawning(void) const;
			void	SetSpawning(bool sp);

	virtual void	GetNthPropertyInfo(int n, PropertyInfo_t& info) const;

	virtual const char *	HumanReadableType(void) const { return "Autogen Node"; }

private:

	WED_PropBoolText	spawning;
};


#endif /* WED_AutogenNode_H */
