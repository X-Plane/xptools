/**************************************************************************\
 * 
 *  FILE: StringRecord.h
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

#ifndef DIME_STRINGRECORD_H
#define DIME_STRINGRECORD_H

#include <dime/records/Record.h>

class DIME_DLL_API dimeStringRecord : public dimeRecord
{
public:
  dimeStringRecord(const int group_code = 0);
  virtual ~dimeStringRecord();

  virtual dimeRecord *copy(dimeMemHandler * const mh) const;
  virtual void setValue(const dimeParam &param, dimeMemHandler * const memhandler = NULL);
  virtual void getValue(dimeParam &param) const;

  void setStringPointer(char * const s);
  bool setString(const char * const s, 
                 dimeMemHandler * const memhandler = NULL);
  char *getString();

public:
  bool isEndOfSectionRecord() const;
  bool isEndOfFileRecord() const;
  int typeId() const;
  bool read(dimeInput * const in);
  bool write(dimeOutput * const out);

protected:
  char *string;
  
}; // class dimeStringRecord

#endif // ! DIME_STRINGRECORD_H

