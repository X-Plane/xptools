/**************************************************************************\
 * 
 *  FILE: RecordHolder.h
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

#ifndef DIME_RECORDHOLDER_H
#define DIME_RECORDHOLDER_H

#include <dime/Base.h>

class dimeInput;
class dimeMemHandler;
class dimeOutput;
class dimeRecord;

class DIME_DLL_API dimeRecordHolder : public dimeBase
{
public:
  dimeRecordHolder(const int separator);
  virtual ~dimeRecordHolder();

  void setRecord(const int groupcode, const dimeParam &value, 
		 dimeMemHandler * const memhandler = NULL);
  void setRecords(const int * const groupcodes,
		  const dimeParam * const params,
		  const int numrecords,
		  dimeMemHandler * const memhandler = NULL);
  void setIndexedRecord(const int groupcode, 
                        const dimeParam &value,
                        const int index,
                        dimeMemHandler * const memhandler = NULL);
  
  virtual bool getRecord(const int groupcode,
			 dimeParam &param,
			 const int index = 0) const;
  
  virtual bool read(dimeInput * const in);
  virtual bool write(dimeOutput * const out);
  virtual bool isOfType(const int thetypeid) const;
  virtual int countRecords() const;

  dimeRecord *findRecord(const int groupcode, const int index = 0);

  int getNumRecordsInRecordHolder(void) const;
  dimeRecord * getRecordInRecordHolder(const int idx) const;

protected:
  virtual bool handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);
  
  bool copyRecords(dimeRecordHolder * const rh, 
		   dimeMemHandler * const memhandler) const;

  virtual bool shouldWriteRecord(const int groupcode) const;

protected:
  dimeRecord **records;
  int numRecords;
  // int separator; // not needed ?

private:
  void setRecordCommon(const int groupcode, const dimeParam &param,
                       const int index, dimeMemHandler * const memhandler);

}; // class dimeRecordHolder

#endif // ! DIME_RECORDHOLDER_H

