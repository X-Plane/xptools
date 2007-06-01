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

void 			WED_Persistent::StateChanged(void)
{
	mArchive->ChangedObject(this);
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


