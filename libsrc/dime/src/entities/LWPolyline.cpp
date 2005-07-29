/**************************************************************************\
 * 
 *  FILE: LWPolyline.cpp
 *
 *  This source file is part of DIME.
 *  Copyright (C) 1998-1999 by Systems In Motion.  All rights reserved.
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
 *  NORWAY                                               Fax: +47 67172912
 *
\**************************************************************************/

/*!
  \class dimeLWPolyline dime/entities/LWPolyline.h
  \brief The dimeLWPolyline class handles an LWPOLYLINE \e entity.
*/

#include <dime/entities/LWPolyline.h>
#include <dime/records/Record.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

static char entityName[] = "LWPOLYLINE";

//
// FIXME: after the first vertex is found, I should make sure no more 
// unknown records are found. If unknown records are found, the LWPOLYLINE
// will be illegal upon writing. The current code will work for releases
// up to r14, but might not work for r15+ if additional per-vertex data
// is added for those future file formats. 
//

/*!
  Constructor.
*/

dimeLWPolyline::dimeLWPolyline() 
  : constantWidth( 0.0 ), elevation( 0.0 ), flags( 0 ), numVertices( 0 ),
    xcoord( NULL ), ycoord( NULL ), startingWidth( NULL ), endWidth( NULL ),
    bulge( NULL )
{
}

/*!
  Destructor.
*/

dimeLWPolyline::~dimeLWPolyline()
{
}

//!

dimeEntity *
dimeLWPolyline::copy(dimeModel * const model) const
{
  dimeLWPolyline *l = new(model->getMemHandler()) dimeLWPolyline;
  if (!l) return NULL;

  dimeMemHandler *mh = model->getMemHandler();

  if (!this->copyRecords(l, model)) {
    // check if allocated on heap.
    if (!mh) delete l;
    l = NULL;
  }
  else {
    const int num = this->numVertices;
    if (num > 0) {
      l->xcoord = ARRAY_NEW(mh, dxfdouble, num);
      l->ycoord = ARRAY_NEW(mh, dxfdouble, num);      
      l->bulge = ARRAY_NEW(mh, dxfdouble, num);
      if (this->startingWidth) {
	l->startingWidth = ARRAY_NEW(mh, dxfdouble, num);
	l->endWidth = ARRAY_NEW(mh, dxfdouble, num);
      }
      for (int i = 0; i < num; i++) {
	l->xcoord[i] = this->xcoord[i];
	l->ycoord[i] = this->ycoord[i];
	l->bulge[i] = this->bulge[i];
	if (this->startingWidth) {
	  l->startingWidth[i] = this->startingWidth[i];
	  l->endWidth[i] = this->endWidth[i];
	}
      }
    }
    l->flags = this->flags;
    l->numVertices = this->numVertices;
    l->constantWidth = this->constantWidth;
    l->elevation = this->elevation;
    l->copyExtrusionData(this);
  }
  return l;  
}

//!

bool 
dimeLWPolyline::write(dimeOutput * const file)
{
  this->preWrite(file);

  bool ret = true;

  file->writeGroupCode(90);
  ret = file->writeInt16((int16)this->numVertices);

  if (ret && this->flags != 0) {
    file->writeGroupCode(70);
    ret = file->writeInt16(this->flags);
  }
  if (ret && this->elevation != 0.0) {
    file->writeGroupCode(38);
    ret = file->writeDouble(this->elevation);
  }
  if (ret && this->constantWidth != 0.0) {
    file->writeGroupCode(43);
    ret = file->writeDouble(this->constantWidth);
  }
  
  if (!ret) return false;
  
  // write extrusion data and unksnown records
  ret = this->writeExtrusionData(file) && 
    dimeEntity::write(file);
  
  if (ret) {
    const int num = this->numVertices;
    for (int i = 0; ret && i < num; i++) {
      file->writeGroupCode(10);
      file->writeDouble(this->xcoord[i]);
      file->writeGroupCode(20);
      ret = file->writeDouble(this->ycoord[i]);
      
      if (ret && this->startingWidth && this->endWidth) {
	if (this->startingWidth[i] != 0.0) {
	  file->writeGroupCode(40);
	  file->writeDouble(this->startingWidth[i]);
	}
	if (this->endWidth[i] != 0.0) {
	  file->writeGroupCode(41);
	  ret = file->writeDouble(this->endWidth[i]);
	}
      }
      if (ret && this->bulge[i] != 0.0) {
	file->writeGroupCode(42);
	ret = file->writeDouble(this->bulge[i]);
      }
    }
  }
  
  return ret;
}

//!

int 
dimeLWPolyline::typeId() const
{
  return dimeBase::dimeLWPolylineType;
}

/*!
  Handles the callback from dimeEntity::readRecords().
*/

bool 
dimeLWPolyline::handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const mh)
{
  switch (groupcode) {
  case 10:
  case 20:
  case 40:
  case 41:
  case 42:
    {
      if (this->xcoord == NULL) { // allocate arrays when first vertex is found
	const int num = this->numVertices;
	if (num <= 0) {
	  fprintf(stderr,"LWPOLYLINE shouldn't have any vertices, but still found one!\n");
	  return true; // data is "handled" so... 
	}
	this->xcoord = ARRAY_NEW(mh, dxfdouble, num);
	this->ycoord = ARRAY_NEW(mh, dxfdouble, num);
	this->bulge = ARRAY_NEW(mh, dxfdouble, num);
	if (this->constantWidth == 0.0) {
	  this->startingWidth = ARRAY_NEW(mh, dxfdouble, num);
	  this->endWidth = ARRAY_NEW(mh, dxfdouble, num);
	}
	// must initialize arrays to default values
	for (int i = 0; i < num; i++) {
	  this->bulge[i] = 0.0;
	  if (this->startingWidth) {
	    this->startingWidth[i] = 0.0;
	    this->endWidth[i] = 0.0;
	  }
	}
	this->tmpCounter = 0; // reset counters before going any further
	this->tmpFlags = 0;
      }
      int flagmask;
      dxfdouble *arrayptr;
      switch (groupcode) {
      case 10:
	flagmask = 0x1;
	arrayptr = this->xcoord;
	break;
      case 20:
	flagmask = 0x2;
	arrayptr = this->ycoord;
	break;
      case 40:
	flagmask = 0x4;
	arrayptr = this->startingWidth;
	break;
      case 41:
	flagmask = 0x8;
	arrayptr = this->endWidth;
	break;
      case 42:
	flagmask = 0x10;
	arrayptr = this->bulge;
	break;
      default:
	flagmask = 0;
	arrayptr = NULL;
	assert(0);
	break;
      }
      if (this->tmpFlags & flagmask) {
	this->tmpFlags = 0;
	this->tmpCounter++;
      }
      if (this->tmpCounter >= this->numVertices) {
	fprintf(stderr,"too many vertices in LWPOLYLINE!\n");
	return true;
      }
      if (arrayptr == NULL) {
	fprintf(stderr,"illegal data found in LWPOLYLINE.\n");
	return true;
      }
      this->tmpFlags |= flagmask;
      arrayptr[this->tmpCounter] = param.double_data;
      return true;
    }
  case 38:
    this->elevation = param.double_data;
    return true;
  case 43:
    this->constantWidth = param.double_data;
    return true;
  case 70:
    this->flags = param.int16_data;
    return true;
  case 90:
    this->numVertices = param.int32_data;
    return true;
  }
  return dimeExtrusionEntity::handleRecord(groupcode, param, mh);
}

//!

const char *
dimeLWPolyline::getEntityName() const
{
  return entityName;
}

//!

bool 
dimeLWPolyline::getRecord(const int groupcode,
			 dimeParam &param,
			 const int index) const
{
  switch(groupcode) {
  case 10:
    if (this->xcoord && index >= 0 && index < this->numVertices) {
      param.double_data = this->xcoord[index];
      return true;
    }
    return false;
  case 20:
    if (this->ycoord && index >= 0 && index < this->numVertices) {
      param.double_data = this->ycoord[index];
      return true;
    }
    return false;
  case 40:
    if (this->startingWidth && index >= 0 && index < this->numVertices) {
      param.double_data = this->startingWidth[index];
      return true;
    }
    return false;
  case 41:
    if (this->endWidth && index >= 0 && index < this->numVertices) {
      param.double_data = this->endWidth[index];
      return true;
    }
    return false;
  case 42:
    if (this->bulge && index >= 0 && index < this->numVertices) {
      param.double_data = this->bulge[index];
      return true;
    }
    return false;
  case 38:
    param.double_data = this->elevation;
    return true;
  case 43:
    param.double_data = this->constantWidth;
    return true;
  case 70:
    param.int16_data = this->flags ;
    return true;
  case 90:
    param.int32_data = this->numVertices;
    return true;
  }
  return dimeExtrusionEntity::getRecord(groupcode, param, index);
}

//!

void
dimeLWPolyline::print() const
{
  fprintf(stderr,"LWPOLYLINE:\n");
  for (int i = 0; i < this->numVertices; i++) { 
    fprintf(stderr,"coord: %f %f\n", xcoord[i], ycoord[i]);   
  } 
}

//!

dimeEntity::GeometryType 
dimeLWPolyline::extractGeometry(dimeArray <dimeVec3f> &verts,
			       dimeArray <int> &/*indices*/,
			       dimeVec3f &extrusionDir,
			       dxfdouble &thickness)
{
  thickness = this->thickness;
  extrusionDir = this->extrusionDir;
    
  const int num = this->numVertices;
  for (int i = 0; i < num; i++) {
    verts.append(dimeVec3f(this->xcoord[i],
			  this->ycoord[i],
			  this->elevation));
  }
  // is POLYLINE closed?
  if (this->flags & 1) {
    verts.append(dimeVec3f(this->xcoord[0],
			  this->ycoord[0],
			  this->elevation));
  }
  return dimeEntity::LINES;
}

//!

int
dimeLWPolyline::countRecords() const
{
  int cnt = 2; // header + numVertices
  
  if (this->elevation != 0.0) cnt++;
  if (this->constantWidth != 0.0) cnt++;
  if (this->flags != 0) cnt++;
  cnt += this->numVertices * 2; // x and y coordinates

  // count optional per-vertex records
  for (int i = 0; i < this->numVertices; i++) { 
    if (this->bulge[i] != 0.0) cnt++;
    if (this->startingWidth && this->startingWidth[i] != 0.0) cnt++; 
    if (this->endWidth && this->endWidth[i] != 0.0) cnt++; 
  }

  return cnt + dimeExtrusionEntity::countRecords();
}

