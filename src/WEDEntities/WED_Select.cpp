#include "WED_Select.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"
#include "WED_Messages.h"

DEFINE_PERSISTENT(WED_Select)
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
	writer->WriteInt(mSelected.size());
	for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		writer->WriteInt(*i);
		
}

void			WED_Select::FromDB(sqlite3 * db)
{
	WED_Thing::FromDB(db);

	mSelected.clear();
	sql_command	cmd(db,"SELECT id FROM WED_selection;",NULL);
	
	sql_row1<int>				id;
	
	int err;
	cmd.begin();
	
	while ((err = cmd.get_row(id)) == SQLITE_ROW)
	{
		mSelected.insert(id.a);
	}
	if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to complete selection query: %d (%s)",err, sqlite3_errmsg(db));	
}

void			WED_Select::ToDB(sqlite3 * db)
{
	WED_Thing::ToDB(db);

	int err;
	sql_command	nuke_all(db,"DELETE FROM WED_selection;",NULL);
	err = nuke_all.simple_exec();
	if(err != SQLITE_DONE)		WED_ThrowPrintf("Unable to clear selection: %d (%s)",err, sqlite3_errmsg(db));	

	if (!mSelected.empty())
	{
		sql_command write_ids(db,"INSERT INTO WED_selection VALUES(@id);","@id");
		for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		{
			sql_row1<int>	id(*i);
			err = write_ids.simple_exec(id);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("UNable to update selection info for %d: %d (%s)",*i,err, sqlite3_errmsg(db));	
		}
	}
}

#pragma mark -

bool		WED_Select::IsSelected(WED_Thing * who) const
{
	return mSelected.count(who->GetID()) > 0;
}

void		WED_Select::Select(WED_Thing * who)
{
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

void		WED_Select::Toggle(WED_Thing * who)
{
	StateChanged();
	if (mSelected.count(who->GetID()) > 0)
		mSelected.erase(who->GetID());
	else
		mSelected.insert(who->GetID());
	BroadcastMessage(msg_SelectionChanged,0);
}

void		WED_Select::Insert(WED_Thing * who)
{
	if (mSelected.count(who->GetID()) == 0)
	{
		StateChanged();
		mSelected.insert(who->GetID());
		BroadcastMessage(msg_SelectionChanged,0);
	}
}

void		WED_Select::Erase(WED_Thing * who)
{
	if (mSelected.count(who->GetID()) > 0)
	{
		StateChanged();
		mSelected.erase(who->GetID());
		BroadcastMessage(msg_SelectionChanged,0);
	}
}
