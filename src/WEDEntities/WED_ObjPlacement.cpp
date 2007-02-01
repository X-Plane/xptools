#include "WED_ObjPlacement.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

WED_ObjPlacement::WED_ObjPlacement(WED_Archive * a, int id) : WED_Entity(a,id), 
	latitude(this,"longtitude",0.0),
	longitude(this,"latitude",0.0),
	heading(this,"heading",0)
{
	model_id = 0;
}

WED_ObjPlacement::~WED_ObjPlacement()
{
}

DEFINE_PERSISTENT(WED_ObjPlacement)
INHERITS_FROM(WED_Entity)
END_CASTING


void 			WED_ObjPlacement::ReadFrom(IOReader * reader)
{
	WED_Entity::ReadFrom(reader);
	reader->ReadDouble(longitude);
	reader->ReadDouble(latitude);
	reader->ReadDouble(heading);
	reader->ReadInt(model_id);
}

void 			WED_ObjPlacement::WriteTo(IOWriter * writer)
{
	WED_Entity::WriteTo(writer);
	writer->WriteDouble(longitude);
	writer->WriteDouble(latitude);
	writer->WriteDouble(heading);
	writer->WriteInt(model_id);
}

void			WED_ObjPlacement::FromDB(sqlite3 * db)
{
	WED_Entity::FromDB(db);
	sql_command	cmd(db,"SELECT latitude,longitude,rotation,model_id FROM WED_objects WHERE id=@i;","@i");
	
	sql_row1<int>						key(GetID());
	sql_row4<double,double,double,int>	me;
	
	int err = cmd.simple_exec(key, me);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete entity query: %d (%s)",err, sqlite3_errmsg(db));

	latitude = me.a;
	longitude = me.b;
	heading = me.c;
	model_id = me.d;
	
}

void			WED_ObjPlacement::ToDB(sqlite3 * db)
{
	WED_Entity::ToDB(db);
	int err;
	sql_command	write_me(db,"UPDATE WED_objects set latitude=@la,longitude=@lo,rotation=@r,model_id=@m WHERE id=@id;",
								"@la,@lo,@r,@m,@id");
	sql_row5 <double,double,double,int,int>bindings(latitude,longitude,heading,model_id,GetID());
	err = write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update entity info: %d (%s)",err, sqlite3_errmsg(db));	

}
