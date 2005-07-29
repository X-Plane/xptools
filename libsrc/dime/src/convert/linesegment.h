/**************************************************************************\
 *
 *  This source file is part of DIME.
 *  Copyright (C) 1998-2001 by Systems In Motion.  All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License (the accompanying file named COPYING) for more
 *  details.
 *
 **************************************************************************
 *
 *  If you need DIME for a non-GPL project, contact Systems In Motion
 *  to acquire a Professional Edition License:
 *
 *  Systems In Motion                                   http://www.sim.no/
 *  Prof. Brochs gate 6                                       sales@sim.no
 *  N-7030 Trondheim                                   Voice: +47 22114160
 *  NORWAY                                               Fax: +47 22207097
 *
\**************************************************************************/

#ifndef _DXF2VRML_LINE_SEGMENT_
#define _DXF2VRML_LINE_SEGMENT_

#include <dime/util/Linear.h>
class dxfLayerData;

class dxfLineSegment 
{
public:
  void set(const dimeVec3f &p0, const dimeVec3f &p1,
	   const dxfdouble startWidth, const dxfdouble endwidth,
	   const dxfdouble thickness);
  void convert(dxfLineSegment *prev, dxfLineSegment *next,
	       dxfLayerData *data, dimeMatrix *matrix);
  
private:

  void calculate_v();
  void calculate_connect(dxfLineSegment *next);
  
  dimeVec3f p[2];
  dxfdouble w[2];
  dxfdouble thickness;
  dxfLineSegment *prev;
  dimeVec3f e;
  dimeVec3f dir;
  dimeVec3f wdir;

  // calculated pts
  int flags;
  dimeVec3f v[4];
  dimeVec3f connect[4];
};


#endif // _DXF2VRML_LINE_SEGMENT_
