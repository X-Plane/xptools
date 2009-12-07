/*
 * Copyright (c) 2007, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "WED_Archive.h"
#include "WED_Persistent.h"
#include "WED_UndoLayer.h"
#include "WED_UndoMgr.h"
#include "AssertUtils.h"
#include "WED_Errors.h"
#include "sqlite3.h"
#include "SQLUtils.h"
#include "WED_Messages.h"

WED_Archive::WED_Archive() : mDying(false), mUndo(NULL), mUndoMgr(NULL), mID(1), mOpCount(0), mCacheKey(0)
{
}

WED_Archive::~WED_Archive()
{
	DebugAssert(mUndo == NULL);		// Shouldn't be mid-op when we do this!

	mDying = true; // flag to self to realize that we don't care about dead objs.

	for (ObjectMap::iterator i = mObjects.begin(); i != mObjects.end(); ++i)
	if (i->second)
		i->second->Delete();
}

void WED_Archive::SetUndo(WED_UndoLayer * inUndo)
{
	DebugAssert((inUndo == NULL) != (mUndo == NULL));
	mUndo = inUndo;
}

WED_Persistent *	WED_Archive::Fetch(int id) const
{
	ObjectMap::const_iterator iter = mObjects.find(id);
	if (iter == mObjects.end()) return NULL;
	return iter->second;
}

void		WED_Archive::ChangedObject(WED_Persistent * inObject, int change_kind)
{
	if (mDying) return;
	++mCacheKey;
	if (mUndo == UNDO_DISCARD) return;
	if (mUndo)	mUndo->ObjectChanged(inObject, change_kind);
	else		DebugAssert(!"Error: object changed outside of a command.");
}

void		WED_Archive::AddObject(WED_Persistent * inObject)
{
	if (mDying) return;
	++mCacheKey;
	mID = max(mID,inObject->GetID()+1);
	ObjectMap::iterator iter = mObjects.find(inObject->GetID());
	DebugAssert(iter == mObjects.end() || iter->second == NULL);

	if (iter == mObjects.end())			mObjects.insert(ObjectMap::value_type(inObject->GetID(), inObject));
	else								iter->second = inObject;

	if (mUndo == UNDO_DISCARD) return;
	if (mUndo) mUndo->ObjectCreated(inObject);
	else		DebugAssert(!"Error: object changed outside of a command.");
}

void		WED_Archive::RemoveObject(WED_Persistent * inObject)
{
	if (mDying) return;
	++mCacheKey;
	ObjectMap::iterator iter = mObjects.find(inObject->GetID());
	Assert(iter != mObjects.end());
	iter->second = NULL;
	if (mUndo == UNDO_DISCARD) return;
	if (mUndo) mUndo->ObjectDestroyed(inObject);
	else		DebugAssert(!"Error: object changed outside of a command.");
}

void	WED_Archive::LoadFromDB(sqlite3 * db, const map<int,int>& mapping)
{
	++mCacheKey;

	{
		sql_command		fetch_objects(db, "SELECT WED_things.id, WED_classes.name FROM WED_things JOIN WED_classes on WED_things.class_id = WED_classes.id;", NULL);

		sql_row2<int,string>	an_obj;
		fetch_objects.begin();
		int err;
		while((err = fetch_objects.get_row(an_obj)) == SQLITE_ROW)
		{
			mID = max(mID,an_obj.a+1);

			WED_Persistent * new_obj = WED_Persistent::CreateByClass(an_obj.b.c_str(), this, an_obj.a);
			if(new_obj==NULL)
				WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
		}
		if (err != SQLITE_DONE)
			WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);
	}

	for (ObjectMap::iterator ob = mObjects.begin(); ob != mObjects.end(); ++ob)
	if (ob->second != NULL)
	if (ob->second->GetDirty())
	{
		ob->second->FromDB(db, mapping);
		ob->second->SetDirty(false);
	}

	mOpCount = 0;
}

void	WED_Archive::ClearAll(void)
{
	++mCacheKey;

	for (ObjectMap::iterator ob = mObjects.begin(); ob != mObjects.end(); ++ob)
	if (ob->second != NULL)
		ob->second->Delete();
}


void	WED_Archive::SaveToDB(sqlite3 * db)
{
	++mCacheKey;

	sql_command nuke_obj(db,"DELETE FROM WED_things WHERE id=@id;","@id");
	
	sql_command nuke_src(db,"DELETE FROM WED_thing_viewers where source=@id;","@id");
	sql_command nuke_vew(db,"DELETE FROM WED_thing_viewers where viewer=@id;","@id");
	
	for (ObjectMap::iterator ob = mObjects.begin(); ob != mObjects.end(); ++ob)
	{

		if (ob->second == NULL)
		{
			sql_row1<int>	key(ob->first);
			int err = nuke_obj.simple_exec(key);
			if (err != SQLITE_DONE)
				WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);

			err = nuke_src.simple_exec(key);
			if (err != SQLITE_DONE)
				WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);

			err = nuke_vew.simple_exec(key);
			if (err != SQLITE_DONE)
				WED_ThrowPrintf("%s (%d)",sqlite3_errmsg(db),err);

		}
		else
		{
			#if OPTIMIZE
				this sucks
			#endif
//			BEN SAYS: always I/O out to disk - otherwise if the enum system is remapped we get killed.
//			if (ob->second->GetDirty())
			{
				ob->second->ToDB(db);
				ob->second->SetDirty(false);
			}
		}
	}

	mOpCount = 0;
}

void			WED_Archive::SetUndoManager(WED_UndoMgr * mgr)
{
	mUndoMgr = mgr;
}

void			WED_Archive::StartCommand(const string& inName)
{
	DebugAssert(mUndoMgr != NULL);
	mUndoMgr->StartCommand(inName);
}

void			WED_Archive::CommitCommand(void)
{
	DebugAssert(mUndoMgr != NULL);
	mUndoMgr->CommitCommand();

	++mOpCount;

#if DEV	
	this->Validate();
#endif
}

void			WED_Archive::AbortCommand(void)
{
	++mCacheKey;

	DebugAssert(mUndoMgr != NULL);
	mUndoMgr->AbortCommand();
}

int	WED_Archive::NewID(void)
{
	return mID++;
}

long long WED_Archive::CacheKey(void)
{
	return mCacheKey;
}

int		WED_Archive::IsDirty(void)
{
	return mOpCount;
}

void	WED_Archive::Validate(void)
{
	for (ObjectMap::iterator ob = mObjects.begin(); ob != mObjects.end(); ++ob)
	if (ob->second != NULL)
		ob->second->Validate();
}
