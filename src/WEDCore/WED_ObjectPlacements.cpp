#include "WED_ObjectPlacements.h"
#include "IODefs.h"

DEFINE_PERSISTENT(WED_ObjectRoot)
DEFINE_PERSISTENT(WED_ObjectLayer)
DEFINE_PERSISTENT(WED_CustomObject)

Point2			WED_CustomObject::GetLocation(void) const
{
	return mLocation;
}

double			WED_CustomObject::GetHeading(void) const
{
	return mHeading;
}

void			WED_CustomObject::SetLocation(const Point2& inLoc)
{
	StateChanged();
	mLocation = inLoc;
}

void			WED_CustomObject::SetHeading(double inHeading)
{
	StateChanged();
	mHeading = inHeading;
}

void 			WED_CustomObject::ReadFrom(IOReader * reader)
{
	reader->ReadDouble(mLocation.x);
	reader->ReadDouble(mLocation.y);
	reader->ReadDouble(mHeading);
}

void 			WED_CustomObject::WriteTo(IOWriter * writer)
{
	writer->WriteDouble(mLocation.x);
	writer->WriteDouble(mLocation.y);
	writer->WriteDouble(mHeading);
}

//---------------------------------------------------------------------------------

int						WED_ObjectLayer::CountObjects(void) const
{
	return mObjects.size();
}

WED_CustomObject *		WED_ObjectLayer::GetNthObject(int n) const
{
	return mObjects[n];
}

string					WED_ObjectLayer::GetName(void) const 
{
	return mName;
}


WED_CustomObject *		WED_ObjectLayer::NewObject(void)
{
	WED_CustomObject * new_obj = WED_CustomObject::CreateTyped(GetArchive(), WED_GUID());
	StateChanged();
	mObjects.push_back(WED_CustomObjectPtr(new_obj));
	
	return new_obj;
}

void					WED_ObjectLayer::DeleteObject(WED_CustomObject * x)
{
	vector<WED_CustomObjectPtr>::iterator i = std::find(
		mObjects.begin(), mObjects.end(), x);
	
	DebugAssert(i != mObjects.end());
	
	StateChanged();
	x->Delete();
	mObjects.erase(i);
}

void					WED_ObjectLayer::SetName(const string& inName)
{
	StateChanged();
	mName = inName;
}

void 			WED_ObjectLayer::ReadFrom(IOReader * reader)
{
	mObjects.clear();
	int n;
	vector<char> buf;
	reader->ReadInt(n);
	buf.resize(n);
	reader->ReadBulk(&*buf.begin(), n, false);
	mName = string(buf.begin(), buf.end());
	reader->ReadInt(n);
	while (n--)
	{
		mObjects.push_back(WED_CustomObjectPtr(GetArchive(), NULL));
		mObjects.back().ReadFrom(reader);
	}
}

void 			WED_ObjectLayer::WriteTo(IOWriter * writer)
{
	writer->WriteInt(mName.size());
	writer->WriteBulk(mName.c_str(), mName.size(), false);
	writer->WriteInt(mObjects.size());
	for (int n = 0; n < mObjects.size(); ++n)
	{
		mObjects[n].WriteTo(writer);
	}
}

//---------------------------------------------------------------------------------

WED_ObjectRoot::WED_ObjectRoot(WED_Archive * archive, const WED_GUID& guid) :
	WED_Persistent(archive, guid)
{
}

WED_ObjectRoot::~WED_ObjectRoot()
{
}

int						WED_ObjectRoot::CountLayers(void) const
{
	return mLayers.size();
}

WED_ObjectLayer *		WED_ObjectRoot::GetNthLayer(int n) const
{
	return mLayers[n];
}


WED_ObjectLayer *		WED_ObjectRoot::NewLayer(void)
{
	WED_ObjectLayer * layer = WED_ObjectLayer::CreateTyped(GetArchive(), WED_GUID());
	StateChanged();
	mLayers.push_back(layer);
	return layer;
}

void					WED_ObjectRoot::DeleteLayer(WED_ObjectLayer * x)
{
	vector<WED_ObjectLayerPtr>::iterator i = std::find(mLayers.begin(), mLayers.end(), x);
	DebugAssert(i != mLayers.end());
	StateChanged();
	x->Delete();
	mLayers.erase(i);
}

void					WED_ObjectRoot::ReorderLayer(int old_n, int new_n)
{
	StateChanged();
	vector<WED_ObjectLayerPtr>::iterator i (mLayers.begin());
	advance(i, old_n);
	WED_ObjectLayerPtr x(*i);
	mLayers.erase(i);
	i = mLayers.begin();
	advance(i, new_n);
	mLayers.insert(i, x);
}

void 			WED_ObjectRoot::ReadFrom(IOReader * reader)
{
	mLayers.clear();
	int n;
	reader->ReadInt(n);
	while (n--)
	{
		mLayers.push_back(WED_ObjectLayerPtr(GetArchive(), NULL));
		mLayers.back().ReadFrom(reader);
	}
}

void 			WED_ObjectRoot::WriteTo(IOWriter * writer)
{
	writer->WriteInt(mLayers.size());
	for (int n = 0; n < mLayers.size(); ++n)
	{
		mLayers[n].WriteTo(writer);
	}
}
