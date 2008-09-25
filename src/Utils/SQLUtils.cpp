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

#include "SQLUtils.h"
#include "AssertUtils.h"
#include "WED_Errors.h"
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
	WED_ThrowOSErr(result);

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
