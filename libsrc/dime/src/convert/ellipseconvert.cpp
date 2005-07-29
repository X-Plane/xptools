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
#include <dime/entities/Ellipse.h>
#include <dime/util/Linear.h>
#include <dime/State.h>

//
// find intersection between ellipse and the line x=r-maxerr,
// and return the number of subdivisions necessary
// to respect the maxerr parameter.
//
static int 
calc_num_sub(dxfdouble maxerr, dxfdouble a, dxfdouble b)
{
  dxfdouble minrad = a < b ? a : b;

  if (maxerr >= minrad) maxerr = minrad / 40.0f;

  dxfdouble x,y;
  
  if (a >= b) {
    x = a-maxerr;
    y = sqrt((1.0-(x*x)/(a*a))*b*b);
  }
  else {
    x = b-maxerr;
    y = sqrt((1.0-(x*x)/(b*b))*a*a);
  }
  
  dxfdouble rad = atan(y/x);  
  return int(M_PI/fabs(rad)) + 1;
}


void 
convert_ellipse(const dimeEntity *entity, const dimeState *state, 
		dxfLayerData *layerData, dxfConverter *converter)
{
  dimeEllipse *ellipse = (dimeEllipse*) entity;

  dimeMatrix matrix;
  state->getMatrix(matrix);

  dimeVec3f e = ellipse->getExtrusionDir();
  dxfdouble thickness = ellipse->getThickness();
  
  // According to the DXF Intern, Ellipse has no Element coordinate
  // system, so this code is disabled

//   if (e != dimeVec3f(0,0,1) && e != dimeVec3f(0,0,-1)) {
//     dimeMatrix m;
//     dimeEntity::generateUCS(e, m);
//     matrix.multRight(m);
//   }

  e *= thickness;

  dimeVec3f center = ellipse->getCenter();

  // do some cross product magic to calculate minor axis
  dimeVec3f xaxis = ellipse->getMajorAxisEndpoint();

  dimeParam param;
  if (ellipse->getRecord(38, param)) {
    center[2] = param.double_data;
    xaxis[2] = param.double_data;
  }

  dxfdouble xlen = xaxis.length() * 0.5;
  xaxis.normalize();
  dimeVec3f yaxis = dimeVec3f(0,0,1).cross(xaxis);
  yaxis.normalize();
  dimeVec3f zaxis = xaxis.cross(yaxis);
  zaxis.normalize();
  yaxis = zaxis.cross(xaxis);
  yaxis.normalize();
    
  yaxis *= ellipse->getMinorMajorRatio() * xlen;
  xaxis *= xlen;

  dxfdouble numpts = (dxfdouble) converter->getNumSub();
  if (numpts <= 0.0) { // use maxerr
    numpts = calc_num_sub(converter->getMaxerr(), 
			  xlen, xlen*ellipse->getMinorMajorRatio());
  }
  
  dxfdouble rad = ellipse->getStartParam();
  dxfdouble end = ellipse->getEndParam();

  while (end <= rad) end += M_PI*2.0;
  
  dxfdouble size = (2*M_PI) / (end-rad);
  dxfdouble inc = (end-rad) / (numpts * size);  
  
  int i;
  
  dimeVec3f v;
  dimeVec3f prev(center[0] + xaxis[0] * cos(rad) + yaxis[0] * sin(rad),
		 center[1] + xaxis[1] * cos(rad) + yaxis[1] * sin(rad),
		 center[2] + xaxis[2] * cos(rad) + yaxis[2] * sin(rad));
  rad += inc;
  
  for (; rad < end; rad += inc) {
    v =   dimeVec3f(center[0] + xaxis[0] * cos(rad) + yaxis[0] * sin(rad),
		    center[1] + xaxis[1] * cos(rad) + yaxis[1] * sin(rad),
		    center[2] + xaxis[2] * cos(rad) + yaxis[2] * sin(rad));
    
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

  rad = end;
  v =   dimeVec3f(center[0] + xaxis[0] * cos(rad) + yaxis[0] * sin(rad),
		  center[1] + xaxis[1] * cos(rad) + yaxis[1] * sin(rad),
		  center[2] + xaxis[2] * cos(rad) + yaxis[2] * sin(rad));
  if (thickness == 0.0) {
    layerData->addLine(prev, v, &matrix);
  }
  else {
    layerData->addQuad(prev, v, v + e, prev + e,
                       &matrix);
  }
}
