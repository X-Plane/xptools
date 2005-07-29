/**************************************************************************\
 * 
 *  FILE: Class.h
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

#ifndef DIME_CLASS_H
#define DIME_CLASS_H

#include <dime/Base.h>
#include <dime/Basic.h>
#include <dime/util/Array.h>
#include <dime/util/Linear.h>
#include <dime/RecordHolder.h>

class dimeInput;
class dimeMemHandler;
class dimeOutput;

class DIME_DLL_API dimeClass : public dimeRecordHolder
{
  friend class dimeClassesSection;
  friend class dimeModel;
  
public:
  dimeClass();
  virtual ~dimeClass();
  
  virtual const char *getDxfClassName() const = 0;  
  virtual dimeClass *copy(dimeModel * const model) const = 0; 
  virtual bool read(dimeInput * const in);
  virtual bool write(dimeOutput * const out);
  virtual bool isOfType(const int thetypeid) const;
  virtual int countRecords() const;

  const char *getClassName() const;
  const char *getApplicationName() const;
  int32 getVersionNumber() const;
  int8 getFlag280() const;
  int8 getFlag281() const;

  void setClassName(const char * const classname, 
		    dimeMemHandler * const memhandler = NULL);
  void setApplicationName(const char * const appname, 
			  dimeMemHandler * const memhandler = NULL);
  void setVersionNumber(const int32 v);
  void setFlag280(const int8 flag);
  void setFlag281(const int8 flag);

protected:
  virtual bool handleRecord(const int groupcode,
			    const dimeParam &param,
			    dimeMemHandler * const memhandler);
  
public:
  static dimeClass *createClass(const char * const name,
			       dimeMemHandler * const memhandler = NULL);    
protected:
  bool copyRecords(dimeClass * const newclass, dimeModel * const model) const;

private:
  char *className;
  char *appName;
  int32 versionNumber;
  int8 flag1;
  int8 flag2;

}; // class dimeClass

inline const char *
dimeClass::getClassName() const
{
  return this->className;
}

inline const char *
dimeClass::getApplicationName() const
{
  return this->appName;
}

inline int32 
dimeClass::getVersionNumber() const
{
  return this->versionNumber;
}

inline int8 
dimeClass::getFlag280() const
{
  return this->flag1;
}

inline int8 
dimeClass::getFlag281() const
{
  return this->flag2;
}

inline void 
dimeClass::setVersionNumber(const int32 v)
{
  this->versionNumber = v;
}

inline void 
dimeClass::setFlag280(const int8 flag)
{
  this->flag1 = flag;
}

inline void 
dimeClass::setFlag281(const int8 flag)
{
  this->flag2 = flag;
}

#endif // ! DIME_CLASS_H

