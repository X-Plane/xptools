#ifndef MeshConformer_H
#define MeshConformer_H

// Copyright (c) 2004  INRIA Sophia-Antipolis (France).
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
// $URL: svn+ssh://scm.gforge.inria.fr/svn/cgal/branches/CGAL-3.4-branch/Mesh_2/include/CGAL/Triangulation_conformer_2.h $
// $Id$
// 
//
// Author(s)     : Laurent RINEAU

#include <CGAL/Mesh_2/Refine_edges_with_clusters.h>

namespace CGAL {

// Lcp = Local conformance predicate.  LCP must already be instantiated compatible with Tr!
template <typename Tr, typename Lcp>
class Triangulation_conformer_any_2
{
  typedef typename Tr::Finite_edges_iterator Finite_edges_iterator;
  typedef typename Tr::Vertex_handle Vertex_handle;

  typedef Mesh_2::Refine_edges_with_clusters<Tr, Lcp > Edges_level_any;

protected:
  /** \name INITIALIZED */

  enum Initialization {
    NONE,     /**< \c this is not initialized. */
    CLUSTERS, /**< \c this clusters are initialized. */
    ANY		  /**< \c this has been \e conform-initialized. */
  };

// --- PROTECTED DATAS ---
  Initialization initialized;
  Tr& tr;
  Null_mesher_level null_level;
  Null_mesh_visitor null_visitor;
  Mesh_2::Clusters<Tr> clusters;
  Edges_level_any edges_level_any;

public:
  /** \name CONSTRUCTORS */
  Triangulation_conformer_any_2(Tr& tr_)
    : initialized(NONE),
      tr(tr_),
      null_level(), null_visitor(),
      clusters(tr_),
      edges_level_any(tr, clusters, null_level)
  {
  }

public:  /** \name ACCESS TO CLUSTERS */
  typedef typename Mesh_2::Clusters<Tr>::Cluster_vertices_iterator 
    Cluster_vertices_iterator;
  typedef typename Mesh_2::Clusters<Tr>::Vertices_in_cluster_iterator
    Vertices_in_cluster_iterator;

public:
  /** \name ACCESS FUNCTIONS */

  /** Access to the private initialized member data. */
  //@{
  void set_initialized(Initialization init) { initialized = init; }
  Initialization get_initialized() const { return initialized; }
  //@}

  int number_of_constrained_edges()
  {
    int nedges = 0;
    for(typename Tr::Finite_edges_iterator eit = tr.finite_edges_begin();
        eit != tr.finite_edges_end();
        ++eit)
      if(eit->first->is_constrained(eit->second))
        ++nedges;
    return nedges;
  }

  int number_of_clusters_vertices() const
  {
    return clusters.size();
  }

  Cluster_vertices_iterator clusters_vertices_begin() const
  {
    return clusters.clusters_vertices_begin();
  }

  Cluster_vertices_iterator clusters_vertices_end() const
  {
    return clusters.clusters_vertices_end();
  }

  unsigned int number_of_clusters_at_vertex(const Vertex_handle& vh)
  {
    return clusters.number_of_clusters_at_vertex(vh);
  }

  // returns the sequence of vertices bellonging to the n-th cluster of vh
  std::pair<Vertices_in_cluster_iterator, Vertices_in_cluster_iterator>
  vertices_in_cluster_sequence(const Vertex_handle& vh,
                               const unsigned int n)
  {
    return clusters.vertices_in_cluster_sequence();
  }

public:
  /** \name CHECKING METHODS */

  bool is_conforming_any()
  {
	Lcp is_locally_conform;
	
    for(Finite_edges_iterator ei = tr.finite_edges_begin();
        ei != tr.finite_edges_end();
        ++ei)
      if(ei->first->is_constrained(ei->second) &&
         !is_locally_conform(this, ei->first, ei->second) )
        return false;
    return true;	
  }

  /** \name CONFORMING FUNCTIONS */

  void make_conforming_any()
  {
    if(initialized!=ANY) init_any();
    edges_level_any.refine(null_visitor);
  }

  /** \name STEP BY STEP FUNCTIONS */

  // Note: step by step functions are not efficient at all!
private:
  void init_clusters()
  {
    if(initialized == NONE)
      clusters.create_clusters();
    initialized = CLUSTERS;
  }

public:
  /**
     Initializes the data structures
     (The call of this function is REQUIRED before any step by step
     operation).
  */
  //@{
  void init_any()
    {
      init_clusters();
      initialized = ANY;
      edges_level_any.scan_triangulation();
    }
  //@}

  /** Tells if all constrained edges are conformed. */
  bool is_conforming_done()
    // This function cannot be "const" because, as edges_to_be_conformed is
    // filtred, its empty() method is not const.
  { return ( edges_level_any.no_longer_element_to_refine() );
  }

  /** Execute on step of the algorithm.
      init_XXX() should have been called before.
  */
  //@{
  bool try_one_step_conforming_any()
  {
    return edges_level_any.one_step(null_visitor);
  }
  
  bool step_by_step_conforming_any()
  {
    return edges_level_any.try_to_insert_one_point(null_visitor);
  }

  //@}

}; // end Triangulation_conformer_any_2


// --- GLOBAL FUNCTIONS ---

template <typename Tr, typename Lcp>
void
make_conforming_any_2(Tr& t)
{
  typedef Triangulation_conformer_any_2<Tr, Lcp> Conform;

  Conform conform(t);
  conform.make_conforming_any();
}

} // end namespace CGAL

#endif
