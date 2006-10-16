/**************************************************************************\
 * 
 *  FILE: Model.cpp
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
  \class dimeModel dime/Model.h
  \brief The dimeModel class organizes a model.

  The constructor accepts a boolean value which specifies whether or not a 
  memory handler should be used. The special purpose memory handler used
  in Coin can be used if you're just going to read a file and write the
  file, and not do too much dynamic work on the model. The memory handler
  yields very fast allocation/deallocation, and has virtually no overhead
  when allocating. This is important if you have large files with millions
  of records. The disadvantage is that memory will not be freed until the
  model is destructed, so if you modify your model, e.g. remove or replace
  an entity, the memory for the now unused entity will not be freed until
  the model is destructed. Then all used memory will be freed at once.

  Also, if you plan to implement your own entities, it takes a bit of extra
  care to support the memory handler. In short, you should always check
  if a memory allocator should be used before allocating memory, since 
  the destructor for entities will never be called when a memory
  handler is used. See the documentation in dimeEntity for more information
  about how to create your own entities and how to support the memory
  handler.
*/

#include <dime/Model.h>

/*!
  This method returns a string saying which version of DIME is used.j
*/

const char *
dimeModel::getVersionString()
{
  static char versionstring[] = "DIME v0.9";
  return versionstring;
}

void 
dimeModel::getVersion(int &major, int &minor)
{
  major = 0;
  minor = 9;
}

#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/Dict.h>
#include <dime/util/MemHandler.h>
#include <dime/State.h>
#include <dime/sections/Section.h>
#include <dime/sections/EntitiesSection.h>
#include <dime/sections/BlocksSection.h>
#include <dime/sections/HeaderSection.h>
#include <dime/entities/Block.h>
#include <dime/records/Record.h>

#include <string.h>
#include <time.h>

#define SECTIONID "SECTION"
#define EOFID     "EOF"

/*!
  Constructor. If \a usememhandler is \e TRUE, the dimeMemHandler will
  be used to allocate entities and records.
*/
dimeModel::dimeModel(const bool usememhandler)
  : refDict( NULL ), 
  layerDict( NULL ), 
  memoryHandler( NULL ), 
  largestHandle(0),
  usememhandler(usememhandler)
{
  this->init();
}

/*!
  Destructor.
*/

dimeModel::~dimeModel()
{
  int i;
  delete this->refDict;
  delete this->layerDict;

  for (i = 0; i < this->layers.count(); i++) 
    delete this->layers[i];
  for (i = 0; i < this->sections.count(); i++) 
    delete this->sections[i];
  
  delete this->memoryHandler; // free memory :)
}

/*!
  Returns a copy of the model.
*/

dimeModel *
dimeModel::copy() const
{
  dimeModel *newmodel = new dimeModel(this->usememhandler);
  
  if (!newmodel || !newmodel->init()) return NULL;

  newmodel->largestHandle = this->largestHandle;
  int i;
  int n = this->sections.count();

  // refDict and layerDict will be updated during the copy operations
  for (i = 0; i < n; i++)
    newmodel->sections.append(sections[i]->copy(newmodel));
  
  // fix forward references
  dimeBlocksSection *bs = (dimeBlocksSection*) newmodel->findSection("BLOCKS");
  dimeEntitiesSection *es =
    (dimeEntitiesSection*) newmodel->findSection("ENTITIES");
  if (bs) bs->fixReferences(newmodel);
  if (es) es->fixReferences(newmodel);
  return newmodel;
}

/*!  
  Should be called before you start working with the model.  Will
  be called by read() so if you're reading a model from a file you
  will not have to worry about this.

  The method cleans up the old data structures and creates
  new data structures for the new model.  
*/
bool
dimeModel::init()
{
  this->sections.setCount(0);
  this->layers.setCount(0);
  delete this->refDict;
  delete this->layerDict;
  delete this->memoryHandler;

  // set all to NULL first to support exceptions.
  this->refDict = NULL;
  this->layerDict = NULL;
  this->memoryHandler = NULL;
  
  this->refDict = new dimeDict;
  this->layerDict = new dimeDict(101); // relatively small
  if (this->usememhandler) this->memoryHandler = new dimeMemHandler;
  
  return true;
}

/*!
  Reads the model file into the internal structures.
*/

bool 
dimeModel::read(dimeInput * const in)
{
  in->model = this; // _very_ important

  this->init();
  
  int32 groupcode;
  const char *string;
  bool ok = true; 
  dimeSection *section;
  
  while (true) {
    ok = false;
    if (!in->readGroupCode(groupcode)) break;
    if (groupcode != 0 && groupcode != 999) break;
    string = in->readString();
    if (string == NULL) break;
    
    if (groupcode == 999) {
      ok = true;
      dimeParam param;
      param.string_data = string;
      this->headerComments.append(dimeRecord::createRecord(groupcode, 
							   param,
							   memoryHandler));
      continue;
    }
    if (!strcmp(string, SECTIONID)) {
      ok = in->readGroupCode(groupcode);
      string = in->readString();
      ok = ok && string != NULL && groupcode == 2;
      if (!ok) break;
      section = dimeSection::createSection(string, in->getMemHandler());
      ok = section != NULL && section->read(in);
      if (!ok) break;
      this->sections.append(section);
    }
    else if (!strcmp(string, EOFID)) {
      ok = true;
      break;
    }
    else break; // something unexpected has happened
  }	
  if (!ok) {
    if (in->aborted) {
#ifndef NDEBUG
      fprintf(stderr,"DXF read aborted by user.\n");
#endif
    }
    else {
#ifndef NDEBUG
      fprintf( stderr, "DXF loading failed at line: %d\n", in->getFilePosition());
#endif
    }
  }
  else {
    dimeBlocksSection *bs = (dimeBlocksSection*)this->findSection("BLOCKS");
    dimeEntitiesSection *es = (dimeEntitiesSection*)this->findSection("ENTITIES");
    if (bs) bs->fixReferences(this);
    if (es) es->fixReferences(this);
#ifndef NDEBUG
    fprintf(stderr,"dimeModel::largestHandle: %d\n", this->largestHandle);
#endif
  }
  return ok;
}

/*!
  Writes the model to file. Currently only DXF files are supported, but
  hopefullt DWG will be supported soon.
*/

bool 
dimeModel::write(dimeOutput * const out)
{
  if (largestHandle > 0) {
    dimeHeaderSection *hs = (dimeHeaderSection*)
      this->findSection("HEADER");
    
    if (hs) {
      dimeParam param;
      int groupcode;
      if (hs->getVariable("$HANDSEED", &groupcode, &param, 1) == 1) {
	char buf[512];
	this->getUniqueHandle(buf, 512);
	this->largestHandle--; // ok to use this handle next time
	param.string_data = buf;
	hs->setVariable("$HANDSEED", &groupcode, &param, 1, 
			this->getMemHandler());
      }
    }
  }
  out->writeHeader();
  int i, n = this->headerComments.count();
  for (i = 0; i < n; i++) {
    this->headerComments[i]->write(out);
  }

  n = sections.count();
  for (i = 0; i < n; i++) {
    out->writeGroupCode(0);
    out->writeString(SECTIONID);
    if (!sections[i]->write(out)) break;
  }
  if (i == n) {
    return out->writeGroupCode(0) && out->writeString(EOFID);
  }
  return false;
}

/*!
  Adds a reference in this model's dictionary. Used by BLOCK and
  INSERT entities to resolve references, but can also be used 
  for other purposes.
*/

const char * 
dimeModel::addReference(const char * const name, void *id)
{
  char *ptr = NULL;
  refDict->enter(name, ptr, id);
  return (const char*) ptr;
}

/*!
  Finds a reference from the dictionary.
*/

void *
dimeModel::findReference(const char * const name) const
{
  void *id;
  if (refDict->find(name, id))
    return id;
  return NULL; 
}

/*!
  Finds a pointer to a string in the dictionary. 
*/

const char *
dimeModel::findRefStringPtr(const char * const name) const
{
  return refDict->find(name);
}

/*!
  Removes a reference from the dictionary.
*/ 

void 
dimeModel::removeReference(const char * const name)
{
  refDict->remove(name);
}

/*!
  Returns a pointer to the memory handler used for this model.
*/

dimeMemHandler *
dimeModel::getMemHandler()
{
  return this->memoryHandler;
}

/*!
  Adds a layer to the list of layers. If the layer allready exists, a
  pointer to the existing layer will be returned.
*/

const dimeLayer *
dimeModel::addLayer(const char * const name, const int16 colnum,
		    const int16 flags)
{
  void *temp = NULL;
  if (!this->layerDict->find(name, temp)) {
    // default layer has layer-num = 0, hence the + 1
    dimeLayer *layer = new dimeLayer(name, this->layers.count()+1, 
				   colnum, flags);
    char *ptr;
    layerDict->enter(name, ptr, layer);
    // this is a little hack...
    layer->layerName = ptr; // need a pointer that won't disappear
    this->layers.append(layer);
    return layer;
  }
  return (dimeLayer*) temp;
}

/*!
  Returns the layer at index \a idx.
  \sa dimeModel::getNumLayers()
*/

const dimeLayer *
dimeModel::getLayer(const int idx) const
{
  assert(idx >= 0 && idx <= this->layers.count());
  return this->layers[idx];
}

/*!
  Returns the layer with name \a layername. Returns \e NULL if no layer
  by that name is found.
*/

const dimeLayer *
dimeModel::getLayer(const char * const layername) const
{
  void *ptr = NULL;
  this->layerDict->find(layername, ptr);
  return (const dimeLayer*) ptr;
}

/*!
  Returns the number of layers in the model. A default layer will always
  be created. It is called "Default DIME layer", and it has layer id
  number 0. All other layers are assigned running numbers from 1.
  
  \sa dimeLayer::getLayerNum()
  \sa dimeModel::getLayer()
*/

int 
dimeModel::getNumLayers() const
{
  return layers.count();
}

/*!
  Use this to add a block to the model. Make sure you also add it in the
  BLOCKS section.
*/

const char *
dimeModel::addBlock(const char * const blockname, dimeBlock * const block)
{  
  char *ptr = NULL;
  refDict->enter(blockname, ptr, block);
  return (const char*) ptr;
}

/*!
  Returns a pointer to the block with name \a blockname, or \e NULL
  if no block with that name exists.
*/

dimeBlock *
dimeModel::findBlock(const char * const blockname)
{
  void *tmp = NULL;
  this->refDict->find(blockname, tmp);
  return (dimeBlock*)tmp;
}

/*!
  Returns the AutoCAD drawing database version number. This function
  return \e NULL if no version number is found in the file.
  Currently (directly) supported versions are: r10, r11/r12, r13 and r14.
*/

const char *
dimeModel::getDxfVersion() const
{
  const dimeHeaderSection *header =
    (const dimeHeaderSection*) this->findSection("HEADER");
  
  if (!header) {
    return NULL;
  }
  
  int groupcode; 
  dimeParam param;
  
  if (header->getVariable("$ACADVER", &groupcode, &param, 1) != 1 ||
      groupcode != 1) {
    return NULL;
  }
  if (!strcmp(param.string_data, "AC1006")) return "r10";
  if (!strcmp(param.string_data, "AC1009")) return "r11/r12";
  if (!strcmp(param.string_data, "AC1012")) return "r13";
  if (!strcmp(param.string_data, "AC1013")) return "r14";
  
  return NULL;
}

/*!
  Counts the number of records in the file. Useful if you need progress
  information while writing the file to disk.
  
  \sa dimeOutput::setCallback()
*/

int 
dimeModel::countRecords() const
{
  int cnt = 0;
  int i, n = sections.count();
  for (i = 0; i < n; i++) {
    cnt += 1 + this->sections[i]->countRecords(); 
  }
  cnt++; // EOF
  return cnt;
}

/*
  Stupid function to reset the z-value of some rare DXF files that have
  some of their z-coordinates set to -999999. I have no clue what this
  is all about... Somebody FIXME please!
*/

// void 
// dimeModel::fixDxfCoords(dimeVec3f &c)
// {
//   const dxfdouble dummy_dxf_val = -999998.0;
//   if (c[0] <= dummy_dxf_val) c[0] = 0.0;
//   if (c[1] <= dummy_dxf_val) c[1] = 0.0;
//   if (c[2] <= dummy_dxf_val) c[2] = 0.0;
// }

/*!
  Traverses all entities in the model.
*/

bool 
dimeModel::traverseEntities(dimeCallback callback, 
			   void *userdata,
			   bool traverseBlocksSection,
			   bool explodeInserts,
			   bool traversePolylineVertices)
{
  int i, n;
  dimeState state(traversePolylineVertices, explodeInserts);
  if (traverseBlocksSection) {
    dimeBlocksSection *bs =
      (dimeBlocksSection*) this->findSection("BLOCKS");
    if (bs) {
      n = bs->getNumBlocks();
      for (i = 0; i < n; i++) {
	if (!bs->getBlock(i)->traverse(&state, callback, userdata))
	  return false;
      }
    }
  }
  dimeEntitiesSection *es = 
    (dimeEntitiesSection*) this->findSection("ENTITIES");
  if (es) {
    n = es->getNumEntities();
    for (i = 0; i < n; i++) {
      if (!es->getEntity(i)->traverse(&state, callback, userdata))
	return false;
    }
  }

  return true;
}

/*!
  Finds the section with section \a sectionname. Currently (directly) 
  supported sections are HEADER, CLASSES, TABLES, BLOCKS, ENTITIES and OBJECTS.
*/

const dimeSection *
dimeModel::findSection(const char * const sectionname) const
{
  int i, n = this->sections.count();
  for (i = 0; i < n; i++) {
    if (strcmp(this->sections[i]->getSectionName(), sectionname) == 0)
      return this->sections[i];
  }
  return NULL;
}

/*!
  \overload
*/

dimeSection *
dimeModel::findSection(const char * const sectionname)
{
  int i, n = this->sections.count();
  for (i = 0; i < n; i++) {
    if (strcmp(this->sections[i]->getSectionName(), sectionname) == 0)
      return this->sections[i];
  }
  return NULL;
}

/*!
  Returns the number of sections in the model. 

  \sa dimeModel::getSection()
*/

int 
dimeModel::getNumSections() const
{
  return this->sections.count();
}

/*!
  Returns the section at index \a idx.

  \sa dimeModel::getNumSections()
*/

dimeSection *
dimeModel::getSection(const int idx)
{
  assert(idx >= 0 && idx < this->sections.count());
  return this->sections[idx];
}

/*!
  Inserts a new section to the list of sections. The argument \a idx,
  specifies the target position of the new section in the list of sections.
  If \a idx is negative, the section will be placed at the end of the list.

  Sections shold never be allocated on the stack. Use the new/delete
  operators to create/destroy section instances.

*/

void 
dimeModel::insertSection(dimeSection * const section, const int idx)
{
  if (idx < 0) this->sections.append(section);
  else {
    assert(idx <= this->sections.count());
    this->sections.insertElem(idx, section);
  }
}

/*!
  Removes a section from the list of sections.
*/

void 
dimeModel::removeSection(const int idx)
{
  assert(idx >= 0 && idx < this->sections.count());
  delete this->sections[idx];
  this->sections.removeElem(idx);
}

/*!
  Newer DXF files has stupid handles (groupcode 5) for all 
  entities, tables etc. I can't undestand they have no real purpose,
  but all handles must be unique when the file is loaded back into
  AutoCAD...
*/
void 
dimeModel::registerHandle(const int handle)
{
  if (handle >= this->largestHandle) {
    this->largestHandle = handle;
  }
}

/*!
  \overload
*/
void 
dimeModel::registerHandle(const char * const handle)
{
  int num;
  if (sscanf(handle, "%x", &num)) {
    this->registerHandle(num);
  }
}

int 
dimeModel::getUniqueHandle()
{
  return ++this->largestHandle;
}

const char *
dimeModel::getUniqueHandle(char *buf, const int)
{
  sprintf(buf,"%x", getUniqueHandle());
  return buf;
}

/*!
  Convenience function
*/
void 
dimeModel::addEntity(dimeEntity *entity)
{
  dimeEntitiesSection *es = 
    (dimeEntitiesSection*) this->findSection("ENTITIES");
  if (es) {
    es->insertEntity(entity);
  }
}

