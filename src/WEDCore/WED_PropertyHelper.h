#ifndef WED_PROPERTYHELPER_H
#define WED_PROPERTYHELPER_H

/*

	WED_PropertyHelper - THEORY OF OPERATION

	IPropertyObject provides an interface for a class to describe and I/O it's own data.  But...implementing that a hundred times over
	for each object would grow old fast.
	
	WED_PropertyHelper is an implementation that uses objects wrapped around member vars to simplify building up objects quickly.
	
*/

#include <vector>
#include "IPropertyObject.h"

using std::vector;

class	WED_PropertyHelper;

class	WED_PropertyItem {
public:
	WED_PropertyItem(WED_PropertyHelper * parent, const char * title);

	virtual void		GetPropertyInfo(PropertyInfo_t& info)=0;	
	virtual	void		GetPropertyDict(PropertyDict_t& dict)=0;
	virtual	void		GetPropertyDictItem(int e, string& item)=0;
	virtual void		GetProperty(PropertyVal_t& val)=0;
	virtual void		SetProperty(const PropertyVal_t& val)=0;

	const char *	mTitle;
	
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
	
	
	virtual	void		PropEditCallback(void)=0;
	
private:

	friend class	WED_PropertyItem;
	vector<WED_PropertyItem *>		mItems;
	
};

// ------------------------------ A LIBRARY OF HANDY MEMBER VARIABLES ------------------------------------

class	WED_PropIntText : public WED_PropertyItem {
public:

	int				value;

	operator int&() { return value; }
	WED_PropIntText& operator=(int v) { value = v; return *this; }

	WED_PropIntText(WED_PropertyHelper * parent, const char * title, int initial)  : WED_PropertyItem(parent, title), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val);
	
};	

class	WED_PropBoolText : public WED_PropertyItem {
public:

	int				value;

	operator int&() { return value; }
	WED_PropBoolText& operator=(int v) { value = v; return *this; }

	WED_PropBoolText(WED_PropertyHelper * parent, const char * title, int initial)  : WED_PropertyItem(parent, title), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val);
	
};	


class	WED_PropDoubleText : public WED_PropertyItem {
public:

	double			value;

						operator double&() { return value; }
	WED_PropDoubleText& operator=(double v) { value = v; return *this; }
	
	WED_PropDoubleText(WED_PropertyHelper * parent, const char * title, double initial)  : WED_PropertyItem(parent, title), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val);
	
};	

class	WED_PropStringText : public WED_PropertyItem {
public:

	string			value;

						operator string&() { return value; }
	WED_PropStringText& operator=(const string& v) { value = v; return *this; }
	
	WED_PropStringText(WED_PropertyHelper * parent, const char * title, const string& initial)  : WED_PropertyItem(parent, title), value(initial) { }

	virtual void		GetPropertyInfo(PropertyInfo_t& info);
	virtual	void		GetPropertyDict(PropertyDict_t& dict);
	virtual	void		GetPropertyDictItem(int e, string& item);
	virtual void		GetProperty(PropertyVal_t& val);
	virtual void		SetProperty(const PropertyVal_t& val);
	
};	


#endif

