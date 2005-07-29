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

#include "convert_funcs.h"
#include <dime/convert/convert.h>
#include <dime/convert/layerdata.h>
#include <dime/entities/Circle.h>
#include <dime/util/Linear.h>
#include <dime/State.h>

//
// find intersection between circle and the line x=r-maxerr,
// and return the number of circle subdivisions necessary
// to respect the maxerr parameter.
//
static int 
calc_num_sub(dxfdouble maxerr, dxfdouble radius)
{
  if (maxerr >= radius || maxerr <= 0.0) maxerr = radius/40.0f;

  dxfdouble x = radius - maxerr;
  dxfdouble y = sqrt(radius*radius - x*x);

  dxfdouble rad = atan(y/x);
  
  return int(M_PI/fabs(rad)) + 1;
}


void 
convert_circle(const dimeEntity *entity, const dimeState *state, 
	       dxfLayerData *layerData, dxfConverter *converter)
{
  dimeCircle *circle = (dimeCircle*) entity;

  dimeMatrix matrix;
  state->getMatrix(matrix);

  dimeVec3f e = circle->getExtrusionDir();
  dxfdouble thickness = circle->getThickness();
  
  if (e != dimeVec3f(0,0,1)) {
    dimeMatrix m;
    dimeEntity::generateUCS(e, m);
    matrix.multRight(m);
  }
  e = dimeVec3f(0,0,1) * thickness;

  dimeVec3f center = circle->getCenter();
  dxfdouble radius = circle->getRadius();

  dimeParam param;
  if (circle->getRecord(38, param)) {
    center[2] = param.double_data;
  }
  
  int numpts = converter->getNumSub();
  if (numpts <= 0) { // use maxerr
    numpts = calc_num_sub(converter->getMaxerr(), radius);
  }
  dxfdouble inc = (2*M_PI) / numpts;  
  dxfdouble rad = inc;
  int i;
  
  dimeVec3f v;
  dimeVec3f prev(center[0] + radius,
		 center[1],
		 center[2]);
  
  for (i = 1; i < numpts; i++) {
    v = dimeVec3f(center[0] + radius * cos(rad),
		  center[1] + radius * sin(rad),
		  center[2]);
    
    if (thickness == 0.0) {
      layerData->addLine(prev, v, &matrix);
    }
    else {
      layerData->addQuad(prev, v, v + e, prev + e,
			 &matrix);
    }
    prev = v;
    rad += inc;
  }

  v = dimeVec3f(center[0] + radius,
		center[1],
		center[2]);
  
  if (thickness == 0.0) {
    layerData->addLine(prev, v, &matrix);
  }
  else {
    layerData->addQuad(prev, v, v + e, prev + e,
		       &matrix);
  }
  // FIXME: code to close cylinder?
}
