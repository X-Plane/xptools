/**************************************************************************\
 * 
 *  FILE: Record.h
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

#ifndef DIME_RECORD_H
#define DIME_RECORD_H

#include <dime/Basic.h>
#include <dime/Base.h>
#include <stdio.h>

class dimeInput;
class dimeOutput;

class DIME_DLL_API dimeRecord : public dimeBase
{
public: 
  dimeRecord(const int group_code);
  virtual ~dimeRecord();
  
  virtual void setValue(const dimeParam &param, dimeMemHandler * const memhandler = NULL) = 0;
  virtual void getValue(dimeParam &param) const = 0;
  virtual dimeRecord *copy(dimeMemHandler * const memhandler) const = 0;
  
  void setGroupCode(const int group_code);
  int getGroupCode() const;

  
public:    
  virtual bool isEndOfSectionRecord() const;
  virtual bool isEndOfFileRecord() const;
  virtual int typeId() const = 0;
  virtual bool read(dimeInput * const in) = 0;
  virtual bool write(dimeOutput * const out);
  virtual void print() const {fprintf(stderr, "rec: %d\n", groupCode);}

public:
  static bool readRecordData(dimeInput * const in, const int group_code,
			     dimeParam &param);
  static dimeRecord *readRecord(dimeInput * const in);
  static dimeRecord *createRecord(const int group_code, 
				 dimeMemHandler * const memhandler);
  static dimeRecord *createRecord(const int group_code,
				 const dimeParam &param,
				 dimeMemHandler * const memhandler);
  static int getRecordType(const int group_code);
  
protected:
  int groupCode;

}; // class dimeRecord

#endif // ! DIME_RECORD_H

