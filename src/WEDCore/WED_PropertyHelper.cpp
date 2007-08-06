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
#include "SQLUtils.h"
#include "XESConstants.h"
#include "WED_EnumSystem.h"
#include <algorithm>

int gIsFeet = 0;
extern int gExclusion;

inline int remap(const map<int,int>& m, int v)
{
	map<int,int>::const_iterator i = m.find(v);
	if (i == m.end()) return -1;
	return i->second;
}

int		WED_PropertyHelper::FindProperty(const char * in_prop)
{
	for (int n = 0; n < mItems.size(); ++n)
		if (strcmp(mItems[n]->mTitle, in_prop)==0) return n;
	return -1;
}

int		WED_PropertyHelper::CountProperties(void) const
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

void		WED_PropertyHelper::GetNthProperty(int n, PropertyVal_t& val) const
{
	mItems[n]->GetProperty(val);
}

void		WED_PropertyHelper::SetNthProperty(int n, const PropertyVal_t& val)
{
	mItems[n]->SetProperty(val,this);
}

WED_PropertyItem::WED_PropertyItem(WED_PropertyHelper * pops, const char * title, const char * table, const char * column) : mTitle(title), mTable(table), mColumn(column), mParent(pops)
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

void 		WED_PropertyHelper::PropsFromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	for (int n = 0; n < mItems.size(); ++n)
		mItems[n]->FromDB(db,where_clause, mapping);
}

void 		WED_PropertyHelper::PropsToDB(sqlite3 * db, const char * id_col, const char * id_val, const char * skip_table)
{
	SQL_Update update;
	for (int n = 0; n < mItems.size(); ++n)
	{
		mItems[n]->ToDB(db,id_col, id_val);
		mItems[n]->GetUpdate(update);
	}
	
	string skip(skip_table ? skip_table : "");
	
	for (SQL_Update::iterator table = update.begin(); table != update.end(); ++table)
	{
		if (table->first == skip) continue;
		string cols = id_col;
		string vals = id_val;
		for (SQL_TableUpdate::iterator col = table->second.begin(); col != table->second.end(); ++col)
		{
			cols += ",";
			cols += col->first;
			vals += ",";
			vals += col->second;
		}
		
		string cmd = string("INSERT OR REPLACE INTO ") + table->first + 
					 string("(") + cols +
					 string(") VALUES(") +
					 vals + ");";
		
		sql_command write_to_table(db, cmd.c_str(), NULL);
		int err = write_to_table.simple_exec();
		if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------
#pragma mark -
//----------------------------------------------------------------------------------------------------------------------------------------------------------------


void		WED_PropIntText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_Int;
	info.prop_name = mTitle;
	info.digits = mDigits;
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

void		WED_PropIntText::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0		k;
	sql_row1<int>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	value = v.a;	
}

void		WED_PropIntText::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropIntText::GetUpdate(SQL_Update& io_update)
{
	char as_int[1024];
	sprintf(as_int,"%d", value);
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, as_int));
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

void		WED_PropBoolText::GetProperty(PropertyVal_t& val) const
{
	val.int_val = value;
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

void		WED_PropBoolText::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0		k;
	sql_row1<int>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	value = v.a;	
}

void		WED_PropBoolText::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropBoolText::GetUpdate(SQL_Update& io_update)
{
	char as_int[1024];
	sprintf(as_int,"%d", value);
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, as_int));
}


//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropDoubleText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_Double;
	info.prop_name = mTitle;
	info.digits = mDigits;
	info.decimals = mDecimals;
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

void		WED_PropDoubleText::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<double>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	value = v.a;	
}

void		WED_PropDoubleText::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropDoubleText::GetUpdate(SQL_Update& io_update)
{
	char as_double[1024];
	sprintf(as_double,"%.10lf", value);
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, as_double));
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


void		WED_PropStringText::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<string>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	value = v.a;	
}

void		WED_PropStringText::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropStringText::GetUpdate(SQL_Update& io_update)
{
	string quoted("'");
	quoted += value;
	quoted += '\'';
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, quoted));
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropFileText::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_FilePath;
	info.prop_name = mTitle;
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


void		WED_PropFileText::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<string>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	value = v.a;	
}

void		WED_PropFileText::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropFileText::GetUpdate(SQL_Update& io_update)
{
	string quoted("'");
	quoted += value;
	quoted += '\'';
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, quoted));
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropIntEnum::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_Enum;
	info.prop_name = mTitle;
}

void		WED_PropIntEnum::GetPropertyDict(PropertyDict_t& dict)
{
	DOMAIN_Members(domain,dict);
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

void		WED_PropIntEnum::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<int>		v;	
	int err = cmd.simple_exec(k,v);	
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	
	value = remap(mapping,v.a);
	DebugAssert(value != -1);
	DebugAssert(ENUM_Domain(value)==domain);
}

void		WED_PropIntEnum::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropIntEnum::GetUpdate(SQL_Update& io_update)
{
	char as_int[1024];
	sprintf(as_int,"%d", value);
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, as_int));
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void		WED_PropIntEnumSet::GetPropertyInfo(PropertyInfo_t& info)
{
	info.can_edit = 1;
	info.prop_kind = prop_EnumSet;
	info.prop_name = mTitle;
}

void		WED_PropIntEnumSet::GetPropertyDict(PropertyDict_t& dict)
{
	DOMAIN_Members(domain,dict);
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

void		WED_PropIntEnumSet::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<int>		v;	
	
	value.clear();
	cmd.begin();
	int rc;
	do {
		rc = cmd.get_row(v);
		if (rc == SQLITE_ROW)
		{
			v.a = remap(mapping, v.a);
			DebugAssert(v.a != -1);
			value.insert(v.a);		
			DebugAssert(ENUM_Domain(v.a)==domain);
		}
	} while (rc == SQLITE_ROW); 
	
	if (rc != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),rc);
}

void		WED_PropIntEnumSet::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
	char cmd_buf[1000];

	{
		sprintf(cmd_buf,"DELETE FROM %s WHERE %s=%s;",mTable, id_col,id_val);
		sql_command cmd(db,cmd_buf,NULL);
		int err = cmd.simple_exec();
		if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}
	
	if (!value.empty())
	{
		sprintf(cmd_buf, "INSERT INTO %s (%s,%s) VALUES(%s,@e);", mTable, id_col, mColumn, id_val);
		sql_command cmd2(db,cmd_buf,"@e");
		for (set<int>::iterator i = value.begin(); i != value.end(); ++i)
		{			
			sql_row1<int>	p;

			p.a = *i;
			int err = cmd2.simple_exec(p);
			if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
		}
	}		
}

void		WED_PropIntEnumSet::GetUpdate(SQL_Update& io_update)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void		WED_PropIntEnumSetFilter::GetPropertyInfo(PropertyInfo_t& info)
{
	int me = mParent->FindProperty(host);
	mParent->GetNthPropertyInfo(me, info);
	info.prop_name = mTitle;	
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

void		WED_PropIntEnumSetFilter::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
}

void		WED_PropIntEnumSetFilter::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropIntEnumSetFilter::GetUpdate(SQL_Update& io_update)
{
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void		WED_PropIntEnumSetUnion::GetPropertyInfo(PropertyInfo_t& info)
{
	info.prop_name = host;
	info.prop_kind = prop_EnumSet;
	info.can_edit = 1;
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
				if (gExclusion)
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

void		WED_PropIntEnumSetUnion::FromDB(sqlite3 * db, const char * where_clause, const map<int,int>& mapping)
{
}

void		WED_PropIntEnumSetUnion::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropIntEnumSetUnion::GetUpdate(SQL_Update& io_update)
{
}

