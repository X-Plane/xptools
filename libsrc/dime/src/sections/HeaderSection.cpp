/**************************************************************************\
 * 
 *  FILE: HeaderSection.cpp
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
  \class dimeHeaderSection dime/sections/HeaderSection.h
  \brief The dimeHeaderSection class handles the HEADER \e section.
*/

#include <dime/sections/HeaderSection.h>
#include <dime/records/Record.h>
#include <dime/records/StringRecord.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>
#include <dime/Input.h>
#include <dime/Output.h>

// FIXME: use a dimeDict to speed up findVariable()

static const char sectionName[] = "HEADER";

/*!
  Constructor
*/

dimeHeaderSection::dimeHeaderSection(dimeMemHandler * const memhandler)
  : dimeSection(memhandler) 
{
}

/*!
  Destructor
*/

dimeHeaderSection::~dimeHeaderSection()
{
  if (!this->memHandler) {
    int i, n = this->records.count();
    for (i = 0; i < n; i++) delete this->records[i];
  }
}

/*!
  Returns a header variable.
  The variable attributes are copied into the \a groupcodes and \a params
  arrays. No more than \a maxparams attributes are copied. 
  This method returns the number of attributes copied into the arrays,
  or -1 if the variable could not be found.
*/

int
dimeHeaderSection::getVariable(const char * const variableName,
			      int * const groupcodes,
			      dimeParam * const params,
			      const int maxparams) const
{
  int i = this->findVariable(variableName);
  if (i >= 0) { // yup, found it!
    i++;
    int cnt = 0;
    int n = this->records.count();
    while (i < n && cnt < maxparams && this->records[i]->getGroupCode() != 9) {
      groupcodes[cnt] = this->records[i]->getGroupCode();
      this->records[i]->getValue(params[cnt]);
      cnt++;
      i++;
    }
    return cnt;
  }
  return -1;
}  

/*!
  Sets a header variable.

  If the variable already exists in the header section, its value is
  overwritten.  Otherwise, a new variable is created and appended to the
  existing variables.
*/

int
dimeHeaderSection::setVariable(const char * const variableName,
			      const int * const groupcodes,
			      const dimeParam * const params,
			      const int numparams,
			      dimeMemHandler * const memhandler)
{
  int i = findVariable(variableName);
  if (i < 0) {
    i = this->records.count();
    dimeStringRecord *sr = (dimeStringRecord*)dimeRecord::createRecord(9, memhandler);
    if (!sr) return false;
    sr->setString(variableName, memhandler);
    
    this->records.append(sr);
    for (int j = 0; j < numparams; j++) {
      this->records.append(dimeRecord::createRecord(groupcodes[j], memhandler));
    }
  }
  i++;
  int cnt = 0;
  for (int j = 0; j < numparams; j++) {
    int k = i;
    int n = this->records.count();
    while (k < n && this->records[k]->getGroupCode() != groupcodes[j] &&
	   this->records[k]->getGroupCode() != 9) {
      k++;
    }
    if (k < n && this->records[k]->getGroupCode() == groupcodes[j]) {
      cnt++;
      this->records[k]->setValue(params[j]);
    }
  }
  return cnt;
}

//!

const char *
dimeHeaderSection::getSectionName() const
{
  return sectionName;
}

//!

dimeSection *
dimeHeaderSection::copy(dimeModel * const model) const
{
  dimeMemHandler *mh = model->getMemHandler();
  dimeHeaderSection *hs = new dimeHeaderSection(mh);
  if (hs) {
    int i, n = this->records.count();
    hs->records.makeEmpty(n);
    for (i = 0; i < n; i++) hs->records.append(this->records[i]->copy(mh));
  }
  return hs;
}

//!

bool 
dimeHeaderSection::read(dimeInput * const file)
{
  dimeRecord *record;
  bool ok = true;
  this->records.makeEmpty(512);
  
  while (true) {
    record = dimeRecord::readRecord(file);
    if (record == NULL) {
      ok = false;
      break;
    }
    if (record->isEndOfSectionRecord()) {
      if (!file->getMemHandler()) delete record; // just delete EOS record
      break;
    }
    this->records.append(record);
  }
  this->records.shrinkToFit();
  return ok;
}

//!

bool 
dimeHeaderSection::write(dimeOutput * const file)
{
  if (file->writeGroupCode(2) && file->writeString(sectionName)) {
    int i, n = this->records.count();
    for (i = 0; i < n; i++) {
      if (!this->records[i]->write(file)) return false;
    }
    // don't forget to write EOS record
    file->writeGroupCode(0);
    return file->writeString("ENDSEC");
  }
  return false;
}

//!

int 
dimeHeaderSection::typeId() const
{
  return dimeBase::dimeHeaderSectionType;
}

//!

int 
dimeHeaderSection::countRecords() const
{
  return this->records.count() + 2; // numrecords + SECTIONNAME + EOS
}

//
// returns the index of the variable, or -1 if variable isn't found. 
//

int
dimeHeaderSection::findVariable(const char * const variableName) const
{
  const int n = this->records.count();
  int i;
  for (i = 0; i < n; i++) {
    if (this->records[i]->getGroupCode() == 9 &&
	!strcmp(((dimeStringRecord*)this->records[i])->getString(),
                variableName)) break;
  }
  if (i < n) return i;
  return -1;
}

