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
	virtual void		GetProperty(PropertyVal_t& val)=0;
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)=0;
	virtual	void 		ReadFrom(IOReader * reader)=0;
	virtual	void 		WriteTo(IOWriter * writer)=0;
	virtual	void		FromDB(sqlite3 * db, const char * where_clause)=0;
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
	virtual int			CountProperties(void);
	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info);	
	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict);
	virtual	void		GetNthPropertyDictItem(int n, int e, string& item);
	virtual void		GetNthProperty(int n, PropertyVal_t& val);
	virtual void		SetNthProperty(int n, const PropertyVal_t& val);
	
	virtual	void		PropEditCallback(int before)=0;


	// Utility to help manage streaming
			void 		ReadPropsFrom(IOReader * reader);
			void 		WritePropsTo(IOWriter * writer);	
			void		PropsFromDB(sqlite3 * db, const char * where_clause);
			void		PropsToDB(sqlite3 * db, const char * id_col, const char * id_val, const char * skip_table);
	
private:

	friend class	WED_PropertyItem;
	vector<WED_PropertyItem *>		mItems;
	
};

// ------------------------------ A LIBRARY OF HANDY MEMBER VARIABLES ------------------------------------

class	WED_PropIntText : public WED_PropertyItem {
public:

	int				value;

	operator int&() { return value; }
	operator int() const { return value; }
	WED_PropIntText& operator=(int v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }

	WED_PropIntText(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, int initial)  : WED_PropertyItem(parent, title, table, column), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause);
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
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);
	
};	


class	WED_PropDoubleText : public WED_PropertyItem {
public:

	double			value;

						operator double&() { return value; }
						operator double() const { return value; }
	WED_PropDoubleText& operator=(double v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }
	
	WED_PropDoubleText(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, double initial)  : WED_PropertyItem(parent, title, table, column), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);
	
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
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause);
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
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);
	
};	

class	WED_PropIntEnumSet : public WED_PropertyItem {
public:

	set<int>	value;
	int			domain;

						operator set<int>&() { return value; }
						operator set<int>() const { return value; }
	WED_PropIntEnumSet& operator=(const set<int>& v) { if (value != v) { if (mParent) mParent->PropEditCallback(1); value = v; if (mParent) mParent->PropEditCallback(0); } return *this; }
	
	WED_PropIntEnumSet(WED_PropertyHelper * parent, const char * title, const char * table, const char * column, int idomain)  : WED_PropertyItem(parent, title, table, column), domain(idomain) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent);
	virtual	void 		ReadFrom(IOReader * reader);
	virtual	void 		WriteTo(IOWriter * writer);
	virtual	void		FromDB(sqlite3 * db, const char * where_clause);
	virtual	void		ToDB(sqlite3 * db, const char * id_col, const char * id_val);
	virtual	void		GetUpdate(SQL_Update& io_update);
	
};	


#endif

