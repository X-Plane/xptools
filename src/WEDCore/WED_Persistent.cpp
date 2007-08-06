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

#include "WED_Persistent.h"
#include "WED_Archive.h"
#include "AssertUtils.h"
WED_Persistent::WED_Persistent(WED_Archive * parent) 
	: mArchive(parent)
{
	mDirty = true;
	mArchive->AddObject(this);
}

WED_Persistent::WED_Persistent(WED_Archive * parent, int id) :
	mArchive(parent), mID(id)
{
	mDirty = true;
}

void			WED_Persistent::Delete(void)
{
	mArchive->RemoveObject(this);
	delete this;
}
			 
WED_Persistent *		WED_Persistent::FetchPeer(int id) const
{
	return mArchive->Fetch(id);
}

void		WED_Persistent::StartCommand(const string& inName)
{
	mArchive->StartCommand(inName);
}

void		WED_Persistent::CommitCommand(void)
{
	mArchive->CommitCommand();
}

void		WED_Persistent::AbortCommand(void)
{
	mArchive->AbortCommand();
}

void 			WED_Persistent::StateChanged(int change_kind)
{
	mArchive->ChangedObject(this, change_kind);
	mDirty = true;
}

WED_Persistent::~WED_Persistent()
{
}

void WED_Persistent::PostCtor()
{
	// Once we are built, we need to tell our archive.  But from our ctor our run-time type is not defined.
	// So we define this routine.  Anyone who builds us calls this AFTER the ctor and THEN we tell the archive
	// that we exist.  That way our run time type is fully known.
	
	// Why not just call AddObject in the archive from the code that calls the ctor?  Well, AddObject is private
	// and only the base clas (WED_persistent) is a friend.  So derived classes can't add themselves directly,
	// which is what create-typed does.
	mArchive->AddObject(this);
}

static hash_map<string, WED_Persistent::CTOR_f>	sStaticCtors;

void WED_Persistent::Register(
							const char * 	id, 
							CTOR_f 			ctor)
{
	string ids(id);
	hash_map<string, WED_Persistent::CTOR_f>::iterator i = sStaticCtors.find(ids);
	DebugAssert(i == sStaticCtors.end());
	if (i != sStaticCtors.end())
		Assert(i->second == ctor);
	else
		sStaticCtors[id] = ctor;
}

WED_Persistent * WED_Persistent::CreateByClass(const char * class_id, WED_Archive * parent, int id)
{
	if(sStaticCtors.count(class_id) == 0) return NULL;
	WED_Persistent * ret = sStaticCtors[class_id](parent, id);
	ret->PostCtor();
	return ret;
}

void			WED_Persistent::SetDirty(int dirty)
{
	mDirty = dirty;
}

int			WED_Persistent::GetDirty(void) const
{
	return mDirty;
}


