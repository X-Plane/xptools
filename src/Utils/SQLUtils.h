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

#ifndef SQLUTILS_H
#define SQLUTILS_H

#include "PlatformUtils.h"
#include "AssertUtils.h"
#include <sqlite3.h>
#include <string>
#include <vector>

using std::string;
using std::vector;

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// OVERLOADED COERSION FUNCTIONS
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// These routines bind params and fetch columns from a statement with type coersions.  If you provide more variants you can use new C++ types with sql_rowN.

inline void		sql_fetch_column(sqlite3_stmt * s, int icol, int& result)			{ result = sqlite3_column_int(s,icol);												}
inline void		sql_fetch_column(sqlite3_stmt * s, int icol, double& result)		{ result = sqlite3_column_double(s,icol);											}
inline void		sql_fetch_column(sqlite3_stmt * s, int icol, string& result)		{ result = (char*)sqlite3_column_text(s, icol);										}

inline int		sql_bind_param(sqlite3_stmt * s, int iparam, int value)				{ return sqlite3_bind_int(s,iparam,value);											}
inline int		sql_bind_param(sqlite3_stmt * s, int iparam, double value)			{ return sqlite3_bind_double(s,iparam,value);										}
inline int		sql_bind_param(sqlite3_stmt * s, int iparam, const string& value)	{ return sqlite3_bind_text(s,iparam,value.c_str(),value.size(), SQLITE_TRANSIENT);	}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// TEMPLATED TYPESAFE ROWS
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// sql_rowN is a template around N datatypes for a single "row" of data.  This struct is used both
// to bind parameters (an input row, sort of) and to get out data.
//
// The method sql_bind binds to the statement in order.
// The method sql_fetch pulls out the current row from the statement.

struct	sql_row0 {
	void	sql_bind(sqlite3_stmt * s, int * swizzle) const {
	}
	void	sql_fetch(sqlite3_stmt * s) {
	}
};

template <typename A>
struct	sql_row1 {
	sql_row1() { }
	sql_row1(const A& ia) : a(ia) { }
	A		a;
	void	sql_bind(sqlite3_stmt * s, int * swizzle) const {
		sql_bind_param(s,swizzle[0],a);
	}
	void	sql_fetch(sqlite3_stmt * s) {
		sql_fetch_column(s,0,a);
	}
};

template <typename A, typename B>
struct	sql_row2 {
	sql_row2() { }
	sql_row2(const A& ia, const B& ib) : a(ia), b(ib) { }
	A		a;
	B		b;
	void	sql_bind(sqlite3_stmt * s, int * swizzle) const {
		sql_bind_param(s,swizzle[0],a);
		sql_bind_param(s,swizzle[1],b);
	}
	void	sql_fetch(sqlite3_stmt * s) {
		sql_fetch_column(s,0,a);
		sql_fetch_column(s,1,b);
	}
};

template <typename A, typename B, typename C>
struct	sql_row3 {
	sql_row3() { }
	sql_row3(const A& ia, const B& ib, const C& ic) : a(ia), b(ib), c(ic) { }
	A		a;
	B		b;
	C		c;
	void	sql_bind(sqlite3_stmt * s, int * swizzle) const {
		sql_bind_param(s,swizzle[0],a);
		sql_bind_param(s,swizzle[1],b);
		sql_bind_param(s,swizzle[2],c);
	}
	void	sql_fetch(sqlite3_stmt * s) {
		sql_fetch_column(s,0,a);
		sql_fetch_column(s,1,b);
		sql_fetch_column(s,2,c);
	}
};

template <typename A, typename B, typename C, typename D>
struct	sql_row4 {
	sql_row4() { }
	sql_row4(const A& ia, const B& ib, const C& ic, const D& id) : a(ia), b(ib), c(ic), d(id) { }
	A		a;
	B		b;
	C		c;
	D		d;
	void	sql_bind(sqlite3_stmt * s, int * swizzle) const {
		sql_bind_param(s,swizzle[0],a);
		sql_bind_param(s,swizzle[1],b);
		sql_bind_param(s,swizzle[2],c);
		sql_bind_param(s,swizzle[3],d);
	}
	void	sql_fetch(sqlite3_stmt * s) {
		sql_fetch_column(s,0,a);
		sql_fetch_column(s,1,b);
		sql_fetch_column(s,2,c);
		sql_fetch_column(s,3,d);
	}
};

template <typename A, typename B, typename C, typename D, typename E>
struct	sql_row5 {
	sql_row5() { }
	sql_row5(const A& ia, const B& ib, const C& ic, const D& id, const E& ie) : a(ia), b(ib), c(ic), d(id), e(ie) { }
	A		a;
	B		b;
	C		c;
	D		d;
	E		e;
	void	sql_bind(sqlite3_stmt * s, int * swizzle) const {
		sql_bind_param(s,swizzle[0],a);
		sql_bind_param(s,swizzle[1],b);
		sql_bind_param(s,swizzle[2],c);
		sql_bind_param(s,swizzle[3],d);
		sql_bind_param(s,swizzle[4],e);
	}
	void	sql_fetch(sqlite3_stmt * s) {
		sql_fetch_column(s,0,a);
		sql_fetch_column(s,1,b);
		sql_fetch_column(s,2,c);
		sql_fetch_column(s,3,d);
		sql_fetch_column(s,4,e);
	}
};


//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// TEMPLATED SQL COMMANDS
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//	This object represents a SQL command.  The command is compiled once and persisted inside the object.
//	cmd must be a SQL command that can be ENTIRELY consumed by the SQL parser - that is, a single statement.
//	params is a comma-separated list of any params that must be keyed.  When binding the params, the order
//	they are bound from comes from "params", NOT the order in the SQL command and NOT the order that sqlite3
//	internally builds up.
//
//	constructor - compiles the command.
//	destructor - releases the command.
//	set_params - binds parameter fill-in values.
//	begin - starts the query
//	get_row - runs the query enough to get one row.  It will return SQLITE_DONE when done and SQLITE_ROW when data is avail.
//	simple_exec - runs the entire query at once returning the last row (if multiple rows are available).
//	simple_exec is overloaded to let you not pass a row, or not pass bindings, or both.

class	sql_command {
public:
 	 sql_command(sqlite3 * db, const char * cmd,const char * params);
	~sql_command();

	template <typename B>
	void set_params(const B& params)
	{
		params.sql_bind(stmt,&*param_index.begin());
	}

	void begin(void);

	template <typename R>
	int get_row(R& row)
	{
		int result = sqlite3_step(stmt);
		if(result == SQLITE_ROW)
			row.sql_fetch(stmt);
		return result;
	}

	template <typename B, typename R>
	int simple_exec(const B& binding, R& row)
	{
		begin();
		set_params(binding);
		int rc;
		do {
			rc = get_row(row);
		} while (rc == SQLITE_ROW);
		return rc;
	}

	template <typename B>
	int simple_exec(const B& binding)
	{
		sql_row0 r;
		return simple_exec(binding,r);
	}

	int simple_exec(void)
	{
		sql_row0 b, r;
		return simple_exec(b,r);
	}

private:
	sqlite3_stmt *	stmt;
	vector<string>	param_name;
	vector<int>		param_index;
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// OTHER SQL COMMAND INTERFACES
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// A few other interfaces to run SQL commands.

// This does one simple sqlite command using the command obj.
int sql_do(sqlite3 * db, const char * sql);

// These routines execute one or more SQL statements consecutively until an error
// is found.  They cannot return data.  They compile on the fly.  These are good
// for initialization code.
int sql_do_bulk(sqlite3 * db, const char * sql);
int sql_do_bulk_range(sqlite3 * db, const char * sql_begin, const char * sql_end);


// Avoid this - this is a very slow wrapper around sql_exec with a callback.
int sql_do_hack(sqlite3 * db, const char * sql, string * out_result);


class sql_init {
public:
	sql_init(sqlite3 * db, const char * sql) { sql_do(db,sql); }
};

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// DATABASE C++ WRAPPER
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// This is a simple C++ wrapper around a sqlite3 DB.  This is useful when you want the database to match C++ object lifetime.

class	sql_db {
public:
	 sql_db(const char * in_filename);
	~sql_db();

	sqlite3 * get(void);
private:

	sqlite3	*	db;

	sql_db();
	sql_db(const sql_db&);
	sql_db& operator=(const sql_db&);
};

inline string sqlite3_quote_string(const string& v)
{
	string quoted = "'";
	for(string::const_iterator c = v.begin(); c != v.end(); ++c)
	{
		if(*c=='\'')
			quoted.push_back(*c);
		quoted.push_back(*c);
	}
	quoted += '\'';
	return quoted;
}



#endif /* SQLUTILS_H */
