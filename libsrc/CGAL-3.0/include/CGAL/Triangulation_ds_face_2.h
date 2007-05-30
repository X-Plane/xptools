// Copyright (c) 1997  INRIA Sophia-Antipolis (France).
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
// $Source: /CVSROOT/CGAL/Packages/Triangulation_2/include/CGAL/Triangulation_ds_face_2.h,v $
// $Revision: 1.40 $ $Date: 2003/09/18 10:26:12 $
// $Name: current_submission $
//
// Author(s)     : Mariette Yvinec

#ifndef CGAL_TRIANGULATION_DS_FACE_2_H
#define CGAL_TRIANGULATION_DS_FACE_2_H

#include <CGAL/basic.h>
#include <CGAL/Triangulation_short_names_2.h>
#include <CGAL/Triangulation_utils_2.h>

CGAL_BEGIN_NAMESPACE 

template < class Fb >
class  Triangulation_ds_face_2
  : public Fb  
{
public:
  typedef typename Fb::Triangulation_data_structure Tds;
  typedef typename Tds::Vertex             Vertex;
  typedef typename Tds::Face               Face;
  typedef typename Tds::Vertex_handle      Vertex_handle;
  typedef typename Tds::Face_handle        Face_handle;

public :
  // creators
  Triangulation_ds_face_2()
    : Fb()
  {}
    
  Triangulation_ds_face_2(Vertex_handle v0, Vertex_handle v1, Vertex_handle v2)
    :  Fb(v0,v1,v2)
  {}
    
  Triangulation_ds_face_2(Vertex_handle v0, Vertex_handle v1, Vertex_handle v2,
			  Face_handle n0, Face_handle n1, Face_handle n2)
    :  Fb(v0,v1,v2,n0,n1,n2)
  {}

  Triangulation_ds_face_2( const Face& f)
    : Fb(f)
    {}

  static int ccw(int i) {return Triangulation_cw_ccw_2::ccw(i);}
  static int  cw(int i) {return Triangulation_cw_ccw_2::cw(i);}

  Vertex_handle mirror_vertex(int i) const;
  int mirror_index(int i) const;

  bool is_valid(bool verbose = false, int level = 0) const;

};

template < class Fb >
inline
typename Triangulation_ds_face_2<Fb>::Vertex_handle
Triangulation_ds_face_2<Fb>::
mirror_vertex(int i) const
{
  CGAL_triangulation_precondition ( Fb::neighbor(i) != NULL &&  Fb::dimension() >= 1);
  //return neighbor(i)->vertex(neighbor(i)->index(this->handle()));
  return Fb::neighbor(i)->vertex(mirror_index(i));
}

template < class Fb >
inline int
Triangulation_ds_face_2<Fb>::
mirror_index(int i) const
{
  // return the index of opposite vertex in neighbor(i);
  CGAL_triangulation_precondition (Fb::neighbor(i) != NULL &&  Fb::dimension() >= 1);
  if (Fb::dimension() == 1) return 1 - (Fb::neighbor(i)->index(Fb::vertex(1-i)));
  return ccw( Fb::neighbor(i)->index(Fb::vertex(ccw(i))));
}


template < class Fb >
bool
Triangulation_ds_face_2<Fb>::  
is_valid(bool verbose, int level) const
{
  bool result = Fb::is_valid(verbose, level);
  for(int i = 0; i <= Fb::dimension(); i++) {
    Face_handle n = Fb::neighbor(i);
    // the strange formulation in case dimension()==2 
    // is used to handle the cases of TDS allowing
    // two faces with two common edges
    int in;
    if (Fb::dimension() == 0) in = 0;
    else in = mirror_index(i);
    result = result && ( this == &*(n->neighbor(in)) );
    switch(Fb::dimension()) {
    case 0 : 
      break;
    case 1 :
      result = result &&  in == 1-i;
      result = result && ( Fb::vertex(1-i) == n->vertex(1-in));
      break;
    case 2 :
      result = result && ( Fb::vertex(cw(i))  == n->vertex(ccw(in)))
	              && ( Fb::vertex(ccw(i)) == n->vertex(cw(in)));
      break;
    }
  }
  return result;
}


CGAL_END_NAMESPACE

#endif //CGAL_TRIANGULATION_DS_FACE_2_H
