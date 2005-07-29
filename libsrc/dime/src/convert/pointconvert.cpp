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
#include <dime/convert/layerdata.h>
#include <dime/entities/Point.h>
#include <dime/util/Linear.h>
#include <dime/State.h>

void 
convert_point(const dimeEntity *entity, const dimeState *state, 
	      dxfLayerData *layerData, dxfConverter *)
{
  dimePoint *point = (dimePoint*)entity;
  dimeVec3f v0 = point->getCoords();

  dimeParam param;
  if (point->getRecord(38, param)) {
    v0[2] = param.double_data;
  }
  
  dimeMatrix matrix;
  state->getMatrix(matrix);
  
  dxfdouble thickness = point->getThickness();
  if (thickness != 0.0) { // line
    dimeVec3f e = point->getExtrusionDir();
    layerData->addLine(v0, v0 + thickness * e);
  }
  else { // point
    layerData->addPoint(v0);
  }
}
