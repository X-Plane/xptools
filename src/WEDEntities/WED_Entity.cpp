#include "WED_Entity.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_Entity)								\

WED_Entity::WED_Entity(WED_Archive * parent, int id) :
	WED_Persistent(parent, id)
{
	locked = 0;
	hidden = 0;
	parent_id = 0;
	name = "unnamed entity";
}

WED_Entity::~WED_Entity()
{
}

void 			WED_Entity::ReadFrom(IOReader * reader)
{
	int ct;
	reader->ReadInt(parent_id);
	reader->ReadInt(ct);
	child_id.resize(ct);
	for (int n = 0; n < ct; ++n)
		reader->ReadInt(child_id[n]);
	
	reader->ReadInt(locked);
	reader->ReadInt(hidden);
	reader->ReadInt(ct);
	
	if (ct > 0)
	{
		vector<char>	buf(ct);
		reader->ReadBulk(&*buf.begin(),ct, 0);
		name=string(buf.begin(),buf.end());
	} else 
		name.clear();
	
}

void 			WED_Entity::WriteTo(IOWriter * writer)
{
	int n;
	writer->WriteInt(parent_id);
	
	writer->WriteInt(child_id.size());
	for (int n = 0; n < child_id.size(); ++n)
		writer->WriteInt(child_id[n]);
	
	writer->WriteInt(locked);
	writer->WriteInt(hidden);
	writer->WriteInt(name.size());
	if (!name.empty())
		writer->WriteBulk(name.c_str(), name.length(), 0);
	
	#if !DEV
	make nice overloading to make this FASTER
	#endif
}

void			WED_Entity::FromDB(sqlite3 * db)
{
	child_id.clear();
//	char	buf[500];
//	sprintf(buf,"SELECT parent,name,locked,hidden FROM WED_entities WHERE id=%d;",GetID());
	sql_command	cmd(db,"SELECT parent,name,locked,hidden FROM WED_entities WHERE id=@i;","@i");
	
	sql_row1<int>						key(GetID());
	sql_row4<int,string,int,int>		me;
	
	int err = cmd.simple_exec(key, me);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete entity query: %d (%s)",err, sqlite3_errmsg(db));
	
	parent_id = me.a;
	name = me.b;
	locked = me.c;
	hidden = me.d;
	
	sql_command kids(db, "SELECT id FROM WED_entities WHERE parent=@id ORDER BY seq;","@id");
	sql_row1<int>	kid;
	
	kids.begin();
	while ((err = kids.get_row(kid)) == SQLITE_ROW)
	{
		child_id.push_back(kid.a);
	}
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to complete entity query on kids: %d (%s)",err, sqlite3_errmsg(db));	
}

void			WED_Entity::ToDB(sqlite3 * db)
{
	int err;
	sql_command	write_me(db,"UPDATE WED_entities set parent=@p, name=@n, locked=@l, hidden=@h WHERE id=@id;",
								"@p,@n,@l,@h,@id");
	sql_row5 <int,string,int,int,int>bindings(parent_id,name,locked,hidden,GetID());
	err = write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update entity info: %d (%s)",err, sqlite3_errmsg(db));	

	if (!child_id.empty())
	{
		sql_command write_kids(db,"UPDATE WED_entities set seq=@n WHERE id=@id;","@n,@id");
		for (int n = 0; n < child_id.size(); ++n)
		{
			sql_row2<int,int>	kid_info(n,child_id[n]);
			err = write_kids.simple_exec(kid_info);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update child info for %d child: %d (%s)",n,err, sqlite3_errmsg(db));	
		}
	}
}
