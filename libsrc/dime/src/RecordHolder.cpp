/**************************************************************************\
 * 
 *  FILE: RecordHolder.cpp
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
  \class dimeRecordHolder dime/RecordHolder.h
  \brief The dimeRecordHolder class is a superclass for objects that store
  records.

  This class makes it very easy to add new classes to dime, as it handles
  all of the reading, error checking and storing of records of no use to the
  subclass.  Subclasses will only need to implement the
  dimeRecordHolder::handleRecord() and dimeRecordHolder::getRecord() methods.
*/

#include <dime/RecordHolder.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/records/Record.h>

/*!
  Constructor. \a separator is the group code that will separate objects,
  to enable the record holder to stop reading the object at the correct time.
  For instance, entities are separated by group code 0 (a group code 0
  signals a new entity is about to start).
*/

dimeRecordHolder::dimeRecordHolder(const int sep)
  : records( NULL ), numRecords( 0 )
{
  if (sep) assert(false);
  // this->separator = sep;
}

/*!
  Destructor.
*/

dimeRecordHolder::~dimeRecordHolder()
{
  int i, n = this->numRecords;
  for (i = 0; i < n; i++) delete this->records[i];
  delete [] records;
}

//!

bool 
dimeRecordHolder::isOfType(const int thetypeid) const
{
  return thetypeid == dimeRecordHolderType ||
    dimeBase::isOfType(thetypeid);
}

/*!
  Copies the stored records into \a rh.
*/

bool
dimeRecordHolder::copyRecords(dimeRecordHolder * const rh,
                             dimeMemHandler * const memh) const
{
  bool ok = true;
  if (this->numRecords) {
    rh->records = ARRAY_NEW(memh, dimeRecord*, this->numRecords);
    if (rh->records) {
      rh->numRecords = this->numRecords;
      for (int i = 0; i < this->numRecords; i++)
	rh->records[i] = this->records[i]->copy(memh);
    } 
    else ok = false;
  }
  else {
    rh->records = NULL;
    rh->numRecords = 0;
  }
  return ok;
}

/*!
  Reads records from \a in until the separator groupcode (specified in 
  constructor)  is found. Can be overloaded by subclasses, but in most 
  cases this will not be necessary as dimeRecordHolder::handleRecord() 
  is called for each record found in the stream.

  \sa dimeEntity::handleRecord().
*/

bool 
dimeRecordHolder::read(dimeInput * const file)
{
  
  dimeRecord *record;
  bool ok = true;
  int32 groupcode;
  dimeArray <dimeRecord*> array(256); // temporary array
  dimeMemHandler *memhandler = file->getMemHandler();

  while (true) {
    if (!file->readGroupCode(groupcode)) {
      ok = false;
      break;
    }
    if (groupcode == 0 /*this->separator*/) {
      file->putBackGroupCode(groupcode);
      break;
    }
    else { // check if subclass will handle this record
      dimeParam param;
      ok = dimeRecord::readRecordData(file, groupcode, param);
      if (!ok) {
        fprintf( stderr, "Unable to read record data for groupcode: %d\n",groupcode);
//	sim_warning("Unable to read record data for groupcode: %d\n",
//		    groupcode);
	break;
      }
      if (!this->handleRecord(groupcode, param, memhandler)) {
	record = dimeRecord::createRecord(groupcode, param, memhandler);
	if (!record) {
          fprintf( stderr, "Could not create record for group code: %d\n", groupcode );
//	  sim_warning("could not create record for group code: %d\n",
//		      groupcode);
	  ok = false;
	  break;
	}
	array.append(record);
      }
    }
  }
  int num = array.count();
  if (ok && num) {
    this->records = ARRAY_NEW(memhandler, dimeRecord*, num);
    this->numRecords = num;
    for (int i = 0; i < num; i++) {
      this->records[i] = array[i];
    }
  }  
  return ok;  
}

/*!
  Will write the records to \a file.
*/

bool
dimeRecordHolder::write(dimeOutput * const file)
{
  int i, n = this->numRecords;
  for (i = 0; i < n; i++) {
    if (this->shouldWriteRecord(this->records[i]->getGroupCode())) {
      if (!this->records[i]->write(file)) break;
    }
  }
  if (i == n) return true;
  return false;
}

/*!  
  Must be overloaded by entities that directly supports a record
  type. During dimeRecordHolder::read(), dimeRecordHolder::setRecord
  and dimeRecordHolder::setRecords, this function is called for every
  record found, and it is up to the subclass if the record should be
  stored internally, or if a generic record should be created and
  stored in this superclass. A subclass should return \e \true when it
  will handle the record, \e false otherwise.  Default function does
  nothing, and returns \e false.

  For entities, records with group codes 8 (layer name) and 62 (color
  number) are automatically handled by the dimeEntity class.

  \sa dimeRecordHolder::read()
  \sa dimeRecordHolder::setRecord() */

bool 
dimeRecordHolder::handleRecord(const int,
			      const dimeParam &,
			      dimeMemHandler * const)
{
  return false;
}

/*!
  Sets the data for the record with group code \a groupcode. If the
  record already exists, it's value will simply be overwritten,
  otherwise a new record will be created.
  If the record is handled by a subclass, \a param will be passed on 
  to the subclass (using dimeRecordHolder::handleRecord()), and will be 
  ignored by dimeRecordHolder.

  For entities, you cannot use this method to set the layer name. Use
  dimeEntity::setLayer() to do that. Also, you should not use
  this function to set the block name for a dimeInsert entity;
  use dimeInsert::setBlock() instead.
  
  \sa dimeRecordHolder::handleRecord()
  \sa dimeRecordHolder::getRecord()
  \sa dimeRecordHolder::setRecords()
*/

void
dimeRecordHolder::setRecord(const int groupcode, const dimeParam &value, 
			   dimeMemHandler * const memhandler)
{
  this->setRecordCommon(groupcode, value, 0, memhandler);
}

/*!  
  Basically the same function as setRecord(), but also allows you
  to specify an index for the record. This is useful if you're going
  to set several records with the same group code.  
  \sa dimeRecordHolder::setRecord()
*/
  
void 
dimeRecordHolder::setIndexedRecord(const int groupcode, 
                                   const dimeParam &value,
                                   const int index,
                                   dimeMemHandler * const memhandler)
{
  this->setRecordCommon(groupcode, value, index, memhandler);
}

/*!
  Will return the value of the record with group code \a groupcode.
  \e false is returned if the record could not be found.
  Subclasses should overload this method if one or several records are 
  stored in the class. If the groupcode queried is not stored internally, the
  subclass should call its parent's method.
*/

bool 
dimeRecordHolder::getRecord(const int groupcode,
			   dimeParam &param,
			   const int index) const
{
  int i, n = this->numRecords;
  int cnt = 0;
  for (i = 0; i < n; i++) {
    if (this->records[i]->getGroupCode() == groupcode) {
      if (cnt++ == index) {
	this->records[i]->getValue(param);
	return true;
      }
    }
  }
  return false;
}

/*!
  Sets an array of entities. It is much more efficient to use this than
  using dimeRecordHolder::setRecord() several times if you are going to set
  the value of more than one record. Otherwise behaves exactly as
  dimeRecordHolder::setRecord().
*/

void 
dimeRecordHolder::setRecords(const int * const groupcodes,
			    const dimeParam * const params,
			    const int numrecords,
			    dimeMemHandler * const memhandler)
{
  int i;
  dimeArray <dimeRecord*> newrecords(64);

  for (i = 0; i < numrecords; i++) {
    const int groupcode = groupcodes[i];
    const dimeParam &param = params[i];

    if (groupcode == 8) {
      fprintf( stderr, "Cannot set layer name in setRecords()!\n");
//      sim_warning("Cannot set layer name in setRecords()!\n");
      assert(0);
    }
    else if (groupcode == 2 && this->typeId() == dimeBase::dimeInsertType) {
      fprintf( stderr, "Cannot set block name for INSERT entities using setRecords()\n");
//      sim_warning("Cannot set block name for INSERT entities using setRecords()\n");
      assert(0);
    }
    else if (!this->handleRecord(groupcode, param, memhandler)) {
      dimeRecord *record = this->findRecord(groupcode);
      if (record) {
	record->setValue(param);
      }
      else {
	dimeRecord *record = dimeRecord::createRecord(groupcode, 
						    param,
						    memhandler);
	newrecords.append(record);
	
      }
    }
  }
  if (newrecords.count()) {
    // don't forget the old records...
    for (i = 0; i < this->numRecords; i++) { 
      newrecords.append(this->records[i]);
    }
    int n = newrecords.count();
    if (!memhandler) delete [] this->records;
    this->numRecords = 0;
    this->records = ARRAY_NEW(memhandler, dimeRecord*, n);
    if (this->records) {
      this->numRecords = n;
      for (i = 0; i < n; i++) {
	this->records[i] = newrecords[i];
      }
    }
  }
}

/*!
  Returns the number of records in the record holder. Should be overloaded by
  subclasses which should count their records, and then call the parent's
  method. This method is used to precalculate the number of records to be
  written. Very useful when progress information is needed during write().
*/

int 
dimeRecordHolder::countRecords() const
{
  return this->numRecords;
}

/*!  
  Returns the record with group code \a groupcode. If \a index > 0,
  the index'th record with group code \a groupcode will be
  returned. Returns \e NULL if the record is not found or \a index is
  out of bounds.  */

dimeRecord *
dimeRecordHolder::findRecord(const int groupcode, const int index)
{
  int i, n = this->numRecords;
  int cnt = 0;
  for (i = 0; i < n; i++) {
    if (this->records[i]->getGroupCode() == groupcode) { 
      if (cnt == index) return this->records[i];
      cnt++;
    }
  }
  return NULL;
}

/*!
  Can be overloaded by subclasses that want the record holder to
  store a record, but handles writing themselves. Default
  method returns \a true for all group codes.
*/
bool 
dimeRecordHolder::shouldWriteRecord(const int /*groupcode*/) const
{
  return true;
}

void 
dimeRecordHolder::setRecordCommon(const int groupcode, const dimeParam &param,
                                  const int index, dimeMemHandler * const memhandler)
{
  // some safety checks
  if (groupcode == 8 && this->isOfType(dimeBase::dimeEntityType)) {
    fprintf( stderr, "Cannot set layer name in setRecord()!\n");
    assert(0);
    return;
  }
  else if (groupcode == 2 && this->typeId() == dimeBase::dimeInsertType) {
    fprintf( stderr, "Cannot set block name for INSERT entities using setRecord()\n");
    assert(0);
    return;
  }
  
  if (!this->handleRecord(groupcode, param, memhandler)) {
    dimeRecord *record = this->findRecord(groupcode, index);
    if (!record) { // create new record
      record = dimeRecord::createRecord(groupcode, memhandler);
      if (!record) {
	fprintf( stderr, "Could not create record for group code: %d\n", groupcode);
	return;
      }
      dimeRecord **newarray = ARRAY_NEW(memhandler, dimeRecord*, 
					this->numRecords+1); 
      memcpy(newarray, this->records, this->numRecords*sizeof(dimeRecord*));
      if (!memhandler) delete [] this->records;
      this->records = newarray;
      this->records[this->numRecords++] = record;
    }
    record->setValue(param);
  }
}

/*!
  Returns the number of records stored in this record holder.
*/
int 
dimeRecordHolder::getNumRecordsInRecordHolder(void) const
{
  return this->numRecords;
}

/*!
  Returns the \a idx'th record in the record holder.
  \sa getNumRecordsInRecordHolder().
*/
dimeRecord * 
dimeRecordHolder::getRecordInRecordHolder(const int idx) const
{
  assert(idx < this->numRecords);
  return this->records[idx];
}

