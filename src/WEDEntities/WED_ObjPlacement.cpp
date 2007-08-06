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

#include "WED_ObjPlacement.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_ObjPlacement)

WED_ObjPlacement::WED_ObjPlacement(WED_Archive * a, int id) : WED_GISPoint_Heading(a,id)
{
	model_id = 0;
}

WED_ObjPlacement::~WED_ObjPlacement()
{
}

void WED_ObjPlacement::CopyFrom(const WED_ObjPlacement * rhs)
{
	WED_GISPoint_Heading::CopyFrom(rhs);
	StateChanged();
	model_id = rhs->model_id;
}

void 			WED_ObjPlacement::ReadFrom(IOReader * reader)
{
	WED_GISPoint_Heading::ReadFrom(reader);
	reader->ReadInt(model_id);
}

void 			WED_ObjPlacement::WriteTo(IOWriter * writer)
{
	WED_GISPoint_Heading::WriteTo(writer);
	writer->WriteInt(model_id);
}

void			WED_ObjPlacement::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
	WED_GISPoint_Heading::FromDB(db, mapping);
	sql_command	cmd(db,"SELECT model_id FROM WED_objects WHERE id=@i;","@i");
	
	sql_row1<int>						key(GetID());
	sql_row1<int>						me;
	
	int err = cmd.simple_exec(key, me);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);

	model_id = me.a;
	
}

void			WED_ObjPlacement::ToDB(sqlite3 * db)
{
	WED_GISPoint_Heading::ToDB(db);
	int err;
	sql_command	write_me(db,"INSERT OR REPLACE INTO WED_objects VALUES(@id,@m);",
								"@id,@m");
	sql_row2 <int,int>bindings(GetID(),model_id);
	err = write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
}
