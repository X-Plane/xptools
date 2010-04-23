/*
 * Copyright (c) 2008, Laminar Research.  All rights reserved.
 *
 */

#ifndef MapPolygon_H
#define MapPolygon_H

#include "MapDefs.h"

typedef	vector<double>			RingInset_t;
typedef vector<RingInset_t>		PolyInset_t;

typedef double (* Inset_f)(Halfedge_const_handle he);
typedef bool   (* Simplify_f)(Halfedge_const_handle he1, Halfedge_const_handle he2);

void	PolygonFromCCB(Pmwx::Ccb_halfedge_const_circulator circ, Polygon_2& out_poly, RingInset_t * out_inset, Inset_f func, Bbox_2 * extent);

void	PolygonFromFace(Pmwx::Face_const_handle in_face, Polygon_with_holes_2& out_ps, PolyInset_t * out_inset, Inset_f func, Bbox_2 * extent);

void PolygonFromFaceEx(
			Pmwx::Face_const_handle		in_face,			// A set of faces - we take the area of all of them.
			Polygon_with_holes_2&		out_ps,				// Polygon with hole to output
			PolyInset_t *				out_insets,			// Inset distance per polygon or null
			Inset_f						func,				// Function that calcs inset distances from edges or null
			CoordTranslator_2 *			xform,				// A function to transform the coordinate system or null
			Simplify_f					simplify);			// A function to pick whether an edge stays in...or null



// Given a polygon, try to reduce the number of sides based on some tollerance.
// void	ReducePolygon(Polygon_2& ioPolygon, double tolerance, double angle, double min_len, double cut_len);

// Given a polygon, simplify its boundary.  The new polygon will not have
// a corresponding segment farther than "max_err" from an original point.
// Only original points will be used.
//
// WARNING: THERE ARE HACKS IN THIS FUNCTION FOR INTEGRAL POINTS!
void	SimplifyPolygonMaxMove(Polygon_set_2& ioPolygon, double max_err);


// Calculate the convex hull of a polygon.
void	MakePolygonConvex(Polygon_2& ioPolygon);

// Fill in bridges outside the polygon where two points are less than 'dist' units
// apart.  (WARNING: O(N^2) right now!)
// (For example, this is used to bridge across "inlets" in an airport.)
void	FillPolygonGaps(Polygon_set_2& ioPolygon, double dist);

// Given a polygon, changes its bounds to make it "more convex" (e.g. each step increases area and
// decreases inset angles), with no addition being greater than max_area.  Runs until no more adds
// can be made or the convex hull is reached.
// (For example, this is used to simplify an airport by annexing surrounding land.)
void	SafeMakeMoreConvex(Polygon_set_2& ioPolygon, double max_area);

// Make the polygon simple, taking the outer hull around all edges if it is self-intersecting.
void	MakePolygonSimple(const Polygon_2& inPolygon, vector<Polygon_2>& out_simple_polygons);



/*
void	TranslatePolygonForward(Polygon_2& io_poly, const CoordTranslator_2& translator);
void	TranslatePolygonForward(Polygon_with_holes_2& io_poly, const CoordTranslator_2& translator);

void	TranslatePolygonReverse(Polygon_2& io_poly, const CoordTranslator_2& translator);
void	TranslatePolygonReverse(Polygon_with_holes_2& io_poly, const CoordTranslator_2& translator);
*/






void	cgal2ben(const Polygon_2& cgal, Polygon2& ben);
void	cgal2ben(const Polygon_with_holes_2& cgal, vector<Polygon2>& ben);
void	ben2cgal(const Polygon2& ben, Polygon_2& cgal);
void	ben2cgal(const vector<Polygon2>& ben, Polygon_with_holes_2& cgal);
#endif /* MapPolygon_H */
