/**************************************************************************\
 * 
 *  FILE: Class.cpp
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
  \class dimeClass dime/classes/Class.h
  \brief The dimeClass class is the superclass for all the \e class classes.
*/

#include <dime/classes/Class.h>
#include <dime/classes/UnknownClass.h>
#include <dime/Input.h>
#include <dime/Output.h>
#include <dime/util/MemHandler.h>
#include <dime/Model.h>

#include <string.h>

/*!
  \fn const char *dimeClass::getDxfClassName() const = 0
  Must be implemented by subclasses to return the dxf name of the class.
*/

/*!
  \fn dimeClass *dimeClass::copy(dimeModel * const model) const = 0
  Must be implemented by subclasses to return a copy of the class.
*/

/*!
  \fn const char *dimeClass::getClassName() const
  Returns the C++ class name.
*/

/*!
  \fn const char *dimeClass::getApplicationName() const
  Returns the class application name.
*/

/*!
  \fn int32 dimeClass::getVersionNumber() const
  Returns the version number of this class.
*/

/*!
  \fn int8 dimeClass::getFlag280() const
  Returns the flag with group code 280.
*/

/*!
  \fn int8 dimeClass::getFlag281() const
  Returns the flag with group code 281.
*/

/*! 
  \fn void dimeClass::setVersionNumber(const int32 v)
  Sets the version number for this class.
*/

/*! 
  \fn void dimeClass::setFlag280(const int8 flag)
  Sets the flag for the flag with group code 280.
*/

/*!
  \fn void dimeClass::setFlag281(const int8 flag)
  Sets the group code for the flag with group code 281.
*/
 
/*!
  Constructor.
*/

dimeClass::dimeClass() 
  : dimeRecordHolder( 0 ), // classes are separated by group code 0
    className( NULL ), appName( NULL ), versionNumber( 0 ), flag1( 0 ),
    flag2( 0 )
{
}

/*!
  Destructor.
*/

dimeClass::~dimeClass()
{
  delete this->className;
  delete this->appName;
}

/*!
  Copies the common and unclassified records.
*/

bool
dimeClass::copyRecords(dimeClass * const myclass, dimeModel * const model) const
{
  dimeMemHandler *memh = model->getMemHandler();
  bool ok = dimeRecordHolder::copyRecords(myclass, memh);
  
  if (ok && this->className) {
    DXF_STRCPY(memh, myclass->className, this->className);
    ok = myclass->className != NULL;
  }
  if (ok && this->appName) {
    DXF_STRCPY(memh, myclass->appName, this->appName);
    ok = myclass->className != NULL;
  }
  if (ok) {
    myclass->versionNumber = this->versionNumber;
    myclass->flag1 = this->flag1;
    myclass->flag2 = this->flag2;
  }
  return ok;
}

/*!
  Writes common and unknown class records to file.
*/

bool 
dimeClass::write(dimeOutput * const file)
{
  file->writeGroupCode(1);
  file->writeString(this->className ? this->className : "Default class name");
  file->writeGroupCode(2);
  file->writeString(this->appName ? this->appName : "Default app name");
  file->writeGroupCode(90);
  file->writeInt32(this->versionNumber);
  file->writeGroupCode(280);
  file->writeInt8(this->flag1);
  file->writeGroupCode(281);
  file->writeInt8(this->flag2);
  return dimeRecordHolder::write(file);
}

/*!
  Static function which creates an class based on its name. 
*/

dimeClass *
dimeClass::createClass(const char * const name, 
		      dimeMemHandler * const memhandler)
{
  return new(memhandler) dimeUnknownClass(name, memhandler);
}

//!

int 
dimeClass::countRecords() const
{
  return 5 + dimeRecordHolder::countRecords();
}

//!

bool 
dimeClass::isOfType(const int thetypeid) const
{
  return thetypeid == dimeClassType || dimeRecordHolder::isOfType(thetypeid);
}

/*!
  Reads a class from \a in. Can be overloaded by subclasses, but in most
  cases this will not be necessary.

  \sa dimeClass::handleRecord().
*/

bool 
dimeClass::read(dimeInput * const file)
{
  return dimeRecordHolder::read(file);
}

//!

bool 
dimeClass::handleRecord(const int groupcode,
			const dimeParam &param,
			dimeMemHandler * const memhandler)
{
  switch (groupcode) {
  case 1:
    DXF_STRCPY(memhandler, this->className, param.string_data);
    return true;
  case 2:
    DXF_STRCPY(memhandler, this->appName, param.string_data);
    return true;
  case 90:
    this->versionNumber = param.int32_data;
    return true;
  case 280:
    this->flag1 = param.int8_data;
    return true;
  case 281:
    this->flag2 = param.int8_data;
    return true;
  }
  return false;
}

/*!
  Sets the C++ class name for this class.
*/

void 
dimeClass::setClassName(const char * const classname, 
		       dimeMemHandler * const memhandler)
{
  if (!memhandler) delete this->className;
  DXF_STRCPY(memhandler, this->className, classname);
}

/*!
  Sets the application name for this class.
*/

void 
dimeClass::setApplicationName(const char * const appname, 
			     dimeMemHandler * const memhandler)
{
  if (!memhandler) delete this->appName;
  DXF_STRCPY(memhandler, this->appName, appname);
}

