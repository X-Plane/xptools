/**************************************************************************\
 * 
 *  FILE: Model.h
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

#ifndef DIME_MODEL_H
#define DIME_MODEL_H

#include <dime/Basic.h>
#include <dime/util/Array.h>
#include <dime/util/Linear.h>
#include <dime/Base.h>
#include <dime/Layer.h>
#include <stdlib.h>

class dimeInput;
class dimeOutput;
class dimeDict;
class dimeMemHandler;
class dimeSection;
class dimeEntitiesSection;
class dimeBlocksSection;
class dimeBlock;
class dimeEntity;
class dimeRecord;

class DIME_DLL_API dimeModel
{
public:
  dimeModel(const bool usememhandler = false);
  ~dimeModel();
  
  dimeModel *copy() const;

  bool init();
  bool read(dimeInput * const in);
  bool write(dimeOutput * const out);

  int countRecords() const;

  bool traverseEntities(dimeCallback callback, 
			void *userdata = NULL,
			bool traverseBlocksSection = false,
			bool explodeInserts = true,
			bool traversePolylineVertices = false);
  
  const char *addReference(const char * const name, void *id);
  void *findReference(const char * const name) const;
  const char *findRefStringPtr(const char * const name) const;
  void removeReference(const char * const name);
  class dimeMemHandler *getMemHandler();
  
  int getNumLayers() const;
  const class dimeLayer *getLayer(const int idx) const;
  const class dimeLayer *getLayer(const char * const layername) const;
  const class dimeLayer *addLayer(const char * const layername, 
				  const int16 colnum = 7,
				  const int16 flags = 0); 
  
  const char * getDxfVersion() const;
  
  static const char *getVersionString();
  static void getVersion(int &major, int &minor);
  const char *addBlock(const char * const blockname, dimeBlock * const block);
  class dimeBlock *findBlock(const char * const blockname);

  class dimeSection *findSection(const char * const sectionname);
  const class dimeSection *findSection(const char * const sectionname) const;

  int getNumSections() const;
  class dimeSection *getSection(const int idx);
  void insertSection(dimeSection * const section, const int idx = -1);
  void removeSection(const int idx);

  void registerHandle(const int handle);
  void registerHandle(const char * const handle);
  int getUniqueHandle();
  const char *getUniqueHandle(char *buf, const int bufsize);
  void addEntity(dimeEntity *entity);

private:
  class dimeDict *refDict;
  class dimeDict *layerDict;
  class dimeMemHandler *memoryHandler;
  dimeArray <class dimeSection*> sections;
  dimeArray <class dimeLayer*> layers;
  dimeArray <dimeRecord*> headerComments;

  int largestHandle;
  bool usememhandler;
}; // class dimeModel

#endif // ! DIME_MODEL_H

