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

#include "WED_PropertyHelper.h"
#include "AssertUtils.h"
#include "WED_Errors.h"
#include "IODefs.h"
#include "STLUtils.h"
#include "MathUtils.h"
#include "XESConstants.h"
#include "WED_EnumSystem.h"
#include <algorithm>
#include "WED_XMLWriter.h"

int gIsFeet = 0;

inline int remap(const map<int,int>& m, int v)
{
	map<int,int>::const_iterator i = m.find(v);
	if (i == m.end()) return -1;
	return i->second;
}

int		WED_PropertyHelper::FindProperty(const char * in_prop) const
{
	for (int n = 0; n < mItems.size(); ++n)
		if (strcmp(mItems[n]->mTitle, in_prop)==0) return n;
	return -1;
}

int		WED_PropertyHelper::CountProperties(void) const
{
	return mItems.size();
}

void		WED_PropertyHelper::GetNthPropertyInfo(int n, PropertyInfo_t& info) const
{
	mItems[n]->GetPropertyInfo(info);
}

void		WED_PropertyHelper::GetNthPropertyDict(int n, PropertyDict_t& dict) const
{
	mItems[n]->GetPropertyDict(dict);
}

void		WED_PropertyHelper::GetNthPropertyDictItem(int n, int e, string& item) const
{
	mItems[n]->GetPropertyDictItem(e, item);
}

void		WED_PropertyHelper::GetNthProperty(int n, PropertyVal_t& val) const
{
	mItems[n]->GetProperty(val);
}

void		WED_PropertyHelper::SetNthProperty(int n, const PropertyVal_t& val)
{
	mItems[n]->SetProperty(val,this);
}

WED_PropertyItem::WED_PropertyItem(WED_PropertyHelper * pops, const char * title, XML_Name xml_col) : mTitle(title), mXMLColumn(xml_col), mParent(pops)
{
	if (pops)
		pops->mItems.push_back(this);
}

void 		WED_PropertyHelper::ReadPropsFrom(IOReader * reader)
{
	for (int n = 0; n < mItems.size(); ++n)
		mItems[n]->ReadFrom(reader);
}

void 		WED_PropertyHelper::WritePropsTo(IOWriter * writer)
{
	for (int n = 0; n < mItems.size(); ++n)
		mItems[n]->WriteTo(writer);
}

void		WED_PropertyHelper::PropsToXML(WED_XMLElement * parent)
{
	for(int n = 0; n < mItems.size(); ++n)
		mItems[n]->ToXML(parent);
}


void		WED_PropertyHelper::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	int n;
	for(n = 0; n < mItems.size(); ++n)
	if(mItems[n]->WantsElement(reader,name))
		return;

	while(*atts)
	{
		const XML_Char * k = *atts++;
		const XML_Char * v = *atts++;
		for(n = 0; n < mItems.size(); ++n)
		if(mItems[n]->WantsAttribute(name,k,v))
			break;
	}		
}

void		WED_PropertyHelper::EndElement(void)
{
}

void		WED_PropertyHelper::PopHandler(void)
{
}

int			WED_PropertyHelper::PropertyItemNumber(const WED_PropertyItem * item) const
{
	for(int n = 0; n < mItems.size(); ++n)
		if(item == mItems[n]) return n;
	return -1;
}

#pragma mark -

void		WED_PropIntText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_Int;
	info.prop_name = mTitle;
	info.digits = mDigits;
	info.synthetic = 0;
}

void		WED_PropIntText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropIntText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropIntText::GetProperty(PropertyVal_t& val) const
{
	val.int_val = value;
	val.prop_kind = prop_Int;
}

void		WED_PropIntText::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_Int);
	if (value != val.int_val)
	{
		parent->PropEditCallback(1);
		value = val.int_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropIntText::ReadFrom(IOReader * reader)
{
	reader->ReadInt(value);
}

void 		WED_PropIntText::WriteTo(IOWriter * writer)
{
	writer->WriteInt(value);
}

void		WED_PropIntText::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_int(mXMLColumn.second,value);
}

bool		WED_PropIntText::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	if(strcasecmp(mXMLColumn.first,ele)==0)
	if(strcasecmp(mXMLColumn.second,att_name)==0)
	{
		value = atoi(att_value);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropBoolText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_Bool;
	info.prop_name = mTitle;
	info.synthetic = 0;
}

void		WED_PropBoolText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropBoolText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropBoolText::GetProperty(PropertyVal_t& val) const
{
	val.int_val = intlim(value,0,1);
	val.prop_kind = prop_Bool;
}

void		WED_PropBoolText::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_Bool);
	if (value != val.int_val)
	{
		parent->PropEditCallback(1);
		value = val.int_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropBoolText::ReadFrom(IOReader * reader)
{
	reader->ReadInt(value);
}

void 		WED_PropBoolText::WriteTo(IOWriter * writer)
{
	writer->WriteInt(value);
}

void		WED_PropBoolText::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_int(mXMLColumn.second,value);
}

bool		WED_PropBoolText::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	if(strcasecmp(mXMLColumn.first,ele)==0)
	if(strcasecmp(mXMLColumn.second,att_name)==0)
	{
		value = atoi(att_value);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropDoubleText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_Double;
	info.prop_name = mTitle;
	info.digits = mDigits;
	info.decimals = mDecimals;
	info.round_down = false;
	info.synthetic = 0;
	info.units = mUnit.c_str();
}

void		WED_PropDoubleText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropDoubleText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropDoubleText::GetProperty(PropertyVal_t& val) const
{
	val.double_val = value;
	val.prop_kind = prop_Double;
}

void		WED_PropDoubleText::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_Double);
	if (value !=  val.double_val)
	{
		parent->PropEditCallback(1);
		value = val.double_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropDoubleText::ReadFrom(IOReader * reader)
{
	reader->ReadDouble(value);
}

void 		WED_PropDoubleText::WriteTo(IOWriter * writer)
{
	writer->WriteDouble(value);
}

void		WED_PropDoubleText::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_double(mXMLColumn.second,value,mDecimals);
}

bool		WED_PropDoubleText::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	if(strcasecmp(mXMLColumn.first,ele)==0)
	if(strcasecmp(mXMLColumn.second,att_name)==0)
	{
		value = atof(att_value);
		return true;
	}
	return false;
}

void		WED_PropFrequencyText::GetPropertyInfo(PropertyInfo_t& info)
{
	WED_PropDoubleText::GetPropertyInfo(info);
	info.round_down = true;
}

int		WED_PropFrequencyText::GetAs10Khz(void) const
{
	// This is kind of a fuck-fest and some explanation is needed.  Unfortunately ATC frequencies are stored in decimal mhz
	// in WED's internal data model, so 123.125 might be 123.124999999999, and there might be other similar rounding crap.
	// We want to TRUNCATE the 1's digit of the khz frequency, e.g.
	// XP treats 123.125 and 123.12.  
	
	int freq_khz = round(this->value * 1000.0);
	return freq_khz / 10;	// Intentional floor - 123.125 -> 12312.
}

void	WED_PropFrequencyText::AssignFrom10Khz(int freq_10khz)
{
	double mhz = (double) freq_10khz / 100.0;
	*this = mhz;
}

void		WED_PropFrequencyText::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_double(mXMLColumn.second,value,mDecimals+1);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropDoubleTextMeters::GetProperty(PropertyVal_t& val) const
{
	WED_PropDoubleText::GetProperty(val);
	if(gIsFeet)val.double_val *= MTR_TO_FT;
}

void		WED_PropDoubleTextMeters::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	PropertyVal_t	ft_val(val);
	if(gIsFeet)ft_val.double_val *= FT_TO_MTR;
	WED_PropDoubleText::SetProperty(ft_val,parent);
}

void		WED_PropDoubleTextMeters::GetPropertyInfo(PropertyInfo_t& info)
{
	WED_PropDoubleText::GetPropertyInfo(info);
	info.units = gIsFeet ? "ft" : "m";
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropStringText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_String;
	info.prop_name = mTitle;
	info.synthetic = 0;
}

void		WED_PropStringText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropStringText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropStringText::GetProperty(PropertyVal_t& val) const
{
	val.string_val = value;
	val.prop_kind = prop_String;
}

void		WED_PropStringText::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_String);
	if (value != val.string_val)
	{
		parent->PropEditCallback(1);
		value = val.string_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropStringText::ReadFrom(IOReader * reader)
{
	int sz;
	reader->ReadInt(sz);
	vector<char> buf(sz);
	reader->ReadBulk(&*buf.begin(),sz,false);
	value = string(buf.begin(),buf.end());
}

void 		WED_PropStringText::WriteTo(IOWriter * writer)
{
	writer->WriteInt(value.size());
	writer->WriteBulk(value.c_str(),value.size(),false);
}

void		WED_PropStringText::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_stl_str(mXMLColumn.second,value);
}

bool		WED_PropStringText::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	if(strcasecmp(mXMLColumn.first,ele)==0)
	if(strcasecmp(mXMLColumn.second,att_name)==0)
	{
		value = att_value;
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropFileText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_FilePath;
	info.prop_name = mTitle;
	info.synthetic = 0;
}

void		WED_PropFileText::GetPropertyDict(PropertyDict_t& dict)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropFileText::GetPropertyDictItem(int e, string& item)
{
	DebugAssert(!"Illegal method.");
}

void		WED_PropFileText::GetProperty(PropertyVal_t& val) const
{
	val.string_val = value;
	val.prop_kind = prop_FilePath;
}

void		WED_PropFileText::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_FilePath);
	if (value != val.string_val)
	{
		parent->PropEditCallback(1);
		value = val.string_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropFileText::ReadFrom(IOReader * reader)
{
	int sz;
	reader->ReadInt(sz);
	vector<char> buf(sz);
	reader->ReadBulk(&*buf.begin(),sz,false);
	value = string(buf.begin(),buf.end());
}

void 		WED_PropFileText::WriteTo(IOWriter * writer)
{
	writer->WriteInt(value.size());
	writer->WriteBulk(value.c_str(),value.size(),false);
}

void		WED_PropFileText::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_stl_str(mXMLColumn.second,value);
}

bool		WED_PropFileText::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	if(strcasecmp(mXMLColumn.first,ele)==0)
	if(strcasecmp(mXMLColumn.second,att_name)==0)
	{
		value = att_value;
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropIntEnum::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_Enum;
	info.prop_name = mTitle;
	info.synthetic = 0;
}

void		WED_PropIntEnum::GetPropertyDict(PropertyDict_t& dict)
{	
	map<int, string>		dm;
	
	DOMAIN_Members(domain,dm);
	
	for(map<int, string>::iterator i = dm.begin(); i != dm.end(); ++i)
	dict.insert(PropertyDict_t::value_type(i->first, make_pair(i->second,true)));
}

void		WED_PropIntEnum::GetPropertyDictItem(int e, string& item)
{
	item = ENUM_Desc(e);
}

void		WED_PropIntEnum::GetProperty(PropertyVal_t& val) const
{
	val.prop_kind = prop_Enum;
	val.int_val = value;
}

void		WED_PropIntEnum::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_Enum);
	if (value != val.int_val)
	{
		if (ENUM_Domain(val.int_val) != domain) return;
		parent->PropEditCallback(1);
		value = val.int_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropIntEnum::ReadFrom(IOReader * reader)
{
	reader->ReadInt(value);
}

void 		WED_PropIntEnum::WriteTo(IOWriter * writer)
{
	writer->WriteInt(value);
}

void		WED_PropIntEnum::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_c_str(mXMLColumn.second,ENUM_Desc(value));
}

bool		WED_PropIntEnum::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	if(strcasecmp(mXMLColumn.first,ele)==0)
	if(strcasecmp(mXMLColumn.second,att_name)==0)
	{
		value = ENUM_LookupDesc(domain,att_value);
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropIntEnumSet::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_EnumSet;
	info.prop_name = mTitle;
	info.exclusive = this->exclusive;
	info.synthetic = 0;
}

void		WED_PropIntEnumSet::GetPropertyDict(PropertyDict_t& dict)
{
	map<int, string>		dm;
	
	DOMAIN_Members(domain,dm);
	
	for(map<int, string>::iterator i = dm.begin(); i != dm.end(); ++i)
	dict.insert(PropertyDict_t::value_type(i->first, make_pair(i->second,true)));
}

void		WED_PropIntEnumSet::GetPropertyDictItem(int e, string& item)
{
	item = ENUM_Desc(e);
}

void		WED_PropIntEnumSet::GetProperty(PropertyVal_t& val) const
{
	val.prop_kind = prop_EnumSet;
	val.set_val = value;
}

void		WED_PropIntEnumSet::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_EnumSet);
	if (value != val.set_val)
	{
		for (set<int>::const_iterator e = val.set_val.begin(); e != val.set_val.end(); ++e)
		if (ENUM_Domain(*e) != domain)
			return;
		parent->PropEditCallback(1);
		value = val.set_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropIntEnumSet::ReadFrom(IOReader * reader)
{
	int sz, ee;
	value.clear();
	reader->ReadInt(sz);
	while (sz--)
	{
		reader->ReadInt(ee);
		value.insert(ee);
	}
}

void 		WED_PropIntEnumSet::WriteTo(IOWriter * writer)
{
	writer->WriteInt(value.size());
	for (set<int>::iterator i = value.begin(); i != value.end(); ++i)
	{
		writer->WriteInt(*i);
	}
}

void		WED_PropIntEnumSet::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	for(set<int>::iterator i = value.begin(); i != value.end(); ++i)
	{
		WED_XMLElement * ele = xml->add_sub_element(mXMLColumn.second);
		ele->add_attr_c_str("value",ENUM_Desc(*i));
	}
}

bool		WED_PropIntEnumSet::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	return false;
}

bool		WED_PropIntEnumSet::WantsElement(WED_XMLReader * reader, const char * name)
{
	if(strcasecmp(name,mXMLColumn.first)==0)
	{
		reader->PushHandler(this);
		value.clear();	
		return true;
	}
	return false;
}

void		WED_PropIntEnumSet::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	if(strcasecmp(name,mXMLColumn.second) == 0)
	{
		const XML_Char * v = get_att("value", atts);
		if(!v) reader->FailWithError("no value");
		else 
		{ 
			int i = ENUM_LookupDesc(domain,v);
			// If asked to add enum of unknown name to set, just ignore it silently.
			// If we were to add "-1" to the set instead, the set would become un-modifyable and 'save file' would bomb.
			if (i >= 0)
				value.insert(i);
		}
	}
}
void		WED_PropIntEnumSet::EndElement(void){ }
void		WED_PropIntEnumSet::PopHandler(void){ }

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropIntEnumBitfield::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_delete = false;
	info.can_edit = 1;
	info.prop_kind = prop_EnumSet;
	info.prop_name = mTitle;
	info.exclusive = false;
	info.synthetic = 0;
}

void		WED_PropIntEnumBitfield::GetPropertyDict(PropertyDict_t& dict)
{
	map<int, string>		dm;
	
	DOMAIN_Members(domain,dm);
	
	for(map<int, string>::iterator i = dm.begin(); i != dm.end(); ++i)
	dict.insert(PropertyDict_t::value_type(i->first, make_pair(i->second,true)));
}

void		WED_PropIntEnumBitfield::GetPropertyDictItem(int e, string& item)
{
	item = ENUM_Desc(e);
}

void		WED_PropIntEnumBitfield::GetProperty(PropertyVal_t& val) const
{
	val.prop_kind = prop_EnumSet;
	val.set_val = value;
}

void		WED_PropIntEnumBitfield::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	DebugAssert(val.prop_kind == prop_EnumSet);
	if (value != val.set_val)
	{
		for (set<int>::const_iterator e = val.set_val.begin(); e != val.set_val.end(); ++e)
		if (ENUM_Domain(*e) != domain)
			return;
		if(!can_be_none && val.set_val.empty())
			return;
		parent->PropEditCallback(1);
		value = val.set_val;
		parent->PropEditCallback(0);
	}
}

void 		WED_PropIntEnumBitfield::ReadFrom(IOReader * reader)
{
	int sz, ee;
	value.clear();
	reader->ReadInt(sz);
	while (sz--)
	{
		reader->ReadInt(ee);
		value.insert(ee);
	}
}

void 		WED_PropIntEnumBitfield::WriteTo(IOWriter * writer)
{
	writer->WriteInt(value.size());
	for (set<int>::iterator i = value.begin(); i != value.end(); ++i)
	{
		writer->WriteInt(*i);
	}
}

void		WED_PropIntEnumBitfield::ToXML(WED_XMLElement * parent)
{
	WED_XMLElement * xml = parent->add_or_find_sub_element(mXMLColumn.first);
	xml->add_attr_int(mXMLColumn.second,ENUM_ExportSet(value));
}

bool		WED_PropIntEnumBitfield::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	if(strcasecmp(mXMLColumn.first,ele)==0)
	if(strcasecmp(mXMLColumn.second,att_name)==0)
	{
		int bf = atoi(att_value);
		ENUM_ImportSet(domain, bf, value);		
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropIntEnumSetFilter::GetPropertyInfo(PropertyInfo_t& info)
{
	int me = mParent->FindProperty(host);
	mParent->GetNthPropertyInfo(me, info);
	info.prop_name = mTitle;
	info.exclusive = exclusive;
	info.synthetic = 1;
}

void		WED_PropIntEnumSetFilter::GetPropertyDict(PropertyDict_t& dict)
{
	int me = mParent->FindProperty(host);
	PropertyDict_t	d;
	mParent->GetNthPropertyDict(me,d);
	for (PropertyDict_t::iterator i = d.begin(); i != d.end(); ++i)
	if (i->first >= minv && i->first <= maxv)
		dict.insert(PropertyDict_t::value_type(i->first,i->second));
}

void		WED_PropIntEnumSetFilter::GetPropertyDictItem(int e, string& item)
{
	int me = mParent->FindProperty(host);
	mParent->GetNthPropertyDictItem(me, e,item);
}

void		WED_PropIntEnumSetFilter::GetProperty(PropertyVal_t& val) const
{
	int me = mParent->FindProperty(host);
	PropertyVal_t	local;
	mParent->GetNthProperty(me,local);
	val = local;
	val.set_val.clear();
	for(set<int>::iterator i = local.set_val.begin(); i != local.set_val.end(); ++i)
	if (*i >= minv && *i <= maxv)
		val.set_val.insert(*i);

}

void		WED_PropIntEnumSetFilter::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	int me = mParent->FindProperty(host);
	PropertyVal_t	clone(val), old;
	clone.set_val.clear();
	set<int>::const_iterator i;
	mParent->GetNthProperty(me, old);
	for(i=old.set_val.begin();i!=old.set_val.end();++i)
	if(*i < minv || *i > maxv)
		clone.set_val.insert(*i);
	for(i=val.set_val.begin();i!=val.set_val.end();++i)
	if(*i >= minv && *i <= maxv)
		clone.set_val.insert(*i);
	mParent->SetNthProperty(me,clone);
}

void 		WED_PropIntEnumSetFilter::ReadFrom(IOReader * reader)
{
}

void 		WED_PropIntEnumSetFilter::WriteTo(IOWriter * writer)
{
}

void		WED_PropIntEnumSetFilter::ToXML(WED_XMLElement * parent)
{
}

bool		WED_PropIntEnumSetFilter::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	return false;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void		WED_PropIntEnumSetUnion::GetPropertyInfo(PropertyInfo_t& info)
{
	info.prop_name = host;
	info.prop_kind = prop_EnumSet;
	info.can_delete = false;
	info.can_edit = 1;
	info.exclusive = this->exclusive;
	info.synthetic = 1;
}

void		WED_PropIntEnumSetUnion::GetPropertyDict(PropertyDict_t& dict)
{
	int nn = mParent->CountSubs();
	for (int n = 0; n < nn; ++n)
	{
		IPropertyObject * inf = mParent->GetNthSub(n);
		if (inf)
		{
			int idx = inf->FindProperty(host);
			if (idx != -1)
			{
				inf->GetNthPropertyDict(idx, dict);
				return;
			}
		}
	}
}

void		WED_PropIntEnumSetUnion::GetPropertyDictItem(int e, string& item)
{
	item = ENUM_Desc(e);
}

void		WED_PropIntEnumSetUnion::GetProperty(PropertyVal_t& val) const
{
	val.prop_kind = prop_EnumSet;
	val.set_val.clear();
	int nn = mParent->CountSubs();
	for (int n = 0; n < nn; ++n)
	{
		IPropertyObject * inf = mParent->GetNthSub(n);
		if (inf)
		{
			PropertyVal_t	local;
			int idx = inf->FindProperty(host);
			if (idx != -1)
			{
				inf->GetNthProperty(idx, local);
				copy(local.set_val.begin(), local.set_val.end(), set_inserter(val.set_val));
			}
		}
	}
}

void		WED_PropIntEnumSetUnion::SetProperty(const PropertyVal_t& val, WED_PropertyHelper * parent)
{
	PropertyVal_t	old_val;
	this->GetProperty(old_val);

	set<int>	added, deleted;
	set_difference(val.set_val.begin(),val.set_val.end(),
					old_val.set_val.begin(),old_val.set_val.end(),
					set_inserter(added));

	set_difference(old_val.set_val.begin(),old_val.set_val.end(),
					val.set_val.begin(),val.set_val.end(),
					set_inserter(deleted));

	int nn = mParent->CountSubs();
	for (int n = 0; n < nn; ++n)
	{
		IPropertyObject * inf = mParent->GetNthSub(n);
		if (inf)
		{
			int idx = inf->FindProperty(host);
			if (idx != -1)
			{
				if (exclusive)
				{
					inf->SetNthProperty(idx, val);
				}
				else
				{
					PropertyVal_t	local, new_val;
					inf->GetNthProperty(idx, local);
					new_val = local;
					copy(added.begin(),added.end(),set_inserter(local.set_val));
					copy(deleted.begin(),deleted.end(),set_eraser(local.set_val));
					inf->SetNthProperty(idx,local);
				}
			}
		}
	}
}

void 		WED_PropIntEnumSetUnion::ReadFrom(IOReader * reader)
{
}

void 		WED_PropIntEnumSetUnion::WriteTo(IOWriter * writer)
{
}

void		WED_PropIntEnumSetUnion::ToXML(WED_XMLElement * parent)
{
}

bool		WED_PropIntEnumSetUnion::WantsAttribute(const char * ele, const char * att_name, const char * att_value)
{
	return false;
}
