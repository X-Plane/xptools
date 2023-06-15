/*
 *  WED_Orthophoto.h
 *  SceneryTools
 *
 *  Created by bsupnik on 5/26/09.
 *  Copyright 2009 Laminar Research. All rights reserved.
 *
 */

#ifndef WED_Orthophoto_h
#define WED_Orthophoto_h

class IResolver;
class WED_Archive;
class WED_Ring;
class WED_Thing;
class WED_MapZoomerNew;
struct Point2;
struct gcp_t;

typedef WED_Thing* (*CreatNodeFunc)(WED_Archive* parent);

namespace
{
	template<class T>
	WED_Thing* CreateThing(WED_Archive* parent)
	{
		return T::CreateTyped(parent);
	}
}

WED_Ring * WED_RingfromImage(char * path, WED_Archive * arch, WED_MapZoomerNew * zoomer, CreatNodeFunc create, gcp_t * gcp = nullptr);
void	WED_MakeOrthos(IResolver * in_resolver, WED_MapZoomerNew * zoomer);
void	WED_MakeTerrain(IResolver* in_resolver, WED_MapZoomerNew* zoomer);

#endif /* WED_Orthophoto_h */