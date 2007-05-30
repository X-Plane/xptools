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
// $Source: /CVSROOT/CGAL/Packages/Triangulation_2/include/CGAL/Triangulation_ds_vertex_2.h,v $
// $Revision: 1.21 $ $Date: 2003/09/18 10:26:13 $
// $Name: current_submission $
//
// Author(s)     : Mariette Yvinec

#ifndef CGAL_TRIANGULATION_DS_VERTEX_2_H
#define CGAL_TRIANGULATION_DS_VERTEX_2_H

#include <CGAL/basic.h>
#include <CGAL/Triangulation_short_names_2.h>

CGAL_BEGIN_NAMESPACE

template <class Vb>
class  Triangulation_ds_vertex_2 
  : public Vb 
{
  typedef typename Vb::Triangulation_data_structure Tds;
public:
  typedef typename Tds::Vertex             Vertex;
  typedef typename Tds::Vertex_handle      Vertex_handle;
  typedef typename Tds::Face_handle        Face_handle;
  typedef typename Tds::Vertex_circulator  Vertex_circulator;
  typedef typename Tds::Face_circulator    Face_circulator;
  typedef typename Tds::Edge_circulator    Edge_circulator;
 
  //CREATORS
  Triangulation_ds_vertex_2() : Vb() {}

  //ACCESS
  int degree(); //should be const

  //Deprecated access to circulators - for bacward compatibility
  // the following should be const
  // when Face_circulator, Vertex_circulator and Edge_circulator
  // are created from 
  // Face_const_handle and Face_const_vertex
  Vertex_circulator incident_vertices()     
    {return Vertex_circulator(handle());}
 
  Vertex_circulator incident_vertices( Face_handle f)  
    {return Vertex_circulator(handle(),f);}
  
  Face_circulator incident_faces()  
    { return Face_circulator(handle()) ;}
  
  Face_circulator incident_faces( Face_handle f)    
    { return Face_circulator(handle(), f);}
  
  Edge_circulator incident_edges()   
    { return Edge_circulator(handle());}
  
  Edge_circulator incident_edges( Face_handle f)  
    { return Edge_circulator(handle(), f);}
  
  bool is_valid(bool verbose = false, int level = 0);

private:
  // used to implement deprected access to circulators
  Vertex_handle handle();
};

template <class Tds>
int
Triangulation_ds_vertex_2 <Tds> ::
degree() //const
{
  int count = 0;
  Vertex_circulator vc = incident_vertices(), done(vc);
  if ( ! vc.is_empty()) {
    do { 
      count += 1;
    } while (++vc != done);
  }
  return count;
}

template <class Tds>
typename Triangulation_ds_vertex_2<Tds>::Vertex_handle
Triangulation_ds_vertex_2 <Tds> ::
handle()
{
  Face_handle fh = this->face();
  for(int i = 0 ; i < 3 ; ++i){
    if ( &*fh->vertex(i) == this) return fh->vertex(i);
  }
  return Vertex_handle();				    
}
    
template <class Vb>
bool 
Triangulation_ds_vertex_2<Vb> ::  
is_valid(bool verbose, int level) 
{
  bool result = Vb::is_valid(verbose, level);
  CGAL_triangulation_assertion(result);
  if (Vb::face() != NULL) { // face==NULL if dim <0
    result = result && ( &*Vb::face()->vertex(0) == this ||
			 &*Vb::face()->vertex(1) == this ||
			 &*Vb::face()->vertex(2) == this );
  }
  CGAL_triangulation_assertion(result);
  return result;
}

CGAL_END_NAMESPACE

#endif //CGAL_TRIANGULATION_DS_VERTEX_2_H
