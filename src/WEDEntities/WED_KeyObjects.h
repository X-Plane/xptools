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

#ifndef WED_KEYOBJECTS_H
#define WED_KEYOBJECTS_H

/*
	WED_KeyObjects	- THEORY OF OPERATION

	This persistent object essentially implements a hash table, mapping arbitrary objects to names
	returned via the directory interface.  Unlike a WED_thing, we can map non-children.  This is used
	to store "important" objects like the current edited airport.
*/

#include "WED_Thing.h"
#include "IDirectory.h"

class	WED_KeyObjects : public WED_Thing, public virtual IDirectoryEdit {

DECLARE_PERSISTENT(WED_KeyObjects)

public:

	virtual	IBase *			Directory_Find(const char * name);
	virtual	void			Directory_Edit(const char * name, IBase * who);

	virtual	bool 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	virtual	void			AddExtraXML(WED_XMLElement * obj);
	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);

	virtual const char *	HumanReadableType(void) const { return "Project Internal Directory"; }	// This better not be visible anywhere!
	
private:

		map<string,int>		choices;

};

#endif
