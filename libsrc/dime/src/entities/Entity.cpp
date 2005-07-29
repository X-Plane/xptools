/**************************************************************************\
 * 
 *  FILE: Entity.cpp
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
  \class dimeEntity dime/entities/Entity.h
  \brief The dimeEntity class is the superclass of all \e entity classes.

  If you plan to implement your own entity you should inherit this
  class.  There are some method you need to implement to create an
  entity, and some steps need to be taken if it's going to work 100%
  with the rest of the library. I recommend looking at the code for
  the dimeArc entity when you read this documentation and before you
  implement your own record. This is a very simple entity. It inherits
  the dimeExtrusionEntity class. The dimeExtrusionEntity class is used
  to store extrusion information for an entity, and you should inherit
  this class if your entity needs all of the following group codes: 39
  (thickness), 210, 220, 230 (extrusion direction vector).

  The getRecord() method should be implemented to enable records to be
  queried using the group code of the record. Usually when you implement
  an entity you store some of the records in class members, and let the
  dimeRecordHolder class store the records you're not interested in.
  So, this method should check if the group code parameter matches
  one of the records stored in one of your class members, and then
  return the value of that member. If the requested group code isn't
  store by your class, you should return with a call to your parent's
  class getRecord() method, usually dimeEntity::getRecord() or 
  dimeExtrusionEntity::getRecord().

  The getEntityName() should simply return the DXF name of the entitiy.

  The copy() method should make an exact copy of the entity and return
  this. The dimeModel argument to copy() is the model the copied
  entity will be a part of. First you should create a new instance of
  your entitiy. The model'l memory handler should be passed as an
  argument to new(). Then you should call the copyRecords() method to
  copy records stored by dimeRecordHolder (dimeEntity inherits
  dimeRecordHolder). Then you should copy your data members into the
  new instance. If you inherit the dimeExtrusionEntity you should 
  class copyExtrusionData() before returning.
  
  The write() method should write your entiy using the dimeOutput
  parameter. Fist your should call the preWrite() method. This will
  take care of writing the entity name, and handle ugly stuff such
  as entity handles and ACAD REACTORS data. Then you should write
  your data members. If you inherit from dimeExtrusionEntity method
  you should call writeExtrusionData(). The last thing to do before
  returning is to call dimeEntity::write() to write records not 
  handled by your class.

  The typeId() method should be implemented if your entity is not
  an abstract class. Now you have to edit the include/dime/Base.h,
  and add a unique type name id for your entity. The typeId()
  method should simply return this enum value.

  The countRecords() method is not critical to implement. It should
  return the number of records that will be written by your entity.
  It can be used to create a progress bar while writing a DXF
  file. It is really only useful for _very_ large DXF files. But you
  should implement it since it's not too much work.
  
  Implement the extractGeometry() method if you feel like it. This
  is just a convenience method so you don't have to do this. 

  The handleRecord() should be implemented to support reading entities
  from a file, and to let users set records based on group codes.
  When reading, the handleRecord() method will be called for every
  record found in the entity. If some group code is not supported by
  you, you should call the parent's handleRecord() and it will be
  stored there.  If you need to allocate memory to store the data you
  should check if the dimeMemHandler parameter != NULL and then use it
  to allocate memory.  There is a convenience macro defines in
  include/dime/Basic.h that copies a string, using the memory handler
  if != NULL.

  Well, that's about it I think. Good luck :) Don't hesitate to contact
  us (dime-support@sim.no) if you have questions about how to create
  entities.  
*/

#include <dime/entities/Entity.h>

#include <dime/entities/UnknownEntity.h>
#include <dime/entities/3DFace.h>
#include <dime/entities/Polyline.h>
#include <dime/entities/Vertex.h>
#include <dime/entities/Line.h>
#include <dime/entities/Block.h>
#include <dime/entities/Insert.h>
#include <dime/entities/Solid.h>
#include <dime/entities/Trace.h>
#include <dime/entities/Point.h>
#include <dime/entities/Circle.h>
#include <dime/entities/Arc.h>
#include <dime/entities/LWPolyline.h>
#include <dime/entities/Spline.h>
#include <dime/entities/Ellipse.h>
#include <dime/records/Record.h>
#include <dime/records/Int16Record.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

#include <string.h>
#include <ctype.h>

// misc defines
#define TMP_BUFFER_LEN 1024


/*!
  \fn dimeEntity *dimeEntity::copy(dimeModel * const model) const = 0
  Must be implemented by subclasses to return a copy of the entity.
  \a model is the model the new entity should belong to.
*/

/*!
  \fn const dimeLayer *dimeEntity::getLayer() const
  Returns the layer of this entity.
*/

/*!
  \fn const char *dimeEntity::getEntityName() const = 0
  Must be implemented by subclasses to return the entity name; 
  e.g. POLYLINE, 3DFACE, etc.
*/

/*!
  \fn void dimeEntity::setColorNumber(const int16 c)
  Sets the color number for this entity.
  Zero indicates the BYBLOCK (floating) color. 256 indicates BYLAYER. 
  A negative value indicates that the layer is turned off.
*/

/*!
  \fn int16 dimeEntity::getColorNumber() const
  Returns the color number for this entity.
  
  \sa dimeEntity::setColorNumber()
*/

/*!
  \fn void dimeEntity::print() const
*/

/*!
  Constructor.
*/

dimeEntity::dimeEntity() 
  : dimeRecordHolder(0), entityFlags(0), colorNumber(256) 
{
  this->layer = dimeLayer::getDefaultLayer();
}

/*!
  Destructor.
*/

dimeEntity::~dimeEntity()
{
}

/*!
  Copies the common and unclassified records.
*/

bool
dimeEntity::copyRecords(dimeEntity * const entity, dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  bool ok = dimeRecordHolder::copyRecords(entity, memh);
  
  if (ok && this->layer) {
    entity->layer = model->addLayer(this->layer->getLayerName());
    if (!entity->layer) ok = false;
  }
  entity->entityFlags = this->entityFlags;
  entity->colorNumber = this->colorNumber;  
  return ok;
}

/*!
  Returns if this entity is marked as deleted or not. 
  \sa dimeEntity::setDeleted().
*/

bool
dimeEntity::isDeleted() const
{
  return this->entityFlags & FLAG_DELETED;
}

/*!
  Sets the deleted state of this entity to \a onOff.
  \sa dimeEntity::isDeleted().
*/

void
dimeEntity::setDeleted(const bool onOff)
{
  if (onOff) {
    this->entityFlags |= FLAG_DELETED;
  }
  else {
    this->entityFlags &= ~FLAG_DELETED;
  }
}

/*!
  Useful for developers (at least for me :-).
  \sa dimeEntity::setTagged()
*/
bool 
dimeEntity::isTagged() const
{
  return this->entityFlags & FLAG_TAGGED ? true : false;
}

/*!
  Useful for developers.
  \sa dimeEntity::isTagged()
*/
void 
dimeEntity::setTagged(const bool onOff)
{
  if (onOff) {
    this->entityFlags |= FLAG_TAGGED;
  }
  else {
    this->entityFlags &= ~FLAG_TAGGED;
  }
}



//!

bool 
dimeEntity::write(dimeOutput * const file)
{
  return dimeRecordHolder::write(file);
}

/*!
  Static function which creates an entity based on its name. 
*/

dimeEntity *
dimeEntity::createEntity(const char * const name, 
			 dimeMemHandler * const memhandler)
{
#ifndef NDEBUG
  //fprintf(stderr,"Entity: %s\n", name);
#endif

  //
  // TODO: optimize 
  //
  // right now, I just check for the most common (for 3D gfx)
  // entities first.
  //
  // Could be quite easily solved by using a hash list with
  // pointers to static methods that returns a new instance
  // of the correct type.
  //

  if (!strcmp(name, "3DFACE"))
    return new(memhandler) dime3DFace;
  if (!strcmp(name, "VERTEX"))
    return new(memhandler) dimeVertex;
  if (!strcmp(name, "POLYLINE"))
    return new(memhandler) dimePolyline;
  if (!strcmp(name, "LINE"))
    return new(memhandler) dimeLine;
  if (!strcmp(name, "INSERT"))
    return new(memhandler) dimeInsert;
  if (!strcmp(name, "BLOCK"))
    return new(memhandler) dimeBlock(memhandler);
  if (!strcmp(name, "SOLID"))
    return new(memhandler) dimeSolid;   
  if (!strcmp(name, "TRACE"))
    return new(memhandler) dimeTrace;
  if (!strcmp(name, "POINT"))
    return new(memhandler) dimePoint;
  if (!strcmp(name, "CIRCLE"))
    return new(memhandler) dimeCircle;
  if (!strcmp(name, "LWPOLYLINE"))
    return new(memhandler) dimeLWPolyline;
  if (!strcmp(name, "SPLINE"))
    return new(memhandler) dimeSpline;
  if (!strcmp(name, "ELLIPSE"))
    return new(memhandler) dimeEllipse;
  if (!strcmp(name, "ARC"))
    return new(memhandler) dimeArc;
  return new(memhandler) dimeUnknownEntity(name, memhandler);
}

/*!
  Static function that reads all entities until an entity of type
  \a stopat is found. Returns \e true if all entities were read OK.
  When reading stops, the group code 0 and the entity name (\a stopat)
  will already have been read.
*/

bool 
dimeEntity::readEntities(dimeInput * const file,
			dimeArray <dimeEntity*> &array, 
			const char * const stopat)
{
  int32 groupcode;
  const char *string;
  bool ok = true;
  dimeEntity *entity = NULL;
  dimeMemHandler *memhandler = file->getMemHandler();
  
  while (true) {
    if (!file->readGroupCode(groupcode) || groupcode != 0) {
      fprintf(stderr,"Error reading groupcode: %d\n", groupcode);
      ok = false;
      break;
    }
    string = file->readString();
    if (!strcmp(string, stopat)) break;
    entity = dimeEntity::createEntity(string, memhandler);
    if (entity == NULL) {
      fprintf(stderr,"error creating entity: %s\n", string);
      ok = false;
      break;
    }
    if (!entity->read(file)) {
      fprintf(stderr,"error reading entity: %s.\n", string);
      ok = false;
      break;
    }
    array.append(entity);
  }
  return ok;
}

/*!
  Static function which copies all non-deleted entites from 
  \a array of length \a nument into a
  new array. Will return the number of copied entities in
  \a nument. This function returns \e NULL either when out of
  memory, or if there was no non-deleted entities.
*/

dimeEntity **
dimeEntity::copyEntityArray(const dimeEntity *const*const array, 
                           int &nument,
                           dimeModel * const model)
{
  int i;
  int num = nument;
  dimeMemHandler *memh = model->getMemHandler();

  nument = 0;
  for (i = 0; i < num; i++) { 
    if (!array[i]->isDeleted()) nument++;
  }
  if (nument == 0) return NULL;
    
  dimeEntity **newarr = ARRAY_NEW(memh, dimeEntity*, nument);
  
  bool ok = newarr != NULL;
  if (ok) {
    int cnt = 0;
    for (i = 0; i < num && ok; i++) {
      if (!array[i]->isDeleted()) {
        newarr[cnt] = array[i]->copy(model);
        if (newarr[cnt] == NULL) {
//          sim_trace("failed to copy entity: %d\n", array[i]->typeId());
        }
        ok = newarr[cnt++] != NULL;
      }
    }
    if (!ok && !memh) { // free memory
      for (i = 0; i < cnt; i++)
        delete newarr[i];
      delete [] newarr;
    }
  }
  return ok ? newarr : (dimeEntity**)NULL;
}

/*!
  Static function which copies all non-deleted entites from 
  \a array of length \a nument into \a destarray.
*/

bool
dimeEntity::copyEntityArray(const dimeEntity *const*const array, 
			   const int nument,
			   dimeModel * const model,
			   dimeArray <dimeEntity*> &destarray)
{
  int i;
//  dimeMemHandler *memh = model->getMemHandler();

  int num = 0;
  for (i = 0; i < nument; i++) { 
    if (!array[i]->isDeleted()) num++;
  }
  if (num == 0) {
    destarray.setCount(0);
    return true;
  }

  destarray.makeEmpty(num);
  
  for (i = 0; i < nument; i++) {
    if (!array[i]->isDeleted()) {
      dimeEntity *entity = array[i]->copy(model);
      if (entity == NULL) {
	destarray.setCount(0);
	return false; 
      }
      destarray.append(entity);
    }
  }
  return true;
}

/*!
  Convenience function which returns the layer name of this entity.
*/

const char *
dimeEntity::getLayerName() const
{
  if (this->layer) 
    return this->layer->getLayerName();
  return NULL;
}

/*!
  Used to find all forward references. 
  \sa dimeEntitiesSection::fixReferences().
*/

void 
dimeEntity::fixReferences(dimeModel * const) 
{
}

#define ARBBOUND  0.015625  /* aka "1/64" */

/*!
  Will return an "arbitrary" axis, based on \a givenaxis. Based on code
  from DXF Parser R10, from Autodesk Inc. Is used to find the x-axis
  bases on the given z-axis.
  
  \sa dimeEntity::generateUCS()
*/

void
dimeEntity::arbitraryAxis(const dimeVec3f &givenaxis, dimeVec3f &newaxis)
{
  dimeVec3f yaxis(0.0, 1.0, 0.0);
  dimeVec3f zaxis(0.0, 0.0, 1.0);
  
  if (fabs(givenaxis[0]) < ARBBOUND && 
      fabs(givenaxis[1]) < ARBBOUND) 
    newaxis = yaxis.cross(givenaxis);
  else
    newaxis = zaxis.cross(givenaxis);
 
  newaxis.normalize();
}

/*!
  Will generate a coordinate system, based on \a givenaxis,
  which will be the z-axis in the new coordinate system.

  \sa dimeEntity::arbitraryAxis()
*/

void
dimeEntity::generateUCS(const dimeVec3f &givenaxis, dimeMatrix &m)
{
  dimeVec3f newaxis;
  dimeEntity::arbitraryAxis(givenaxis, newaxis);
  newaxis.normalize();
  dimeVec3f yaxis = givenaxis.cross(newaxis);
  yaxis.normalize();
  m.setRotate(newaxis, yaxis, givenaxis);
}

//!

int 
dimeEntity::countRecords() const
{
  int cnt = 0;
  if (this->layer) cnt++;
  if (this->colorNumber != 256) cnt++;
  return cnt + dimeRecordHolder::countRecords();
}

/*!
  The traversal function used when dimeModel::traverseEntities()
  is called. Most entities use this default method, but some
  entities (INSERT, BUILD) will need to overload it.
*/

bool 
dimeEntity::traverse(const dimeState * const state, 
		    dimeCallback callback,
		    void *userdata)
{
  if (this->isDeleted()) return true;
  return callback(state, this, userdata);
}

/*!
  
  A special convenience function, included for your pleasure.  Enables
  the user to ignore the type of entity, and just call this method
  when extracting geometry.  Very useful for 3D viewers that need DXF
  support. Check out the dxf2vrml directory for an example on how to
  convert a DXF file to VRML.

  Should be overloaded by all subclasses that have geometry, default
  function returns no geometry. Don't forget to transform vertices by
  the current transformation matrix if used in a callback from
  dimeEntity::traverse().
  
  If there are coordinates, but no indices, this means running
  indices for the entire vertex array.
  
  Different faces and/or line segments are separated by a \e -1 in
  the indices array, just as in VRML files.

  If \a thickness != 0.0, the data should, before transformation,
  be extruded by that length along the extrusion direction. Hence,
  a point becomes a line, a line becomes a quad, and a polygon 
  becomes an object with a volume.

  If \a thickness == 0.0 and \a extrusionDir != (0,0,1) all the
  vertices should be transformed by a matrix that can be created
  using dimeEntity::generateUCS(). If you are using 
  dimeModel::traverseEntities() to extract the geometry, simply
  right-multiply the UCS matrix with the matrix found in dimeState
  to get the correct transformation.  

*/

dimeEntity::GeometryType 
dimeEntity::extractGeometry(dimeArray <dimeVec3f> &verts,
			   dimeArray <int> &indices,
			   dimeVec3f &extrusionDir,
			   dxfdouble &thickness)
{
  extrusionDir = dimeVec3f(0,0,1);
  verts.setCount(0);
  indices.setCount(0);
  thickness = 0.0f;
  return NONE;
}

//!

bool 
dimeEntity::isOfType(const int thetypeid) const
{
  return thetypeid == dimeEntityType ||
    dimeRecordHolder::isOfType(thetypeid);
}

/*!
  Reads an entity from \a in. Can be overloaded by subclasses, but in most
  cases this will not be necessary.

  \sa dimeEntity::handleRecord().
*/

bool 
dimeEntity::read(dimeInput * const file)
{
  // a little hack to avoid storing a useless extra pointer in the class.
  // this->layer is used as a temporary char * pointer which will
  // be used in handleRecord().
  // could have used a static buffer, but then the library would not
  // not have been thread safe anymore.
  // As an extra precaution, a flag is set in this->entityFlags to signal to
  // handleRecord() that it is safe to treat this->layer as a char *
  // pointer.

  char tmpbuffer[TMP_BUFFER_LEN+1];
  tmpbuffer[0] = 0;
  const dimeLayer *tmplayer = this->layer;
  this->layer = (const dimeLayer*) tmpbuffer;
  this->entityFlags |= FLAG_TMP_BUFFER_SET;
  bool ok = dimeRecordHolder::read(file); // handleRecord() will change tmpbuffer...
  this->entityFlags &= ~FLAG_TMP_BUFFER_SET;
  this->layer = tmplayer; // reset pointer and flag immediately
  if (ok) {
    // this is a hack. See handleRecord() to understand what happens
    if (tmpbuffer[0] != 0) {
      this->layer = file->getModel()->addLayer(tmpbuffer);
    }
    else this->layer = dimeLayer::getDefaultLayer();
  }
  return ok;  
}

/*!
  Sets the layer for this entity. This will change the record with
  group code 8. If \a layer equals \e NULL, the layer will be set
  to the default layer.
*/

void 
dimeEntity::setLayer(const dimeLayer * const layer)
{
  if (layer == NULL)
    this->layer = dimeLayer::getDefaultLayer();
  else
    this->layer = layer;
}

//!

bool 
dimeEntity::handleRecord(const int groupcode,
			const dimeParam &param,
			dimeMemHandler * const memhandler)
{
  if (groupcode == 8) {
    // Should only arrive her during read(). this->layer is then used
    // as a temporary storage for the character string.
    // Checking flag just to be safe...
    if (this->entityFlags & FLAG_TMP_BUFFER_SET) {
      strncpy((char*)this->layer, param.string_data, TMP_BUFFER_LEN);
    }
    else assert(0);
    return true;
  }
  else if (groupcode == 62) {
    this->entityFlags |= FLAG_COLOR_NUMBER;
    this->colorNumber = param.int16_data;
    return true;
  }
  else if (groupcode == 67) {
    this->entityFlags |= FLAG_PAPERSPACE;
    return true;
  }
  else if (groupcode == 100) {
    this->entityFlags |= FLAG_SUBCLASS_MARKER;
    if (strcmp(param.string_data, "AcDbEntity") != 0) {
      return false; // store in record-holder
    }
    return true;
  }
  else if (groupcode == 102) {
    // ignore these, don't store
    return true;
  }
  else if (groupcode == 330) {
    this->entityFlags |= FLAG_ACAD_REACTORS;
    return false; // store in recordholder
  }
  else if (groupcode == 360) {
    this->entityFlags |= FLAG_ACAD_XDICTIONARY;
    return false; // store in recordholder
  }
  else if (groupcode == 5) {
    this->entityFlags |= FLAG_HANDLE;
    return false; // store in recordholder
  }
  else if (groupcode == 6) {
    this->entityFlags |= FLAG_LINETYPE;
    return false; // store in recordholder
  }
  return false;
}

//!

bool 
dimeEntity::getRecord(const int groupcode,
		     dimeParam &param,
		     const int index) const
{
  switch (groupcode) {
  case 8:
    param.string_data = this->layer->getLayerName();
    return true;
  case 62:
    param.int16_data = this->colorNumber;
    return true;
  }
  return dimeRecordHolder::getRecord(groupcode, param, index);
}

//
// FIXME: getRecord
//

/*!
  Writes the group code 0, layer name, and some other stuff.
*/
bool
dimeEntity::preWrite(dimeOutput * const file)
{
  file->writeGroupCode(0);
  bool ret = file->writeString(this->getEntityName()) ;

  // write stupid handle?
  if (this->entityFlags & FLAG_HANDLE) {
    dimeParam param;
    if (this->getRecord(5, param)) {
      file->writeGroupCode(5);
      file->writeString(param.string_data);
    }
  }

#if 1
  if (this->entityFlags & FLAG_ACAD_REACTORS) {
    file->writeGroupCode(102);
    file->writeString("{ACAD_REACTORS");
    dimeParam param;
    if (this->getRecord(330, param)) {
      file->writeGroupCode(330);
      file->writeString(param.string_data);
    }
    file->writeGroupCode(102);
    file->writeString("}");
  }
  if (this->entityFlags & FLAG_ACAD_XDICTIONARY) {
    file->writeGroupCode(102);
    file->writeString("{ACAD_XDICTIONARY");
    dimeParam param;
    if (this->getRecord(360, param)) {
      file->writeGroupCode(360);
      file->writeString(param.string_data);
    }
    file->writeGroupCode(102);
    file->writeString("}");
  }
#endif

  // write stupid subclass marker data?
  if (this->entityFlags & FLAG_SUBCLASS_MARKER) {
    file->writeGroupCode(100);
    file->writeString("AcDbEntity");
  }
  if (this->entityFlags & FLAG_PAPERSPACE) {
    file->writeGroupCode(67);
    file->writeInt16(1);
  }

  // write layer
  file->writeGroupCode(8);
  ret = file->writeString(this->layer->getLayerName());

  // write linetype
  if (this->entityFlags & FLAG_LINETYPE) {
    dimeParam param;
    if (this->getRecord(6, param)) {
      file->writeGroupCode(6);
      file->writeString(param.string_data);
    }
  }
  
  // write color number
  if (this->colorNumber != 256) {
    file->writeGroupCode(62);
    ret = file->writeInt16(this->colorNumber);
  }

#if 1
  if ((this->entityFlags & FLAG_SUBCLASS_MARKER)) {
    dimeParam param;
    int idx = 0;
    while (this->getRecord(100, param, idx)) {
      file->writeGroupCode(100);
      file->writeString(param.string_data);
      idx++;
    }
  }
#endif
  return ret;
}

//!
bool 
dimeEntity::shouldWriteRecord(const int groupcode) const
{
  switch (groupcode) {
  case 5:
  case 6:
  case 100:
  case 330:
  case 360:
    return false;
  default:
    return true;
  }
}
