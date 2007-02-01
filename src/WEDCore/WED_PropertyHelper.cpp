#include "WED_PropertyHelper.h"
#include "AssertUtils.h"

int		WED_PropertyHelper::FindProperty(const char * in_prop)
{
	for (int n = 0; n < mItems.size(); ++n)
		if (strcmp(mItems[n]->mTitle, in_prop)==0) return n;
	return -1;
}

int		WED_PropertyHelper::CountProperties(void)
{
	return mItems.size();
}

void		WED_PropertyHelper::GetNthPropertyInfo(int n, PropertyInfo_t& info)
{
	mItems[n]->GetPropertyInfo(info);
}

void		WED_PropertyHelper::GetNthPropertyDict(int n, PropertyDict_t& dict)
{
	mItems[n]->GetPropertyDict(dict);
}

void		WED_PropertyHelper::GetNthPropertyDictItem(int n, int e, string& item)
{
	mItems[n]->GetPropertyDictItem(e, item);
}

void		WED_PropertyHelper::GetNthProperty(int n, PropertyVal_t& val)
{
	mItems[n]->GetProperty(val);
}

void		WED_PropertyHelper::SetNthProperty(int n, const PropertyVal_t& val)
{
	mItems[n]->SetProperty(val);
	PropEditCallback();
}

WED_PropertyItem::WED_PropertyItem(WED_PropertyHelper * pops, const char * title) : mTitle(title)
{
	pops->mItems.push_back(this);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//----------------------------------------------------------------------------------------------------------------------------------------------------------------


void		WED_PropIntText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_Int;
	info.prop_name = mTitle;
}

void		WED_PropIntText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropIntText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropIntText::GetProperty(PropertyVal_t& val)
{
	val.int_val = value;
	val.prop_kind = prop_Int;
}

void		WED_PropIntText::SetProperty(const PropertyVal_t& val)
{
	DebugAssert(val.prop_kind == prop_Int);
	value = val.int_val;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropBoolText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_Bool;
	info.prop_name = mTitle;
}

void		WED_PropBoolText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropBoolText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropBoolText::GetProperty(PropertyVal_t& val)
{
	val.int_val = value;
	val.prop_kind = prop_Bool;
}

void		WED_PropBoolText::SetProperty(const PropertyVal_t& val)
{
	DebugAssert(val.prop_kind == prop_Bool);
	value = val.int_val;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropDoubleText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_Double;
	info.prop_name = mTitle;
}

void		WED_PropDoubleText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropDoubleText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropDoubleText::GetProperty(PropertyVal_t& val)
{
	val.double_val = value;
	val.prop_kind = prop_Double;
}

void		WED_PropDoubleText::SetProperty(const PropertyVal_t& val)
{
	DebugAssert(val.prop_kind == prop_Double);
	value = val.double_val;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropStringText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_String;
	info.prop_name = mTitle;
}

void		WED_PropStringText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropStringText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropStringText::GetProperty(PropertyVal_t& val)
{
	val.string_val = value;
	val.prop_kind = prop_String;
}

void		WED_PropStringText::SetProperty(const PropertyVal_t& val)
{
	DebugAssert(val.prop_kind == prop_String);
	value = val.string_val;
}
