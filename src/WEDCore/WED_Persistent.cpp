#include "WED_Persistent.h"
#include "WED_Archive.h"
#include "AssertUtils.h"
WED_Persistent::WED_Persistent(WED_Archive * parent) 
	: mArchive(parent)
{
	mArchive->AddObject(this);
}

WED_Persistent::WED_Persistent(WED_Archive * parent, const WED_GUID& inGUID) :
	mArchive(parent), mGUID(inGUID)
{
	mArchive->AddObject(this);
}

WED_Persistent::WED_Persistent(const WED_Persistent& rhs) :
	mArchive(rhs.mArchive)
{
	mArchive->AddObject(this);
}

WED_Persistent& WED_Persistent::operator=(const WED_Persistent& rhs)
{
	DebugAssert(mArchive == rhs.mArchive);
	return *this;
}

void			WED_Persistent::WED_Persistent::Delete(void)
{
	mArchive->RemoveObject(this);
}
			 
WED_Persistent *		WED_Persistent::Fetch(const WED_GUID& inGUID) const
{
	return mArchive->Fetch(inGUID);
}

void 			WED_Persistent::StateChanged(void)
{
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

WED_Persistent * WED_Persistent::CreateByClass(const char * id, WED_Archive * parent, const WED_GUID& inGUID)
{
	return sStaticCtors[id](parent, inGUID);
}
