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

/*

	WED_PropertyHelper - THEORY OF OPERATION

	IPropertyObject provides an interface for a class to describe and I/O it's own data.  But...implementing that a hundred times over
	for each object would grow old fast.

	WED_PropertyHelper is an implementation that uses objects wrapped around member vars to simplify building up objects quickly.

	As a side note besides providing prop interfaces, it provides a way to stream properties to IODef reader/writers.  This is used to
	save undo work in WED_thing.

*/

#include <vector>
#include "IPropertyObject.h"
#include "WED_XMLReader.h"
#include "WED_Globals.h"
using std::vector;

class	WED_PropertyHelper;
class	IOWriter;
class	IOReader;
struct	sqlite3;
class	WED_XMLElement;

typedef pair<string,string>				SQL_ColumnUpdate;
typedef vector<SQL_ColumnUpdate>		SQL_TableUpdate;
typedef map<string, SQL_TableUpdate>	SQL_Update;

// We could have used type-def here, but this forces us to use the right type in the right place, for code clarity.  
struct SQL_Name : public pair<const char *, const char *> { SQL_Name(const char * a, const char * b) : pair<const char *, const char *>(a,b) { } };
struct XML_Name : public pair<const char *, const char *> { XML_Name(const char * a, const char * b) : pair<const char *, const char *>(a,b) { } };

class	WED_PropertyItem {
public:
	WED_PropertyItem(WED_PropertyHelper * parent, const char * title, SQL_Name sql_column, XML_Name xml_column);

	virtual void		GetPropertyInfo(PropertyInfo_t& info)=0;
	virtual	void		GetPropertyDict(PropertyDict_t& dict)=0;
	virtual	void		GetPropertyDictItem(int e, string& item)=0;
	virtual void		GetProperty(PropertyVal_t& val) const=0;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)=0;
	virtual	void 		ReadFrom(IOReader * reader)=0;
	virtual	void 		WriteTo(IOWriter * writer)=0;
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)=0;
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val)=0;
	virtual	void		ToXML(WED_XMLElement * parent)=0;
	virtual	void		GetUpdate(SQL_Update& io_update)=0;

	virtual	bool		WantsElement(WED_XMLReader * reader, const char * name) { return false; }
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value)=0;

	const char *			mTitle;
	SQL_Name				mSQLColumn;
	XML_Name				mXMLColumn;
	WED_PropertyHelper *	mParent;
private:
	WED_PropertyItem();
};

class WED_PropertyHelper : public WED_XMLHandler, public IPropertyObject {
public:

	virtual	int			FindProperty(const char * in_prop);
	virtual int			CountProperties(void) const;
	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info);
	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict);
	virtual	void		GetNthPropertyDictItem(int n, int e, string& item);
	virtual void		GetNthProperty(int n, PropertyVal_t& val) const;
	virtual void		SetNthProperty(int n, const PropertyVal_t& val);

	virtual	void				PropEditCallback(int before)=0;
	virtual	int					CountSubs(void)=0;
	virtual	IPropertyObject *	GetNthSub(int n)=0;


	// Utility to help manage streaming
			void 		ReadPropsFrom(IOReader * reader);
			void 		WritePropsTo(IOWriter * writer);
			void		PropsFromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
			void		PropsToDB(sqlite3 * db, const char * id_col, const char * id_val, const char * skip_table);
			void		PropsToXML(WED_XMLElement * parent);

	virtual void		StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts);
	virtual	void		EndElement(void);
	virtual	void		PopHandler(void);



			int			PropertyItemNumber(const WED_PropertyItem * item) const;
private:

	friend class	WED_PropertyItem;
	vector<WED_PropertyItem *>		mItems;

};

// ------------------------------ A LIBRARY OF HANDY MEMBER VARIABLES ------------------------------------

// An integer value entered as text.
class	WED_PropIntText : public WED_PropertyItem {
public:

	int				value;

	int				mDigits;

	operator int&() { return value; }
	operator int() const { return value; }
	WED_PropIntText& operator=(int v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropIntText(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, int initial, int digits)  : WED_PropertyItem(parent, title, sql_col,xml_col), value(initial), mDigits(digits) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

// A true-false value, stored as an int, but edited as a check-box.
class	WED_PropBoolText : public WED_PropertyItem {
public:

	int				value;

	operator int&() { return value; }
	operator int() const { return value; }
	WED_PropBoolText& operator=(int v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropBoolText(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, int initial)  : WED_PropertyItem(parent, title, sql_col,xml_col), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

// A double value edited as text.
class	WED_PropDoubleText : public WED_PropertyItem {
public:

	double			value;

	int				mDigits;
	int				mDecimals;

						operator double&() { return value; }
						operator double() const { return value; }
	WED_PropDoubleText& operator=(double v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropDoubleText(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, double initial, int digits, int decimals)  : WED_PropertyItem(parent, title, sql_col,xml_col), mDigits(digits), mDecimals(decimals), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

// A double value edited as text.  Stored in meters, but displayed in feet or meters, depending on UI settings.
class	WED_PropDoubleTextMeters : public WED_PropDoubleText {
public:
	WED_PropDoubleTextMeters(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, double initial, int digits, int decimals)  : WED_PropDoubleText(parent, title, sql_col,xml_col, initial, digits, decimals) { }

	WED_PropDoubleText& operator=(double v) { WED_PropDoubleText::operator=(v); return *this; }

	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
};

// A string, edited as text.
class	WED_PropStringText : public WED_PropertyItem {
public:

	string			value;

						operator string&() { return value; }
						operator string() const { return value; }
	WED_PropStringText& operator=(const string& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropStringText(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, const string& initial)  : WED_PropertyItem(parent, title, sql_col,xml_col), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

// A file path, saved as an STL string, edited by the file-open dialog box.
class	WED_PropFileText : public WED_PropertyItem {
public:

	string			value;

						operator string&() { return value; }
						operator string() const { return value; }
	WED_PropFileText& operator=(const string& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropFileText(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, const string& initial)  : WED_PropertyItem(parent, title, sql_col,xml_col), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

// An enumerated item.  Stored as an int, edited as a popup menu.  Property knows the "domain" the enum belongs to.
class	WED_PropIntEnum : public WED_PropertyItem {
public:

	int			value;
	int			domain;

						operator int&() { return value; }
						operator int() const { return value; }
	WED_PropIntEnum& operator=(int v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropIntEnum(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, int idomain, int initial)  : WED_PropertyItem(parent, title, sql_col,xml_col), value(initial), domain(idomain) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

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
	WED_PropIntEnumSet& operator=(const set<int>& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }
	WED_PropIntEnumSet& operator+=(const int v) { if(value.count(v) == 0) { if (mParent) mParent->PropEditCallback(1); value.insert(v); if (mParent) mParent->PropEditCallback(0); } return *this; }
	WED_PropIntEnumSet(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, int idomain, int iexclusive)  : WED_PropertyItem(parent, title, sql_col,xml_col), domain(idomain), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	bool		WantsElement(WED_XMLReader * reader, const char * name);
	virtual	void		GetUpdate(SQL_Update& io_update);

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

	set<int>	value;
	int			domain;
	int			can_be_none;
						
						operator set<int>&() { return value; }
						operator set<int>() const { return value; }
	WED_PropIntEnumBitfield& operator=(const set<int>& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropIntEnumBitfield(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, int idomain, int be_none)  : WED_PropertyItem(parent, title, sql_col,xml_col), domain(idomain), can_be_none(be_none) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};


// VIRTUAL ITEM: A FILTERED display.
// This item doesn't REALLY create data - it provides a filtered view of another enum set, showing only the enums within a given range.
// This is used to take ALL taxiway attributes and show only lights or only lines.
class	WED_PropIntEnumSetFilter : public WED_PropertyItem {
public:

	const char *			host;
	int						minv;
	int						maxv;
	int						exclusive;

	WED_PropIntEnumSetFilter(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, const char * ihost, int iminv, int imaxv, int iexclusive)  : WED_PropertyItem(parent, title, sql_col,xml_col), host(ihost), minv(iminv), maxv(imaxv), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

// VIRTUAL ITEM: a UNION display.  Property helpers can contain "sub" property helpers.  For the WED hierarchy, each hierarchy item (WED_Thing) is a 
// property helper (with properties inside it) and the sub-items in the hierarchy are the sub-helpers.  Thus a property item's parent (the "helper" sub-class)
// gives access to sub-items.  This filter looks at all enums on all children and unions them.
// We use this to let a user edit the marking attributes of all lines by editing the taxiway itself.
class	WED_PropIntEnumSetUnion : public WED_PropertyItem {
public:

	const char *			host;
	int						exclusive;

	WED_PropIntEnumSetUnion(WED_PropertyHelper * parent, const char * title, SQL_Name sql_col, XML_Name xml_col, const char * ihost, int iexclusive)  : WED_PropertyItem(parent, title, sql_col,xml_col), host(ihost), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		ToXML(WED_XMLElement * parent);
	virtual	bool		WantsAttribute(const char * ele, const char * att_name, const char * att_value);
	virtual	void		GetUpdate(SQL_Update& io_update);

};





#endif

