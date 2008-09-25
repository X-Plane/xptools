/*
 * Copyright (c) 2007, Laminar Research.
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

#include "MapDefs.h"
#include "AssertUtils.h"

static int gAdded = 0;
static GISHalfedge * gLast = NULL;

void notifier(GISHalfedge * h_old, GISHalfedge * h_new, void *)
{
	if (h_new && !h_old)
	{ ++gAdded;
	gLast = h_new;
	}
}

void TEST_MapDefs(void)
{
	// EMPTY MAP TESTS

	Pmwx	amap;
	TEST_Run(amap.empty());
	TEST_Run(amap.unbounded_face()->is_unbounded());
	TEST_Run(amap.number_of_vertices() == 0);
	TEST_Run(amap.number_of_halfedges() == 0);
	TEST_Run(amap.number_of_faces() == 1);
	TEST_Run(amap.is_valid());

	GISHalfedge * 		he;
	Pmwx::Locate_type		loc;
	he = amap.locate_point(Point2(0,0), loc);
	TEST_Run(he == NULL);
	TEST_Run(loc == Pmwx::locate_Face);

	// INSERT EDGE INTO EMPTY MAP

	gAdded = 0;
	amap.insert_edge(Point2(0,0), Point2(10, 0), notifier, NULL);
	TEST_Run(gAdded  == 1);
	TEST_Run(gLast != NULL);
	TEST_Run(gLast->source()->point() == Point2(0,0));
	TEST_Run(gLast->target()->point() == Point2(10,0));
	TEST_Run(gLast->face() == amap.unbounded_face());
	TEST_Run(gLast->twin() == gLast->next());
	TEST_Run(gLast->next()->twin() == gLast);
	TEST_Run(!amap.empty());
	TEST_Run(amap.number_of_vertices() == 2);
	TEST_Run(amap.number_of_halfedges() == 2);
	TEST_Run(amap.number_of_faces() == 1);
	GISHalfedge * one = gLast;

	he = amap.locate_point(Point2(0,0), loc);
	TEST_Run(he == one->twin());
	TEST_Run(loc == Pmwx::locate_Vertex);
	he = amap.locate_point(Point2(10,0), loc);
	TEST_Run(he == one);
	TEST_Run(loc == Pmwx::locate_Vertex);
	he = amap.locate_point(Point2(5,0), loc);
	TEST_Run(he == one || he == one->twin());
	TEST_Run(loc == Pmwx::locate_Halfedge);

	he = amap.locate_point(Point2(5,-2), loc);
	TEST_Run(he->face() == amap.unbounded_face());
	TEST_Run(loc == Pmwx::locate_Face);

	Point2 found;
	he = amap.ray_shoot(Point2(5, -2), Pmwx::locate_Face, NULL, Point2(5, 2), found, loc);
	TEST_Run(loc == Pmwx::locate_Halfedge);
	TEST_Run(found == Point2(5, 0));
	TEST_Run(he == one);
	he = amap.ray_shoot(Point2(0, -2), Pmwx::locate_Face, NULL, Point2(0, 2), found, loc);
	TEST_Run(loc == Pmwx::locate_Vertex);
	TEST_Run(found == Point2(0, 0));
	TEST_Run(he == one->twin());
	he = amap.ray_shoot(Point2(10, -2), Pmwx::locate_Face, NULL, Point2(10, 2), found, loc);
	TEST_Run(loc == Pmwx::locate_Vertex);
	TEST_Run(found == Point2(10, 0));
	TEST_Run(he == one);
	he = amap.ray_shoot(Point2(-10, -2), Pmwx::locate_Face, NULL, Point2(-10, 2), found, loc);
	TEST_Run(loc == Pmwx::locate_Face);
	TEST_Run(found == Point2(-10, 2));
	TEST_Run(he == NULL || he->face() == amap.unbounded_face());

	he = amap.ray_shoot(Point2(-10, -2), Pmwx::locate_Face, NULL, Point2(10, -2), found, loc);
	TEST_Run(loc == Pmwx::locate_Face);
	TEST_Run(found == Point2(10, -2));
	TEST_Run(he == NULL || he->face() == amap.unbounded_face());
	he = amap.ray_shoot(Point2(-15, 0), Pmwx::locate_Face, NULL, Point2(15, 0), found, loc);
	TEST_Run(loc == Pmwx::locate_Vertex);
	TEST_Run(found == Point2(0, 0));
	TEST_Run(he == one->twin());
	he = amap.ray_shoot(Point2(15, 0), Pmwx::locate_Face, NULL, Point2(-15, 0), found, loc);
	TEST_Run(loc == Pmwx::locate_Vertex);
	TEST_Run(found == Point2(10, 0));
	TEST_Run(he == one);
	TEST_Run(amap.is_valid());

	amap.insert_edge(Point2(10, 0), Point2(10, 10), notifier, NULL);
	TEST_Run(gAdded == 2);
	TEST_Run(gLast->source()->point() == Point2(10,0));
	TEST_Run(gLast->target()->point() == Point2(10,10));
	TEST_Run(gLast->face() == amap.unbounded_face());
	TEST_Run(amap.number_of_vertices() == 3);
	TEST_Run(amap.number_of_halfedges() == 4);
	TEST_Run(amap.number_of_faces() == 1);
	GISHalfedge * two = gLast;
	TEST_Run(one->next() == two);
	TEST_Run(two->next() == two->twin());
	TEST_Run(two->twin()->next() == one->twin());
	TEST_Run(one->twin()->next() == one);
	TEST_Run(amap.is_valid());

	amap.insert_edge(Point2(10, 10), Point2(-10, 10), notifier, NULL);
	TEST_Run(gAdded == 3);
	TEST_Run(gLast->source()->point() == Point2(10,10));
	TEST_Run(gLast->target()->point() == Point2(-10,10));
	TEST_Run(gLast->face() == amap.unbounded_face());
	TEST_Run(amap.number_of_vertices() == 4);
	TEST_Run(amap.number_of_halfedges() == 6);
	TEST_Run(amap.number_of_faces() == 1);
	GISHalfedge * three = gLast;
	TEST_Run(two->next() == three);
	TEST_Run(three->next() == three->twin());
	TEST_Run(three->twin()->next() == two->twin());
	TEST_Run(amap.is_valid());

	TEST_Run(three->twin()->next() == two->twin());
	TEST_Run(two->next() == three);
	TEST_Run(one->next() == two);
	TEST_Run(two->twin()->next() == one->twin());

	he = amap.locate_point(Point2(0,15), loc);
	TEST_Run(loc == Pmwx::locate_Face);
	TEST_Run(he == NULL || he->face() == amap.unbounded_face());

	he = amap.ray_shoot(Point2(0, 15), loc, he, Point2(0, -5), found, loc);
	TEST_Run(loc == Pmwx::locate_Halfedge);
	TEST_Run(found == Point2(0, 10));

	amap.split_edge(one, Point2(5, 0));
	TEST_Run(one->target()->point() == Point2(5, 0));
	TEST_Run(amap.number_of_vertices() == 5);
	TEST_Run(amap.number_of_halfedges() == 8);
	TEST_Run(amap.number_of_faces() == 1);
	TEST_Run(amap.is_valid());
	TEST_Run(one->next()->twin()->next() == one->twin());
	TEST_Run(one->target() == one->next()->twin()->target());
	TEST_Run(one->target()->point() == Point2(5,0));
	TEST_Run(one->source()->point() == Point2(0,0));
	TEST_Run(one->next()->target()->point() == Point2(10,0));
	TEST_Run(one->next()->source()->point() == Point2(5,0));
	TEST_Run(one->next()->next()->twin()->target()->point() == Point2(10,0));
	TEST_Run(one->source() != one->target());
	TEST_Run(one->next()->source() != one->next()->target());

	amap.insert_edge(Point2(0, 15), Point2(0, -5), notifier, NULL);

	TEST_Run(gAdded == 6);
	TEST_Run(amap.number_of_vertices() == 8);
	TEST_Run(amap.number_of_halfedges() == 16);
	TEST_Run(amap.number_of_faces() == 2);
	GISHalfedge * four = gLast;
	TEST_Run(amap.is_valid());

	// TODO: when we build a horizontal bar last, we "Catch" our own bounds as a hole and
	// try to move it if we are just finishing a hole in something. Probably the hole code
	// shouldn't run if we are a hole.

	// TODO: test case of two horizontal then two vertical lines to build a box!
	// halfedge insert must join holes, NOT create a face.


	// test: split edege that is an antenna in various dirs (none, some, all)
	// test: halfedge returned by ray shoot in a face with spikes!
	// test: insert edge in all its cases - this code is INSANE
	// heal edge
	// remove edge

}
