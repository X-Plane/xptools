#include "WED_Thing.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"
#include <algorithm>

WED_Thing::WED_Thing(WED_Archive * parent, int id) :
	WED_Persistent(parent, id),
	name(this,"Name","WED_things", "name","unnamed entity")	
{
	parent_id = 0;
}

WED_Thing::~WED_Thing()
{
}

void 			WED_Thing::ReadFrom(IOReader * reader)
{
	int ct;
	reader->ReadInt(parent_id);
	reader->ReadInt(ct);
	child_id.resize(ct);
	for (int n = 0; n < ct; ++n)
		reader->ReadInt(child_id[n]);
	
	ReadPropsFrom(reader);
}

void 			WED_Thing::WriteTo(IOWriter * writer)
{
	int n;
	writer->WriteInt(parent_id);
	
	writer->WriteInt(child_id.size());
	for (int n = 0; n < child_id.size(); ++n)
		writer->WriteInt(child_id[n]);
	
	WritePropsTo(writer);
}

void			WED_Thing::FromDB(sqlite3 * db)
{
	child_id.clear();
	sql_command	cmd(db,"SELECT parent FROM WED_things WHERE id=@i;","@i");
	
	sql_row1<int>						key(GetID());
	sql_row1<int>						me;
	
	int err = cmd.simple_exec(key, me);
	if (err != SQLITE_DONE)	WED_ThrowPrintf("Unable to complete thing query: %d (%s)",err, sqlite3_errmsg(db));
	
	parent_id = me.a;
	
	sql_command kids(db, "SELECT id FROM WED_things WHERE parent=@id ORDER BY seq;","@id");
	sql_row1<int>	kid;
	
	kids.set_params(key);
	kids.begin();
	while ((err = kids.get_row(kid)) == SQLITE_ROW)
	{
		child_id.push_back(kid.a);
	}
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to complete thing query on kids: %d (%s)",err, sqlite3_errmsg(db));	
	
	char where_crud[100];
	sprintf(where_crud,"id=%d",GetID());	
	PropsFromDB(db,where_crud);
}

void			WED_Thing::ToDB(sqlite3 * db)
{
	int err;
	
	int persistent_class_id;
	
	{
		const char * my_class = this->GetClass();
		
		sql_command	find_my_class(db,"SELECT id FROM WED_classes WHERE name=@name;","@name");
		sql_row1<string>	class_key(my_class);
		
		find_my_class.set_params(class_key);
		sql_row1<int>	found_id;
		if (find_my_class.get_row(found_id) == SQLITE_ROW)
		{
			persistent_class_id = found_id.a;		
		}
		else
		{
			sql_command	find_highest_id(db,"SELECT MAX(id) FROM WED_classes;",NULL);
			sql_row1<int>	highest_key;
			err = find_highest_id.simple_exec(sql_row0(), highest_key);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update thing info: %d (%s)",err, sqlite3_errmsg(db));	
			
			persistent_class_id = highest_key.a + 1;
			
			sql_command record_new_class(db,"INSERT INTO WED_classes VALUES(@id,@name);","@id,@name");
			sql_row2<int,string> new_class_info(persistent_class_id,my_class);
			err = record_new_class.simple_exec(new_class_info);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update thing info: %d (%s)",err, sqlite3_errmsg(db));	
		}
	}
	
	sql_command write_me(db,"INSERT OR REPLACE INTO WED_things VALUES(@id,@parent,@seq,@name,@class_id);","@id,@parent,@seq,@name,@class_id");
	sql_row5<int,int,int,string,int>	bindings(
												GetID(),
												parent_id,
												GetMyPosition(),
												name.value,
												persistent_class_id);
	
	err =  write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update thing info: %d (%s)",err, sqlite3_errmsg(db));	
	
	char id_str[20];
	sprintf(id_str,"%d",GetID());	
	PropsToDB(db,"id",id_str, "WED_things");
}

int					WED_Thing::CountChildren(void) const
{
	return child_id.size();
}

WED_Thing *		WED_Thing::GetNthChild(int n) const
{
	return SAFE_CAST(WED_Thing,FetchPeer(child_id[n]));
}

WED_Thing *		WED_Thing::GetNamedChild(const string& s) const
{
	int c = CountChildren();
	for (int n = 0; n < c; ++n)
	{
		WED_Thing * t = GetNthChild(n);
		if (t)
		{
			string n;
			t->GetName(n);
			if (s==n)
				return t;
		}
	}
	return NULL;
}


void	WED_Thing::GetName(string& n) const
{
	n = name.value;
}

void	WED_Thing::SetName(const string& n)
{
	StateChanged();
	name.value = n;
}

WED_Thing *		WED_Thing::GetParent(void) const
{
	return SAFE_CAST(WED_Thing,FetchPeer(parent_id));
}

void				WED_Thing::SetParent(WED_Thing * parent, int nth)
{
	StateChanged();
	WED_Thing * old_parent = SAFE_CAST(WED_Thing, FetchPeer(parent_id));
	if (old_parent) old_parent->RemoveChild(GetID());
	parent_id = parent ? parent->GetID() : 0;
	if (parent) parent->AddChild(GetID(),nth);
}

int			WED_Thing::GetMyPosition(void) const
{
	WED_Thing * parent = SAFE_CAST(WED_Thing, FetchPeer(parent_id));
	if (!parent) return 0;
	int n = 0;
	vector<int>::iterator i = find(parent->child_id.begin(), parent->child_id.end(), this->GetID());
	return distance(parent->child_id.begin(), i);
}

void				WED_Thing::AddChild(int id, int n)
{
	StateChanged();
	DebugAssert(n >= 0);
	DebugAssert(n <= child_id.size());	
	vector<int>::iterator i = find(child_id.begin(),child_id.end(),id);
	DebugAssert(i == child_id.end());
	child_id.insert(child_id.begin()+n,id);
}

void				WED_Thing::RemoveChild(int id)
{
	StateChanged();
	vector<int>::iterator i = find(child_id.begin(),child_id.end(),id);
	DebugAssert(i != child_id.end());
	child_id.erase(i);
}

void		WED_Thing::PropEditCallback(int before)
{
	if (before)
		StateChanged();
}

int				WED_Thing::Array_Count (void )
{
	return CountChildren();
}

IBase *		WED_Thing::Array_GetNth(int n)
{
	return GetNthChild(n);
}
	
IBase *		WED_Thing::Directory_Find(const char * name)
{
	return GetNamedChild(name);
}

void	WED_Thing::StartOperation(const char * op_name)
{
	StartCommand(op_name);
}

void	WED_Thing::CommitOperation(void)
{
	CommitCommand();
}

void	WED_Thing::AbortOperation(void)
{
	AbortCommand();
}
