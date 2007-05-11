#include "WED_Thing.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"

START_CASTING(WED_Thing)
IMPLEMENTS_INTERFACE(IPropertyObject)
IMPLEMENTS_INTERFACE(IArray)
IMPLEMENTS_INTERFACE(IDirectory)
IMPLEMENTS_INTERFACE(IOperation)
IMPLEMENTS_INTERFACE(WED_Persistent)
BASE_CASE
END_CASTING


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
	sql_command	write_me(db,"UPDATE WED_things set parent=@p WHERE id=@id;",
								"@p,@id");
	sql_row2 <int,int>bindings(parent_id,GetID());
	err = write_me.simple_exec(bindings);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update thing info: %d (%s)",err, sqlite3_errmsg(db));	

	if (!child_id.empty())
	{
		sql_command write_kids(db,"UPDATE WED_things set seq=@n WHERE id=@id;","@n,@id");
		for (int n = 0; n < child_id.size(); ++n)
		{
			sql_row2<int,int>	kid_info(n,child_id[n]);
			err = write_kids.simple_exec(kid_info);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update child info for %d child: %d (%s)",n,err, sqlite3_errmsg(db));	
		}
	}
	
	char id_str[20];
	sprintf(id_str,"i%d",GetID());	
	PropsToDB(db,"id",id_str);
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

IUnknown *		WED_Thing::Array_GetNth(int n)
{
	return GetNthChild(n);
}
	
IUnknown *		WED_Thing::Directory_Find(const char * name)
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
