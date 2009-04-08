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

using std::vector;

extern	int	gIsFeet;

class	WED_PropertyHelper;
class	IOWriter;
class	IOReader;
struct	sqlite3;

typedef pair<string,string>				SQL_ColumnUpdate;
typedef vector<SQL_ColumnUpdate>		SQL_TableUpdate;
typedef map<string, SQL_TableUpdate>	SQL_Update;

class	WED_PropertyItem {
public:
	WED_PropertyItem(WED_PropertyHelper * parent, const char * title, const char * table, const char * column);

	virtual void		GetPropertyInfo(PropertyInfo_t& info)=0;
	virtual	void		GetPropertyDict(PropertyDict_t& dict)=0;
	virtual	void		GetPropertyDictItem(int e, string& item)=0;
	virtual void		GetProperty(PropertyVal_t& val) const=0;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)=0;
	virtual	void 		ReadFrom(IOReader * reader)=0;
	virtual	void 		WriteTo(IOWriter * writer)=0;
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)=0;
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val)=0;
	virtual	void		GetUpdate(SQL_Update& io_update)=0;

	const char *			mTitle;
	const char *			mTable;
	const char *			mColumn;
	WED_PropertyHelper *	mParent;
private:
	WED_PropertyItem();
};

class WED_PropertyHelper : public IPropertyObject {
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

private:

	friend class	WED_PropertyItem;
	vector<WED_PropertyItem *>		mItems;

};

// ------------------------------ A LIBRARY OF HANDY MEMBER VARIABLES ------------------------------------

class	WED_PropIntText : public WED_PropertyItem {
public:

	int				value;

	int				mDigits;

	operator int&() { return value; }
	operator int() const { return value; }
	WED_PropIntText& operator=(int v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropIntText(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, int initial, int digits)  : WED_PropertyItem(parent, title, table, column), value(initial), mDigits(digits) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

class	WED_PropBoolText : public WED_PropertyItem {
public:

	int				value;

	operator int&() { return value; }
	operator int() const { return value; }
	WED_PropBoolText& operator=(int v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropBoolText(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, int initial)  : WED_PropertyItem(parent, title, table, column), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};


class	WED_PropDoubleText : public WED_PropertyItem {
public:

	double			value;

	int				mDigits;
	int				mDecimals;

						operator double&() { return value; }
						operator double() const { return value; }
	WED_PropDoubleText& operator=(double v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropDoubleText(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, double initial, int digits, int decimals)  : WED_PropertyItem(parent, title, table, column), mDigits(digits), mDecimals(decimals), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

class	WED_PropDoubleTextMeters : public WED_PropDoubleText {
public:
	WED_PropDoubleTextMeters(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, double initial, int digits, int decimals)  : WED_PropDoubleText(parent, title, table, column, initial, digits, decimals) { }

	WED_PropDoubleText& operator=(double v) { WED_PropDoubleText::operator=(v); return *this; }

	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
};

class	WED_PropStringText : public WED_PropertyItem {
public:

	string			value;

						operator string&() { return value; }
						operator string() const { return value; }
	WED_PropStringText& operator=(const string& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropStringText(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, const string& initial)  : WED_PropertyItem(parent, title, table, column), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

class	WED_PropFileText : public WED_PropertyItem {
public:

	string			value;

						operator string&() { return value; }
						operator string() const { return value; }
	WED_PropFileText& operator=(const string& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropFileText(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, const string& initial)  : WED_PropertyItem(parent, title, table, column), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};


class	WED_PropIntEnum : public WED_PropertyItem {
public:

	int			value;
	int			domain;

						operator int&() { return value; }
						operator int() const { return value; }
	WED_PropIntEnum& operator=(int v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropIntEnum(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, int idomain, int initial)  : WED_PropertyItem(parent, title, table, column), value(initial), domain(idomain) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

class	WED_PropIntEnumSet : public WED_PropertyItem {
public:

	set<int>	value;
	int			domain;
	int			exclusive;

						operator set<int>&() { return value; }
						operator set<int>() const { return value; }
	WED_PropIntEnumSet& operator=(const set<int>& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropIntEnumSet(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, int idomain, int iexclusive)  : WED_PropertyItem(parent, title, table, column), domain(idomain), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};


class	WED_PropIntEnumSetFilter : public WED_PropertyItem {
public:

	const char *			host;
	int						minv;
	int						maxv;
	int						exclusive;

	WED_PropIntEnumSetFilter(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, const char * ihost, int iminv, int imaxv, int iexclusive)  : WED_PropertyItem(parent, title, table, column), host(ihost), minv(iminv), maxv(imaxv), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};

class	WED_PropIntEnumSetUnion : public WED_PropertyItem {
public:

	const char *			host;
	int						exclusive;

	WED_PropIntEnumSetUnion(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, const char * ihost, int iexclusive)  : WED_PropertyItem(parent, title, table, column), host(ihost), exclusive(iexclusive) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val) const;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);

};





#endif

