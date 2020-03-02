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

#ifndef WED_LinePlacement_H
#define WED_LinePlacement_H

#include "IHasResource.h"
#include "WED_GISChain.h"

class WED_LinePlacement : public WED_GISChain, public IHasResource {

DECLARE_PERSISTENT(WED_LinePlacement)

public:

	virtual	bool			IsClosed	(void	) const	;
			void			SetClosed(int closure);

	virtual void			GetResource(	  string& r) const;
	virtual void			SetResource(const string& r);

	virtual const char *	HumanReadableType(void) const { return "Line"; }

	// IPropertyObject
	int			FindProperty(const char * in_prop) const;
	int			CountProperties(void) const;
	void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;

	void		GetNthPropertyDict(int n, PropertyDict_t& dict) const;
	void		GetNthPropertyDictItem(int n, int e, string& item) const;

	void		GetNthProperty(int n, PropertyVal_t& val) const;
	void		SetNthProperty(int n, const PropertyVal_t& val);

protected:

	virtual	bool			IsJustPoints(void) const { return false; }

private:

	WED_PropBoolText		closed;
	WED_PropStringText		resource;

};



#endif /* WED_LinePlacement_H */
