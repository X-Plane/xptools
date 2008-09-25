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
#ifndef _XMLObject_h_
#define _XMLObject_h_

#include <string>
#include <vector>

class	XMLObject {
public:

				XMLObject(const std::string& inTag);
	virtual		~XMLObject();

	// Access
	void		GetTag(std::string& outTag);
	void		GetContents(std::string& outContents);
	XMLObject *	GetSubObject(long inIndex);
	XMLObject *	GetTaggedSubObject(const std::string& inKey, long inIndex);

	XMLObject *	GetNestedSubObject(const std::string& inKey);
	int			GetContentsInt(void);
	double		GetContentsDouble(void);

	// Building

	void		AccumContents(const char * inChars, long inLen);
	void		AddObject(const std::string& inKey, XMLObject * inObject);

	void		Dump(void);

protected:

	typedef	std::pair<std::string, XMLObject *>	ObjectPair;
	typedef	std::vector<ObjectPair>				ObjectMap;

	std::string		mTag;
	std::string		mContents;
	ObjectMap		mMap;

};

XMLObject *	ParseXML(const char * inBuf, int inLen);

#endif