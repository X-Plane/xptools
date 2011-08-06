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

string	WED_XMLReader::ReadFile(const char * filename)
{
	XML_ParserReset(parser, NULL);
	XML_SetElementHandler(parser, StartElementHandler, EndElementHandler);
	XML_SetUserData(parser, reinterpret_cast<void*>(this));

	FILE * fi = fopen(filename,"rb");
	if(!fi)
		return string("Unable to open file:") + string(filename);

	char buf[1024];

	int total = 0;
	while(!feof(fi))
	{
		int len = fread(buf,1,sizeof(buf),fi);
		if(len > 0)
		if(XML_Parse(parser, buf, len, 0) == XML_STATUS_ERROR)
		{
			total += len;
			XML_Error e = XML_GetErrorCode(parser);
			printf("Got error: %d (%s).\n", e, XML_ErrorString(e));
			printf("At: %d,%d\n", XML_GetCurrentLineNumber(parser), XML_GetCurrentColumnNumber(parser));
			break;
		}
	}
	XML_Parse(parser, buf, 0, 1);
	printf("read %d bytes.\n", total);
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
	


