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

#include "WED_XMLWriter.h"
#include "AssertUtils.h"
#include "GUI_Unicode.h"
/*
	PERFORMANCE NOTES:

	We could improve memory management by allocating a big shared block of memory and just pushing a ptr
	to get memory to stash strings (rather than using STL strings).
	
	If we do this we need to know when to purge out the memory...the answer:
	
	- Each time an obj starts using a block, add a ref to it.
	- Each time the obj is nuked, decrease ref count.
	- each time the current block is filled, start a new one.
	
	The result: as we run through the files and purge out the <object> blocks, 
	filled blocks will be retired for re-use.  We save a million tiny alloc/deallocs for STL strings.	

*/

#define FIX_EMPTY 0

inline void fi_indent(int n, FILE * fi) { while(n--) fputc(' ', fi); }

inline void fi_escape(const char * str, FILE * fi)
{
	UTF8 * b = (UTF8 *) str;
	UTF8 * e = b + strlen(str);
	
	// This fixes a problem, but not the way I intended, and may be worth some examination.
	// WED uses UTF8.  Period.  That is all it has ever displayed sanely, and it should be the only thing
	// you can get INTO it.  (At least, on mac and windows if you get a non-ASCII char in, it DOES come in
	// as UTF8.  I assume Linux isn't the offender putting ISO-Latin-1 in.)
	//
	// But.  Names of airports come straight from apt.dat, and some users encode the apt.dat file 
	// incorrectly as ISO-Latin-1.  We really only wanted ASCII, and X-Plane never liked ISO, so who knows
	// what is up.  Anyway: it is conceivable that at run time we have ISO-Latin-1 chars that are not valid 
	// UTF8 sequences.
	// This routine writes each invalid 8-bit char as a numeric code reference into the XML.  This has the
	// wrong effect: on read-in, we interpret what WAS a byte as a Unicode char.
	// 
	// And yet, this is actually useful.  It turns out that ISO-Latin-1 128-255 are mostly mapped to
	// unicode 128-255 (but NOT UTF8 128-255, which are bit encodings).  So for example:
	//
	// User wants unicode U+00C5.  In ISO-Latin-1 we have 0xC5.  This displays as a U or something wrong in
	// WED because the char printer will "muddle through" randomly with invalid input.  But we write it out
	// as %#xC5.  On read-in, Expat thinsk this is U+00C5 and gives us the correct 2-byte sequence UTF8 valid
	// sequence 0xC3 0x85.  This is of course what the user ORIGINALLY wanted.
	 
	while(b < e)
	{
		const UTF8 * v = UTF8_ValidRange(b,e);
		while(b < v)
		{	
			switch(*b) {
			case '<':
				fprintf(fi,"&lt;");
				break;			
			case '>':
				fprintf(fi,"&gt;");
				break;
			case '"':
				fprintf(fi,"&quot;");
				break;
			case '&':
				fprintf(fi,"&amp;");
				break;
			default:
				fputc(*b,fi);
				break;
			}
			++b;
		}
		const UTF8 * iv = UTF8_InvalidRange(b,e);
		while(b < iv)
		{
			fprintf(fi,"&#x%2X;",(int) *b);
			++b;
		}
	}	
}

WED_XMLElement::WED_XMLElement(
									const char *		n,
									int					i,
									FILE *				f) : 
	file(f), indent(i), name(n), flushed(false), parent(NULL)
{
}

WED_XMLElement::~WED_XMLElement()
{
	if(!flushed)
	{
		fi_indent(indent, file);
		fprintf(file,"<%s",name.c_str());

		for(map<string,string>::iterator a = attrs.begin(); a != attrs.end(); ++a)
		{
			fprintf(file," %s=\"", a->first.c_str());
			fi_escape(a->second.c_str(), file);
			fputc('"',file);
		}
		
		if(children.empty())
			fprintf(file,"/>\n");
		else
			fprintf(file,">\n");
	}
	
	for(vector<WED_XMLElement *>::iterator c = children.begin(); c != children.end(); ++c)
		delete *c;
	
	if(!children.empty() || flushed)
	{
		fi_indent(indent,file);
		fprintf(file,"</%s>\n",name.c_str());
	}
}

void WED_XMLElement::flush()
{
	flush_from(NULL);
}

void WED_XMLElement::flush_from(WED_XMLElement * who)
{
	if(who == NULL && children.empty())	return;
	
	if(parent) 
		parent->flush_from(this);
	parent = NULL;
	
	if(!flushed)
	{
		fi_indent(indent, file);
		fprintf(file,"<%s",name.c_str());

		for(map<string,string>::iterator a = attrs.begin(); a != attrs.end(); ++a)
		{
			fprintf(file," %s=\"", a->first.c_str());
			fi_escape(a->second.c_str(), file);
			fputc('"',file);
		}
		
		fprintf(file,">\n");
	}
	
	DebugAssert(who == children.back() || who == NULL);
	
	for(vector<WED_XMLElement *>::iterator c = children.begin(); c != children.end(); ++c)
	if(*c != who)
		delete *c;

	children.clear();
	if(who)
		children.push_back(who);
	flushed = true;
}

	
void					WED_XMLElement::add_attr_int(const char * name, int value)
{
#if FIX_EMPTY
	if(name == 0 || *name == 0)	name = "tbd";
#endif
	DebugAssert(!flushed);
	char buf[256];
	sprintf(buf,"%d",value);
	attrs[name] = buf;
}

void					WED_XMLElement::add_attr_double(const char * name, double value, int dec)
{
#if FIX_EMPTY
	if(name == 0 || *name == 0)	name = "tbd";
#endif
	DebugAssert(!flushed);
	char fmt[15], buf[256];
	sprintf(fmt,"%%.%dlf",dec);
	sprintf(buf,fmt,value);
	attrs[name] = buf;
}

void					WED_XMLElement::add_attr_c_str(const char * name, const char * str)
{
#if FIX_EMPTY
	if(name == 0 || *name == 0)	name = "tbd";
#endif
	DebugAssert(!flushed);
	attrs[name] = str;
}

void					WED_XMLElement::add_attr_stl_str(const char * name, const string& str)
{
#if FIX_EMPTY
	if(name == 0 || *name == 0)	name = "tbd";
#endif
	DebugAssert(!flushed);
	attrs[name] = str;
}

WED_XMLElement *		WED_XMLElement::add_sub_element(const char * name)
{
#if FIX_EMPTY
	if(name == 0 || *name == 0)	name = "tbd";
#endif

	WED_XMLElement * child = new WED_XMLElement(name, indent + 4, file);
	children.push_back(child);
	child->parent = this;
	return child;
}

WED_XMLElement *		WED_XMLElement::add_or_find_sub_element(const char * name)
{
#if FIX_EMPTY
	if(name == 0 || *name == 0)	name = "tbd";
#endif

	DebugAssert(!flushed);
	string n(name);
	for(int i = 0; i < children.size(); ++i)
	if(children[i]->name == n)
		return children[i];
		
	WED_XMLElement * child = new WED_XMLElement(name, indent + 4, file);
	children.push_back(child);
	child->parent = this;
	return child;
	
}
