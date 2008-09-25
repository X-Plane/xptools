/**************************************************************************\
 *
 *  FILE: Base.h
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

#ifndef DIME_BASE_H
#define DIME_BASE_H

#include <dime/Basic.h>
#include <stddef.h>  // for size_t

class dimeMemHandler;

class DIME_DLL_API dimeBase
{
public:
  enum {
    dimeBaseType = 1,
    dimeRecordType,
    dimeStringRecordType,
    dimeFloatRecordType,
    dimeDoubleRecordType,
    dimeInt8RecordType,
    dimeInt16RecordType,
    dimeInt32RecordType,
    dimeHexRecordType,
    dimeRecordHolderType,
    dimeClassType,
    dimeUnknownClassType,
    dimeObjectType,
    dimeUnknownObjectType,
    dimeEntityType,
    dimeUnknownEntityType,
    dimePolylineType,
    dimeVertexType,
    dimeFaceEntityType,
    dimeExtrusionEntityType,
    dime3DFaceType,
    dimeSolidType,
    dimeTraceType,
    dimeLineType,
    dimePointType,
    dimeBlockType,
    dimeInsertType,
    dimeCircleType,
    dimeArcType,
    dimeLWPolylineType,
    dimeEllipseType,
    dimeSplineType,
    dimeSectionType,
    dimeUnknownSectionType,
    dimeEntitiesSectionType,
    dimeBlocksSectionType,
    dimeTablesSectionType,
    dimeHeaderSectionType,
    dimeClassesSectionType,
    dimeObjectsSectionType,
    dimeTableType,
    dimeTableEntryType,
    dimeUnknownTableType,
    dimeUCSTableType,
    dimeLayerTableType,

    // this should be last
    dimeLastTypeTag
  };
  dimeBase(void);
  virtual ~dimeBase();

  virtual int typeId() const = 0;
  virtual bool isOfType(const int thetypeid) const;
public:
  void *operator new(size_t size, dimeMemHandler *memhandler = NULL,
		     const int alignment = 4);
  void operator delete(void *ptr);

}; // class dimeBase

#endif // ! DIME_BASE_H

