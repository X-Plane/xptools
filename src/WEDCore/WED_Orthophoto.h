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

void	WED_MakeOrthos(IResolver * in_resolver, string resource, WED_Archive * archive);

#endif /* WED_Orthophoto_h */