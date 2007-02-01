#include "WED_Entity.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_Entity)
INHERITS_FROM(WED_Thing)
END_CASTING


WED_Entity::WED_Entity(WED_Archive * parent, int id) :
	WED_Thing(parent, id),
	locked(this,"locked",0),
	hidden(this,"hidden",0)
{
}

WED_Entity::~WED_Entity()
{
}

void 			WED_Entity::ReadFrom(IOReader * reader)
{
	WED_Thing::ReadFrom(reader);
	reader->ReadInt(locked);
	reader->ReadInt(hidden);
}

void 			WED_Entity::WriteTo(IOWriter * writer)
{
	WED_Thing::WriteTo(writer);
	writer->WriteInt(locked);
	writer->WriteInt(hidden);
}

void			WED_Entity::FromDB(sqlite3 * db)
{
	WED_Thing::FromDB(db);
	sql_command	cmd(db,"SELECT locked,hidden FROM WED_entities WHERE id=@i;","@i");
	
	sql_row1<int>						key(GetID());
	sql_row2<int,int>					me;
	
	int err = cmd.simple_exec(key, me);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete entity query: %d (%s)",err, sqlite3_errmsg(db));
	
	locked = me.a;
	hidden = me.b;	
}

void			WED_Entity::ToDB(sqlite3 * db)
{
	WED_Thing::ToDB(db);
	int err;
	sql_command	write_me(db,"UPDATE WED_entities set locked=@l, hidden=@h WHERE id=@id;", "@l,@h,@id");
	sql_row3 <int,int,int>bindings(locked,hidden,GetID());
	err = write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update entity info: %d (%s)",err, sqlite3_errmsg(db));	
}

