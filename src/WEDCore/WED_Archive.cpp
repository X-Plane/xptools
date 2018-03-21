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
#if WITHNWLINK
#include "WED_NWLinkAdapter.h"
#endif
#include "WED_Archive.h"
#include "WED_Persistent.h"
#include "WED_UndoLayer.h"
#include "WED_UndoMgr.h"
#include "AssertUtils.h"
#include "WED_Errors.h"
#include "WED_XMLWriter.h"
#include "WED_Messages.h"

WED_Archive::WED_Archive(IResolver * r) : mResolver(r), mDying(false), mUndo(NULL), mUndoMgr(NULL),
 #if WITHNWLINK
 mNWAdapter(NULL),
 #endif
 mID(1), mOpCount(0), mCacheKey(0)
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
#if WITHNWLINK
	if (mNWAdapter) mNWAdapter->ObjectChanged(inObject, change_kind);
#endif
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
#if WITHNWLINK
	if (mNWAdapter) mNWAdapter->ObjectCreated(inObject);
#endif
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
#if WITHNWLINK
	if (mNWAdapter) mNWAdapter->ObjectDestroyed(inObject);
#endif
	if (mUndo == UNDO_DISCARD) return;
	if (mUndo) mUndo->ObjectDestroyed(inObject);
	else		DebugAssert(!"Error: object changed outside of a command.");
}

void	WED_Archive::ClearAll(void)
{
	++mCacheKey;

	for (ObjectMap::iterator ob = mObjects.begin(); ob != mObjects.end(); ++ob)
	if (ob->second != NULL)
		ob->second->Delete();
}

void			WED_Archive::SaveToXML(WED_XMLElement * parent)
{
	// old code bumps cache key on save...wHY?!
	//++mCacheKey;
	WED_XMLElement * obj = parent->add_sub_element("objects");
	for (ObjectMap::iterator ob = mObjects.begin(); ob != mObjects.end(); ++ob)
	if(ob->second != NULL)
	{
		ob->second->ToXML(obj);
		obj->flush();
	}

	mOpCount = 0;
}
#if WITHNWLINK
void			WED_Archive::SetNWLinkAdapter(WED_NWLinkAdapter * inAdapter)
{
	mNWAdapter = inAdapter;
}
#endif
void			WED_Archive::SetUndoManager(WED_UndoMgr * mgr)
{
	mUndoMgr = mgr;
}

void			WED_Archive::__StartCommand(const string& inName, const char * file, int line)
{
	DebugAssert(mUndoMgr != NULL);
	mUndoMgr->__StartCommand(inName, file, line);
}

void			WED_Archive::CommitCommand(void)
{
	DebugAssert(mUndoMgr != NULL);
	// Inc this first, so that anyone listening (via the undo mgr) sees we are dirty!
	++mOpCount;
	mUndoMgr->CommitCommand();


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


void		WED_Archive::StartElement(
								WED_XMLReader * reader,
								const XML_Char *	name,
								const XML_Char **	atts)
{
	const XML_Char ** a = atts;
	const char * class_name = NULL;
	const char * id_str = NULL;
	while(*a)
	{
		if(strcasecmp(*a,"class")==0)
		{
			++a;
			class_name = *a;
			++a;
		}
		if(strcasecmp(*a,"id")==0)
		{
			++a;
			id_str = *a;
			++a;
		}
		else
		{
			++a;
			++a;
		}
	}
	if(id_str == NULL || class_name == NULL)
	{
		reader->FailWithError("Object missing ID/Class.");
		return;
	}

	WED_Persistent * new_obj = WED_Persistent::CreateByClass(class_name, this, atoi(id_str));
	if(new_obj==NULL)
	{
		reader->FailWithError("Create obj failed.");
		return;
	}
	new_obj->FromXML(reader, atts);

}

void		WED_Archive::EndElement(void)
{
}

void		WED_Archive::PopHandler(void)
{
	mOpCount = 0;
	++mCacheKey;
}
