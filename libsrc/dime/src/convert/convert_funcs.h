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

#ifndef _DXF2VRML_CONVERT_FUNCS_H_
#define _DXF2VRML_CONVERT_FUNCS_H_

class dimeEntity;
class dimeState;
class dxfLayerData;
class dxfConverter;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif // M_PI

void convert_3dface(const dimeEntity *, const dimeState *,
		    dxfLayerData *, dxfConverter *);
void convert_line(const dimeEntity *, const dimeState *,
		  dxfLayerData *, dxfConverter *);
void convert_point(const dimeEntity *, const dimeState *,
		   dxfLayerData *, dxfConverter *);
void convert_circle(const dimeEntity *, const dimeState *,
		    dxfLayerData *, dxfConverter *);
void convert_ellipse(const dimeEntity *, const dimeState *,
		     dxfLayerData *, dxfConverter *);
void convert_arc(const dimeEntity *, const dimeState *,
		 dxfLayerData *, dxfConverter *);
void convert_solid(const dimeEntity *, const dimeState *,
		   dxfLayerData *, dxfConverter *);
void convert_trace(const dimeEntity *, const dimeState *,
		   dxfLayerData *, dxfConverter *);
void convert_polyline(const dimeEntity *, const dimeState *,
		      dxfLayerData *, dxfConverter *);
void convert_lwpolyline(const dimeEntity *, const dimeState *,
			dxfLayerData *, dxfConverter *);

#endif // _DXF2VRML_CONVERT_FUNCS_H_
