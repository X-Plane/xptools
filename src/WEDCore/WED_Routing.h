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

#ifndef WED_Routing_H
#define WED_Routing_H

#include "CompGeomDefs2.h"
// Ben says: some day the real definition of a routing will live somewhere else, but since routings don't
// exist at all, make a shell definition so we can at least "show what's going on".

class	IGISEntity;
class	IResolver;

struct WED_route_edge_t;

struct WED_route_node_t {

	Point2						loc;	
	bool						ini;
	bool						dst;
	vector<WED_route_edge_t *>	edges;

};

struct WED_route_edge_t {

	WED_route_node_t *			src;
	WED_route_node_t *			dst;
	vector<Point2>				shp; // might be empty.  this is additional "shaping" points.

};

struct WED_route_t {
	vector<WED_route_edge_t *>	edges;
	vector<WED_route_node_t *>	nodes;
};

void WED_generate_routes(
					const vector<IGISEntity*>&		n_pavement,			// All possible pavement, in a big damned pile.
					const vector<Point2>&		in_poi,					// Points of interest - places that must be final destinations.
					WED_route_t&				out_route);				// The routing.

void WED_MakeRouting(IResolver * in_resolver);

#endif /* WED_Routing_H */
