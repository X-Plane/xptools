// Copyright (c) 2001  Tel-Aviv University (Israel).
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
// $Source: /CVSROOT/CGAL/Packages/Arrangement/include/CGAL/IO/Arr_polyline_traits_Window_stream.h,v $
// $Revision: 1.4 $ $Date: 2003/09/18 10:19:47 $
// $Name: current_submission $
//
// Author(s)     : Ron Wein <wein@post.tau.ac.il>

#ifdef CGAL_ARR_POLYLINE_TRAITS_H
#ifndef CGAL_ARR_POLYLINE_TRAITS_WINDOW_STREAM_H   
#define CGAL_ARR_POLYLINE_TRAITS_WINDOW_STREAM_H  

#include <CGAL/Segment_2.h>
#include <CGAL/IO/Window_stream.h>    

CGAL_BEGIN_NAMESPACE

template <class Segment_traits_>
Window_stream& operator<< (Window_stream& ws,
			   const Polyline_2<Segment_traits_>& pl)
{ 
  typedef Polyline_2<Segment_traits_>          Curve_2;
  typedef typename Curve_2::const_iterator     Points_iterator;
  typedef typename Curve_2::Segment_2          Segment_2;

  Points_iterator   its = pl.begin();

  // Disregard empty polylines:
  if (its == pl.end())
    return (ws);

  // Draw the first point.
  ws << (*its);

  // Draw each segment of the polyline.
  Points_iterator   itt = pl.begin();
  itt++;

  while (itt != pl.end())
  {  
    ws << Segment_2(*its, *itt);
    its++; itt++;
  }

  // Now (*its) is the last polyline point -- draw it as well.
  ws << (*its);

  return (ws);
}

CGAL_END_NAMESPACE

#endif
#endif 









