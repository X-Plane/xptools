/**************************************************************************\
 * 
 *  FILE: Insert.h
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

#ifndef DIME_INSERT_H
#define DIME_INSERT_H

#include <dime/Basic.h>
#include <dime/entities/Entity.h>
#include <dime/util/Linear.h>

class DIME_DLL_API dimeInsert : public dimeEntity
{
  friend class dimeEntitiesSection;
  friend class dimeBlocksSection;

public:
  dimeInsert();
  virtual ~dimeInsert();

  void setBlock(dimeBlock * const block);
  dimeBlock * getBlock() const;

  virtual dimeEntity *copy(dimeModel * const model) const;
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;
  virtual const char *getEntityName() const;

  virtual bool read(dimeInput * const in);
  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;

  void setInsertionPoint(const dimeVec3f &v);
  const dimeVec3f &getInsertionPoint() const;

  void setScale(const dimeVec3f &v);
  const dimeVec3f & getScale() const;
  
  void setRotAngle(dxfdouble angle);
  dxfdouble getRotAngle() const;

  // FIXME: more set and get methods
  
protected:
  virtual void fixReferences(dimeModel * const model);
  virtual bool handleRecord(const int groupcode, 
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);
  virtual bool traverse(const dimeState * const state, 
                        dimeCallback callback,
                        void *userdata);

private:
  void makeMatrix(dimeMatrix &m) const;

  int16 attributesFollow;
  const char *blockName;
  dimeVec3f insertionPoint;
  dimeVec3f extrusionDir;
  dimeVec3f scale;
  dxfdouble rotAngle;
  dimeEntity **entities;
  int numEntities;
#ifdef DIME_FIXBIG
  int32 rowCount;
#else
  int16 rowCount;
#endif
  int16 columnCount;
  dxfdouble rowSpacing;
  dxfdouble columnSpacing;
  dimeEntity *seqend;
  dimeBlock *block;

}; // class dimeInsert



inline void 
dimeInsert::setInsertionPoint(const dimeVec3f &v)
{
  this->insertionPoint = v;
}

inline const dimeVec3f &
dimeInsert::getInsertionPoint() const
{
  return this->insertionPoint;
}

inline dimeBlock *
dimeInsert::getBlock() const
{
  return this->block;
}

inline void 
dimeInsert::setScale(const dimeVec3f &v)
{
  this->scale = v;
}

inline const dimeVec3f &
dimeInsert::getScale() const
{
  return this->scale;
}

inline void 
dimeInsert::setRotAngle(dxfdouble angle)
{
  this->rotAngle = angle;
}

inline dxfdouble
dimeInsert::getRotAngle() const
{
  return this->rotAngle;
}


#endif // ! DIME_INSERT_H

