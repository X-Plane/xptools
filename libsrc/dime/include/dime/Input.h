/**************************************************************************\
 * 
 *  FILE: Input.h
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

#ifndef DIME_INPUT_H
#define DIME_INPUT_H

#include <dime/Basic.h>
#include <dime/util/Array.h>

#define DXF_MAXLINELEN 4096

class DIME_DLL_API dimeInput
{
public:
  dimeInput();
  ~dimeInput();
  
  bool setFileHandle(FILE *fp);
  bool setFile(const char * const filename);
  bool setFilePointer(const int fd);
  bool eof() const;
  void setCallback(int (*cb)(float, void *), void *cbdata);
  float relativePosition();

  void putBackGroupCode(const int32 code);
  bool readGroupCode(int32 &code);
  bool readInt8(int8 &val);
  bool readInt16(int16 &val);
  bool readInt32(int32 &val);
  bool readFloat(float &val);
  bool readDouble(dxfdouble &val);
  const char *readString();

  class dimeModel *getModel();
  class dimeMemHandler *getMemHandler();
    
  int getFilePosition() const;
  
  bool isBinary() const;
  int getVersion() const;
  bool isAborted() const;
  
private:
  friend class dimeModel;
  dimeModel *model;              // set by the dimeModel class.
  int filePosition;
  bool binary;
  bool binary16bit;
  int version;

  int fd;
#ifdef USE_GZFILE
  void *gzfp; // gzip file pointer
  bool gzeof;
#else // ! USE_GZFILE
  FILE *fp;
  bool fpeof;
#endif // ! USE_GZFILE
  long filesize;
  char *readbuf;
  int readbufIndex;
  int readbufLen;
  
  dimeArray <char> backBuf;
  int backBufIndex; 

  char lineBuf[DXF_MAXLINELEN];
  int32 putBackCode;
  bool hasPutBack;
  int (*callback)(float, void*);
  void *callbackdata;
  float prevposition;
  int cbcnt;
  bool aborted;
  bool prevwashandle;
  bool didOpenFile;
  bool endianSwap;

private:
  bool init();
  bool doBufferRead();
  void putBack(const char c);
  void putBack(const char * const string);
  bool get(char &c);
  bool read(char &c);
  bool skipWhiteSpace();
  bool nextLine();
  bool readInteger(long &l);
  bool readUnsignedInteger(unsigned long &l);
  bool readUnsignedIntegerString(char * const str);
  int readDigits(char * const string);
  int readHexDigits(char * const string);
  int readChar(char * const string, char charToRead);
  bool readReal(dxfdouble &d);
  bool checkBinary();
}; // class dimeInput

#endif // ! DIME_INPUT_H

