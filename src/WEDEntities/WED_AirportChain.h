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

#ifndef WED_AIRPORTCHAIN_H
#define WED_AIRPORTCHAIN_H

#include "WED_GISChain.h"

struct	AptMarking_t;

class	WED_AirportChain : public WED_GISChain {

DECLARE_PERSISTENT(WED_AirportChain)

public:

//	virtual	IGISPoint *		SplitSide   (int n	)		;		// Split the side from pt N to pt N + 1 in half. Return the new pt.
	virtual	bool			IsClosed	(void	) const	;

			void			SetClosed(int closure);

	// WED_Persistent
	virtual	bool 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);
	// WED_Thing
	virtual	void			AddExtraXML(WED_XMLElement * obj);

	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);

			void			Import(const AptMarking_t& x, void (* print_func)(void *, const char *, ...), void * ref);
			void			Export(		 AptMarking_t& x) const;

	virtual const char *	HumanReadableType(void) const { return "Airport Line Marking"; }

protected:

	virtual	bool			IsJustPoints(void) const { return false; }

private:

	WED_PropIntEnumSetUnion	lines;
	WED_PropIntEnumSetUnion	lights;

	// Why is "closed" not a user-settable property?  Well, airport chains are used as the children of a number of
	// entities.  Some, like taxiway pavement polygons, _cannot_ be non-closed.  So don't let the user go around manually
	// opening up these polygons.
	int		closed;

};

#endif /* WED_AIRPORTCHAIN_H */
