#ifndef IPROPERTYOBJECT_H
#define IPROPERTYOBJECT_H

#include "IBase.h"

/*
	IPropertyObject - THEORY OF OPERATION
	
	This is a simple interface that lets client UI code "edit" an object by named properties, which are self-described in a format
	adequate for UI presentation.  The idea here is to describe the datamodel - not to spec UI.  At least, that's my rationalization.

*/


enum {
	prop_Int,
	prop_Double,
	prop_String,	
	prop_FilePath,
	prop_Bool,			// Returns as int
	prop_Enum,			// Returns as int
	prop_EnumSet
};

typedef	map<int,string>	PropertyDict_t;

struct PropertyInfo_t {
	int				can_edit;
	int				prop_kind;	
	string			prop_name;
	int				digits;
	int				decimals;
};

struct	PropertyVal_t {
	int			prop_kind;
	int			int_val;
	string		string_val;
	double		double_val;
	set<int>	set_val;
};

class IPropertyObject : public virtual IBase {
public:

	virtual	int			FindProperty(const char * in_prop)=0;
	virtual int			CountProperties(void)=0;
	virtual void		GetNthPropertyInfo(int n, PropertyInfo_t& info)=0;
	virtual	void		GetNthPropertyDict(int n, PropertyDict_t& dict)=0;			// Ben says: dictionary ops are broken out (and one vs all lookup are split too) for performance.
	virtual	void		GetNthPropertyDictItem(int n, int e, string& item)=0;		// It may be slow to get all enums, so give the UI code a way to say if it needs this info.
	
	virtual void		GetNthProperty(int n, PropertyVal_t& val)=0;
	virtual void		SetNthProperty(int n, const PropertyVal_t& val)=0;
	
};

#endif /* IPROPERTYOBJECT_H */
