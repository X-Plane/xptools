#include "WED_KeyObjects.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

DEFINE_PERSISTENT(WED_KeyObjects)

WED_KeyObjects::WED_KeyObjects(WED_Archive * a, int id) : WED_Thing(a,id)
{
}

WED_KeyObjects::~WED_KeyObjects()
{
}

IBase *	WED_KeyObjects::Directory_Find(const char * name)
{
	map<string,int>::iterator i = choices.find(name);
	if (i != choices.end())
		return FetchPeer(i->second);
	return WED_Thing::Directory_Find(name);
}


void WED_KeyObjects::Directory_Edit(const char * name, IBase * who)
{
	string n(name);
	WED_Persistent * target = SAFE_CAST(WED_Persistent,who);
	if (target == NULL)
		AssertPrintf("Can't set a non-persistent to %s\n", name);

	int id = target->GetID();
	
	map<string,int>::iterator i = choices.find(n);
	if (i != choices.end() && i->second == id) return;
	
	StateChanged();
	choices[n] = id;
}

void 			WED_KeyObjects::ReadFrom(IOReader * reader)
{
	WED_Thing::ReadFrom(reader);
	choices.clear();
	int n;
	reader->ReadInt(n);
	while (n--)
	{
		int b;
		int id;
		reader->ReadInt(b);
		vector<char> buf(b);
		reader->ReadBulk(&*buf.begin(), b, false);
		string key(buf.begin(),buf.end());
		reader->ReadInt(id);
		choices[key] = id;
	}
}

void 			WED_KeyObjects::WriteTo(IOWriter * writer)
{
	WED_Thing::WriteTo(writer);
	writer->WriteInt(choices.size());
	for (map<string,int>::iterator it = choices.begin(); it != choices.end(); ++it)
	{
		writer->WriteInt(it->first.size());
		writer->WriteBulk(it->first.c_str(),it->first.size(),false);
		writer->WriteInt(it->second);
	}
}

void			WED_KeyObjects::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
	WED_Thing::FromDB(db, mapping);
	choices.clear();
	
	int err;
	sql_command cmd(db,"SELECT key,value FROM WED_key_objects WHERE id=@i;","@i");
	
	sql_row1<int>			key(GetID());
	sql_row2<string,int>	pair;
	
	cmd.set_params(key);
	cmd.begin();
	while((err = cmd.get_row(pair)) == SQLITE_ROW)
	{
		choices[pair.a] = pair.b;
	}
	if (err != SQLITE_DONE) WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
}

void			WED_KeyObjects::ToDB(sqlite3 * db)
{
	WED_Thing::ToDB(db);
	int err;
	
	{
		sql_command clear_it(db,"DELETE FROM WED_key_objects WHERE id=@i;","@i");
		sql_row1<int>	key(GetID());
		
		err = clear_it.simple_exec(key);
		if (err != SQLITE_DONE) WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}
	{
		sql_command add_pair(db,"INSERT INTO WED_key_objects(id,key,value) VALUES(@i,@k,@v);","@i,@k,@v");
		
		for (map<string,int>::iterator i = choices.begin(); i != choices.end(); ++i)
		{
			sql_row3<int, string, int>	binding(GetID(),i->first,i->second);
			err = add_pair.simple_exec(binding);
			if (err != SQLITE_DONE) WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
		}
	}
}

