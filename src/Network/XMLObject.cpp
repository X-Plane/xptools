/* 
 * Copyright (c) 2004, Laminar Research.
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
#include "XMLObject.h"
#include "xmlparse.h"
				
XMLObject::XMLObject(const std::string& inTag) : mTag(inTag)
{
}

XMLObject::~XMLObject()
{
	for (ObjectMap::iterator iter = mMap.begin(); iter != mMap.end(); ++iter)
	{
		delete iter->second;
	}
}

void	XMLObject::GetTag(std::string& outTag)
{
	outTag = mTag;
}

void	XMLObject::GetContents(std::string& outContents)
{
	outContents = mContents;
}

XMLObject *	XMLObject::GetNestedSubObject(const std::string& inKey)
{
	std::string::size_type p = inKey.find("/");
	if (p == inKey.npos)
		return GetTaggedSubObject(inKey, 0);
	
	XMLObject * sub = GetTaggedSubObject(inKey.substr(0, p), 0);
	if (sub == NULL) return sub;
	return sub->GetNestedSubObject(inKey.substr(p+1));
}

int			XMLObject::GetContentsInt(void)
{
	return atoi(mContents.c_str());
}

double			XMLObject::GetContentsDouble(void)
{
	return atof(mContents.c_str());
}


XMLObject *	XMLObject::GetSubObject(long inIndex)
{
	if (inIndex >= mMap.size())
		return NULL;
		
	return mMap[inIndex].second;
}

XMLObject *	XMLObject::GetTaggedSubObject(const std::string& inKey, long inIndex)
{
	for (ObjectMap::iterator iter = mMap.begin(); iter != mMap.end(); ++iter)
	{
		if (iter->first == inKey)
		{
			if (inIndex)
				inIndex--;
			else
				return iter->second;
		}
	}
	return NULL;
}

void		XMLObject::AccumContents(const char * inChars, long inLen)
{
	mContents += std::string(inChars, inLen);
}

void		XMLObject::AddObject(const std::string& inKey, XMLObject * inObject)
{
	mMap.push_back(ObjectMap::value_type(inKey, inObject));
}

struct	ParserInfo {
	vector<XMLObject *>	stack;
};

void StartElement(void *userData, const char *name, const char **atts)
{
	ParserInfo* p = (ParserInfo*) userData;
	XMLObject * newObj = new XMLObject(name);
	p->stack.push_back(newObj);	
}

void EndElement(void *userData, const char *name)
{
	ParserInfo* p = (ParserInfo*) userData;
	if (p->stack.empty())
		return;
	XMLObject *	finishedObj = p->stack.back();
	std::string	key;
	finishedObj->GetTag(key);
	p->stack.pop_back();
	if (p->stack.empty())
		return;
	p->stack.back()->AddObject(key, finishedObj);
}

void HandleChars(void *userData,const XML_Char *s,int len)
{
	ParserInfo* p = (ParserInfo*) userData;
	if (p->stack.empty())
		return;	
	p->stack.back()->AccumContents(s, len);
}

XMLObject *	ParseXML(const char * inBuf, int inLen)
{
	XML_Parser p = XML_ParserCreate(NULL);
	ParserInfo	info;
	info.stack.push_back(new XMLObject("root"));

	XML_SetUserData(p, (void *) &info);
	XML_SetElementHandler(p, StartElement, EndElement);
	XML_SetCharacterDataHandler(p, HandleChars);

	if (!XML_Parse(p, inBuf, inLen, 1)) 
	{
		for (vector<XMLObject *>::iterator i = info.stack.begin();
			i != info.stack.end(); ++i)
		{
			delete *i;
		}
		return NULL;
//				fprintf(stderr,
//				  "%s at line %d\n",
//				  XML_ErrorString(XML_GetErrorCode(p)),
//				  XML_GetCurrentLineNumber(p));
	}
	return info.stack.back();
}

void		XMLObject::Dump(void)
{
	static	int level = 0;
	for (int n = 0; n < level; ++n) printf(" ");
	printf("<%s>%s",mTag.c_str(), mContents.c_str());
	level += 2;
	if (!mMap.empty())
		printf("\n");
	for (ObjectMap::iterator iter = mMap.begin(); iter != mMap.end(); ++iter)
		iter->second->Dump();
	level -= 2;
	for (int n = 0; n < level; ++n) printf(" ");
	printf("</%s>\n", mTag.c_str());
}
