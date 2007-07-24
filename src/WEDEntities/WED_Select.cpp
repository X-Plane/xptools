#include "WED_Select.h"
#include "IODefs.h"
#include "SQLUtils.h"
#include "WED_Errors.h"
#include "WED_Messages.h"

DEFINE_PERSISTENT(WED_Select)

WED_Select::WED_Select(WED_Archive * parent, int id) :
	WED_Thing(parent, id)
{
}

WED_Select::~WED_Select()
{
}

void WED_Select::CopyFrom(const WED_Select * rhs)
{
	DebugAssert(!"We should not be copying selection objects.");
	WED_Thing::CopyFrom(rhs);
	StateChanged();
	mSelected = rhs->mSelected;
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
	
}

void 			WED_Select::WriteTo(IOWriter * writer)
{
	WED_Thing::WriteTo(writer);
	writer->WriteInt(mSelected.size());
	for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		writer->WriteInt(*i);
		
}

void			WED_Select::FromDB(sqlite3 * db, const map<int,int>& mapping)
{
	WED_Thing::FromDB(db, mapping);

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
	if(err != SQLITE_DONE)		WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
}

void			WED_Select::ToDB(sqlite3 * db)
{
	WED_Thing::ToDB(db);

	int err;
	sql_command	nuke_all(db,"DELETE FROM WED_selection where id=@id;","@id");
	sql_row1<int>	me(GetID());
	err = nuke_all.simple_exec(me);
	if(err != SQLITE_DONE)		WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);

	if (!mSelected.empty())
	{
		sql_command write_ids(db,"INSERT INTO WED_selection(id,item) VALUES(@id,@item);","@id,@item");
		for (set<int>::iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		{
			sql_row2<int, int>	id(GetID(),*i);
			err = write_ids.simple_exec(id);
			if(err != SQLITE_DONE)		WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
		}
	}
}

#pragma mark -

bool		WED_Select::IsSelected(ISelectable * iwho) const
{
	return mSelected.count(iwho->GetSelectionID()) > 0;
}

void		WED_Select::Select(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	if (mSelected.size() != 1 || mSelected.count(id) == 0)
	{
		StateChanged(wed_Change_Selection);
		mSelected.clear();
		mSelected.insert(id);
	}
}

void		WED_Select::Clear(void)
{
	if (!mSelected.empty())
	{
		StateChanged(wed_Change_Selection);
		mSelected.clear();
	}
}

void		WED_Select::Toggle(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	StateChanged(wed_Change_Selection);
	if (mSelected.count(id) > 0)
		mSelected.erase(id);
	else
		mSelected.insert(id);
}

void		WED_Select::Insert(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	if (mSelected.count(id) == 0)
	{
		StateChanged(wed_Change_Selection);
		mSelected.insert(id);
	}
}

void		WED_Select::Erase(ISelectable * iwho)
{
	int id = iwho->GetSelectionID();

	if (mSelected.count(id) > 0)
	{
		StateChanged(wed_Change_Selection);
		mSelected.erase(id);
	}
}

int				WED_Select::GetSelectionCount(void) const
{
	return mSelected.size();
}

void			WED_Select::GetSelectionSet(set<ISelectable *>& sel) const
{
	sel.clear();
	for (set<int>::const_iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		sel.insert(FetchPeer(*i));
}

void			WED_Select::GetSelectionVector(vector<ISelectable *>& sel) const
{
	sel.clear();
	if (mSelected.empty()) return;
	sel.reserve(mSelected.size());
	for (set<int>::const_iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		sel.push_back(FetchPeer(*i));
}

ISelectable *		WED_Select::GetNthSelection(int n) const
{
	DebugAssert(n >= 0 && n < mSelected.size());
	if (n < 0) return NULL;
	set<int>::const_iterator i = mSelected.begin();
	while (n > 0 && i != mSelected.end())
	{
		--n;
		++i;
	}
	if (i == mSelected.end()) return NULL;
	return FetchPeer(*i);	
}

int			WED_Select::IterateSelection(int (* func)(ISelectable * who, void * ref), void * ref) const
{
	int n = 0;
	for (set<int>::const_iterator i = mSelected.begin(); i != mSelected.end(); ++i)
		if ((n=func(FetchPeer(*i), ref)) != 0) 
			return n;
	return 0;
}
