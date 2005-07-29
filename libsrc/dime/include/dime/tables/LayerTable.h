/**************************************************************************\
 * 
 *  FILE: LayerTable.h
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

#ifndef DIME_LAYERTABLE_H
#define DIME_LAYERTABLE_H

#include <dime/tables/TableEntry.h>

class DIME_DLL_API dimeLayerTable : public dimeTableEntry 
{
public:
  dimeLayerTable();
  virtual ~dimeLayerTable();
  
  void setLayerName(const char * name, dimeMemHandler * const memhandler);
  const char * getLayerName(void) const;
  
  void setColorNumber(const int16 colnum);
  int16 getColorNumber(void) const;

  void registerLayer(dimeModel * model);

  virtual dimeTableEntry *copy(dimeModel * const model) const;

  virtual const char *getTableName() const;
  virtual bool read(dimeInput * const in);
  virtual bool write(dimeOutput * const out);
  virtual int typeId() const;
  virtual int countRecords() const;

protected:

  virtual bool handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);

private:
  int16 colorNumber;
  char * layerName;
  class dimeLayer * layerInfo;

}; // class dimeLayerTable

#endif // ! DIME_LAYERTABLE_H

