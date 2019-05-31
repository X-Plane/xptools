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

#ifndef WED_XMLWriter_H
#define WED_XMLWriter_H

/* 
	IMPORTANT: const char * name parameters must be "permanent" - that is, they need to remain
	valid for the duration of the XML element's life.
	
	C strings passed to add_attr_c_str are copied - you don't need to retire them.

 */

class	WED_XMLElement {
public:

				 WED_XMLElement(
									const char *		name,
									int					indent,
									FILE *				destination);
				~WED_XMLElement();
		
	void					add_attr_int(const char * name, int value);
	void					add_attr_double(const char * name, double value, int dec);
	void					add_attr_c_str(const char * name, const char * str);
	void					add_attr_stl_str(const char * name, const string& str);
	
	WED_XMLElement *		add_sub_element(const char * name);
	WED_XMLElement *		add_or_find_sub_element(const char * name);
	
	// Call flush to write out data so far.  In order to do this:
	// - All attributes must be done for this obj and all parents.  Adding attributes to a flushed obj or its parents is an error!
	// - You won't try to find existing sub-elements on a flushed element or its parents.  That's an error too!
	// - No existing older child to this object or any of its parents can be used again.  So my older sibling and my parent's older sibling
	// are both GONE after we flush!  
	// We use flush to keep memory bubble down.
	void					flush(void);
	
private:

	void					flush_from(WED_XMLElement * child);

		bool									flushed;
		FILE *									file;
		int										indent;
		const char *							name;
		vector<pair<const char *,string> >	attrs;
		vector<WED_XMLElement *>			children;
		WED_XMLElement *						parent;
};	
	
#endif /* WED_XMLWriter_H */

