#ifndef WED_OBJECTPLACEMENTS_H
#define WED_OBJECTPLACEMENTS_H

#include "WED_Persistent.h"
#include "CompGeomDefs2.h"

class WED_CustomObject : public WED_Persistent {

	DECLARE_PERSISTENT(WED_CustomObject)
	
public:

	Point2			GetLocation(void) const;
	double			GetHeading(void) const;
	
	void			SetLocation(const Point2& inLoc);
	void			SetHeading(double inHeading);

	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);

private:

	Point2		mLocation;
	double		mHeading;

};
typedef WED_PersistentPtr<WED_CustomObject>	WED_CustomObjectPtr;




class WED_ObjectLayer : public WED_Persistent {

	DECLARE_PERSISTENT(WED_ObjectLayer)
	
public:
	
	int						CountObjects(void) const;
	WED_CustomObject *		GetNthObject(int n) const;
	
	WED_CustomObject *		NewObject(void);
	void					DeleteObject(WED_CustomObject *);

	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);

private:

	vector<WED_CustomObjectPtr>		mObjects;

};
typedef WED_PersistentPtr<WED_ObjectLayer> WED_ObjectLayerPtr;




class WED_ObjectRoot : public WED_Persistent {

	DECLARE_PERSISTENT(WED_ObjectRoot)
	
public:

	int						CountLayers(void) const;
	WED_ObjectLayer *		GetNthLayer(int n) const;
	
	WED_ObjectLayer *		NewLayer(void);
	void					DeleteLayer(WED_ObjectLayer *);
	void					ReorderLayer(int old_n, int new_n);
	
	virtual	void 			ReadFrom(IOReader * reader);
	virtual	void 			WriteTo(IOWriter * writer);

private:

	vector<WED_ObjectLayerPtr>		mLayers;

};



#endif
