/**************************************************************************\
 * 
 *  FILE: Output.cpp
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
  \class dimeOutput dime/Output.h
  \brief The dimeOutput class handles writing of DXF and DXB files.
*/

#include <dime/Output.h>
#include <math.h>

/*!
  \fn bool dimeOutput::writeHeader()
  This method does nothing now, but if binary files are supported in the
  future, it must be called.
*/

/*!
  Constructor.
*/

dimeOutput::dimeOutput()
  : fp( NULL ), binary( false ), callback( NULL ), callbackdata( NULL ),
    aborted( false ), didOpenFile(false)
{
}

/*!
  Destructor.
*/

dimeOutput::~dimeOutput()
{
  if (this->fp && this->didOpenFile) fclose(this->fp);
}

/*!
  This method sets a callback function that is called with progress
  information.  The first argument of the callback is a float in the
  range between 0 and 1.  The second argument of the callback is the
  void * \a cbdata argument.
*/

void 
dimeOutput::setCallback(const int num_records, 
		       int (*cb)(float, void *), void *cbdata)
{
  this->callback = cb;
  this->callbackdata = cbdata;
  this->numwrites = 0;
  this->numrecords = num_records;
}

/*!
  Sets the filename for the output file. The file will be opened,
  and \e true is returned if all was ok. The file is closed in
  the destructor.
*/

bool
dimeOutput::setFilename(const char * const filename)
{
  if (this->fp && this->didOpenFile) fclose(this->fp);
  this->fp = fopen(filename, "wb");
  this->didOpenFile = true;
  return (this->fp != NULL);
}

/*!
  Sets the output stream. \fp should be a valid file/stream, and
  it will not be closed in the destructor.
 */
bool 
dimeOutput::setFileHandle(FILE *fp)
{
  if (this->fp && this->didOpenFile) fclose(this->fp);

  assert(fp);
  this->fp = fp;
  this->didOpenFile = false;
  return true;
}

/*!
  Sets binary (DXB) or ASCII (DXF) format. Currently only ASCII
  is supported.
*/

void
dimeOutput::setBinary(const bool state)
{
  this->binary = state;
}

/*!
  Returns if binary or ASCII will be used when writing.
*/

bool
dimeOutput::isBinary() const
{
  return this->binary;
}

/*!
  Writes a record group code to the file.
*/

bool
dimeOutput::writeGroupCode(const int groupcode)
{
  if (this->aborted) return false;
  if (this->callback && this->numrecords) {
    if ((this->numwrites & 255) == 0) {
      float val = float(this->numwrites) / float(this->numrecords);
      if (val > 1.0f) val = 1.0f;
      this->aborted = !(bool) callback(val, this->callbackdata);
    }
    this->numwrites++;
  }
  return fprintf(this->fp, "%3d\n", groupcode) > 0;
}

/*!
  Writes an 8 bit integer to the file.
*/

bool
dimeOutput::writeInt8(const int8 val)
{
  return fprintf(this->fp,"%6d\n", (int)val) > 0;
}

/*!
  Writes a 16 bit integer to the file.
*/

bool
dimeOutput::writeInt16(const int16 val)
{
  return fprintf(this->fp,"%6d\n", (int)val) > 0;
}

/*!
  Writes a 32 bit integer to the file.
*/

bool
dimeOutput::writeInt32(const int32 val)
{
  return fprintf(this->fp,"%6d\n", val) > 0;
}

/*!
  Writes a single precision floating point number to the file.
*/

bool
dimeOutput::writeFloat(const float val)
{
  return fprintf(this->fp, "%g\n", val);
}

/*!
  Writes a double precision floating point number to the file.  
*/

bool
dimeOutput::writeDouble(const dxfdouble val)
{
  return fprintf(this->fp,"%g\n", val) > 0;
}

/*!
  Writes a nul-terminated string to the file. 
*/

bool
dimeOutput::writeString(const char * const str)
{
  return fprintf(this->fp, "%s\n", str) > 0;
}

int
dimeOutput::getUniqueHandleId()
{
  // FIXME
  return 1;
}

