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
#include <dime/entities/3DFace.h>
#include <dime/util/Linear.h>
#include <dime/State.h>

void 
convert_3dface(const dimeEntity *entity, const dimeState *state, 
	       dxfLayerData *layerData, dxfConverter *)
{
  dimeMatrix matrix;
  state->getMatrix(matrix);
  dime3DFace *face = (dime3DFace*)entity;
  
  dimeVec3f v0, v1, v2, v3;
  face->getVertices(v0, v1, v2, v3);

  if (v2 == v3) {
    layerData->addTriangle(v0, v1, v2, &matrix);
  }
  else {
    layerData->addQuad(v0, v1, v2, v3, &matrix);
  }
}
