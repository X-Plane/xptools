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

void			WED_ObjPlacement::FromDB(sqlite3 * db)
{
	WED_GISPoint_Heading::FromDB(db);
	sql_command	cmd(db,"SELECT model_id FROM WED_objects WHERE id=@i;","@i");
	
	sql_row1<int>						key(GetID());
	sql_row1<int>						me;
	
	int err = cmd.simple_exec(key, me);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete entity query: %d (%s)",err, sqlite3_errmsg(db));

	model_id = me.a;
	
}

void			WED_ObjPlacement::ToDB(sqlite3 * db)
{
	WED_GISPoint_Heading::ToDB(db);
	int err;
	sql_command	write_me(db,"UPDATE WED_objects set model_id=@m WHERE id=@id;",
								"@m,@id");
	sql_row2 <int,int>bindings(model_id,GetID());
	err = write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update entity info: %d (%s)",err, sqlite3_errmsg(db));	
}
