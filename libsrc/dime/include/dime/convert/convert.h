/**************************************************************************\
 *
 *  This source file is part of DIME.
 *  Copyright (C) 1998-2001 by Systems In Motion.  All rights reserved.
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
 *  NORWAY                                               Fax: +47 22207097
 *
\**************************************************************************/

#ifndef _DXF2VRML_CONVERT_H_
#define _DXF2VRML_CONVERT_H_

#include <stdio.h>
#include <dime/Basic.h>

class dimeModel;
class dxfLayerData;
class dimeState;
class dimeEntity;

class DIME_DLL_API dxfConverter
{
public:
  dxfConverter();
  ~dxfConverter();
  
  void setMaxerr(const dxfdouble maxerr) {
    this->maxerr = maxerr;
  }
  void findHeaderVariables(dimeModel &model);
  bool doConvert(dimeModel &model);
  bool writeVrml(FILE *out, const bool vrml1 = false,
                 const bool only2d = false);

  void setNumSub(const int num) {
    this->numsub = num;
  }
  int getNumSub() const {
    return numsub;
  }
  dxfdouble getMaxerr() const {
    return this->maxerr;
  }

  void setFillmode(const bool fill) {
    this->fillmode = fill;
  }
  bool getFillmode() const {
    return this->fillmode;
  }

  bool getLayercol() const {
    return this->layercol;
  }
  
  void setLayercol(const bool v) {
    this->layercol = v;
  }

  dxfLayerData *getLayerData(const int colidx);
  dxfLayerData *getLayerData(const dimeEntity *entity);
  dxfLayerData ** getLayerData();
  int getColorIndex(const dimeEntity *entity);
  int getCurrentInsertColorIndex() const {
    return currentInsertColorIndex;
  }

private:
  friend class dime2Profit;
  friend class dime2So;

  dxfLayerData *layerData[255];
  int dummy[4];
  dxfdouble maxerr;
  int currentInsertColorIndex;
  dimeEntity *currentPolyline;
  int numsub;
  bool fillmode;
  bool layercol;
  
  bool private_callback(const dimeState * const state, 
			dimeEntity *entity);
  static bool dime_callback(const dimeState * const state, 
			    dimeEntity *entity, void *);

};

#endif // _DXF2VRML_CONVERT_H_
