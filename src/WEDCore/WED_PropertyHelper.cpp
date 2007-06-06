#include "WED_PropertyHelper.h"
#include "AssertUtils.h"
#include "WED_Errors.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_EnumSystem.h"

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

void 		WED_PropertyHelper::PropsFromDB(sqlite3 * db, const char * where_clause)
{
	for (int n = 0; n < mItems.size(); ++n)
		mItems[n]->FromDB(db,where_clause);
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
		if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd.c_str(), err, sqlite3_errmsg(db));
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

void		WED_PropIntText::GetProperty(PropertyVal_t& val)
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

void		WED_PropIntText::FromDB(sqlite3 * db, const char * where_clause)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0		k;
	sql_row1<int>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, err, sqlite3_errmsg(db));
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

void		WED_PropBoolText::GetProperty(PropertyVal_t& val)
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

void		WED_PropBoolText::FromDB(sqlite3 * db, const char * where_clause)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0		k;
	sql_row1<int>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, err, sqlite3_errmsg(db));
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

void		WED_PropDoubleText::GetProperty(PropertyVal_t& val)
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

void		WED_PropDoubleText::FromDB(sqlite3 * db, const char * where_clause)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<double>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, err, sqlite3_errmsg(db));
	value = v.a;	
}

void		WED_PropDoubleText::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropDoubleText::GetUpdate(SQL_Update& io_update)
{
	char as_double[1024];
	sprintf(as_double,"%lf", value);
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, as_double));
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


void		WED_PropStringText::FromDB(sqlite3 * db, const char * where_clause)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<string>	v;	
	int err = cmd.simple_exec(k,v);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, err, sqlite3_errmsg(db));
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

void		WED_PropIntEnum::GetProperty(PropertyVal_t& val)
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

void		WED_PropIntEnum::FromDB(sqlite3 * db, const char * where_clause)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<string>	v;	
	int err = cmd.simple_exec(k,v);	
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, err, sqlite3_errmsg(db));
	
	value = ENUM_Lookup(v.a.c_str());
	DebugAssert(ENUM_Domain(value)==domain);
}

void		WED_PropIntEnum::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
}

void		WED_PropIntEnum::GetUpdate(SQL_Update& io_update)
{
	string quoted("'");
	quoted += ENUM_Fetch(value);
	quoted += '\'';
	io_update[mTable].push_back(SQL_ColumnUpdate(mColumn, quoted));
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

void		WED_PropIntEnumSet::GetProperty(PropertyVal_t& val)
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

void		WED_PropIntEnumSet::FromDB(sqlite3 * db, const char * where_clause)
{
	char cmd_buf[1000];
	sprintf(cmd_buf,"SELECT %s FROM %s WHERE %s;",mColumn,mTable, where_clause);
	sql_command cmd(db, cmd_buf,NULL);
	sql_row0			k;
	sql_row1<string>	v;	
	
	value.clear();
	cmd.begin();
	int rc;
	do {
		rc = cmd.get_row(v);
		if (rc == SQLITE_ROW)
		{
			value.insert(ENUM_Lookup(v.a.c_str()));		
			DebugAssert(ENUM_Domain(ENUM_Lookup(v.a.c_str()))==domain);
		}
	} while (rc == SQLITE_ROW); 
	
	if (rc != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, rc, sqlite3_errmsg(db));
}

void		WED_PropIntEnumSet::ToDB(sqlite3 * db, const char * id_col, const char * id_val)
{
	char cmd_buf[1000];

	{
		sprintf(cmd_buf,"DELETE FROM %s WHERE %s=%s;",mTable, id_col,id_val);
		sql_command cmd(db,cmd_buf,NULL);
		int err = cmd.simple_exec();
		if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, err, sqlite3_errmsg(db));
	}
	
	if (!value.empty())
	{
		#if OPTIMIZE
			Ben says - I could not get an insert to work with variable bindings.  Reparsing the statement is real inefficient.
		#endif
		for (set<int>::iterator i = value.begin(); i != value.end(); ++i)
		{
			sprintf(cmd_buf, "INSERT INTO %s (%s,%s) VALUES(%s,@e);", mTable, id_col, mColumn, id_val);
			sql_command cmd2(db,cmd_buf,"@e");
			
			sql_row1<string>	p;

			p.a = ENUM_Fetch(*i);
			int err = cmd2.simple_exec(p);
			if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete query '%s': %d (%s)",cmd_buf, err, sqlite3_errmsg(db));
		}
	}		
}

void		WED_PropIntEnumSet::GetUpdate(SQL_Update& io_update)
{
}
