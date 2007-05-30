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
// $Source: /CVSROOT/CGAL/Packages/Triangulation_2/include/CGAL/Constrained_Delaunay_triangulation_2.h,v $
// $Revision: 1.66 $ $Date: 2003/09/18 10:26:01 $
// $Name: current_submission $
//
// Author(s)     : Mariette Yvinec, Jean Daniel Boissonnat
 
#ifndef CGAL_CONSTRAINED_DELAUNAY_TRIANGULATION_2_H
#define CGAL_CONSTRAINED_DELAUNAY_TRIANGULATION_2_H


#include <CGAL/triangulation_assertions.h>
#include <CGAL/Triangulation_short_names_2.h>
#include <CGAL/Constrained_triangulation_2.h>

CGAL_BEGIN_NAMESPACE


template <class Gt, 
          class Tds = Triangulation_data_structure_2 <
                      Triangulation_vertex_base_2<Gt>,
		      Constrained_triangulation_face_base_2<Gt> >,
	  class Itag = No_intersection_tag >		
class Constrained_Delaunay_triangulation_2
  : public  Constrained_triangulation_2<Gt, Tds, Itag> 
{
public:
  typedef Constrained_triangulation_2<Gt,Tds,Itag>             Ctr;
  typedef Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>    CDt;
  typedef typename Ctr::Geom_traits      Geom_traits;
  typedef typename Ctr::Intersection_tag Intersection_tag;

  typedef typename Ctr::Constraint    Constraint;
  typedef typename Ctr::Vertex_handle Vertex_handle;
  typedef typename Ctr::Face_handle   Face_handle;
  typedef typename Ctr::Edge          Edge;
  typedef typename Ctr::Finite_faces_iterator Finite_faces_iterator;
  typedef typename Ctr::Face_circulator       Face_circulator;
  typedef typename Ctr::Locate_type           Locate_type;
 
  typedef typename Ctr::List_edges List_edges;  
  typedef typename Ctr::List_faces List_faces;
  typedef typename Ctr::List_vertices  List_vertices;
  typedef typename Ctr::List_constraints List_constraints;
  typedef typename Ctr::Less_edge less_edge;
  typedef typename Ctr::Edge_set Edge_set;

  typedef typename Geom_traits::Point_2  Point;


  Constrained_Delaunay_triangulation_2(const Geom_traits& gt=Geom_traits()) 
    : Ctr(gt) { }

  Constrained_Delaunay_triangulation_2(const CDt& cdt)
    : Ctr(cdt) {}

  Constrained_Delaunay_triangulation_2(List_constraints& lc, 
				       const Geom_traits& gt=Geom_traits())
    : Ctr(gt) 
    {   
      typename List_constraints::iterator itc = lc.begin();
      for( ; itc != lc.end(); ++itc) {
	insert((*itc).first, (*itc).second);
      }
      CGAL_triangulation_postcondition( is_valid() );
    }

  template<class InputIterator>
  Constrained_Delaunay_triangulation_2(InputIterator it,
				       InputIterator last,
				       const Geom_traits& gt=Geom_traits() )
    : Ctr(gt) 
    {
      for ( ; it != last; it++) {
      	insert((*it).first, (*it).second);
      }
      CGAL_triangulation_postcondition( is_valid() );
    }

  virtual ~Constrained_Delaunay_triangulation_2() {}
  

  // FLIPS
  bool is_flipable(Face_handle f, int i) const;
  void flip(Face_handle& f, int i);
  void flip_around(Vertex_handle va);
  void flip_around(List_vertices & new_vertices);
  void propagating_flip(Face_handle& f,int i);
  void propagating_flip(List_edges & edges);

  // CONFLICTS
  bool test_conflict(Face_handle fh, const Point& p) const; //deprecated
  bool test_conflict(const Point& p, Face_handle fh) const;
  void find_conflicts(const Point& p, std::list<Edge>& le,  //deprecated
		      Face_handle hint= Face_handle(NULL)) const;
  //  //template member functions, declared and defined at the end 
  // template <class OutputItFaces, class OutputItBoundaryEdges> 
  // std::pair<OutputItFaces,OutputItBoundaryEdges>
  // get_conflicts_and_boundary(const Point  &p, 
  // 		                OutputItFaces fit, 
  // 		                OutputItBoundaryEdges eit,
  // 		                Face_handle start) const;
  // template <class OutputItFaces>
  // OutputItFaces
  // get_conflicts (const Point  &p, 
  //                OutputItFaces fit, 
  // 		    Face_handle start ) const;
  // template <class OutputItBoundaryEdges>
  // OutputItBoundaryEdges
  // get_boundary_of_conflicts(const Point  &p, 
  // 			       OutputItBoundaryEdges eit, 
  // 			       Face_handle start ) const;
   

  // INSERTION-REMOVAL
  Vertex_handle insert(const Point & a, Face_handle start = Face_handle(NULL));
  Vertex_handle insert(const Point& p,
		       Locate_type lt,
		       Face_handle loc, int li );
  Vertex_handle push_back(const Point& a);
//   template < class InputIterator >
//   int insert(InputIterator first, InputIterator last);

  void remove(Vertex_handle v);
  void remove_incident_constraints(Vertex_handle v);
  void remove_constrained_edge(Face_handle f, int i);
 
  //for backward compatibility
  void insert(Point a, Point b) { insert_constraint(a, b);}
  void insert(Vertex_handle va, Vertex_handle  vb) {insert_constraint(va,vb);}
  void remove_constraint(Face_handle f, int i){remove_constrained_edge(f,i);}
  // CHECK
  bool is_valid(bool verbose = false, int level = 0) const;
 
protected:
  virtual Vertex_handle virtual_insert(const Point & a, 
				       Face_handle start = Face_handle(NULL));
  virtual Vertex_handle virtual_insert(const Point& a,
				       Locate_type lt,
				       Face_handle loc, 
				       int li );
//Vertex_handle special_insert_in_edge(const Point & a, Face_handle f, int i);
  void remove_2D(Vertex_handle v );
  virtual void triangulate_hole(List_faces& intersected_faces,
				List_edges& conflict_boundary_ab,
				List_edges& conflict_boundary_ba);

public:
  // MESHING 
  // suppressed meshing functions from here

  //template member functions
public:
  template < class InputIterator >
#if defined(_MSC_VER) || defined(__SUNPRO_CC)
   int insert(InputIterator first, InputIterator last, int i = 0)
#else
   int insert(InputIterator first, InputIterator last) 
#endif
    {
      int n = Ctr::number_of_vertices();
      while(first != last){
	insert(*first);
	++first;
      }
      return Ctr::number_of_vertices() - n;
    }

  template <class OutputItFaces, class OutputItBoundaryEdges> 
  std::pair<OutputItFaces,OutputItBoundaryEdges>
  get_conflicts_and_boundary(const Point  &p, 
			     OutputItFaces fit, 
			     OutputItBoundaryEdges eit,
			     Face_handle start = Face_handle(NULL)) const {
    CGAL_triangulation_precondition( Ctr::dimension() == 2);
    int li;
    Locate_type lt;
    Face_handle fh = locate(p,lt,li, start);
    switch(lt) {
    case Ctr::OUTSIDE_AFFINE_HULL:
    case Ctr::VERTEX:
      return std::make_pair(fit,eit);
    case Ctr::FACE:
    case Ctr::EDGE:
    case Ctr::OUTSIDE_CONVEX_HULL:
      *fit++ = fh; //put fh in OutputItFaces
      std::pair<OutputItFaces,OutputItBoundaryEdges>
	pit = std::make_pair(fit,eit);
      pit = propagate_conflicts(p,fh,0,pit);
      pit = propagate_conflicts(p,fh,1,pit);
      pit = propagate_conflicts(p,fh,2,pit);
      return std::make_pair(fit,eit);    
    }
    CGAL_triangulation_assertion(false);
    return std::make_pair(fit,eit);
  } 

  template <class OutputItFaces>
  OutputItFaces
  get_conflicts (const Point  &p, 
		 OutputItFaces fit, 
		 Face_handle start= Face_handle(NULL)) const {
    std::pair<OutputItFaces,Emptyset_iterator> pp = 
      get_conflicts_and_boundary(p,fit,Emptyset_iterator(),start);
    return pp.first;
  }

  template <class OutputItBoundaryEdges> 
  OutputItBoundaryEdges
  get_boundary_of_conflicts(const Point  &p, 
			    OutputItBoundaryEdges eit, 
			    Face_handle start= Face_handle(NULL)) const {
    std::pair<Emptyset_iterator, OutputItBoundaryEdges> pp = 
      get_conflicts_and_boundary(p,Emptyset_iterator(),eit,start);
    return pp.second;
  }



private:
 template <class OutputItFaces, class OutputItBoundaryEdges> 
 std::pair<OutputItFaces,OutputItBoundaryEdges>
 propagate_conflicts (const Point  &p,
		      Face_handle fh, 
		      int i,
		      std::pair<OutputItFaces,OutputItBoundaryEdges>
		      pit)  const {
   Face_handle fn = fh->neighbor(i);
   OutputItFaces fit = pit.first;
   OutputItBoundaryEdges eit = pit.second;
   if ( fh->is_constrained(i) || ! test_conflict(p,fn)) {
     *eit++ = Edge(fn, fn->index(fh));
     return std::make_pair(fit,eit);
   }
   *fit++ = fn;
   int j = fn->index(fh);
   pit = propagate_conflicts(p,fn,Ctr::ccw(j),pit);
   pit = propagate_conflicts(p,fn,Ctr::cw(j), pit);
   return pit;
 }
};


template < class Gt, class Tds, class Itag >
bool 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
is_flipable(Face_handle f, int i) const
  // determines if edge (f,i) can be flipped 
{
  Face_handle ni = f->neighbor(i); 
  if (is_infinite(f) || is_infinite(ni)) return false; 
  if (f->is_constrained(i)) return false;
  return (side_of_oriented_circle(ni, f->vertex(i)->point()) 
                                        == ON_POSITIVE_SIDE);
}

template < class Gt, class Tds, class Itag >
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
flip (Face_handle& f, int i)
{
  Face_handle g = f->neighbor(i);
  int j = f->mirror_index(i);

  // save wings neighbors to be able to restore contraint status
  Face_handle f1 = f->neighbor(Ctr::cw(i));
  int i1 = f->mirror_index(Ctr::cw(i));
  Face_handle f2 = f->neighbor(Ctr::ccw(i));
  int i2 = f->mirror_index(Ctr::ccw(i));
  Face_handle f3 = g->neighbor(Ctr::cw(j));
  int i3 = g->mirror_index(Ctr::cw(j));
  Face_handle f4 = g->neighbor(Ctr::ccw(j));
  int i4 = g->mirror_index(Ctr::ccw(j));

  // The following precondition prevents the test suit 
  // of triangulation to work on constrained Delaunay triangulation
  //CGAL_triangulation_precondition(is_flipable(f,i));
  this->_tds.flip(f, i);
   
  // restore constraint status
  f->set_constraint(f->index(g), false);
  g->set_constraint(g->index(f), false);
  f1->neighbor(i1)->set_constraint(f1->mirror_index(i1),
				   f1->is_constrained(i1));
  f2->neighbor(i2)->set_constraint(f2->mirror_index(i2),
				   f2->is_constrained(i2));
  f3->neighbor(i3)->set_constraint(f3->mirror_index(i3),
				   f3->is_constrained(i3));
  f4->neighbor(i4)->set_constraint(f4->mirror_index(i4),
				   f4->is_constrained(i4));
  return;
}

template < class Gt, class Tds, class Itag >
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
flip_around(Vertex_handle va)
  // makes the triangles incident to vertex va Delaunay using flips
{
  if (Ctr::dimension() <= 1) return;
  Face_handle f=va->face();
  Face_handle next;    
  Face_handle start(f);
  int i;
  do {
    i = f->index(va); // FRAGILE : DIM 1
    next = f->neighbor(Ctr::ccw(i));  // turns ccw around a
    propagating_flip(f,i);
    f=next;
  } while(next != start);
}

template < class Gt, class Tds, class Itag >
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
flip_around(List_vertices& new_vertices)
{
  typename List_vertices::iterator itv=new_vertices.begin();
  for( ; itv != new_vertices.end(); itv++) {
    flip_around(*itv);
  }
  return;
}


template < class Gt, class Tds, class Itag >
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
propagating_flip(Face_handle& f,int i)
  // similar to the corresponding function in Delaunay_triangulation_2.h 
{ 
  if (!is_flipable(f,i)) return;
  Face_handle ni = f->neighbor(i); 
  flip(f, i); // flip for constrained triangulations
  propagating_flip(f,i); 
  i = ni->index(f->vertex(i)); 
  propagating_flip(ni,i); 
} 


template < class Gt, class Tds, class Itag >
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
propagating_flip(List_edges & edges)
  // makes the triangulation Delaunay by flipping 
  // List edges contains an initial list of edges to be flipped
  // Precondition : the output triangulation is Delaunay if the list 
  // edges contains all edges of the input triangulation that need to be
  // flipped (plus possibly others)
{
  int i, ii, indf, indn;
  Face_handle ni, f,ff;
  Edge ei,eni; 
  typename Ctr::Edge_set edge_set;
  typename Ctr::Less_edge less_edge;
  Edge e[4];
  typename List_edges::iterator itedge=edges.begin();

  // initialization of the set of edges to be flip
  while (itedge != edges.end()) {
    f=(*itedge).first;
    i=(*itedge).second;
    if (is_flipable(f,i)) {
      eni=Edge(f->neighbor(i),f->mirror_index(i));
      if (less_edge(*itedge,eni)) edge_set.insert(*itedge);
      else edge_set.insert(eni);
    }
    ++itedge;
  }

  // flip edges and updates the set of edges to be flipped
  while (!(edge_set.empty())) {
    f=(*(edge_set.begin())).first;
    indf=(*(edge_set.begin())).second;
 
    // erase from edge_set the 4 edges of the wing of the edge to be
    // flipped (edge_set.begin) , i.e. the edges of the faces f and
    // f->neighbor(indf) that are distinct from the edge to be flipped

    ni = f->neighbor(indf); 
    indn=f->mirror_index(indf);
    ei= Edge(f,indf);
    edge_set.erase(ei);
    e[0]= Edge(f,Ctr::cw(indf));
    e[1]= Edge(f,Ctr::ccw(indf));
    e[2]= Edge(ni,Ctr::cw(indn));
    e[3]= Edge(ni,Ctr::ccw(indn));

    for(i=0;i<4;i++) { 
      ff=e[i].first;
      ii=e[i].second;
      if (is_flipable(ff,ii)) {
	eni=Edge(ff->neighbor(ii),ff->mirror_index(ii));
	if (less_edge(e[i],eni)) { 
	  edge_set.erase(e[i]);}
	else {
	  edge_set.erase(eni);} 
      }
    } 

    // here is the flip 
    flip(f, indf); 

    //insert in edge_set the 4 edges of the wing of the edge that
    //have been flipped 
    e[0]= Edge(f,indf);
    e[1]= Edge(f,Ctr::cw(indf));
    e[2]= Edge(ni,indn);
    e[3]= Edge(ni,Ctr::cw(indn));

    for(i=0;i<4;i++) { 
      ff=e[i].first;
      ii=e[i].second;
      if (is_flipable(ff,ii)) {
	eni=Edge(ff->neighbor(ii),ff->mirror_index(ii));
	if (less_edge(e[i],eni)) { 
	  edge_set.insert(e[i]);}
	else {
	  edge_set.insert(eni);} 
      }
    } 
  }  
}

template < class Gt, class Tds, class Itag >
inline bool
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
test_conflict(const Point& p, Face_handle fh) const
  // true if point P lies inside the circle circumscribing face fh
{
  return ( side_of_oriented_circle(fh,p) == ON_POSITIVE_SIDE );
}

template < class Gt, class Tds, class Itag >
inline bool
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
test_conflict(Face_handle fh, const Point& p) const
  // true if point P lies inside the circle circumscribing face fh
{
  return test_conflict(p,fh);
}

template < class Gt, class Tds, class Itag >    
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
find_conflicts(const Point& p, std::list<Edge>& le, Face_handle hint) const
{
  // sets in le the counterclocwise list of the edges of the boundary of the 
  // union of the faces in conflict with p
  // an edge is represented by the incident face that is not in conflict with p
  get_boundary_of_conflicts(p, std::back_inserter(le), hint);
}
  
template < class Gt, class Tds, class Itag >  
inline 
typename Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::Vertex_handle 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
virtual_insert(const Point & a, Face_handle start)
  // virtual version of the insertion
{
  return insert(a,start);
}

template < class Gt, class Tds, class Itag >  
inline 
typename Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::Vertex_handle 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
virtual_insert(const Point& a,
	       Locate_type lt,
	       Face_handle loc, 
	       int li )
// virtual version of insert
{
  return insert(a,lt,loc,li);
}

template < class Gt, class Tds, class Itag >  
inline 
typename Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::Vertex_handle 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
insert(const Point & a, Face_handle start)
 // inserts a in the triangulation
// constrained edges are updated
// Delaunay property is restored
{
  Vertex_handle va= Ctr::insert(a, start);
  flip_around(va); 
  return va;
}

template < class Gt, class Tds, class Itag >
typename Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::Vertex_handle
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
insert(const Point& a, Locate_type lt, Face_handle loc, int li)
// insert a point p, whose localisation is known (lt, f, i)
// constrained edges are updated
// Delaunay property is restored
{
  Vertex_handle va= Ctr::insert(a,lt,loc,li);
  flip_around(va); 
  return va;
}

template < class Gt, class Tds, class Itag >
inline
typename Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::Vertex_handle
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
push_back(const Point &p)
{
  return insert(p);
}

template < class Gt, class Tds, class Itag >
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
triangulate_hole(List_faces& intersected_faces,
		 List_edges& conflict_boundary_ab,
		 List_edges& conflict_boundary_ba)
{
  List_edges new_edges;
  Ctr::triangulate_hole(intersected_faces,
		       conflict_boundary_ab,
		       conflict_boundary_ba,
		       new_edges);
  propagating_flip(new_edges);
  return;
}


template < class Gt, class Tds, class Itag >  
inline void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
remove(Vertex_handle v)
  // remove a vertex and updates the constrained edges of the new faces
  // precondition : there is no incident constraints
{
  CGAL_triangulation_precondition( v != NULL );
  CGAL_triangulation_precondition( ! is_infinite(v));
  CGAL_triangulation_precondition( ! are_there_incident_constraints(v));
  if  (Ctr::dimension() <= 1)    Ctr::remove(v);
  else  remove_2D(v);
  return;
}

// template < class Gt, class Tds, class Itag >  
// typename
// Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::Vertex_handle
// Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
// special_insert_in_edge(const Point & a, Face_handle f, int i)
//   // insert  point p in edge(f,i)
//   // bypass the precondition for point a to be in edge(f,i)
//   // update constrained status
//   // this member fonction is not robust with exact predicates 
//   // and approximate construction. Should be removed 
// {
//   Vertex_handle vh=Ctr::special_insert_in_edge(a,f,i);
//   flip_around(vh);
//   return vh;
// }

template < class Gt, class Tds, class Itag >  
void 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
remove_2D(Vertex_handle v)
{
 if (test_dim_down(v)) {  this->_tds.remove_dim_down(v);  }
  else {
     std::list<Edge> hole;
    make_hole(v, hole);
    std::list<Edge> shell=hole; //because hole will be emptied by fill_hole
    fill_hole_delaunay(hole);
    update_constraints(shell);
    delete_vertex(v);
  }
  return;
}

template < class Gt, class Tds, class Itag >
void
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
remove_incident_constraints(Vertex_handle v)
{
   List_edges iconstraints;
   if (are_there_incident_constraints(v,
				      std::back_inserter(iconstraints))) {
     Ctr::remove_incident_constraints(v);
     if (Ctr::dimension()==2) propagating_flip(iconstraints);
   }
   return;
}

template < class Gt, class Tds, class Itag >
void
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
remove_constrained_edge(Face_handle f, int i)
{
  Ctr::remove_constrained_edge(f,i);
  if(Ctr::dimension() == 2) {
    List_edges le;
    le.push_back(Edge(f,i));
    propagating_flip(le);
  }
  return;  
}


template < class Gt, class Tds, class Itag >  
bool 
Constrained_Delaunay_triangulation_2<Gt,Tds,Itag>::
is_valid(bool verbose, int level) const
{
  bool result = Ctr::is_valid(verbose, level);
  CGAL_triangulation_assertion( result );

    Finite_faces_iterator fit= Ctr::finite_faces_begin();
    for (; fit != Ctr::finite_faces_end(); fit++) {
      for(int i=0;i<3;i++) {
	result = result && !is_flipable(fit,i);
	CGAL_triangulation_assertion( result );
      }
    }
    return result;
}



CGAL_END_NAMESPACE
#endif // CGAL_CONSTRAINED_DELAUNAY_TRIANGULATION_2_H
