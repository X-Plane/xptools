#include "WED_Archive.h"
#include "WED_Persistent.h"
#include "WED_UndoLayer.h"
#include "AssertUtils.h"

WED_Archive::WED_Archive() : mDying(false), mUndo(NULL)
{
}

WED_Archive::~WED_Archive()
{
	DebugAssert(mUndo == NULL);		// Shouldn't be mid-op when we do this!
	
	mDying = true; // flag to self to realize that we don't care about dead objs.

	for (ObjectMap::iterator i = mObjects.begin(); i != mObjects.end(); ++i)
		i->second->Delete();
}

void WED_Archive::SetUndo(WED_UndoLayer * inUndo)
{
	DebugAssert((inUndo == NULL) != (mUndo == NULL));
	mUndo = inUndo;
}

WED_Persistent *	WED_Archive::Fetch(const WED_GUID& inGUID) const
{
	ObjectMap::const_iterator iter = mObjects.find(inGUID);
	if (iter == mObjects.end()) return NULL;
	return iter->second;
}

void		WED_Archive::ChangedObject(WED_Persistent * inObject)
{
	if (mDying) return;
	if (mUndo) mUndo->ObjectChanged(inObject);
}

void		WED_Archive::AddObject(WED_Persistent * inObject)
{
	if (mDying) return;
	DebugAssert(mObjects.count(inObject->GetGUID()) == 0);
	
	mObjects.insert(ObjectMap::value_type(inObject->GetGUID(), inObject));
	if (mUndo) mUndo->ObjectCreated(inObject);
}

void		WED_Archive::RemoveObject(WED_Persistent * inObject)
{
	if (mDying) return;
	WED_GUID guid;
	
	ObjectMap::iterator iter = mObjects.find(inObject->GetGUID());
	Assert(iter != mObjects.end());
	mObjects.erase(iter);
	if (mUndo) mUndo->ObjectDestroyed(inObject);
}
