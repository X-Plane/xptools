/* 
 * Copyright (c) 2011, Laminar Research.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
 * THE SOFTWARE.
 *
 */

#ifndef MeshSimplify_H
#define MeshSimplify_H

/*

	This algorithm simplifies the constraints in a constrained delaunay triangulation.

	The passed in error function measures how far constraints can deviate from their
	original positions.

	The constraints are simplified such that not only do no they not cross when done,
	but no set of constraints will cross a topological boundary to another set.

 */

#include "MeshDefs.h"

typedef double (*mesh_error_f)(const Point_2& p, const Point_2& q, const Point_2& r);


class	MeshSimplify {
public:

				MeshSimplify(CDT& io_mesh, mesh_error_f err);
	void		simplify(double max_error);

private:

	bool		can_remove_topo(CDT::Vertex_handle p, CDT::Vertex_handle q, CDT::Vertex_handle r);
	bool		can_remove_locked(CDT::Vertex_handle q, CDT::Vertex_handle& p, CDT::Vertex_handle& r);
	double		calc_remove_error(CDT::Vertex_handle p, CDT::Vertex_handle q, CDT::Vertex_handle r);

	void		init_q(void);
	void		run_vertex(CDT::Vertex_handle v);
	void		update_q(CDT::Vertex_handle v);

	CDT&			mesh;
	VertexQueue		queue;
	mesh_error_f	err_f;
	double			max_err;
	
};	
	

#endif /* MeshSimplify_H */
