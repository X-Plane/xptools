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
#include <dime/entities/Arc.h>
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
  radius = fabs(radius);
  
  if (maxerr >= radius || maxerr <= 0.0) maxerr = radius/40.0f;

  dxfdouble x = radius - maxerr;
  dxfdouble y = sqrt(radius*radius - x*x);
  
  dxfdouble rad = atan(y/x);

  return int(M_PI/fabs(rad)) + 1;
}


void 
convert_arc(const dimeEntity *entity, const dimeState *state, 
	    dxfLayerData *layerData, dxfConverter *converter)
{
  dimeArc *arc = (dimeArc*) entity;

  dimeMatrix matrix;
  state->getMatrix(matrix);

  dimeVec3f e = arc->getExtrusionDir();
  dxfdouble thickness = arc->getThickness();

  if (e != dimeVec3f(0,0,1)) {
    dimeMatrix m;
    dimeEntity::generateUCS(e, m);
    matrix.multRight(m);
  }
  e = dimeVec3f(0,0,1);

  dimeVec3f center;
  arc->getCenter(center);

  dimeParam param;
  if (arc->getRecord(38, param)) {
    center[2] = param.double_data;
  }
  
  dxfdouble radius = arc->getRadius();
  
  double end = arc->getEndAngle();

  while (end < arc->getStartAngle()) {
    end += 360.0;
  }

  double delta = DXFDEG2RAD(end - arc->getStartAngle());

  if (delta == 0.0) {
#ifndef NDEBUG
    fprintf(stderr,"ARC with startAngle == endAngle!\n");
#endif
    end += 2*M_PI;
    delta = DXFDEG2RAD(end - arc->getStartAngle());
  }
  
  int ARC_NUMPTS = converter->getNumSub();
  if (ARC_NUMPTS <= 0) { // use maxerr
    ARC_NUMPTS = calc_num_sub(converter->getMaxerr(), radius);
  }

  // find the number of this ARC that fits inside 2PI
  int parts = (int) DXFABS((2*M_PI) / delta);
  
  // find # pts to use for arc
  // add one to avoid arcs with 0 line segments
  int numpts = ARC_NUMPTS / parts + 1;
  if (numpts > ARC_NUMPTS) numpts = ARC_NUMPTS;
  
  double inc = delta / numpts;
  double rad = DXFDEG2RAD(arc->getStartAngle());
  int i;

  dimeVec3f v;
  dimeVec3f prev(center[0] + radius * cos(rad),
		 center[1] + radius * sin(rad),
		 center[2]);
  rad += inc;
  
  for (i = 1; i < numpts; i++) {
    v = dimeVec3f(center[0] + radius * cos(rad),
		  center[1] + radius * sin(rad),
		  center[2]);
    if (thickness == 0.0) {
      layerData->addLine(prev, v, &matrix);
    }
    else {
      layerData->addQuad(prev, v, v + e * thickness, prev + e * thickness,
			 &matrix);
    }
    prev = v;
    rad += inc;
  }
  rad = DXFDEG2RAD(end);
  v = dimeVec3f(center[0] + radius * cos(rad),
		center[1] + radius * sin(rad),
		center[2]);
  if (thickness == 0.0) {
    layerData->addLine(prev, v, &matrix);
  }
  else {
    layerData->addQuad(prev, v, v + e * thickness, prev + e * thickness,
		       &matrix);
  }
}
