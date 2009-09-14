/* 
 * Copyright (c) 2009, Laminar Research.
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

#ifndef AptRouting_H
#define AptRouting_H

#include "CGALDefs.h"
#include "CompGeomDefs2.h"

struct cgal_node_t;
struct cgal_edge_t;

struct cgal_node_t {
	Point_2					loc;
	vector<Point2>			poi;		// All POI within us...if this is empty, we are not THAT important.
	vector<cgal_edge_t *>	edges;
	// djikstra temps
	cgal_edge_t *			prev;		// Edge that got us here if we have one or NULL if not visited.  All hot nodes except start have this!
	double					cost;		// cost to get here.
	bool					done;		// True if we have fully explored node..can't go back.	
	bool					yuck;		// try to avoid this node - it is yucky -- too close to the grass!
};

struct cgal_edge_t {
	cgal_node_t *			src;
	cgal_node_t *			dst;
	vector<Point_2>			shp;
	bool					used;
	cgal_node_t * other(cgal_node_t * who) const;
	double cost(void) const;
};

struct cgal_net_t {
	vector<cgal_node_t *>	nodes;
	vector<cgal_edge_t *>	edges;

	cgal_node_t * new_node_at(Point_2 p);
	cgal_edge_t * new_edge_between(cgal_node_t * v1,cgal_node_t * v2);
	void clear(void);
	void kill_edge(cgal_edge_t * who);
	void purge_unused_edges(void);
	void purge_unused_nodes(void);
	cgal_node_t * split_edge_at(cgal_edge_t * e, Point_2 where);
};

bool make_map_with_skeleton(
					const vector<Polygon2>&		in_pavement,
					const vector<Point2>&		in_poi,
					cgal_net_t&					out_route);


#endif /* AptRouting_H */
