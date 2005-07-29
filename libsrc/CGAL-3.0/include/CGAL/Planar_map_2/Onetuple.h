// Copyright (c) 1999  Tel-Aviv University (Israel).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source: /CVSROOT/CGAL/Packages/Planar_map/include/CGAL/Planar_map_2/Onetuple.h,v $
// $Revision: 1.4 $ $Date: 2003/09/18 10:24:46 $
// $Name: current_submission $
//
// Author(s)     : Oren Nechushtan
 

#ifndef CGAL_PLANAR_MAP_2_ONETUPLE_H
#define CGAL_PLANAR_MAP_2_ONETUPLE_H

CGAL_BEGIN_NAMESPACE

template < class T >
class _Onetuple : public Rep
{
public:
  T  e0;

  _Onetuple() {}
  _Onetuple(const T & a0) : e0(a0) {}
  ~_Onetuple() {}
};

template < class T >
class Onetuple
{
public:
  T  e0;

  Onetuple() {}
  Onetuple(const T & a0) : e0(a0) {}
};

CGAL_END_NAMESPACE

#endif // CGAL_PLANAR_MAP_2_ONETUPLE_H
