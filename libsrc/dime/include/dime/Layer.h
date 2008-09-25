/**************************************************************************\
 *
 *  FILE: Layer.h
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

#ifndef DIME_LAYER_H
#define DIME_LAYER_H

#include <dime/Basic.h>

class DIME_DLL_API dimeLayer
{
public:

  enum Flags {
    FROZEN               = 0x1,
    FROZEN_NEW_VIEWPORTS = 0x2,
    LOCKED               = 0x4
  };

  const char *getLayerName() const;
  int getLayerNum() const;

  int16 getColorNumber() const;
  void setColorNumber(const int16 num);

  int16 getFlags() const;
  void setFlags(const int16 &flags);

  bool isDefaultLayer() const;

  static const dimeLayer *getDefaultLayer();

  static void colorToRGB(const int colornum,
                         dxfdouble &r, dxfdouble &g, dxfdouble &b);

private:
  friend class dimeModel;

  dimeLayer();
  dimeLayer(const char * const name, const int num,
            const int16 colnum, const int16 flags);
  const char *layerName;
  int layerNum;
  int16 colorNum;
  int16 flags;

  static void cleanup_default_layer(void);
  static dimeLayer * defaultLayer;

}; // class dimeLayer

inline const char *
dimeLayer::getLayerName() const
{
  return layerName;
}

inline int
dimeLayer::getLayerNum() const
{
  return layerNum;
}

inline int16
dimeLayer::getColorNumber() const
{
  return colorNum;
}

inline void
dimeLayer::setColorNumber(const int16 num)
{
  this->colorNum = num;
}

inline int16
dimeLayer::getFlags() const
{
  return this->flags;
}

inline void
dimeLayer::setFlags(const int16 &flags)
{
  this->flags = flags;
}

inline bool
dimeLayer::isDefaultLayer() const
{
  return this == dimeLayer::getDefaultLayer();
}

#endif // ! DIME_LAYER_H

