/*
 *  WED_Orthophoto.h
 *  SceneryTools
 *
 *  Created by bsupnik on 5/26/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef WED_Orthophoto_h
#define WED_Orthophoto_h

#include "WED_Archive.h"

class IResolver;
class WED_MapZoomerNew;

void	WED_MakeOrthos(IResolver * in_resolver, WED_MapZoomerNew * zoomer);

#endif /* WED_Orthophoto_h */