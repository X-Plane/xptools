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

#ifndef _DXF2VRML_LAYERDATA_H_
#define _DXF2VRML_LAYERDATA_H_

#include <dime/util/Linear.h>
#include <dime/util/Array.h>
#include <dime/util/BSPTree.h>
#include <stdio.h>

class DIME_DLL_API dxfLayerData {
public:
  dxfLayerData(const int colidx);
  ~dxfLayerData();

  void setFillmode(const bool fillmode);
  
  void addLine(const dimeVec3f &v0, const dimeVec3f &v1,
	       const dimeMatrix * const matrix = NULL);

  void addPoint(const dimeVec3f &v,
		const dimeMatrix * const matrix = NULL);

  void addTriangle(const dimeVec3f &v0,
		   const dimeVec3f &v1,
		   const dimeVec3f &v2,
		   const dimeMatrix * const matrix = NULL);
  void addQuad(const dimeVec3f &v0,
	       const dimeVec3f &v1,
	       const dimeVec3f &v2,
	       const dimeVec3f &v3,
	       const dimeMatrix * const matrix = NULL);
  
  void writeWrl(FILE *fp, int indent, const bool vrml1,
                const bool only2d);

//private:
public: // 20011001 thammer - please don't kill me for this ;-)

  friend class dime2So;
  friend class dime2Profit;

  bool fillmode;
  int colidx;
  dimeBSPTree facebsp;
  dimeArray <int> faceindices;
  dimeBSPTree linebsp;
  dimeArray <int> lineindices;
  dimeArray <dimeVec3f> points;
};

#endif // _DXF2VRML_LAYERDATA_H_
