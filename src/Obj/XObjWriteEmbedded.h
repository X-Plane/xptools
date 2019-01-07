/*
 *  XObjWriteEmbedded.h
 *  SceneryTools
 *
 *  Created by bsupnik on 8/29/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef XObjWriteEmbedded_h
#define XObjWriteEmbedded_h

#include "XObjDefs.h"

bool	XObjWriteEmbedded(const char * inFile, const XObj8& inObj);
void	XObjFixKeyframes(XObj8& inObj8);

#endif
