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

#include "WED_XMLReader.h"
#include "AssertUtils.h"

WED_XMLReader::WED_XMLReader()
{
	parser = XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, StartElementHandler, EndElementHandler);
	XML_SetUserData(parser, reinterpret_cast<void*>(this));
}

WED_XMLReader::~WED_XMLReader()
{
	XML_ParserFree(parser);
}

void	WED_XMLReader::PushHandler(WED_XMLHandler * handler)
{
	if(!new_handler_for_element.empty())
		new_handler_for_element.back() = true;
	handlers.push_back(handler);
}

void	WED_XMLReader::FailWithError(const string& e)
{
	err = e;
	XML_StopParser(parser, false);		// we're dead!
}

string	WED_XMLReader::ReadFile(const char * filename, bool * exists)
{
	XML_ParserReset(parser, NULL);
	XML_SetElementHandler(parser, StartElementHandler, EndElementHandler);
	XML_SetUserData(parser, reinterpret_cast<void*>(this));

	FILE * fi = fopen(filename,"rb");
	if(exists) *exists = (fi != NULL);

	if(!fi)
		return string("Unable to open file:") + string(filename);

	while(!feof(fi))
	{
		void* const buf = XML_GetBuffer(parser, 4 * 1024);
		int len = fread(buf, 1, 4 * 1024, fi);
		if(XML_ParseBuffer(parser, len, feof(fi)) == XML_STATUS_ERROR)
		{
			LOG_MSG("E/XML %s At: %d,%d\n", err.c_str(), (int) XML_GetCurrentLineNumber(parser), (int) XML_GetCurrentColumnNumber(parser));
			break;
		}
	}
	XML_Error result = XML_GetErrorCode(parser);
	if(err.empty() &&  result != XML_ERROR_NONE)
		err = XML_ErrorString(result);

	fclose(fi);

	return err;
}

void	WED_XMLReader::StartElementHandler(void *userData,
						const XML_Char *name,
						const XML_Char **atts)
{
	WED_XMLReader * me = reinterpret_cast<WED_XMLReader*>(userData);
	me->new_handler_for_element.push_back(false);
	DebugAssert(!me->handlers.empty());
	me->handlers.back()->StartElement(me,name,atts);
}

void WED_XMLReader::EndElementHandler(void *userData,
						  const XML_Char *name)
{
	WED_XMLReader * me = reinterpret_cast<WED_XMLReader*>(userData);
	me->handlers.back()->EndElement();
	if(me->new_handler_for_element.back())
	{
		me->handlers.back()->PopHandler();
		me->handlers.pop_back();
		DebugAssert(!me->handlers.empty());
	}
	me->new_handler_for_element.pop_back();	
}
	


