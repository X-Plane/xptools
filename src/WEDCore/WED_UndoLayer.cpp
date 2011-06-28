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

#include "WED_UndoLayer.h"
#include "WED_Persistent.h"
#include "WED_Buffer.h"
#include "WED_Archive.h"
#include "AssertUtils.h"
#include "WED_FastBuffer.h"
// NOTE: we could store no turd for created objs

WED_UndoLayer::WED_UndoLayer(WED_Archive * inArchive, const string& inName, const char * inFile, int inLine) :
	mArchive(inArchive), mName(inName), mChangeMask(0), mFile(inFile), mLine(inLine)
{
	mStorage = new WED_FastBufferGroup;
}

WED_UndoLayer::~WED_UndoLayer(void)
{
//	for(ObjInfoMap::iterator i = mObjects.begin(); i != mObjects.end(); ++i)
//	{
//		if (i->second.buffer)
//			delete i->second.buffer;
//	}
	delete mStorage;
}

void 	WED_UndoLayer::ObjectCreated(WED_Persistent * inObject)
{
		mChangeMask |= wed_Change_CreateDestroy;

	ObjInfoMap::iterator iter = mObjects.find(inObject->GetID());
	if (iter != mObjects.end())
	{
		// There is only one possible "rewrite" - if we are creating and knew
		// about the obj - it better have been destroyed!  In this case
		// destroy + recreate = change.  But keep the original data, the earliest
		// not this recent data.
		Assert(iter->second.op == op_Destroyed);
		iter->second.op = op_Changed;

	} else {
		// Brand new object
		ObjInfo	info;
		info.the_class = inObject->GetClass();
		info.op = op_Created;
		info.id = inObject->GetID();
		info.buffer = NULL;
		mObjects.insert(ObjInfoMap::value_type(inObject->GetID(), info));
	}
}


void	WED_UndoLayer::ObjectChanged(WED_Persistent * inObject, int change_kind)
{
	mChangeMask |= change_kind;
	ObjInfoMap::iterator iter = mObjects.find(inObject->GetID());
	if (iter != mObjects.end())
	{
		// Object is changed - it must not have been destroyed before!
		Assert(iter->second.op != op_Destroyed);
		// Create + change = create, change + change = change, so
		// no change in the op.  Not even a change is needed, because we
		// want to store the original data, not this latest change.

	} else {
		// First time changed
		ObjInfo	info;
		info.the_class = inObject->GetClass();
		info.op = op_Changed;
		info.id = inObject->GetID();
		info.buffer = mStorage->MakeNewBuffer();
//		info.buffer = new WED_Buffer;
		inObject->WriteTo(info.buffer);
		info.buffer->WriteInt(inObject->GetDirty());
		mObjects.insert(ObjInfoMap::value_type(inObject->GetID(), info));
	}
}

void	WED_UndoLayer::ObjectDestroyed(WED_Persistent * inObject)
{
		mChangeMask |= wed_Change_CreateDestroy;
	ObjInfoMap::iterator iter = mObjects.find(inObject->GetID());
	if (iter != mObjects.end())
	{
		// Object destroyed.  Object better not already be destroyed.
		Assert(iter->second.op != op_Destroyed);
		// Create + destroy = nothing.  Change + destroy = destroy

		if (iter->second.op == op_Created)
		{
			// Special case - a created and nuked object basically is temporary
			// and is unneeded in the bigger scheme of things.
			// Ben says: yes this sadly leaks space in the undo buffer, but
			// generally creating and then nuking a huge number of objects
			// is a rare use pattern....the compression and speed the consolidated
			// buffer gives us is a lot more important.
			mObjects.erase(iter);
		} else {
			// Note that we don't need to save the data - the original
			// change op already saved it!
			iter->second.op = op_Destroyed;
		}
	} else {
		// First time changed
		ObjInfo	info;
		info.the_class = inObject->GetClass();
		info.op = op_Destroyed;
		info.id = inObject->GetID();
		info.buffer = mStorage->MakeNewBuffer();
//		info.buffer = new WED_Buffer;
		inObject->WriteTo(info.buffer);
		info.buffer->WriteInt(inObject->GetDirty());
		mObjects.insert(ObjInfoMap::value_type(inObject->GetID(), info));
	}

}

void	WED_UndoLayer::Execute(void)
{
	int d;
	for (ObjInfoMap::iterator i = mObjects.begin(); i != mObjects.end(); ++i)
	{
		WED_Persistent * obj;
		switch(i->second.op) {
		case op_Created:
			obj = mArchive->Fetch(i->first);
			DebugAssert(i->second.buffer == NULL);
			Assert(obj != NULL);
			obj->Delete();
			break;
		case op_Changed:
			obj = mArchive->Fetch(i->first);
			Assert(obj != NULL);
			DebugAssert(i->second.buffer != NULL);
			i->second.buffer->ResetRead();
			obj->StateChanged();
			obj->ReadFrom(i->second.buffer);
			i->second.buffer->ReadInt(d);
			obj->SetDirty(d);
			break;
		case op_Destroyed:
			obj = WED_Persistent::CreateByClass(i->second.the_class, mArchive, i->first);
			DebugAssert(obj != NULL);
			DebugAssert(i->second.buffer != NULL);
			i->second.buffer->ResetRead();
			obj->ReadFrom(i->second.buffer);
			i->second.buffer->ReadInt(d);
			obj->SetDirty(d);
			break;
		}
	}
}

