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

#ifndef WED_XMLReader_H
#define WED_XMLReader_H

class	WED_XMLReader;

#include <list>
#include <expat.h>

inline const XML_Char * get_att(const char * name, const XML_Char ** atts)
{
	const XML_Char ** a = atts;
	while(*a)
	{
		if(strcmp(*a,name)==0)
		{
			++a;
			return *a;
		} else
		{
			++a;
			++a;
		}
	}
	return NULL;
}

class WED_XMLHandler {
public:

	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)=0;
	virtual	void		EndElement(void)=0;
	virtual	void		PopHandler(void)=0;
};

class WED_XMLReader {
public:
			 WED_XMLReader();
			~WED_XMLReader();

	void	PushHandler(WED_XMLHandler * handler);
	void	FailWithError(const string& err);
	
	// Returns err msg or "" for none.
	string	ReadFile(const char * filename, bool * exists);

private:

	list<WED_XMLHandler *>	handlers;
	list<bool>				new_handler_for_element;
	XML_Parser				parser;
	string					err;
	
	static void	StartElementHandler(void *userData,
						const XML_Char *name,
						const XML_Char **atts);

	static void EndElementHandler(void *userData,
						  const XML_Char *name);
	
};

#endif /* WED_XMLReader_H */
