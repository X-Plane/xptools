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

#ifndef WED_PROPERTYHELPER_H
#define WED_PROPERTYHELPER_H

/*	WED_PropertyHelper - THEORY OF OPERATION

	IPropertyObject provides an interface for a class to describe and I/O it's own data.  But...implementing that a hundred times over
	for each object would grow old fast.

	WED_PropertyHelper is an implementation that uses objects wrapped around member vars to simplify building up objects quickly.

	As a side note besides providing prop interfaces, it provides a way to stream properties to IODef reader/writers.  This is used to
	save undo work in WED_thing.
*/

#include "IPropertyObject.h"
#include "WED_XMLReader.h"
#include "WED_Globals.h"

class	WED_PropertyHelper;
class	IOWriter;
class	IOReader;
class	WED_XMLElement;

/* These macros create a *single* string containing a properties WED name and XML names,
   saving 2 pointers in each property item, after the sqlite removal already removed 2 pointers.
   This reduces the WED memory size with large sceneries (like importing the global apt.dat) by 30%
*/

#define XML_Name(x,y) x "\0" y
#define PROP_Name(wed_name, xml_name) wed_name "\0" xml_name, sizeof(wed_name) + 256 * (strlen(xml_name)+1+sizeof(wed_name))

/* more memory size and access time optimization for LARGE datasets, like the Global Airports:

   Every WED_Thing has on average 10 WED_Properties and a pair of pointers to/from each property.
   The Global Airports have as of mid 2019 ~14 million WED_Things, so this adds up to over 2GB.
   
   And the STL container vector<WED_PropertyItem *> is responsible for a good chunk of all that pain,
   as the pointer array is on the heap, requireing a second memory access to resolve. With large
   data structures, pretty much every memory access is a cache miss.
   
   Since the WED_PropertyItems are part of the same class - they are all within 2kB in memory from
   the WED__PropertyHelper class. Due to alignof(class) == 8 that relative distance can be encoded 
   with just 1 byte. The Vector<void *> is reduces to a memory-local char[] - saving overall 
   another 24% Memory and 5% CPU time on load, save and export.
   
   The maximum number of properties for any WEDEntity is 22 right now (Runways).
   Set below to 0 to disable most of the trickery.
*/   

#define PROP_PTR_OPT 23

class	WED_PropertyItem {
public:
	WED_PropertyItem(WED_PropertyHelper * parent, const char * title, int offset);

	virtual void		GetPropertyInfo(PropertyInfo_t& info)=0;
	virtual	void		GetPropertyDict(PropertyDict_t& dict)=0;
	virtual	void		GetPropertyDictItem(int e, string& item)=0;
	virtual void		GetProperty(PropertyVal_t& val) const=0;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)=0;
	virtual	void 		ReadFrom(IOReader * reader)=0;
	virtual	void 		WriteTo(IOWriter * writer)=0;
	virtual	void		ToXML(WED_XMLElement * parent)=0;

	virtual	bool		WantsElement(WED_XMLReader * reader, const char * name) { return false; }
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value)=0;

#if PROP_PTR_OPT
#define PTR_FIX(x)  x & ((1L << 45) - 1)
	WED_PropertyHelper *	GetParent(void)      const { return reinterpret_cast<WED_PropertyHelper *>((char *) this - (mTitle >> 45-3 & 0xFF << 3)); }
	const char *			GetWedName(void)     const { return reinterpret_cast<const char *>(PTR_FIX(mTitle)); }
	const char *			GetXmlName(void)     const { return reinterpret_cast<const char *>(PTR_FIX(mTitle)) + (mTitle >> 53 & 0x1F); }
	const char *			GetXmlAttrName(void) const { return reinterpret_cast<const char *>(PTR_FIX(mTitle)) + (mTitle >> 58 & 0x3F); }
private:
	uintptr_t				mTitle;     // this now holds THREE tightly packed offsets in its 18 MSBits to save even more RAM usage
#else
#define PTR_FIX(x) x & ((1L << 47) - 1)
	WED_PropertyHelper *	GetParent(void) const { return mParent; }
	const char *			GetWedName(void)     const { return reinterpret_cast<const char *>(PTR_FIX(mTitle)); }
	const char *			GetXmlName(void)     const { return reinterpret_cast<const char *>(PTR_FIX(mTitle)) + ((mTitle >> 48) & 0xFF); }
	const char *			GetXmlAttrName(void) const { return reinterpret_cast<const char *>(PTR_FIX(mTitle)) + ((mTitle >> 56) & 0xFF); }
private:
	uintptr_t				mTitle;      // the two MSBytes hold the offset to be added to get the const char * to the 2nd and 3rd word in the string
	WED_PropertyHelper *	mParent;
#endif
};

#if PROP_PTR_OPT
class relPtr {
public:
							relPtr() : mItemsCount(0) {};
	WED_PropertyItem * operator [] (int n) const;
	int					size(void) const { return mItemsCount; }
	void 					push_back(WED_PropertyItem * ptr);
private:
#pragma pack (push)
#pragma pack (1)
	unsigned char 		mItemsCount;
	unsigned char 		mItemsOffs[PROP_PTR_OPT];
#pragma pack (pop)
};
#endif

class WED_PropertyHelper : public WED_XMLHandler, public IPropertyObject {
public:

	virtual	int		FindProperty(const char * in_prop) const;
	virtual	int		CountProperties(void) const;
	virtual	void		GetNthPropertyInfo(int n, PropertyInfo_t& info) const;
	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict) const;
	virtual	void		GetNthPropertyDictItem(int n, int e, string& item) const;
	virtual	void		GetNthProperty(int n, PropertyVal_t& val) const;
	virtual	void		SetNthProperty(int n, const PropertyVal_t& val);
	virtual	void		DeleteNthProperty(int n) { };

	virtual	void		PropEditCallback(int before)=0;
	virtual	int					CountSubs(void)=0;
	virtual	IPropertyObject *	GetNthSub(int n)=0;

	// Utility to help manage streaming
				void 		ReadPropsFrom(IOReader * reader);
				void 		WritePropsTo(IOWriter * writer);
				void		PropsToXML(WED_XMLElement * parent);

	virtual	void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);

	// This is virtual so remappers like WED_Runway can "fix" the results
	virtual	int		PropertyItemNumber(const WED_PropertyItem * item) const;
	
#if PROP_PTR_OPT
	relPtr				mItems;
#else
	vector<WED_PropertyItem *>		mItems;
#endif

	friend class		WED_PropertyItem;
};

// ------------------------------ A LIBRARY OF HANDY MEMBER VARIABLES ------------------------------------

// An integer value entered as text.
class	WED_PropIntText : public WED_PropertyItem {
public:

	int				value;
	int				mDigits;

							operator int&() { return value; }
							operator int() const { return value; }
	WED_PropIntText& operator=(int v);

	WED_PropIntText(WED_PropertyHelper * parent, const char * title, int offset, int initial, int digits) :
		WED_PropertyItem(parent, title, offset), value(initial), mDigits(digits) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);

};

// A true-false value, stored as an int, but edited as a check-box.
class	WED_PropBoolText : public WED_PropertyItem {
public:

	int				value;

							operator int&() { return value; }
							operator int() const { return value; }
	WED_PropBoolText& operator=(int v);

	WED_PropBoolText(WED_PropertyHelper * parent, const char * title, int offset, int initial) :
		WED_PropertyItem(parent, title, offset), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
};

// A double value edited as text.
class	WED_PropDoubleText : public WED_PropertyItem {
public:

	double			value;

#pragma pack (push)
#pragma pack (1)
	char			mDigits;
	char			mDecimals;
	char 			mUnit[6];  // this can be non-zero terminated if desired unit text is 6 chars (or longer, but its truncated then)
#pragma pack (pop)

							operator double&() { return value; }
							operator double() const { return value; }
	WED_PropDoubleText&	operator=(double v);

	WED_PropDoubleText(WED_PropertyHelper * parent, const char * title, int offset, double initial, int digits, int decimals, const char * unit = "") :
		WED_PropertyItem(parent, title, offset), mDigits(digits), mDecimals(decimals), value(initial) { strncpy(mUnit,unit,6); }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
};

class	WED_PropFrequencyText : public WED_PropDoubleText {
public:
	WED_PropFrequencyText(WED_PropertyHelper * parent, const char * title, int offset, double initial, int digits, int decimals)
		: WED_PropDoubleText(parent, title, offset, initial, digits, decimals) { AssignFrom1Khz(GetAs1Khz()); }

	WED_PropFrequencyText& operator=(double v) { WED_PropDoubleText::operator=(v); return *this; }

	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);

	int		GetAs1Khz(void) const;
	void	AssignFrom1Khz(int freq_1khz);

};

// A double value edited as text.  Stored in meters, but displayed in feet or meters, depending on UI settings.
class	WED_PropDoubleTextMeters : public WED_PropDoubleText {
public:
	WED_PropDoubleTextMeters(WED_PropertyHelper * parent, const char * title, int offset, double initial, int digits, int decimals) :
		WED_PropDoubleText(parent, title, offset, initial, digits, decimals) { }

	WED_PropDoubleTextMeters& operator=(double v) { WED_PropDoubleText::operator=(v); return *this; }

	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual void		GetPropertyInfo(PropertyInfo_t& info);
};

// A string, edited as text.
class	WED_PropStringText : public WED_PropertyItem {
public:

	string			value;

							operator string&() { return value; }
							operator string() const { return value; }
	WED_PropStringText&	operator=(const string& v);

	WED_PropStringText(WED_PropertyHelper * parent, const char * title, int offset, const string& initial) :
		WED_PropertyItem(parent, title, offset), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
};

// A file path, saved as an STL string, edited by the file-open dialog box.
class	WED_PropFileText : public WED_PropertyItem {
public:

	string			value;

						operator string&() { return value; }
						operator string() const { return value; }
	WED_PropFileText& operator=(const string& v);

	WED_PropFileText(WED_PropertyHelper * parent, const char * title, int offset, const string& initial) :
		WED_PropertyItem(parent, title, offset), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
};

// An enumerated item.  Stored as an int, edited as a popup menu.  Property knows the "domain" the enum belongs to.
class	WED_PropIntEnum : public WED_PropertyItem {
public:

	int			value;
	int			domain;

						operator int&() { return value; }
						operator int() const { return value; }
	WED_PropIntEnum& operator=(int v);

	WED_PropIntEnum(WED_PropertyHelper * parent, const char * title, int offset, int idomain, int initial) :
		WED_PropertyItem(parent, title, offset), value(initial), domain(idomain) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
};

// A set of enumerated items.  Stored as an STL set of int values, edited as a multi-check popup.  We store the domain.
// Exclusive?  While the data model is always a set, the exclusive flag enforces "pick at most 1" behavior in the UI (e.g. pick a new value deselects the old) - some users like that sometimes.
// In exclusive a user CAN pick no enums at all.  (Set enums usually don't have a "none" enum value.)
class	WED_PropIntEnumSet : public WED_PropertyItem, public WED_XMLHandler {
public:

	set<int>	value;
	int			domain;
	int			exclusive;

						operator set<int>&() { return value; }
						operator set<int>() const { return value; }
	WED_PropIntEnumSet& operator=(const set<int>& v);
	
	WED_PropIntEnumSet& operator+=(const int v) 
	{ if(value.count(v) == 0) 
		{ if (GetParent()) GetParent()->PropEditCallback(1); 
			value.insert(v); 
			if (GetParent()) GetParent()->PropEditCallback(0);
		} 
		return *this; 
	}
	WED_PropIntEnumSet(WED_PropertyHelper * parent, const char * title, int offset, int idomain, int iexclusive) :
		WED_PropertyItem(parent, title, offset), domain(idomain), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	bool		WantsElement(WED_XMLReader * reader, const char * name);

	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);

};

// Set of enums stored as a bit-field.  The export values for the enum domain must be a bitfield.
// This is:
// - Stored as a set<int> internally.
// - Almost always saved/restored as a bit-field.
// - Edited as a popup with multiple checks.
class	WED_PropIntEnumBitfield : public WED_PropertyItem {
public:

	set<int>		value;
	int			domain;
	int			can_be_none;

						operator set<int>&() { return value; }
						operator set<int>() const { return value; }
	WED_PropIntEnumBitfield& operator=(const set<int>& v);

	WED_PropIntEnumBitfield(WED_PropertyHelper * parent, const char * title, int offset, int idomain, int be_none) :
		WED_PropertyItem(parent, title, offset), domain(idomain), can_be_none(be_none) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);

};


// VIRTUAL ITEM: A FILTERED display.
// This item doesn't REALLY create data - it provides a filtered view of another enum set, showing only the enums within a given range.
// This is used to take ALL taxiway attributes and show only lights or only lines.
class	WED_PropIntEnumSetFilter : public WED_PropertyItem {
public:

	const char *		host;

	short					minv;
	short					maxv;
	bool					exclusive;

	WED_PropIntEnumSetFilter(WED_PropertyHelper * parent, const char * title, int offset, const char * ihost, int iminv, int imaxv, int iexclusive) :
		WED_PropertyItem(parent, title, offset), host(ihost), minv(iminv), maxv(imaxv), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
};

// VIRTUAL ITEM: a UNION display.  Property helpers can contain "sub" property helpers.  For the WED hierarchy, each hierarchy item (WED_Thing) is a
// property helper (with properties inside it) and the sub-items in the hierarchy are the sub-helpers.  Thus a property item's parent (the "helper" sub-class)
// gives access to sub-items.  This filter looks at all enums on all children and unions them.
// We use this to let a user edit the marking attributes of all lines by editing the taxiway itself.
class	WED_PropIntEnumSetUnion : public WED_PropertyItem {
public:

	const char *		host;
	int					exclusive;

	WED_PropIntEnumSetUnion(WED_PropertyHelper * parent, const char * title, int offset, const char * ihost, int iexclusive) :
		WED_PropertyItem(parent, title, offset), host(ihost), exclusive(iexclusive) { }
	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
};

// VIRTUAL ITEM: A FILTERED matrix display.
// This item doesn't REALLY create data - it provides a filtered view of another enum set, showing only the enums within a given range.
// This is used to take ALL taxiway attributes and show only lights or only lines.

class	WED_PropIntEnumSetFilterVal : public WED_PropIntEnumSetFilter {
public:

	WED_PropIntEnumSetFilterVal(WED_PropertyHelper * parent, const char * title, int offset, const char * ihost, int iminv, int imaxv, int iexclusive) :
		WED_PropIntEnumSetFilter(parent, title, offset, ihost, iminv, imaxv, iexclusive) { }

	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
};

#endif
