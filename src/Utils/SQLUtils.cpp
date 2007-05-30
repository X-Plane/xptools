#include "SQLUtils.h"
#include "AssertUtils.h"

sql_db::sql_db(const char * in_filename)
{
	int	result = sqlite3_open(in_filename, &db);
	if (result != SQLITE_OK)
	{
		sqlite3_close(db);
		AssertPrintf("Unable to open file %s: %d\n", in_filename, result);
	}
}

sql_db::~sql_db() 
{ 
	int result = 	sqlite3_close(db);
	DebugAssert(result == SQLITE_OK);
}

sqlite3 * sql_db::get(void) { return db; }

int sql_do_bulk(sqlite3 * db, const char * sql)
{
	char * err;
	int result = sqlite3_exec(db, sql, NULL, NULL, &err);
	if (result != SQLITE_OK)
		DoUserAlert(err);
	return result;
}

int sql_do_bulk_range(sqlite3 * db, const char * sql_begin, const char * sql_end)
{
	string buf(sql_begin, sql_end);
	return sql_do_bulk(db,buf.c_str());
}

static int strptr_cb(void*ref,int col_count,char** col_data, char** col_names)
{
	if (col_data[0])
	*((string *) ref) = col_data[0];
	else
		((string *) ref)->clear();
	return 0;
}


int sql_do_hack(sqlite3 * db, const char * sql, string * out_result)
{
	char * errmsg;
	int e = sqlite3_exec(db, sql, strptr_cb, out_result, &errmsg);
	if (e != SQLITE_OK && out_result) *out_result = errmsg;
	return e;
}


sql_command::sql_command(sqlite3 * db, const char * cmd,const char * params)
{
	int n = 0;
	if(params)
	{
		const char * s = params;
		while(*s != 0)
		{
			const char * e = s;
			while(*e != 0 && *e != ',') ++e;
			param_name.push_back(string(s,e));
			s = e;
			if (*s == ',') ++s;
		}
	}
	
	param_index.resize(param_name.size());
	
	const char * tail;
	int result = sqlite3_prepare_v2(db, cmd, -1, &stmt, &tail);
	if (result != SQLITE_OK)
	{
		const char * e = sqlite3_errmsg(db);
		DoUserAlert(e);
		throw result;
		#if ERROR_CHECK
		this sucks
		#endif
	}
	
	for (int n = 0; n < param_name.size(); ++n)
	{
		param_index[n] = sqlite3_bind_parameter_index(stmt, param_name[n].c_str());
		DebugAssert(param_index[n] != 0);
	}
}

sql_command::~sql_command()
{
	int result = sqlite3_finalize(stmt);
	DebugAssert(result == SQLITE_OK);
}

void sql_command::begin(void)
{
	int result = sqlite3_reset(stmt);
	DebugAssert(result == SQLITE_OK);
}

int sql_do(sqlite3 * db, const char * sql) 
{
	sql_command c(db,sql, NULL); 
	return c.simple_exec(); 
}
