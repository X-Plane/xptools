#include "WED_Select.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"
#include "WED_Messages.h"

DEFINE_PERSISTENT(WED_Select)
START_CASTING(WED_Select)
IMPLEMENTS_INTERFACE(ISelection)
INHERITS_FROM(WED_Thing)
END_CASTING


WED_Select::WED_Select(WED_Archive * parent, int id) :
	WED_Thing(parent, id)
{
}

WED_Select::~WED_Select()
{
}


void 			WED_Select::ReadFrom(IOReader * reader)
{
	WED_Thing::ReadFrom(reader);
	int n,id;
	reader->ReadInt(n);
	mSelected.clear();
	while(n--)
	{
		reader->ReadInt(id);
		mSelected.insert(id);
	}
	
	BroadcastMessage(msg_SelectionChanged,0);
}

void 			WED_Select::WriteTo(IOWriter * writer)
{
	WED_Thing::WriteTo(writer);
	writer->WriteInt(mSelected.size());
	for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		writer->WriteInt(*i);
		
}

void			WED_Select::FromDB(sqlite3 * db)
{
	WED_Thing::FromDB(db);

	mSelected.clear();
	sql_command	cmd(db,"SELECT item FROM WED_selection WHERE id=@id;","@id");
	
	sql_row1<int>				id(GetID());
	sql_row1<int>				item;
	
	int err;
	cmd.set_params(id);
	cmd.begin();
	
	while ((err = cmd.get_row(item)) == SQLITE_ROW)
	{
		mSelected.insert(item.a);
	}
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to complete selection query: %d (%s)",err, sqlite3_errmsg(db));	
}

void			WED_Select::ToDB(sqlite3 * db)
{
	WED_Thing::ToDB(db);

	int err;
	sql_command	nuke_all(db,"DELETE FROM WED_selection where id=@id;","@id");
	sql_row1<int>	me(GetID());
	err = nuke_all.simple_exec(me);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("Unable to clear selection: %d (%s)",err, sqlite3_errmsg(db));	

	if (!mSelected.empty())
	{
		sql_command write_ids(db,"INSERT INTO WED_selection(id,item) VALUES(@id,@item);","@id,@item");
		for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		{
			sql_row2<int, int>	id(GetID(),*i);
			err = write_ids.simple_exec(id);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update selection info for %d: %d (%s)",*i,err, sqlite3_errmsg(db));	
		}
	}
}

#pragma mark -

bool		WED_Select::IsSelected(IUnknown * iwho) const
{
	DebugAssert(iwho != NULL);
	WED_Persistent * who = SAFE_CAST(WED_Persistent, iwho);
	DebugAssert(who != NULL);
	if (who == NULL) return false;

	return mSelected.count(who->GetID()) > 0;
}

void		WED_Select::Select(IUnknown * iwho)
{
	DebugAssert(iwho != NULL);
	WED_Persistent * who = SAFE_CAST(WED_Persistent, iwho);
	DebugAssert(who != NULL);
	if (who == NULL) return;
	
	if (mSelected.size() != 1 || mSelected.count(who->GetID()) == 0)
	{
		StateChanged();
		mSelected.clear();
		mSelected.insert(who->GetID());
		BroadcastMessage(msg_SelectionChanged,0);
	}
}

void		WED_Select::Clear(void)
{
	if (!mSelected.empty())
	{
		StateChanged();
		mSelected.clear();
		BroadcastMessage(msg_SelectionChanged,0);
	}
}

void		WED_Select::Toggle(IUnknown * iwho)
{
	DebugAssert(iwho != NULL);
	WED_Persistent * who = SAFE_CAST(WED_Persistent, iwho);
	DebugAssert(who != NULL);
	if (who == NULL) return;

	StateChanged();
	if (mSelected.count(who->GetID()) > 0)
		mSelected.erase(who->GetID());
	else
		mSelected.insert(who->GetID());
	BroadcastMessage(msg_SelectionChanged,0);
}

void		WED_Select::Insert(IUnknown * iwho)
{
	DebugAssert(iwho != NULL);
	WED_Persistent * who = SAFE_CAST(WED_Persistent, iwho);
	DebugAssert(who != NULL);
	if (who == NULL) return;

	if (mSelected.count(who->GetID()) == 0)
	{
		StateChanged();
		mSelected.insert(who->GetID());
		BroadcastMessage(msg_SelectionChanged,0);
	}
}

void		WED_Select::Erase(IUnknown * iwho)
{
	DebugAssert(iwho != NULL);
	WED_Persistent * who = SAFE_CAST(WED_Persistent, iwho);
	DebugAssert(who != NULL);
	if (who == NULL) return;

	if (mSelected.count(who->GetID()) > 0)
	{
		StateChanged();
		mSelected.erase(who->GetID());
		BroadcastMessage(msg_SelectionChanged,0);
	}
}

int				WED_Select::GetSelectionCount(void) const
{
	return mSelected.size();
}

void			WED_Select::GetSelectionSet(set<IUnknown *>& sel) const
{
	sel.clear();
	for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		sel.insert(FetchPeer(*i));
}

void			WED_Select::GetSelectionVector(vector<IUnknown *>& sel) const
{
	sel.clear();
	if (mSelected.empty()) return;
	sel.reserve(mSelected.size());
	for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		sel.push_back(FetchPeer(*i));
}

