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

class IResolver;

#if !NO_CGAL_BEZIER
void	WED_CheckPolys(IResolver * in_resolver);
#endif
void	WED_MakeOrthos(IResolver * in_resolver);


#endif /* WED_Orthophoto_h */