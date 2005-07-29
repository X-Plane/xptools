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
#include <dime/entities/Trace.h>
#include <dime/util/Linear.h>
#include <dime/State.h>


//
// implementation in solid.cpp
//
extern void 
convert_solid_data(dimeVec3f *v, dimeVec3f &e, dxfdouble thickness,
		   const dimeState *state,
		   dxfLayerData *layerData);


void 
convert_trace(const dimeEntity *entity, const dimeState *state, 
	      dxfLayerData *layerData, dxfConverter *converter)
{
  dimeTrace *trace = (dimeTrace*)entity;
  
  // respect the value in the $FILLMODE header variable
  layerData->setFillmode(converter->getFillmode());

  dimeVec3f v[4];
  trace->getVertices(v[0], v[1], v[2], v[3]);

  dimeParam param;
  if (trace->getRecord(38, param)) {
    v[0][2] = param.double_data;
    v[1][2] = param.double_data;
    v[2][2] = param.double_data;
    v[3][2] = param.double_data;
  }
   
  dimeVec3f e;
  trace->getExtrusionDir(e);
  dxfdouble thickness = trace->getThickness();
  
  convert_solid_data(v, e, thickness, state, layerData);
}
