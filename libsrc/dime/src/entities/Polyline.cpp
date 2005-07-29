/**************************************************************************\
 * 
 *  FILE: Polyline.cpp
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
  \class dimePolyline dime/entities/Polyline.h
  \brief The dimePolyline class handles a POLYLINE \e entity.
*/

#include <dime/entities/Polyline.h>
#include <dime/entities/Vertex.h>
#include <dime/records/Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/State.h>
#include <string.h>

static char entityName[] = "POLYLINE";

/*!
  Constructor.
*/

dimePolyline::dimePolyline()
  : flags( 0 ), countM( 0 ), countN( 0 ),              
  smoothCountM( 0 ), smoothCountN( 0 ), surfaceType( 0 ), coordCnt( 0 ),
  indexCnt( 0 ), frameCnt(0), coordVertices( NULL ), indexVertices( NULL ), 
  frameVertices( NULL),  seqend( NULL ), elevation(0,0,0)
{
}

/*!
  Destructor.
*/

dimePolyline::~dimePolyline()
{
  int i;
  delete this->seqend;
  for (i = 0; i < this->coordCnt; i++) 
    delete this->coordVertices[i];
  for (i = 0; i < this->indexCnt; i++) 
    delete this->indexVertices[i];
  for (i = 0; i < this->frameCnt; i++)
    delete this->frameVertices[i];
  delete [] this->coordVertices;
  delete [] this->indexVertices;
  delete [] this->frameVertices;
}

//!

dimeEntity *
dimePolyline::copy(dimeModel * const model) const
{
  int i;
  dimeMemHandler *memh = model->getMemHandler();
  dimePolyline *pl = new(memh) dimePolyline;

  bool ok = pl != NULL;
  if (ok && this->indexCnt) {
    int num = this->indexCnt;
    pl->indexVertices = (dimeVertex**) 
      dimeEntity::copyEntityArray((const dimeEntity**)this->indexVertices,
				 num,
				 model);
    if (num > 0 && pl->indexVertices == NULL) {
      ok = false;
    }
    else {
      pl->indexCnt = num;
      for (i = 0; i < num; i++) pl->indexVertices[i]->polyline = pl;
    }
  }
  if (ok && this->coordCnt) {
    int num = this->coordCnt;
    pl->coordVertices = (dimeVertex**)
      dimeEntity::copyEntityArray((const dimeEntity**)this->coordVertices,
				 num,
				 model);
    if (num > 0 && pl->coordVertices == NULL) {
      ok = false;
    }
    else {
      pl->coordCnt = num;
      for (i = 0; i < num; i++) pl->coordVertices[i]->polyline = pl;
    }
  }
  if (ok && this->frameCnt) {
    int num = this->frameCnt;
    pl->frameVertices = (dimeVertex**)
      dimeEntity::copyEntityArray((const dimeEntity**)this->frameVertices,
				  num,
				  model);
    if (num > 0 && pl->frameVertices == NULL) {
      ok = false;
    }
    else {
      pl->frameCnt = num;
      for (i = 0; i < num; i++) pl->frameVertices[i]->polyline = pl;
    }
  }


  if (ok) {
    // set polyline for each vertex (each vertex needs a pointer).
    for (i = 0; i < pl->coordCnt; i++)
      pl->coordVertices[i]->polyline = pl;
    for (i = 0; i < pl->indexCnt; i++)
      pl->indexVertices[i]->polyline = pl;


    pl->countM = this->countM;
    pl->countN = this->countN;
    pl->smoothCountM = this->smoothCountM;
    pl->smoothCountN = this->smoothCountN;
    pl->surfaceType = this->surfaceType;
    pl->flags = this->flags;
    pl->elevation = this->elevation;
    pl->copyExtrusionData(this);
    
    pl->seqend = this->seqend->copy(model);
    if (!pl->seqend) ok = false;
  }
  if (!ok || !this->copyRecords(pl, model)) {
    if (!memh) delete pl; // delete if allocated on heap
    pl = NULL; // just return NULL
  }
  return pl;
}

/*!
  Reads a POLYLINE entity.
*/

bool
dimePolyline::read(dimeInput * const file)
{
  bool ret = dimeEntity::read(file);  
  
  if (ret && this->entityFlags & FLAG_VERTICES_FOLLOW) {      
    // read all vertices.
    dimeArray <dimeVertex*> array(1024);
    int32 groupcode;
    const char *string;
    dimeVertex *vertex;
    dimeMemHandler *memhandler = file->getMemHandler();

    int idxcnt = 0;
    int vcnt = 0;
    int framecnt = 0;

    while (true) {
      if (!file->readGroupCode(groupcode) || groupcode != 0) {
	fprintf(stderr,"Error reading groupcode: %d\n", groupcode);
//	sim_warning("Error reading groupcode: %d\n", groupcode);
	ret = false;
	break;
      }
      string = file->readString();
      if (!strcmp(string, "SEQEND")) {
	this->seqend = (dimeEntity*) 
	  dimeEntity::createEntity(string, memhandler);
	ret = this->seqend && this->seqend->read(file);
	break; // ok, no more vertices.
      }
      if (strcmp(string, "VERTEX")) {ret = false; break;}
      vertex = (dimeVertex*) dimeEntity::createEntity(string, memhandler);
      
      if (vertex == NULL) {
	fprintf(stderr,"error creating vertex\n");
//	sim_warning("error creating vertex\n");
	ret = false;
	break;
      }
      if (!vertex->read(file)) {
	fprintf(stderr,"error reading vertex.\n");
//	sim_warning("error reading vertex.\n");
	ret = false;
	break;
      }

      if (vertex->getFlags() & 16) framecnt++;
      if (vertex->numIndices()) idxcnt++;
      else vcnt++;
      vertex->polyline = this;
      array.append(vertex);
    }
    int n = array.count();
    if (ret && n) {
      if (idxcnt) {
	this->indexVertices = ARRAY_NEW(memhandler, dimeVertex*, idxcnt);
	if (!this->indexVertices) ret = false;
      }
      if (vcnt && ret) {
	this->coordVertices = ARRAY_NEW(memhandler, dimeVertex*, vcnt);
	ret = this->coordVertices != NULL;
      }
      if (framecnt && ret) {
	this->frameVertices = ARRAY_NEW(memhandler, dimeVertex*, framecnt);
	ret = this->frameVertices != NULL;
      }

      if (ret) {
	this->coordCnt = 0;
	this->indexCnt = 0;
	this->frameCnt = 0;
	for (int i = 0; i < n; i++) {
	  vertex = array[i];
	  if (vertex->getFlags() & 16) 
	    this->frameVertices[this->frameCnt++] = vertex;
	  else if (vertex->numIndices())
	    this->indexVertices[this->indexCnt++] = vertex;
	  else
	    this->coordVertices[this->coordCnt++] = vertex;
	}
      }
    }
  } 
  return ret;
}

/*!
  Writes POLYLINE data to \a file.   
*/

bool 
dimePolyline::write(dimeOutput * const file)
{
  if (this->isDeleted()) return true;

#if 0
  static int dummycnt = -1;
  dummycnt++;
  if (dummycnt != 70) return true;
#endif

  dimeEntity::preWrite(file);

  assert(this->coordCnt == this->numCoordVertices());
  assert(this->indexCnt == this->numIndexVertices());
  
  if (this->coordCnt || this->indexCnt || this->frameCnt) {
    file->writeGroupCode(66); // vertices follow flag
    file->writeInt16(1);
  }

  file->writeGroupCode(10);
  file->writeDouble(elevation[0]);
  file->writeGroupCode(20);
  file->writeDouble(elevation[1]);
  file->writeGroupCode(30);
  file->writeDouble(elevation[2]);
  file->writeGroupCode(70);
  file->writeInt16(flags);

  this->writeExtrusionData(file);

  if (this->flags & 64) {
    // FIXME: is this necessary?
    file->writeGroupCode(71);
#ifdef DIME_FIXBIG
    file->writeInt32(this->numCoordVertices());
#else
    file->writeInt16((int16)this->numCoordVertices());
#endif
    file->writeGroupCode(72);
#ifdef DIME_FIXBIG
    file->writeInt32(this->numIndexVertices());
#else
    file->writeInt16((int16)this->numIndexVertices());
#endif
  }
  else {
    if (this->countM != 0) {
      file->writeGroupCode(71);
#ifdef DIME_FIXBIG
      file->writeInt32(this->countM);
#else
      file->writeInt16(this->countM);
#endif
    }
    if (this->countN != 0) {
      file->writeGroupCode(72);
#ifdef DIME_FIXBIG
      file->writeInt32(this->countN);
#else
      file->writeInt16(this->countN);
#endif
    }
    if (this->smoothCountM != 0) {
      file->writeGroupCode(73);
#ifdef DIME_FIXBIG
      file->writeInt32(this->smoothCountM);
#else
      file->writeInt16(this->smoothCountM);
#endif
    }
    if (this->smoothCountN != 0) {
      file->writeGroupCode(74);
#ifdef DIME_FIXBIG
      file->writeInt32(this->smoothCountN);   
#else
      file->writeInt16(this->smoothCountN);   
#endif
    }
    if (this->surfaceType != 0) {
      file->writeGroupCode(75);
      file->writeInt16(this->surfaceType);
    }
  }
  bool ret = dimeEntity::write(file); // write unknown records
  if (!ret) return false; // too lazy to check every write
    
  int i;
  // write all spline frame control points
  for (i = 0; i < this->frameCnt; i++) {
    this->frameVertices[i]->write(file);
  }
  // write all coord vertices
  for (i = 0; i < this->coordCnt; i++) {
    this->coordVertices[i]->write(file);
  }
  // write all index vertices
  for (i = 0; i < this->indexCnt; i++) {
    this->indexVertices[i]->write(file);
  }
  
  // write end-of-vertex signature...
  if (this->seqend) return this->seqend->write(file);
  // ... or just put one there!
  file->writeGroupCode(0);
  file->writeString("SEQEND");
  file->writeGroupCode(8);
  return file->writeString(this->getLayerName());
}

//!

int 
dimePolyline::typeId() const
{
  return dimeBase::dimePolylineType;
}

//!

bool 
dimePolyline::handleRecord(const int groupcode,
			  const dimeParam &param,
			  dimeMemHandler * const memhandler)
{
  switch(groupcode) {
  case 66:
    this->entityFlags |= FLAG_VERTICES_FOLLOW;
    return true;
  case 70:
    this->flags = param.int16_data;
    return true;
  case 71:
#ifdef DIME_FIXBIG
    this->countM = param.int32_data;
#else
    this->countM = param.int16_data;
#endif
    return true;
  case 72:
#ifdef DIME_FIXBIG
    this->countN = param.int32_data;
#else
    this->countN = param.int16_data;
#endif
    return true;
  case 73:
#ifdef DIME_FIXBIG
    this->smoothCountM = param.int32_data;
#else
    this->smoothCountM = param.int16_data;
#endif
    return true;
  case 74:
#ifdef DIME_FIXBIG
    this->smoothCountN = param.int32_data;
#else
    this->smoothCountN = param.int16_data;
#endif
    return true;
  case 75:
    this->surfaceType = param.int16_data;
    return true;
  case 10:
  case 20:
  case 30:
    this->elevation[groupcode / 10 - 1] = param.double_data;
    return true;
  }
  return dimeExtrusionEntity::handleRecord(groupcode, param, memhandler);
}

//!

const char *
dimePolyline::getEntityName() const
{
  return entityName;
}

//!

bool 
dimePolyline::getRecord(const int groupcode,
		       dimeParam &param,
		       const int index) const
{
  switch(groupcode) {
  case 66:
    param.int16_data = (this->coordCnt || this->indexCnt) ? 1 : 0;
  case 70:
    param.int16_data = this->flags;
    return true;
  case 71:
#ifdef DIME_FIXBIG
    param.int32_data = this->countM;
#else
    param.int16_data = this->countM;
#endif
    return true;
  case 72:
#ifdef DIME_FIXBIG
    param.int32_data = this->countN;
#else
    param.int16_data = this->countN;
#endif
    return true;
  case 73:
#ifdef DIME_FIXBIG
    param.int32_data = this->smoothCountM;
#else
    param.int16_data = this->smoothCountM;
#endif
    return true;
  case 74:
#ifdef DIME_FIXBIG
    param.int32_data = this->smoothCountN;
#else
    param.int16_data = this->smoothCountN;
#endif
    return true;
  case 75:
    param.int16_data = this->surfaceType;
    return true;
  case 10:
  case 20:
  case 30:
    param.double_data = this->elevation[groupcode / 10 - 1];
    return true;
  }
  return dimeExtrusionEntity::getRecord(groupcode, param, index);
}

/*!
  Convenience function that returns the type of the polyline.
*/

int 
dimePolyline::getType() const
{
  if (this->flags & IS_POLYLINE_3D) return POLYLINE;
  if (this->flags & IS_POLYFACE_MESH) return POLYFACE_MESH;
  if (this->flags & IS_POLYMESH_3D) return POLYGON_MESH;

  return this->POLYLINE; // no type flags set => (2D?) POLYLINE
}

//!

dimeEntity::GeometryType 
dimePolyline::extractGeometry(dimeArray <dimeVec3f> &verts,
			     dimeArray <int> &indices,
			     dimeVec3f &extrusionDir,
			     dxfdouble &thickness)
{
  int i;  

  verts.setCount(0);
  indices.setCount(0);
  
  thickness = this->thickness;
  extrusionDir = this->extrusionDir;

  if ((((this->flags & 0x58) == 0) || (this->flags & 0x8)) && 
      this->coordCnt > 1) {
    // this is a polyline 
    for (i = 0; i < this->coordCnt; i++) {
      verts.append(this->coordVertices[i]->coords);
    }
    if (this->flags & 0x1) { // closed polyline 
      dimeVec3f tmp = verts[0];
      verts.append(tmp);
    }
    return dimeEntity::LINES;
  }

  // now we know POLYLINE contains polygonal data
  for (i = 0; i < this->coordCnt; i++) {
    verts.append(this->coordVertices[i]->coords);
  }
  
  if ((this->flags & 0x10) && this->coordCnt > 1) {
    // this is a polygon mesh
    int m = this->countM;
    int n = this->countN;

    int m2 = 0;
    int n2 = 0;

    if (this->surfaceType && this->smoothCountM && this->smoothCountN) {
      m2 = this->smoothCountM;
      n2 = this->smoothCountN;
    }
    
    if (m*n + m2*n2 != this->coordCnt) {
      // FIXME: quick bugfix for stehlen.dxf... weird !
      if ((m-1)*n + m2*n2 == this->coordCnt) m--;
      else {
	if (m*n == this->coordCnt) {
	  m2 = n2 = 0;
	}
	else if (m2*n2 == this->coordCnt) {
	  m = n = 0;
	}
	else { // give up
	  fprintf(stderr,"vertices and faces do no add up: %d * %d + %d * %d != %d.\n",
		  m, n, m2, n2, this->coordCnt);
	  
	  fprintf(stderr,"polyline: %d %d\n", flags, surfaceType);
	  
	  verts.setCount(0);
	  return dimeEntity::NONE;
	}
      }
    }

   
    int idx;
    int idxadd = m2*n2;
    int nexti, nextj;
    int endm = (this->flags & 1) ? m : m-1;
    int endn = (this->flags & 32) ? n : n-1;

    for (i = 0; i < endm; i++) {
      nexti = i+1;
      if (nexti == m) nexti = 0;
      for (int j = 0; j < endn; j++) {
	nextj = j+1;
	if (nextj == n) nextj = 0;
	idx = i*n + j;
	indices.append(idxadd+idx);
	 
	idx = i*n + nextj;
	indices.append(idxadd+idx);
	
	idx = nexti*n + nextj;
	indices.append(idxadd+idx);

	idx = nexti*n + j;
	indices.append(idxadd+idx);
	indices.append(-1);
      }
    }

    idxadd = 0;
    // copied code from above. I'm too lazy to write a loop or
    // a separate function :)
    m = m2;
    n = n2;
    endm = (this->flags & 1) ? m : m-1;
    endn = (this->flags & 32) ? n : n-1;

    for (i = 0; i < endm; i++) {
      nexti = i+1;
      if (nexti == m) nexti = 0;
      for (int j = 0; j < endn; j++) {
	nextj = j+1;
	if (nextj == n) nextj = 0;
	idx = i*n + j;
	indices.append(idxadd+idx);
	 
	idx = i*n + nextj;
	indices.append(idxadd+idx);
	
	idx = nexti*n + nextj;
	indices.append(idxadd+idx);
	
	idx = nexti*n + j;
	indices.append(idxadd+idx);
	indices.append(-1);
      }
    }
    return dimeEntity::POLYGONS;
  }
  
  // this must be a polyface mesh
  if (!this->indexCnt || !this->coordCnt) {
    verts.setCount(0);
    return dimeEntity::NONE;
  }
  for (i = 0; i < this->indexCnt; i++) {
    dimeVertex *v = this->indexVertices[i];
    if (!v->isDeleted()) {
      int num = v->numIndices();
      int idx;
      for (int j = 0; j < num; j++) {
	idx = v->getIndex(j);
	if (idx < 0) { // negative means hidden edge
	  idx = -idx;
	}
	indices.append(idx-1);
      }
      indices.append(-1);
    }
  }
  return dimeEntity::POLYGONS;
  // phew, should probably have spilt this function into several 
  // smaller ones, but I'm not a coward so...
}

/*!
  Returns the number of coordinate vertices.
*/

int 
dimePolyline::numCoordVertices() const
{
  int cnt = 0;
  for (int i = 0; i < this->coordCnt; i++) {
    dimeVertex *v = this->coordVertices[i];
    if (!v->isDeleted()) cnt++;
  }
  return cnt;
}

/*!
  Returns the number of index vertices.
*/

int 
dimePolyline::numIndexVertices() const
{
  int cnt = 0;
  for (int i = 0; i < this->indexCnt; i++) {
    dimeVertex *v = this->indexVertices[i];
    if (!v->isDeleted()) cnt++;
  }
  return cnt;
}

//!

int
dimePolyline::countRecords() const
{
  int cnt = 5; // header + elevation + flags

  if (this->coordCnt || this->indexCnt) cnt++; // vertices follow flag

  if (this->flags & 64) cnt += 2;
  else {
    if (this->countM != 0) cnt++;
    if (this->countN != 0) cnt++;
    if (this->smoothCountM != 0) cnt++;
    if (this->smoothCountN != 0) cnt++;
    if (this->surfaceType != 0) cnt++;
  }

  int i, n = this->coordCnt;
  for (i = 0; i < n; i++) {
    if (!this->coordVertices[i]->isDeleted())
      cnt += this->coordVertices[i]->countRecords();
  }
  n = this->indexCnt;
  for (i = 0; i < n; i++) {
    if (!this->indexVertices[i]->isDeleted())
      cnt += this->indexVertices[i]->countRecords();
  }
  n = this->frameCnt;
  for (i = 0; i < n; i++) {
    if (!this->frameVertices[i]->isDeleted())
      cnt += this->frameVertices[i]->countRecords();
  }
  if (this->seqend) cnt+= this->seqend->countRecords();
  else cnt++; // endseq
  return cnt;
}

/*!
  Sets the coordinate vertices for this polyline. Old vertices will
  be deleted.
*/

void 
dimePolyline::setCoordVertices(dimeVertex **vertices, const int num,
			      dimeMemHandler * const memhandler)
{
  int i;
  if (!memhandler) {
    for (i = 0; i < this->coordCnt; i++) {
      delete this->coordVertices[i];
    } 
    delete [] this->coordVertices;
  }
  if (vertices && num) {
    if (!memhandler || num > this->coordCnt) {
      this->coordVertices = ARRAY_NEW(memhandler, dimeVertex*, num);
    }
    if (this->coordVertices) {
      for (i = 0; i < num; i++) {
	this->coordVertices[i] = vertices[i];
	this->coordVertices[i]->polyline = this;
	
      }
    }
    this->coordCnt = num;
  }
  else {
    this->coordVertices = NULL;
    this->coordCnt = 0;
  }
}

/*!
  Sets the spline frame control point vertices for this polyline. 
  Old control points will be deleted.
*/

void 
dimePolyline::setSplineFrameControlPoints(dimeVertex **vertices, const int num,
					  dimeMemHandler * const memhandler)
{
  int i;
  if (!memhandler) {
    for (i = 0; i < this->frameCnt; i++) {
      delete this->frameVertices[i];
    } 
    delete [] this->frameVertices;
  }
  if (vertices && num) {
    if (!memhandler || num > this->frameCnt) {
      this->frameVertices = ARRAY_NEW(memhandler, dimeVertex*, num);
    }
    if (this->frameVertices) {
      for (i = 0; i < num; i++) {
	this->frameVertices[i] = vertices[i];
	this->frameVertices[i]->polyline = this;
	
      }
    }
    this->frameCnt = num;
  }
  else {
    this->frameVertices = NULL;
    this->frameCnt = 0;
  }
}

/*!
  Sets the index vertices for this polyline. Old vertices will
  be deleted.
*/

void 
dimePolyline::setIndexVertices(dimeVertex **vertices, const int num,
			      dimeMemHandler * const memhandler)
{
  int i;
  if (!memhandler) {
    for (i = 0; i < this->indexCnt; i++) {
      delete this->indexVertices[i];
    } 
    delete [] this->indexVertices;
  }
  if (vertices && num) {
    if (!memhandler || num > this->indexCnt) {
      this->indexVertices = ARRAY_NEW(memhandler, dimeVertex*, num);
    }
    if (this->indexVertices) {
      for (i = 0; i < num; i++) {
	this->indexVertices[i] = vertices[i];
	this->indexVertices[i]->polyline = this;
      }
    }
    this->indexCnt = num;
  }
  else {
    this->indexVertices = NULL;
    this->indexCnt = 0;
  }
}

/*!
  Overloaded from dimeEntity. Will first do a callback for this entity, 
  then for all vertices. Each vertex will have a pointer to its 
  polyline "parent".
*/

bool
dimePolyline::traverse(const dimeState * const state, 
		      dimeCallback callback,
		      void *userdata)
{
  if (this->isDeleted()) return true;
  callback(state, this, userdata);
  int i, n;
  if (state->getFlags() & dimeState::TRAVERSE_POLYLINE_VERTICES) {
    n = this->frameCnt;
    for (i = 0; i < n; i++) {
      if (!this->frameVertices[i]->traverse(state, callback, userdata))
	return false;
    }
    n = this->coordCnt;
    for (i = 0; i < n; i++) {
      if (!this->coordVertices[i]->traverse(state, callback, userdata))
	return false;
    }
    n = this->indexCnt;
    for (i = 0; i < n; i++) {
      if (!this->indexVertices[i]->traverse(state, callback, userdata))
	return false;
    }
  }
  return true;
}

//!

void 
dimePolyline::setLayer(const dimeLayer * const layer)
{
  dimeEntity::setLayer(layer);
  int i;
  for (i = 0; i < this->frameCnt; i++) {
    this->frameVertices[i]->setLayer(layer);
  }
  for (i = 0; i < this->coordCnt; i++) {
    this->coordVertices[i]->setLayer(layer);
  }
  for (i = 0; i < this->indexCnt; i++) {
    this->indexVertices[i]->setLayer(layer);
  }
}

void 
dimePolyline::clearSurfaceData()
{
  this->smoothCountN = this->smoothCountM = 0;
  this->surfaceType = 0;
}

