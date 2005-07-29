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
#include <dime/entities/Solid.h>
#include <dime/util/Linear.h>
#include <dime/State.h>


void 
convert_solid_data(dimeVec3f *v, dimeVec3f &e, dxfdouble thickness,
		   const dimeState *state,
		   dxfLayerData *layerData)
{
  dimeMatrix matrix;
  state->getMatrix(matrix);

  if (e != dimeVec3f(0,0,1)) {
    dimeMatrix m;
    dimeEntity::generateUCS(e, m);
    matrix.multRight(m);
  }
  e = dimeVec3f(0,0,1) * thickness;
  
  int numunique = 0;
  dimeVec3f u[4];
  
  int i,j;
  for (i = 0; i < 4; i++) {
    for (j = 0; j < numunique; j++) {
      if (u[j] == v[i]) break;
    }
    if (j == numunique) {
      u[numunique++] = v[i];
    }
  }

  switch (numunique) {
  case 1:
    if (thickness != 0.0) {
      layerData->addLine(u[0], u[0]+e, &matrix);
    }
    else {
      layerData->addPoint(u[0], &matrix);
    }
    break;
  case 2:
    if (thickness != 0.0) {
      layerData->addQuad(u[0], u[1], u[1]+e, u[0]+e, &matrix);
    }
    else {
      layerData->addLine(u[0], u[1], &matrix);
    }
    break;
  case 3:
    // FIXME: check fillmode
    layerData->addTriangle(u[0], u[1], u[2], &matrix);
    if (thickness != 0.0) {
      layerData->addTriangle(u[0]+e, u[1]+e, u[2]+e, &matrix);
      layerData->addQuad(u[0], u[1], u[1]+e, u[0]+e, &matrix);
      layerData->addQuad(u[1], u[2], u[2]+e, u[1]+e, &matrix);
      layerData->addQuad(u[2], u[0], u[0]+e, u[2]+e, &matrix);
    }
    break;
  case 4:
    // FIXME: check fillmode
    layerData->addQuad(u[0], u[1], u[3], u[2], &matrix);
    if (thickness != 0) {
      layerData->addQuad(u[0]+e, u[1]+e, u[3]+e, u[2]+e, &matrix);
      layerData->addQuad(u[0], u[1], u[1]+e, u[0]+e, &matrix);
      layerData->addQuad(u[1], u[3], u[3]+e, u[1]+e, &matrix);
      layerData->addQuad(u[3], u[2], u[2]+e, u[3]+e, &matrix);
      layerData->addQuad(u[2], u[0], u[0]+e, u[2]+e, &matrix);
    }
    break;
  default:
    fprintf(stderr,"Unexpected error converting SOLID\n");
    break;
  }

}  


void 
convert_solid(const dimeEntity *entity, const dimeState *state, 
	      dxfLayerData *layerData, dxfConverter *converter)
{
  // respect the value in the $FILLMODE header variable
  layerData->setFillmode(converter->getFillmode());

  dimeSolid *solid = (dimeSolid*)entity;
  
  dimeVec3f v[4];
  solid->getVertices(v[0], v[1], v[2], v[3]);

  dimeParam param;
  if (solid->getRecord(38, param)) {
    v[0][2] = param.double_data;
    v[1][2] = param.double_data;
    v[2][2] = param.double_data;
    v[3][2] = param.double_data;
  }

   
  dimeVec3f e;
  solid->getExtrusionDir(e);
  dxfdouble thickness = solid->getThickness();

  convert_solid_data(v, e, thickness, state, layerData);
}
