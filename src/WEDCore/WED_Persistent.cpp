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
	mArchive->AddObject(this);
}

WED_Persistent::WED_Persistent(const WED_Persistent& rhs) :
	mArchive(rhs.mArchive)
{
	mDirty = true;
	mArchive->AddObject(this);
}

WED_Persistent& WED_Persistent::operator=(const WED_Persistent& rhs)
{
	mDirty = true;
	DebugAssert(mArchive == rhs.mArchive);
	return *this;
}

void			WED_Persistent::WED_Persistent::Delete(void)
{
	mArchive->RemoveObject(this);
	delete this;
}
			 
WED_Persistent *		WED_Persistent::FetchPeer(int id) const
{
	return mArchive->Fetch(id);
}

void 			WED_Persistent::StateChanged(void)
{
	mDirty = true;
	mArchive->ChangedObject(this);
}

WED_Persistent::~WED_Persistent()
{
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
	return sStaticCtors[class_id](parent, id);
}

void			WED_Persistent::SetDirty(int dirty)
{
	mDirty = dirty;
}

int			WED_Persistent::GetDirty(void) const
{
	return mDirty;
}

