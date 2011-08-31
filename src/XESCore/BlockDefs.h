/* 
 * Copyright (c) 2010, Laminar Research.
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

#ifndef BlockDefs_H
#define BlockDefs_H

#include "MapDefs.h"

enum {
	
	usage_Road,
	usage_Road_End,

	usage_Empty,
	usage_Steep,

	usage_Point_Feature,
	usage_Polygonal_Feature,	
	usage_Forest,
	
	
	usage_OOB

};

struct BLOCK_vertex_data { };
struct BLOCK_halfedge_data { bool operator==(const BLOCK_halfedge_data& rhs) const { return true; } };

struct BLOCK_face_data {
	BLOCK_face_data() : usage(usage_Empty), feature(0) { }
	BLOCK_face_data(int u,int f) : usage(u), feature(f) { }
	int		usage;
	int		feature;
//	int		pre_placed;
	Point2	center;
	double	heading;
	
};

typedef Arr_seg_traits_					Block_traits_2;

typedef CGAL::Arr_extended_dcel<Block_traits_2,
								BLOCK_vertex_data,
								BLOCK_halfedge_data,
								BLOCK_face_data,
								CGAL::Arr_vertex_base<Point_2>,
								CGAL::Arr_halfedge_base<Block_traits_2::X_monotone_curve_2>,
								CGAL::Gps_face_base>								Block_dcel;

typedef CGAL::Arrangement_2<Block_traits_2,Block_dcel>					Block_2;

typedef CGAL::Arr_landmarks_point_location<Block_2>  Block_locator;


inline bool operator<(const Block_2::Face_handle& lhs, const Block_2::Face_handle& rhs)			{	return &*lhs < &*rhs;	}
inline bool operator<(const Block_2::Vertex_handle& lhs, const Block_2::Vertex_handle& rhs)		{	return &*lhs < &*rhs;	}
inline bool operator<(const Block_2::Halfedge_handle& lhs, const Block_2::Halfedge_handle& rhs)	{	return &*lhs < &*rhs;	}
inline bool operator<(const Block_2::Face_const_handle& lhs, const Block_2::Face_const_handle& rhs)			{	return &*lhs < &*rhs;	}
inline bool operator<(const Block_2::Vertex_const_handle& lhs, const Block_2::Vertex_const_handle& rhs)		{	return &*lhs < &*rhs;	}
inline bool operator<(const Block_2::Halfedge_const_handle& lhs, const Block_2::Halfedge_const_handle& rhs)	{	return &*lhs < &*rhs;	}




#endif /* BlockDefs_H */
