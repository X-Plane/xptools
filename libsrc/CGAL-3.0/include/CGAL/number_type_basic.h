// Copyright (c) 1999  Utrecht University (The Netherlands),
// ETH Zurich (Switzerland), Freie Universitaet Berlin (Germany),
// INRIA Sophia-Antipolis (France), Martin-Luther-University Halle-Wittenberg
// (Germany), Max-Planck-Institute Saarbruecken (Germany), RISC Linz (Austria),
// and Tel-Aviv University (Israel).  All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; version 2.1 of the License.
// See the file LICENSE.LGPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $Source: /CVSROOT/CGAL/Packages/Number_types/include/CGAL/number_type_basic.h,v $
// $Revision: 1.15 $ $Date: 2003/10/21 12:21:47 $
// $Name: current_submission $
//
// Author(s)     : Stefan Schirra
 

#ifndef CGAL_NUMBER_TYPE_BASIC_H
#define CGAL_NUMBER_TYPE_BASIC_H

#define CGAL_PI 3.14159265358979323846

// CGAL_NTS is deprecated, and should be replaced by CGAL::
#define CGAL_NTS CGAL::
// #define CGAL_NTS CGAL::NTS::

#if ((__GNUC__ == 2) && (__GNUC_MINOR__ == 95))
#include <cmath>
#endif

// CGAL uses std::min and std::max if feasible

#include <algorithm>

CGAL_BEGIN_NAMESPACE

#ifndef CGAL_CFG_RETURN_TYPE_BUG

using std::min;
using std::max;

#else

// We have to redefine them to change the return type from const NT& to NT.

template <class NT>
inline
NT
min(const NT& x, const NT& y)
{ return (y < x) ? y : x; }

template <class NT, class Comp>
inline
NT
min(const NT& x, const NT& y, const Comp& c)
{ return c(x, y) ? x : y; }

template <class NT>
inline
NT
max(const NT& x, const NT& y)
{ return (x < y) ? y : x; }

template <class NT, class Comp>
inline
NT
max(const NT& x, const NT& y, const Comp& c)
{ return c(x, y) ? y : x; }

#endif

CGAL_END_NAMESPACE

#include <CGAL/Number_type_traits.h>
#include <CGAL/number_utils.h>
#include <CGAL/double.h>
#include <CGAL/float.h>
#include <CGAL/int.h>
#include <CGAL/number_utils_classes.h>

#endif // CGAL_NUMBER_TYPE_BASIC_H
