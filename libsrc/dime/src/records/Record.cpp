/**************************************************************************\
 * 
 *  FILE: Record.cpp
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
  \class dimeRecord dime/records/Record.h
  \brief The dimeRecord class is the superclass of all \e record classes.
*/

#include <dime/records/Record.h>
#include <dime/util/MemHandler.h>
#include <dime/records/StringRecord.h>
#include <dime/Input.h>
#include <dime/Output.h>

#include <dime/records/FloatRecord.h>
#include <dime/records/DoubleRecord.h>
#include <dime/records/HexRecord.h>
#include <dime/records/Int8Record.h>
#include <dime/records/Int16Record.h> 
#include <dime/records/Int32Record.h>

/*!
  \fn dimeRecord::print() const
  Prints information about this record to \e stderr.
*/

/*!
  \fn dimeRecord *dimeRecord::copy(dimeMemHandler * const memhandler) const = 0
  Returns a copy of this record.
*/

/*!
  \fn void dimeRecord::setValue(const dimeParam &param, dimeMemHandler * const memhandler = NULL) = 0
  Sets the value of this record. The memory handler is needed by dimeStringRecord.
*/

/*!
  \fn void dimeRecord::getValue(dimeParam &param) const = 0
  Returns the value of this record.
*/

/*!
  Constructor which sets the group code.
*/

dimeRecord::dimeRecord(const int group_code)
  : groupCode( group_code )
{
}

/*!
  Destructor.
*/

dimeRecord::~dimeRecord()
{
}

/*!
  Sets the group code of this record.
*/

void 
dimeRecord::setGroupCode(const int group_code)
{
  this->groupCode = group_code;
}

/*!
  Returns the group code for this record.
*/

int 
dimeRecord::getGroupCode() const
{
  return this->groupCode;
}

/*!
  Returns \e true if this record is an end of section record.
*/

bool
dimeRecord::isEndOfSectionRecord() const
{
  return false;
}

/*!
  Returns \e true if this records is an end of file record.
*/

bool
dimeRecord::isEndOfFileRecord() const
{
  return false;
}

/*!
  \fn int dimeRecord::typeId() const
  This virtual function will return the type of the record. 
*/

/*!
  \fn bool dimeRecord::read(dimeInput * const in)
  This function will read the record from the dimeInput file.
*/

/*!
  This function will write the record to the dimeOutput file.
*/

bool 
dimeRecord::write(dimeOutput * const out)
{
  return out->writeGroupCode(groupCode);
}

// * static methods *******************************************************

/*!
  Reads and returns the next record int file \a in.
*/

dimeRecord *
dimeRecord::readRecord(dimeInput * const in)
{
  int32 groupcode;
  dimeRecord *rec = NULL;
  if (in->readGroupCode(groupcode)) {
    rec = dimeRecord::createRecord(groupcode, in->getMemHandler());
    if (rec) rec->read(in);
  }
  return rec;
}

/*!
  Static function that creates a record based on the group code.
  if \a memhandler != NULL, it will be used to allocate the other,
  otherwise the default memory handler will be used.
*/

dimeRecord *
dimeRecord::createRecord(const int group_code, 
			dimeMemHandler * const memhandler)
{  
  int type = getRecordType(group_code);
  dimeRecord *record = NULL;
  switch (type) {
  case dimeBase::dimeStringRecordType:
    record = new(memhandler) dimeStringRecord(group_code);
    break;
  case dimeBase::dimeFloatRecordType:
    record = new(memhandler) dimeFloatRecord(group_code);
    break;
  case dimeBase::dimeDoubleRecordType:
    record = new(memhandler, sizeof(dxfdouble)) dimeDoubleRecord(group_code);
    break;
  case dimeBase::dimeInt8RecordType:
    record = new(memhandler) dimeInt8Record(group_code);
    break;
  case dimeBase::dimeInt16RecordType:
    record  = new(memhandler) dimeInt16Record(group_code);
    break;
  case dimeBase::dimeInt32RecordType:
    record = new(memhandler) dimeInt32Record(group_code);
    break;
  case dimeBase::dimeHexRecordType:
    record = new(memhandler) dimeHexRecord(group_code);
    break;
  default:
    assert(0);
    break;
  }
  return record;
}

//!

dimeRecord *
dimeRecord::createRecord(const int group_code,
			const dimeParam &param,
			dimeMemHandler * const memhandler)
{
  dimeRecord *record = createRecord(group_code, memhandler);
  if (record) record->setValue(param, memhandler);
  return record;
}

//
// local function that returns the type based on the group code
// used to build a look-up table
//

static int 
get_record_type(const int group_code)
{
  int type = dimeBase::dimeStringRecordType;
  
  if (group_code < 0) {
    // not normally used in DXF files, but return string record to 
    // ensure correct read & write
    type = dimeBase::dimeStringRecordType;
  }

  else if (group_code <= 9) {
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 59) {
    // double precision floating point values.
    type = dimeBase::dimeDoubleRecordType;
  }
  // FIXME: this is a fix for some illegal files !!!!
#ifdef DIME_FIXBIG
  else if (group_code <= 70) {
    return dimeBase::dimeInt16RecordType;
  }
  else if (group_code <= 74) {
    return dimeBase::dimeInt32RecordType;
  }
#endif // DIME_FIXBIG
  else if (group_code <= 79) {
    type = dimeBase::dimeInt16RecordType;
  }
  else if (group_code <= 89) {
    // not defined yet. Use string.
    type = dimeBase::dimeStringRecordType;
  } 
  else if (group_code <= 99) {
    type = dimeBase::dimeInt32RecordType;
  }
  else if (group_code < 140) {
    // only 100, 102 and 105 are defined. But use string for the rest also. 
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 147) {
    type = dimeBase::dimeDoubleRecordType;
  }
  else if (group_code < 170) {
    // not defined. Use string.
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 178) {
    type = dimeBase::dimeInt16RecordType;
  }
  // XXX: this is not specified in the spec., but...
  else if (group_code == 210 || group_code == 220 || group_code == 230) {
    type = dimeBase::dimeDoubleRecordType;
  }
  else if (group_code < 270) {
    // not defined. Use string.
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 275) {
    type = dimeBase::dimeInt8RecordType;
  }
  else if (group_code < 280) {
    // not defined.
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 289) {
    type = dimeBase::dimeInt8RecordType;
  }
  else if (group_code < 300) {
    // not defined.
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 309) {
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 319) {
    // binary chunk of data
    type = dimeBase::dimeHexRecordType;
  }
  else if (group_code <= 329) {
    // hex handle value
    type = dimeBase::dimeHexRecordType;
  }
  else if (group_code <= 369) {
    // hexvalue for object ID
    type = dimeBase::dimeHexRecordType;
  }
  else if (group_code < 999) {
    // not defined.
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code == 999) {
    // comment
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 1009) {
    // 255 character max string.
    type = dimeBase::dimeStringRecordType;
  }
  else if (group_code <= 1059) {
    type = dimeBase::dimeStringRecordType;
    // should be float according to spec, but I have found
    // _huge_ numbers here that do not fit into a float.
    //    type = dimeBase::dimeFloatRecordType;
  }
  else if (group_code <= 1070) {
    type = dimeBase::dimeInt16RecordType;
  }
  else if (group_code == 1071) {
    type = dimeBase::dimeInt32RecordType;
  }
  else type = dimeBase::dimeStringRecordType;
  return type;
}

/*!
  Static function that returns the record type based on
  the group code.
*/

int 
dimeRecord::getRecordType(const int group_code)
{
  static int first = 1;
  static int translation[1072];
  if (first) {
    first = 0;
    for (int i = 0; i < 1072; i++) {
      translation[i] = get_record_type(i);
    }
  }
  if (group_code < 0 || group_code >= 1072)
    return dimeBase::dimeStringRecordType;
  else return translation[group_code];
}

/*!
  Will read the next item from \a in, and store result in
  \a param, based on the \a group_code.
*/

bool 
dimeRecord::readRecordData(dimeInput * const in, const int group_code,
			  dimeParam &param)
{
  bool ret;
  int type = getRecordType(group_code);
  
  switch(type) {
  case dimeBase::dimeInt8RecordType:
    ret = in->readInt8(param.int8_data);
    break;
  case dimeBase::dimeInt16RecordType:
    ret = in->readInt16(param.int16_data);
    break;
  case dimeBase::dimeInt32RecordType:
    ret = in->readInt32(param.int32_data);
    break;
  case dimeBase::dimeFloatRecordType:
    ret = in->readFloat(param.float_data);
    break;
  case dimeBase::dimeDoubleRecordType:
    ret = in->readDouble(param.double_data);
    break;
  case dimeBase::dimeStringRecordType:
    param.string_data = in->readString();
    ret = param.string_data != NULL;
    break;
  case dimeBase::dimeHexRecordType:
    param.hex_data = in->readString();
    ret = param.hex_data != NULL;
    break;
  default:
    assert(0);
    ret = false;
    break;
  }
  return ret;
}

