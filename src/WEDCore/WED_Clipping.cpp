/* 
 * Copyright (c) 2013, Laminar Research.
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

#include "WED_Clipping.h"
#include "CompGeomDefs2.h"
#include "AssertUtils.h"
#include "CompGeomUtils.h"

/*

	todo - when we split a bezier, we should check the control handles to make sure that the derivative at the line points AWAY
	from the clip line on both sides.

 */
 
/*

	Template ideas
	
	This code someitmes makes use of:
	
	Sequences - a type follows the sequence pattern if it has a ++ operator, a * operator (like an iteraotr), but also
	a function operator that returns true if the sequence is non-empty.  The sequence is thus a self-terminating
	forward iterator.
	
	Filters - a filter predicate returns true for items in a sequence and can be used to reduce a sequence.
	
	Splitters - a splitter returns 0 or more items for each item in a sequence - the input and output types are matched.

	Validation
	
	In debug mode, each one of these routines prints the reason why it fails a clip operation for quick initial debugging
	of (possibly) fubar user data.

*/


// ------------------------------------------------------------------------------------------------------------------------
// GENERAL POLYGON ROUTINES
// ------------------------------------------------------------------------------------------------------------------------

// Return true if this general polygon is fully closed and continuous.
template <typename I>
bool validate_poly_closed(I begin, I end)
{
	if(begin == end) 
		return false;
		
	I first(begin);
	I prev(begin);
	++begin;
	while(begin != end)
	{
		if(prev->p2 != begin->p1)
			return false;
		++begin;
		++prev;
	}
	return first->p1 == prev->p2;
}

template <typename T>
bool side_is_degenerate(const T& s)
{
	return s.p1 == s.p2;
}



template <typename T>
void bbox_for_any(const T& e, Bbox2& b)
{
	b += e;
}
template <>
void bbox_for_any<BezierPoint2>(const BezierPoint2& e, Bbox2& b)
{
	b += e.lo;
	b += e.pt;
	b += e.hi;
}

template <>
void bbox_for_any<Segment2>(const Segment2& e, Bbox2& b)
{
	b += e.p1;
	b += e.p2;
}

//template <>
//void bbox_for_any<Segment2p>(const Segment2p& e, Bbox2& b)
//{
//	b += e.p1;
//	b += e.p2;
//}

template <>
void bbox_for_any<Bezier2>(const Bezier2& e, Bbox2& b)
{
	b += e.p1;
	b += e.c1;
	b += e.p2;
	b += e.c2;
}

template <>
void bbox_for_any<Bezier2p>(const Bezier2p& e, Bbox2& b)
{
	b += e.p1;
	b += e.c1;
	b += e.p2;
	b += e.c2;
}


template <typename T>
void bbox_for_any_vector(const vector<T>& e, Bbox2& b)
{
	for(typename vector<T>::const_iterator i = e.begin(); i != e.end(); ++i)
		bbox_for_any(*i,b);
}


template <typename T>
void bbox_for_any_vector2(const vector<T>& e, Bbox2& b)
{
	for(typename vector<T>::const_iterator i = e.begin(); i != e.end(); ++i)
		bbox_for_any_vector(*i,b);
}


// ------------------------------------------------------------------------------------------------------------------------


// A filter for lines and cuvres - returns true if we're on the given side of the line - 1 for positive,
// 0 for negative.
struct on_side_of_line_h {
	on_side_of_line_h(double in_y, int which_side) : y(in_y), side(which_side) { 
		DebugAssert(which_side == -1 || which_side == 1);
	}
	
	bool operator()(const Segment2& s)
	{
		bool has_pos = (s.p1.y() > y || s.p2.y() > y);
		bool has_neg = (s.p1.y() < y || s.p2.y() < y);
		DebugAssert(!has_pos || !has_neg);
		if(side == 1)	return !has_neg;
		else			return !has_pos;
	}

	bool operator()(const Bezier2& s)
	{
		Vector2 dt1(s.derivative(0.0));
		Vector2 dt2(s.derivative(1.0));
		bool has_pos = (s.p1.y() > y || s.p2.y() > y);
		bool has_neg = (s.p1.y() < y || s.p2.y() < y);

		if(s.p1.y() == y && dt1.dy > 0.0)	has_pos = true;
		if(s.p1.y() == y && dt1.dy < 0.0)	has_neg = true;
		if(s.p2.y() == y && dt2.dy < 0.0)	has_pos = true;
		if(s.p2.y() == y && dt2.dy > 0.0)	has_neg = true;

		DebugAssert(!has_pos || !has_neg);
		if(side == 1)	return !has_neg;
		else			return !has_pos;
	}
	
	double y;
	int side;
};

struct on_side_of_line_v {
	on_side_of_line_v(double in_x, int which_side) : x(in_x), side(which_side) { 
		DebugAssert(which_side == -1 || which_side == 1);
	}
	
	bool operator()(const Segment2& s)
	{
		bool has_pos = (s.p1.x() > x || s.p2.x() > x);
		bool has_neg = (s.p1.x() < x || s.p2.x() < x);
		DebugAssert(!has_pos || !has_neg);
		if(side == 1)	return !has_neg;
		else			return !has_pos;
	}

	bool operator()(const Bezier2& s)
	{
		Vector2 dt1(s.derivative(0.0));
		Vector2 dt2(s.derivative(1.0));
		bool has_pos = (s.p1.x() > x || s.p2.x() > x);
		bool has_neg = (s.p1.x() < x || s.p2.x() < x);

		if(s.p1.x() == x && dt1.dx > 0.0)	has_pos = true;
		if(s.p1.x() == x && dt1.dx < 0.0)	has_neg = true;
		if(s.p2.x() == x && dt2.dx < 0.0)	has_pos = true;
		if(s.p2.x() == x && dt2.dx > 0.0)	has_neg = true;
		
		DebugAssert(!has_pos || !has_neg);
		if(side == 1)	return !has_neg;
		else			return !has_pos;
	}
	
	double x;
	int side;
};

// Line splitters for all types - splits lines into up to 2 segs, and beziers into up to 4 curves.
// These lines correctly substitute intersections with exact split coordinates.
struct split_at_line_h {
	split_at_line_h(double in_y) : y(in_y) { }

	int operator()(const Segment2& in_seg, Segment2 out_segs[2])
	{
		if((in_seg.p1.y() <= y && in_seg.p2.y() <= y) ||
			in_seg.p1.y() >= y && in_seg.p2.y() >= y)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		
		double x = in_seg.x_at_y(y);
		out_segs[0] = Segment2(in_seg.p1,Point2(x,y));
		out_segs[1] = Segment2(Point2(x,y),in_seg.p2);
		DebugAssert(!side_is_degenerate(out_segs[0]));
		DebugAssert(!side_is_degenerate(out_segs[1]));
		return 2;
	}

	int operator()(const Segment2p& in_seg, Segment2p out_segs[2])
	{
		if((in_seg.p1.y() <= y && in_seg.p2.y() <= y) ||
			in_seg.p1.y() >= y && in_seg.p2.y() >= y)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		
		double x = in_seg.x_at_y(y);
		out_segs[0] = Segment2p(in_seg.p1,Point2(x,y),in_seg.param);
		out_segs[1] = Segment2p(Point2(x,y),in_seg.p2,in_seg.param);
		DebugAssert(!side_is_degenerate(out_segs[0]));
		DebugAssert(!side_is_degenerate(out_segs[1]));
		return 2;
	}

	int operator()(const Bezier2& in_seg, Bezier2 out_segs[4])
	{
		if(in_seg.is_segment())
		{
			Segment2 os[2];
			int n = this->operator()(in_seg.as_segment(),os);
			for(int i = 0; i < n; ++i)
			{
				out_segs[i] = Bezier2(os[i]);
				DebugAssert(!side_is_degenerate(out_segs[i]));
			}
			return n;
		}
		
		double t[3];
		Point2	p[3];
		int cuts = in_seg.t_at_y(y,t);
		if(cuts == 0)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		int i;
		for(i = 0; i < cuts; ++i)
		{
			p[i] = in_seg.midpoint(t[i]);
			p[i].y_ = y;
		}
		
		int ret = 0;
		if(t[0] > 0.0)
		{
			in_seg.subcurve(out_segs[ret],0.0,t[0]);
			out_segs[ret].p2 = p[0];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}
		
		for(i = 1; i < cuts; ++i)
		{
			in_seg.subcurve(out_segs[ret],t[i-1],t[i]);
			out_segs[ret].p1 = p[i-1];
			out_segs[ret].p2 = p[i  ];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}

		if(t[cuts-1] < 1.0)
		{
			in_seg.subcurve(out_segs[ret],t[cuts-1],1.0);
			out_segs[ret].p1 = p[cuts-1];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}
		
		return ret;
	}

	int operator()(const Bezier2p& in_seg, Bezier2p out_segs[4])
	{
		if(in_seg.is_segment())
		{
			Segment2 os[2];
			int n = this->operator()(in_seg.as_segment(),os);
			for(int i = 0; i < n; ++i)
			{
				out_segs[i] = Bezier2p(os[i], in_seg.param);
				DebugAssert(!side_is_degenerate(out_segs[i]));
			}	
			return n;
		}
		
		double t[3];
		Point2	p[3];
		int cuts = in_seg.t_at_y(y,t);
		if(cuts == 0)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		int i;
		for(i = 0; i < cuts; ++i)
		{
			p[i] = in_seg.midpoint(t[i]);
			p[i].y_ = y;
		}
		
		int ret = 0;
		if(t[0] > 0.0)
		{
			in_seg.subcurve(out_segs[ret],0.0,t[0]);
			out_segs[ret].p2 = p[0];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}
		
		for(i = 1; i < cuts; ++i)
		{
			in_seg.subcurve(out_segs[ret],t[i-1],t[i]);
			out_segs[ret].p1 = p[i-1];
			out_segs[ret].p2 = p[i  ];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}

		if(t[cuts-1] < 1.0)
		{
			in_seg.subcurve(out_segs[ret],t[cuts-1],1.0);
			out_segs[ret].p1 = p[cuts-1];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}

		for(i = 0; i < ret; ++i)
			out_segs[ret].param = in_seg.param;
		
		return ret;
	}
	
	double y;
};

struct split_at_line_v {
	split_at_line_v(double in_x) : x(in_x) { }

	int operator()(const Segment2& in_seg, Segment2 out_segs[2])
	{
		if((in_seg.p1.x() <= x && in_seg.p2.x() <= x) ||
			in_seg.p1.x() >= x && in_seg.p2.x() >= x)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		
		double y = in_seg.y_at_x(x);
		out_segs[0] = Segment2(in_seg.p1,Point2(x,y));
		out_segs[1] = Segment2(Point2(x,y),in_seg.p2);
		DebugAssert(!side_is_degenerate(out_segs[0]));
		DebugAssert(!side_is_degenerate(out_segs[1]));
		return 2;
	}

	int operator()(const Segment2p& in_seg, Segment2p out_segs[2])
	{
		if((in_seg.p1.x() <= x && in_seg.p2.x() <= x) ||
			in_seg.p1.x() >= x && in_seg.p2.x() >= x)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		
		double y = in_seg.y_at_x(x);
		out_segs[0] = Segment2p(in_seg.p1,Point2(x,y),in_seg.param);
		out_segs[1] = Segment2p(Point2(x,y),in_seg.p2,in_seg.param);
		DebugAssert(!side_is_degenerate(out_segs[0]));
		DebugAssert(!side_is_degenerate(out_segs[1]));
		return 2;
	}
	
	int operator()(const Bezier2& in_seg, Bezier2 out_segs[4])
	{
		if(in_seg.is_segment())
		{
			Segment2 os[2];
			int n = this->operator()(in_seg.as_segment(),os);
			for(int i = 0; i < n; ++i)
			{
				out_segs[i] = Bezier2(os[i]);
				DebugAssert(!side_is_degenerate(out_segs[i]));
			}
			return n;
		}
		
		double t[3];
		Point2	p[3];
		int cuts = in_seg.t_at_x(x,t);
		if(cuts == 0)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		int i;
		for(i = 0; i < cuts; ++i)
		{
			p[i] = in_seg.midpoint(t[i]);
			p[i].x_ = x;
		}
		
		int ret = 0;
		if(t[0] > 0.0)
		{
			in_seg.subcurve(out_segs[ret],0.0,t[0]);
			out_segs[ret].p2 = p[0];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}
		
		for(i = 1; i < cuts; ++i)
		{
			in_seg.subcurve(out_segs[ret],t[i-1],t[i]);
			out_segs[ret].p1 = p[i-1];
			out_segs[ret].p2 = p[i  ];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}

		if(t[cuts-1] < 1.0)
		{
			in_seg.subcurve(out_segs[ret],t[cuts-1],1.0);
			out_segs[ret].p1 = p[cuts-1];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}
		
		return ret;
	}

	int operator()(const Bezier2p& in_seg, Bezier2p out_segs[4])
	{
		if(in_seg.is_segment())
		{
			Segment2 os[2];
			int n = this->operator()(in_seg.as_segment(),os);
			for(int i = 0; i < n; ++i)
			{
				out_segs[i] = Bezier2p(os[i],in_seg.param);
				DebugAssert(!side_is_degenerate(out_segs[i]));
			}
			return n;
		}
		
		double t[3];
		Point2	p[3];
		int cuts = in_seg.t_at_x(x,t);
		if(cuts == 0)
		{
			out_segs[0] = in_seg;
			DebugAssert(!side_is_degenerate(out_segs[0]));
			return 1;
		}
		int i;
		for(i = 0; i < cuts; ++i)
		{
			p[i] = in_seg.midpoint(t[i]);
			p[i].x_ = x;
		}
		
		int ret = 0;
		if(t[0] > 0.0)
		{
			in_seg.subcurve(out_segs[ret],0.0,t[0]);
			out_segs[ret].p2 = p[0];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}
		
		for(i = 1; i < cuts; ++i)
		{
			in_seg.subcurve(out_segs[ret],t[i-1],t[i]);
			out_segs[ret].p1 = p[i-1];
			out_segs[ret].p2 = p[i  ];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}

		if(t[cuts-1] < 1.0)
		{
			in_seg.subcurve(out_segs[ret],t[cuts-1],1.0);
			out_segs[ret].p1 = p[cuts-1];
			DebugAssert(!side_is_degenerate(out_segs[ret]));
			ret++;
		}

		for(i = 0; i < ret; ++i)
			out_segs[ret].param = in_seg.param;
		return ret;
	}
	
	
	double x;
};

// TEMPLATE UTILITIES
// Since we don't have C++11, creating the 'complete' filtered sequence of all
// split and filter types is insanely nasty to write - typedefs everywhere.
//
// So...we get lazy: we have utilities that, given a container and filter or
// splitter, process the container via the filter.  The sequence types are
// generated internally and don't leak out into client code.
//
// If we had 'auto' this probably would not be needed.
template <typename T, typename F>
void apply_filter(T& container, F filter)
{
	T	dst;
	
	typedef sequence_for_container<T>	src_seq_t;
	typedef filtered_seq<src_seq_t,F>	filtered_seq_t;
	
	filtered_seq_t fs(src_seq_t(container),filter);
	
	sequence_push_back(dst,fs);
	dst.swap(container);
	
}

// Note: for splitters the template needs an 'N' - the maximum split count for 
// a given input!  This is used to size internal storage in the split-seq.
template <typename T, typename F, int N>
void apply_split(T& container, F filter)
{
	T	dst;
	
	typedef sequence_for_container<T>	src_seq_t;
	typedef split_seq<src_seq_t,F,N>	split_seq_t;
	
	split_seq_t ss(src_seq_t(container),filter);
	
	sequence_push_back(dst,ss);
	dst.swap(container);
	
}

// ------------------------------------------------------------------------------------------------------------------------
// SEGMENT CLIPPING
// ------------------------------------------------------------------------------------------------------------------------

// These routines clip segments to an AABB using our filters.  The algortihm is stupidly simple:
// 1. For every segment, replace it with up to N segments so that it doesn't cross the split line.
// 2. Then, throw out everything on the wrong side of the split line.
// 3. Repeat 1 & 2 for all 4 sides of our AABB.

void	clip_segments(vector<Segment2>& out_segs, const Bbox2& box)
{
	split_at_line_h			split_bot(box.ymin()), split_top(box.ymax());
	split_at_line_v			split_lft(box.xmin()), split_rgt(box.xmax());
	on_side_of_line_h		clip_bot(box.ymin(),1), clip_top(box.ymax(),-1);
	on_side_of_line_v		clip_lft(box.xmin(),1), clip_rgt(box.xmax(),-1);	
	
	apply_split<vector<Segment2>, split_at_line_h,2>(out_segs, split_bot);	
	apply_split<vector<Segment2>, split_at_line_h,2>(out_segs, split_top);	
	apply_split<vector<Segment2>, split_at_line_v,2>(out_segs, split_lft);	
	apply_split<vector<Segment2>, split_at_line_v,2>(out_segs, split_rgt);	
	apply_filter(out_segs, clip_bot);
	apply_filter(out_segs, clip_top);
	apply_filter(out_segs, clip_lft);
	apply_filter(out_segs, clip_rgt);
}

void	clip_segments(vector<Segment2p>& out_segs, const Bbox2& box)
{
	split_at_line_h			split_bot(box.ymin()), split_top(box.ymax());
	split_at_line_v			split_lft(box.xmin()), split_rgt(box.xmax());
	on_side_of_line_h		clip_bot(box.ymin(),1), clip_top(box.ymax(),-1);
	on_side_of_line_v		clip_lft(box.xmin(),1), clip_rgt(box.xmax(),-1);	
	
	apply_split<vector<Segment2p>, split_at_line_h,2>(out_segs, split_bot);	
	apply_split<vector<Segment2p>, split_at_line_h,2>(out_segs, split_top);	
	apply_split<vector<Segment2p>, split_at_line_v,2>(out_segs, split_lft);	
	apply_split<vector<Segment2p>, split_at_line_v,2>(out_segs, split_rgt);	
	apply_filter(out_segs, clip_bot);
	apply_filter(out_segs, clip_top);
	apply_filter(out_segs, clip_lft);
	apply_filter(out_segs, clip_rgt);
}

void	clip_segments(vector<Bezier2>& out_segs, const Bbox2& box)
{
	split_at_line_h			split_bot(box.ymin()), split_top(box.ymax());
	split_at_line_v			split_lft(box.xmin()), split_rgt(box.xmax());
	on_side_of_line_h		clip_bot(box.ymin(),1), clip_top(box.ymax(),-1);
	on_side_of_line_v		clip_lft(box.xmin(),1), clip_rgt(box.xmax(),-1);	
	
	apply_split<vector<Bezier2>, split_at_line_h,4>(out_segs, split_bot);	
	apply_split<vector<Bezier2>, split_at_line_h,4>(out_segs, split_top);	
	apply_split<vector<Bezier2>, split_at_line_v,4>(out_segs, split_lft);	
	apply_split<vector<Bezier2>, split_at_line_v,4>(out_segs, split_rgt);	
	apply_filter(out_segs, clip_bot);
	apply_filter(out_segs, clip_top);
	apply_filter(out_segs, clip_lft);
	apply_filter(out_segs, clip_rgt);
}

void	clip_segments(vector<Bezier2p>& out_segs, const Bbox2& box)
{
	split_at_line_h			split_bot(box.ymin()), split_top(box.ymax());
	split_at_line_v			split_lft(box.xmin()), split_rgt(box.xmax());
	on_side_of_line_h		clip_bot(box.ymin(),1), clip_top(box.ymax(),-1);
	on_side_of_line_v		clip_lft(box.xmin(),1), clip_rgt(box.xmax(),-1);	
	
	apply_split<vector<Bezier2p>, split_at_line_h,4>(out_segs, split_bot);	
	apply_split<vector<Bezier2p>, split_at_line_h,4>(out_segs, split_top);	
	apply_split<vector<Bezier2p>, split_at_line_v,4>(out_segs, split_lft);	
	apply_split<vector<Bezier2p>, split_at_line_v,4>(out_segs, split_rgt);	
	apply_filter(out_segs, clip_bot);
	apply_filter(out_segs, clip_top);
	apply_filter(out_segs, clip_lft);
	apply_filter(out_segs, clip_rgt);
}

// ------------------------------------------------------------------------------------------------------------------------
// UTILITIES FOR POLYGON CLIPPING
// ------------------------------------------------------------------------------------------------------------------------

// The construct-segment template builds a new linear sub-curve off of type S and jams it into the push-back container C.
// The curve C passed in is an 'example' whose parameters are copied.  (e.g. if it is a segment-with-param and the param 
// value is "6", then the new segment's param will be 6.
template <typename C, typename S>
struct construct_segment {
	C&	c_;
	construct_segment(C& c) : c_(c) {}
	void operator()(const Point2& p1, const Point2& p2, const S& example)
	{
		S clone(example);
		clone.p1 = p1;
		clone.p2 = p2;
		c_.push_back(clone);
	}
};

template <typename C>
struct construct_segment<C, Bezier2> {
	C&	c_;
	construct_segment(C& c) : c_(c) {}
	void operator()(const Point2& p1, const Point2& p2, const Bezier2& example)
	{
		Bezier2 clone(example);
		clone.p1 = p1;
		clone.p2 = p2;
		clone.c1 = p1;
		clone.c2 = p2;
		c_.push_back(clone);
	}
};

template <typename C>
struct construct_segment<C, Bezier2p> {
	C&	c_;
	construct_segment(C& c) : c_(c) {}
	void operator()(const Point2& p1, const Point2& p2, const Bezier2p& example)
	{
		Bezier2p clone(example);
		clone.p1 = p1;
		clone.p2 = p2;
		clone.c1 = p1;
		clone.c2 = p2;
		c_.push_back(clone);
	}
};


// These routines generate the 'capping' of a clipped polygon.  Basically for each line segment
// along our clipping edge, where a segment exits, then enters later, we make a straight line 
// sgment connecting them.  Given a polygon with correct winding, this means we induce a line
// whose directed left side is the border of the polygon interior with the clip line.

// If the clipped polygon is screwed up, this routine will fail.  For example, clipping a figure-8
// polygon will often result in two lines both exiting in a row, which is an indication that there
// was a self-intersection _somewhere_.

// Dir: 1 if the INSIDE of the polygon is to the left of the line Y = y when going to the RIGHT.
//    in other words, clipping the BOTTOM of the polygon we use dir 1 because the line has direction 
// dx,dy = 1,0.
template <typename C, typename F>
bool	cap_edge_h(const C& in_segs, F make_curve, double y, int dir)
{
	typedef multimap<double, pair<int, typename C::const_iterator> >	imap_t;
	
	imap_t xons;
	
	for(typename C::const_iterator i = in_segs.begin(); i != in_segs.end(); ++i)
	{
		DebugAssert(!side_is_degenerate(*i));
		if(i->p1.y() == y)
			xons.insert(typename imap_t::value_type(i->p1.x(),make_pair(-1,i)));
		if(i->p2.y() == y)
			xons.insert(typename imap_t::value_type(i->p2.x(),make_pair(1,i)));
	}

	typename imap_t::iterator start = xons.begin(), stop = xons.end(), end;
	
	bool	is_open = false;
	double	prev_x;
	typename C::const_iterator prev_curve;
	
	while(start != stop)
	{
		end = start;
		while(end != stop && end->first == start->first)
			++end;
			
		double now_x = start->first;
		typename C::const_iterator now_curve = start->second.second;
		
		// We now have a range of equal points along the sweep.
		// We are going to verify its count.
		
		int count = 0;
		while(start != end)
		{
			count += start->second.first;
			++start;
		}

		if(count < -1)
		{
			#if DEV
			printf("Failed polygon during seal-off: two lines originate at the cut with no match.\n");
			#endif
			return false;
		}
		if(count > 1)
		{
			#if DEV
			printf("Failed polygon during seal-off: two lines start at the cut with no match.\n");
			#endif
			return false;
		}
		
		switch(count) {
		case -1:		
			if(dir == 1)
			{
				if(is_open)
				{
					make_curve(Point2(prev_x,y),Point2(now_x,y),*prev_curve);
					is_open = false;
				} 
				else
				{
					#if DEV
					printf("Failed polygon during seal-off: close a line while closed, going right.\n");
					#endif
					return false;
				}
			}
			else
			{
				if(is_open)
				{
					#if DEV
					printf("Failed polygon during seal-off: opened a second line while the first is in progress going left.\n");
					#endif
					return false;
				}
				else
				{
					is_open = true;
					prev_x = now_x;
					prev_curve = now_curve;
				}
			}
			break;
		case 0:
			if(is_open)
			{
				if(dir == 1)
					make_curve(Point2(prev_x,y),Point2(now_x,y),*prev_curve);
				else
					make_curve(Point2(now_x,y),Point2(prev_x,y),*now_curve);
				prev_x = now_x;
				prev_curve = now_curve;				
			}
			break;
		case 1:
			if(dir == 1)
			{
				if(is_open)
				{
					#if DEV
					printf("Failed polygon during seal-off: opened a second line while a first is in progress going right.\n");
					#endif
					return false;
				}
				else
				{
					prev_x = now_x;
					prev_curve = now_curve;				
					is_open = true;
				}
			}
			else
			{
				if(is_open)
				{
					make_curve(Point2(now_x,y),Point2(prev_x,y),*prev_curve);
					is_open = false;
				} else
				{
					#if DEV
					printf("Failed polygon during seal-off: close a line while closed, going left.\n");
					#endif
					return false;
				}
			
			}
			break;
		}
	}
	return true;
	
}


template <typename C, typename F>
bool	cap_edge_v(const C& in_segs, F make_curve, double x, int dir)
{
	typedef multimap<double, pair<int, typename C::const_iterator> >	imap_t;
	
	imap_t xons;
	
	for(typename C::const_iterator i = in_segs.begin(); i != in_segs.end(); ++i)
	{
		DebugAssert(!side_is_degenerate(*i));
		if(i->p1.x() == x)
			xons.insert(typename imap_t::value_type(i->p1.y(),make_pair(-1,i)));
		if(i->p2.x() == x)
			xons.insert(typename imap_t::value_type(i->p2.y(),make_pair(1,i)));
	}

	typename imap_t::iterator start = xons.begin(), stop = xons.end(), end;
	
	bool	is_open = false;
	double	prev_y;
	typename C::const_iterator prev_curve;
	
	while(start != stop)
	{
		end = start;
		while(end != stop && end->first == start->first)
			++end;
			
		double now_y = start->first;
		typename C::const_iterator now_curve = start->second.second;
		
		// We now have a range of equal points along the sweep.
		// We are going to verify its count.
		
		int count = 0;
		while(start != end)
		{
			count += start->second.first;
			++start;
		}

		if(count < -1)
		{
			#if DEV
			printf("Failed polygon during seal-off: two lines originate at the cut with no match.\n");
			#endif
			return false;
		}
		if(count > 1)
		{
			#if DEV
			printf("Failed polygon during seal-off: two lines start at the cut with no match.\n");
			#endif
			return false;
		}
		
		switch(count) {
		case -1:		
			if(dir == 1)
			{
				if(is_open)
				{
					make_curve(Point2(x,prev_y),Point2(x,now_y),*prev_curve);
					is_open = false;
				} 
				else
				{
					#if DEV
					printf("Failed polygon during seal-off: close a line while closed, going right.\n");
					#endif
					return false;
				}
			}
			else
			{
				if(is_open)
				{
					#if DEV
					printf("Failed polygon during seal-off: opened a second line while the first is in progress going left.\n");
					#endif
					return false;
				}
				else
				{
					is_open = true;
					prev_y = now_y;
					prev_curve = now_curve;
				}
			}
			break;
		case 0:
			if(is_open)
			{
				if(dir == 1)
					make_curve(Point2(x,prev_y),Point2(x,now_y),*prev_curve);
				else
					make_curve(Point2(x,now_y),Point2(x,prev_y),*now_curve);
				prev_y = now_y;
				prev_curve = now_curve;
			}
			break;
		case 1:
			if(dir == 1)
			{
				if(is_open)
				{
					#if DEV
					printf("Failed polygon during seal-off: opened a second line while a first is in progress going right.\n");
					#endif
					return false;
				}
				else
				{
					prev_y = now_y;
					prev_curve = now_curve;
					is_open = true;
				}
			}
			else
			{
				if(is_open)
				{
					make_curve(Point2(x,now_y),Point2(x,prev_y),*now_curve);
					is_open = false;
				} else
				{
					#if DEV
					printf("Failed polygon during seal-off: close a line while closed, going left.\n");
					#endif
					return false;
				}
			
			}
			break;
		}
	}
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------
// MORE POLYGON UTILITY TEMPLATES
// ------------------------------------------------------------------------------------------------------------------------

// A functor to tell us if a point is inside a general polygon sequence.  The functor uses
// operator overloading to 'know' if we are a segment or bezier polygon - we need a
// different algorithm for the bezier case.
template <typename I>
struct inside_polygon_curve {

	bool operator()(I b, I e, const Segment2& s) const {
		return inside_polygon_seg(b,e,s.p1);
	}
	bool operator()(I b, I e, const Bezier2& s) const {
		return inside_polygon_bez(b,e,s.p1);
	}
};

// These routines convert a general-polygon-with-holes to a straight segment list
// for the initial clip.  Topology info is lost, but that's okay - we will have 
// to reconstruct it anyway after the clip.

template <typename C1, typename C2>
void flatten_general_polygon_with_holes(C1& src, C2& dst)
{
	for(typename C1::const_iterator i = src.begin(); i != src.end(); ++i)
		dst.insert(dst.end(),i->sides_begin(),i->sides_end());
}


// ------------------------------------------------------------------------------------------------------------------------
// CLIPPING LINE UTILS
// ------------------------------------------------------------------------------------------------------------------------

// At this point it has become annoying to specal-case H vs V all of the time.  So we create
// a standard structure that can manage the direction and axis of a clipping line.
// dir "d" is the direction ALONG the line, e.g. dir=1 is_vertical=true is a clipping line 
// whose equation is x*-coord + y * 0 = 0 (that is, a line going from bottom to top with x=coord.)

struct clipping_line {
	clipping_line(bool v, double c, int d) : is_vertical(v), coord(c), dir(d) { }
	bool	is_vertical;
	double	coord;
	int		dir;
};

// Traits overload to know what our max-split is for bezier vs non-bezier.
template <typename T>
struct split_traits {
	static const int N = 2;
};

template<>
struct split_traits<Bezier2> {
	static const int N = 4;
};
template<>
struct split_traits<Bezier2p> {		// needed?  I don't know.  C++ paranoia!!
	static const int N = 4;
};

// This routine clips a collection of ANY type of curve along an ARBITRARY clipping line
// and optionally "caps" it if desired.  This saves us having to special-case out the h 
// vs v case.
// The algo is similar to how we manage clipping segments - when done, the directed in/out
// pattern of the polygon along the clip edge tells us where we need to cap.
template <typename SC>
bool clip_any(SC& io_segs, const clipping_line& l, bool cap)
{
	#if DEV
		for(typename SC::iterator s = io_segs.begin(); s != io_segs.end(); ++s)
			DebugAssert(!side_is_degenerate(*s));
	#endif
	typedef typename SC::value_type S;
	if(l.is_vertical)
	{
		split_at_line_v		splitter(l.coord);
		on_side_of_line_v	filter(l.coord, -l.dir);	// vertical line going UP, we want the NEGATIVE x side.

		apply_split<SC, split_at_line_v,split_traits<S>::N>(io_segs, splitter);	

		for(typename SC::iterator s = io_segs.begin(); s != io_segs.end(); ++s)
			DebugAssert(!side_is_degenerate(*s));

		apply_filter(io_segs, filter);		

		for(typename SC::iterator s = io_segs.begin(); s != io_segs.end(); ++s)
			DebugAssert(!side_is_degenerate(*s));
		
		if(cap)
		{
			SC new_segs;
			if(!cap_edge_v(io_segs, construct_segment<SC, S>(new_segs), l.coord, l.dir))
				return false;
				
			io_segs.insert(io_segs.end(),new_segs.begin(),new_segs.end());
		}
	}
	else
	{
		split_at_line_h		splitter(l.coord);
		on_side_of_line_h	filter(l.coord, l.dir);		// horizonta line goes right, we want the UP side - positive Y!

		apply_split<SC, split_at_line_h,split_traits<S>::N>(io_segs, splitter);	

		for(typename SC::iterator s = io_segs.begin(); s != io_segs.end(); ++s)
			DebugAssert(!side_is_degenerate(*s));

		apply_filter(io_segs, filter);		

		for(typename SC::iterator s = io_segs.begin(); s != io_segs.end(); ++s)
			DebugAssert(!side_is_degenerate(*s));
			
		if(cap)
		{
			SC new_segs;
			if(!cap_edge_h(io_segs, construct_segment<SC, S>(new_segs), l.coord, l.dir))
				return false;
			io_segs.insert(io_segs.end(),new_segs.begin(),new_segs.end());
		}
	}	
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------
// CCB RECONSTRUCTION
// ------------------------------------------------------------------------------------------------------------------------

// This set of utils lets us reconstruct a series of close counter-clockwise boundary curves from the mess of cuts we made.
// This is what puts humpty-dumpty back together after we just clip and cap.

// This functor sorts curves lexicographically - this ensures that the
// FIRST curve we find will be an outer CCB (because it has an extrema
// point).
struct sort_segment_x_then_y {
	bool operator()(const Segment2& lhs, const Segment2& rhs) const { 
		lesser_x_then_y comp;
		if(comp(lhs.p1,rhs.p1))
			return true;
		if(comp(lhs.p1,rhs.p1))
			return false;
		return comp(lhs.p2,rhs.p2);
	}
	bool operator()(const Bezier2& lhs, const Bezier2& rhs) const { 
		lesser_x_then_y comp;
		if(comp(lhs.p1,rhs.p1))
			return true;
		if(comp(lhs.p1,rhs.p1))
			return false;
		return comp(lhs.p2,rhs.p2);
	}
};

// Predicate for matching edges - we win if this next curve matches
// our current curve's end.
struct src_matches_target {
	src_matches_target(const Point2& target) : target_(target) { }
	bool operator()(const Segment2& s) const { return s.p1 == target_; }
	bool operator()(const Bezier2& s) const { return s.p1 == target_; }
	Point2 target_;
};

// return true if new_candidate fits prev better than best_so_far in terms of
// finding the outermost boundary.  Since our boundary is CCB, we want to make the
// right-most turns to stay on the outer edge of the polygon.
template <typename C>
bool better_next_seg(const C& prev, const C& best_so_far, const C& new_candidate)
{
	DebugAssert(prev.p2 == best_so_far.p1);
	DebugAssert(prev.p2 == new_candidate.p1);
	
	Vector2 v1(prev.p2,prev.p1);
	Vector2 v2(best_so_far.p1,best_so_far.p2);
	Vector2 v3(new_candidate.p1,new_candidate.p2);
	
	return Is_CCW_Between(v1,v3,v2);
}

// CCB reconstruction.  Some polygon clipping algorithms attempt to track segments while
// capping them.  We do not.  We generate the complete mess of segments, then go through
// and try to find the polygons.
//
// THIS IS NOT FAST.  Time is O(NM) (N = number of segs, M = number of contours) because we 
// linearly search for the next segment for each contour.  We could do better with more
// aggressive indexing but it's not necessarily worth it, especially if M < lon(N).
//
// The basic idea is: sort the curves lexicograhically.  Pull out the lowest one - it has
// to be an outer-most CCB.*  Walk the rest of the curves, taking the rght-most turn while 
// always matching until our contour is closed.  Do it again until we drain all edges.
//
// *I originally thought this segment mix might have holes in it, in which case we would
// want to always pull the CCBs first and holes second, hence the sort.  This isn't actually
// necessary - see the treatment of holes in clip_pwh.
template <typename GP>
bool reconstruct_ccbs(const GP& outer_boundary, vector<GP>& outer_ccbs)
{
	typedef typename GP::value_type C;
	typedef multiset<C, sort_segment_x_then_y>	seg_q;
	seg_q	sorted_ccb(outer_boundary.begin(), outer_boundary.end());
	while(!sorted_ccb.empty())
	{
		C root(*sorted_ccb.begin());
		
		sorted_ccb.erase(sorted_ccb.begin());
		
		outer_ccbs.push_back(GP());
		GP& cur(outer_ccbs.back());
		
		cur.push_back(root);
		
		while(cur.front().p1 != cur.back().p2)
		{
			typename seg_q::iterator s = sorted_ccb.begin(), best = sorted_ccb.end();
			while(s != sorted_ccb.end())
			{
				s = find_if(s, sorted_ccb.end(), src_matches_target(cur.back().p2));
				if(s != sorted_ccb.end())
				{
					if(best == sorted_ccb.end() || better_next_seg(cur.back(), *best, *s))
						best = s;
					++s;
				}
			}
			
			if(best == sorted_ccb.end())
			{
				#if DEV
				printf("Polygon reconstruction failed: we could not find the end of a boundary.\n");
				#endif
				return false;
			}
			cur.push_back(*best);
			sorted_ccb.erase(best);
		}		
	}
	
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------
// POLYGON CLIPPING - FOR REALZIES THIS TIME!
// ------------------------------------------------------------------------------------------------------------------------

// This is similar to validate_polygon_closed, except that we do NOT expect the sequence of curves
// to be (1) only one contour or (2) in order.  Thus we keep a map and count...if our starts and ends
// don't match, something has gone wrong.

template <typename C>
bool validate_polygon_links(const C& segs)
{
	map<Point2,int,lesser_y_then_x>		counts;
	for(typename C::const_iterator c = segs.begin(); c != segs.end(); ++c)
	{
		counts[c->p1]++;
		counts[c->p2]--;
	}
	
	for(map<Point2,int,lesser_y_then_x>::iterator i = counts.begin(); i != counts.end(); ++i)
	if(i->second != 0)
	{	
		printf("Validation failed: %d extra count of %lf,%lf\n",i->second, i->first.x(),i->first.y());
		return false;
	}
	return true;
}

// This set of templates tells us the side of a clipping line for all sorts of stuff.
// Returns: 1 if all of thingie is ENTIRELY on the left side of "L".
// Returns: -1 if all of thingie is ENTIRELY on the right side of "L".
// Returns: 0 if any of thingie crosses L (or is fully on L).
template <typename T>
int side_of_clipping_line(const T& thingie, const clipping_line& l)
{
	if(thingie.begin() == thingie.end())
	{
		DebugAssert(!"empty polygon!?!");
		return 0;
	}
	typename T::const_iterator i = thingie.begin();
	int t0 = side_of_clipping_line(*i,l);
	++i;
	while(i != thingie.end())
	{
		int t = side_of_clipping_line(*i,l);
		if(t != t0)
			return 0;
		++i;
	}
	return t0;
}

template<>
int side_of_clipping_line<Point2>(const Point2& p, const clipping_line& l)
{
	double v = l.is_vertical ? (p.x() - l.coord) : (p.y() - l.coord);
	int sign = l.is_vertical ? -1 : 1;

	if(v > 0) return l.dir * sign;
	if(v < 0) return -l.dir * sign;
	return 0;
}

template<>
int side_of_clipping_line<Segment2>(const Segment2& p, const clipping_line& l)
{
	int t0 = side_of_clipping_line(p.p1,l);
	int t1 = side_of_clipping_line(p.p2,l);
	if(t0 == 0) return 0;
	if(t0 != t1) return 0;
	return t0;
}

template<>
int side_of_clipping_line<Segment2p>(const Segment2p& p, const clipping_line& l)
{
	return side_of_clipping_line<Segment2>(p,l);
}

template<>
int side_of_clipping_line<Bezier2>(const Bezier2& b, const clipping_line& l)
{
	double t[4];
	if(l.is_vertical)
	{
		int count = b.t_at_x(l.coord, t);
		if(count) return 0;
	} else {
		int count = b.t_at_y(l.coord, t);
		if(count) return 0;
	}
	return side_of_clipping_line(b.as_segment(),l);
}

template<>
int side_of_clipping_line<Bezier2p>(const Bezier2p& p, const clipping_line& l)
{
	return side_of_clipping_line<Bezier2>(p,l);
}

template<>
int side_of_clipping_line<Polygon2>(const Polygon2& p, const clipping_line& l)
{
	if(p.empty())
	{
		DebugAssert(!"empty polygon!?!");
		return 0;
	}
	int t0 = side_of_clipping_line(p[0],l);
	if(t0 == 0)
		return 0;
	for(int i = 1; i < p.size(); ++i)
	{
		int t = side_of_clipping_line(p[i],l);
		if(t != t0)
			return 0;
	}
	return t0;
}

// This is the money routine - this is where we take one general-polygon-with-holes (GPW) and
// produce 0 or more GPWH based on the clipping line.
//
// TOPOLOGY OF THIS ALGORITHM
//
// First, note that a single polygon, when clipped by a half-plane, may form multiple polygons.
// cut off the top half of the letter W and you end up with two separate lines from the ends of
// the letter.  In other words, a polygon may be severed if two sub-areas are linked only by
// an area outside the clip half-plane.

// Observation: all holes in a GPWH will fall into three categories:
// 1. The hole is entirely inside the clip half-plane - the hole remains unmodified, but we do
// have to figure out which polygon it is now part of (since our polygon may have been severed
// in two.
//
// 2. The hole is entirely outside the clip half-plane.  In this case, the hole, as a whole,
// is gone, as is the surrounding polygon around it.
//
// 3. The hole spans the clip plane.  In this case, we _know_ that the hole is going to 
// disappear and its remaining contour will become part of the outer boudnary of the polygon!
//
// Case 3 is important for how we handle topology.  HOW do we know that?  Well...
//
// - The hole is contained within the outer boundary.  So if the hole intersects the clipping 
//	 line, its parent polygon's outer boundary must intersect it too.
//
// - The outer boundary and inner hole are connected by 'polygon interior'.
//
// Therefore the clipping line is going to become polygon outer boundary, connecting the 
// original outer boundary to the inner hole boundary in at least two segments.
//
// Therefore when done, the hole won't be a hole because it will be connected to an outer
// boundary!
//
//
// So...when we clip the pwh, we basically treat all line-intersecting holes as "more CCB"
// because they will be in the end; all non-intersecting holes are either dropped or 
// re-stashed as holes into the new polygons, as a whole.

template <typename GP>
bool clip_pwh(const vector<GP>& pwh, vector<vector<GP> >& out_pwh_list, const clipping_line& l)
{
	typedef typename GP::value_type C;
	
	DebugAssert(pwh.size() > 0);
	DebugAssert(pwh[0].size() > 0);
	out_pwh_list.clear();
	
	vector<GP>	keeper_holes;			// All holes that are unclipped and remain in the polygon when done.
	GP			outer_boundary;			// All segments that are or will become outer boundary.

	// Early exit cases: if our outer bounary is entirely clipped or unclipped, bail
	// now.  Besides being fast, this ensures that the pwh is _unmodified_ in order when
	// not clipped, something that is convenient for WED/10.00 export of facades.
	
	int sic = side_of_clipping_line(pwh[0], l);
	if(sic == -1)
		return true;
		
	if(sic == 1)
	{
		out_pwh_list.push_back(pwh);
		return true;
	}

	if(!is_ccw_polygon_seg(pwh[0].begin(), pwh[0].end()))
	{
		#if DEV
			printf("Outer boundary is not ccw.\n");
		#endif
		return false;
	}
	outer_boundary = pwh[0];
	
	// For each hole, we are goin to either chuck them, keep them as is, or put them in the
	// outer boundary for clipping.
	
	for(int h = 1; h < pwh.size(); ++h)
	{
		DebugAssert(pwh[h].size() > 0);
		if(is_ccw_polygon_seg(pwh[h].begin(), pwh[h].end()))
		{
			#if DEV
				printf("Hole is not clockwise.\n");
			#endif
			return false;
		}
		sic = side_of_clipping_line(pwh[h], l);
		if(sic == 1)
			keeper_holes.push_back(pwh[h]);
		else if(sic == 0)
			outer_boundary.insert(outer_boundary.end(),pwh[h].begin(),pwh[h].end());
	}
	
	if(!validate_polygon_links(outer_boundary))
	{
		DebugAssert(!"We should not have a broken boundary this early on!");
		return false;
	}

	// Clip-n-cap operation.  At this point the outer boundary has every segment of the new
	// polygon(s) with holes, but not in any particular order.

	if(!clip_any(outer_boundary, l, true))
	{
		return false;
	}

	if(!validate_polygon_links(outer_boundary))
	{
		return false;
	}
	
	// Go back and recontruct CCW boundaries from our outer CCBs.  This is how we find
	// out how many polygons we will have.
	
	vector<GP>	outer_ccbs;
	if(!reconstruct_ccbs(outer_boundary, outer_ccbs))
		return false;
		
	// C++ fugliness: convert polygon to polygon-with-hole format for output.  This could
	// be less stupid in the future.
		
	for(typename vector<GP>::iterator i = outer_ccbs.begin(); i != outer_ccbs.end(); ++i)
	{
		DebugAssert(i->size() == 2 || (i->size() > 2 && is_ccw_polygon_seg(i->begin(),i->end())));
		
		if(i->size() > 2)
		{
			out_pwh_list.push_back(vector<GP>());
			out_pwh_list.back().push_back(*i);
		}
	}
	
	// Hole bucketing - we are now going to go through all of our fully retained holes and use one
	// test point on the hole to figure out which outer CCB they go in if we were split.
	// Drop them there.
	
	inside_polygon_curve<typename GP::iterator>		curve_tester;

	for(typename vector<GP>::iterator kh = keeper_holes.begin(); kh != keeper_holes.end(); ++kh)
	{
		int ccb;
		for (ccb = 0; ccb < out_pwh_list.size(); ++ccb)
		{
			if(curve_tester(out_pwh_list[ccb].front().begin(),out_pwh_list[ccb].front().end(), kh->front()))
			{
				out_pwh_list[ccb].push_back(*kh);
				break;
			}
		}
		if(ccb == out_pwh_list.size())
		{
			#if DEV
			printf("We could not 'sink' a hole - perhaps it is not inside its polygon.\n");
			#endif
			return false;
		}
	}

	return true;
}

// Polygon-with-hole list clipper - a utility to iterate on the entire list of disjoint polygons with holes.
template <typename GP>
bool clip_pwh_list(const vector<vector<GP> >& pwh_list, vector<vector<GP> >& out_pwh_list, const clipping_line& l)
{
	out_pwh_list.clear();

	for(int p = 0; p < pwh_list.size(); ++p)
	{
		vector<vector<GP> > post_clip;
		if(!clip_pwh(pwh_list[p], post_clip, l))
			return false;
		out_pwh_list.insert(out_pwh_list.end(), post_clip.begin(),post_clip.end());		
	}
	return true;
}

// Polygon-box clipper.  Basically we just clip to the four half-planes of the bounding box sides.
// We use clip_pwh_list after the first clip because we may hav severed the polygon in the first clip
// and then must separately clip each disjoint fragment.
template <typename GP>
bool clip_general_polygon(const vector<GP>& in_pwh, vector<vector<GP> >& out_pwh_list, const Bbox2& box)
{
	// This special case is sort of a hack.  We detect the case where we are -entirely- inside the clip box
	// and return our direct input.  This isn't just a "fast exit" case.  It turns out that the math deep
	// inside the line-bezier intersection code goes absolutely bat-shit if you try to intersect a finite
	// length bezier with a line that's "really far" from the cubic.*  We can avoid this by recognizing that
	// the clip lines are -nowhere near- us and avoiding the math pitfall.
	//
	// * The bug is that when the cubic is far from the intersection line, the partial calculations become
	//   so huge (in magnitude) that they truncate; dividing them does not reconstitute a high precision 
	//   answer and we get -false- intersections within the time 0...1 (which makes it look like a valid
	//   intersection).
	
	Bbox2	bounds;
	bbox_for_any_vector2(in_pwh,bounds);
	if(box.contains(bounds))
	{
		out_pwh_list.push_back(in_pwh);
		return true;
	}

	for(typename vector<GP>::const_iterator p = in_pwh.begin(); p != in_pwh.end(); ++p)
	for(typename GP::const_iterator s = p->begin(); s != p->end(); ++s)
	{
		if(side_is_degenerate(*s))
		{
			#if DEV
				printf("Zero length side on input.");			
			#endif
			return false;
		}
	}

	clipping_line	left (true, box.xmin(),-1);
	clipping_line	right(true, box.xmax(), 1);
	
	clipping_line	bottom(false,box.ymin(), 1);
	clipping_line	top(false,box.ymax(),-1);
	
	vector<vector<GP> >	temp;
	
	if(!clip_pwh(in_pwh, out_pwh_list, left))
		return false;

	if(!clip_pwh_list(out_pwh_list, temp, right))
		return false;

	if(!clip_pwh_list(temp, out_pwh_list, bottom))
		return false;

	if(!clip_pwh_list(out_pwh_list, temp, top))
		return false;
		
	swap(out_pwh_list,temp);
	return true;
}

// ------------------------------------------------------------------------------------------------------------------------
// PUBLIC POLYGON CLIPPING API
// ------------------------------------------------------------------------------------------------------------------------

// The actual polygon clipper is templated - here we instantiate the template for the 4 types of polygons we know about.

// These utilities convert from a polygon to a general-polygon and back.  The actual clipper utils only work on 
// general-polygons.  (In other words, we have to go from point-sequence to segment representation.  It is too ugly
// trying to template the clipper without having a concrete curve container.)

void	decompose_polygon(const Polygon2& in_poly,vector<Segment2>& out_poly)
{
	out_poly.clear();
	out_poly.insert(out_poly.end(),in_poly.sides_begin(),in_poly.sides_end());
}

void	decompose_polygon_with_holes(const vector<Polygon2>& in_poly,vector<vector<Segment2> >& out_poly)
{
	out_poly.clear();
	out_poly.resize(in_poly.size());
	for(int i = 0; i < in_poly.size(); ++i)
		decompose_polygon(in_poly[i],out_poly[i]);
}

bool recompose_polygon(const vector<Segment2>& in_poly, Polygon2& out_poly)
{
	if(!validate_poly_closed(in_poly.begin(),in_poly.end()))
	{
		#if DEV
			printf("We could not reconstruct our polygon - general polygon is not well-linked.\n");
		#endif
		return false;
	}
	out_poly.resize(in_poly.size());
	for(int i = 0; i < in_poly.size(); ++i)
	{
		out_poly[i] = in_poly[i].p1;
	}
	return true;
}

bool recompose_polygon_with_holes(const vector<vector<Segment2> > & in_poly, vector<Polygon2>& out_poly)
{
	out_poly.resize(in_poly.size());
	for(int i = 0; i < in_poly.size(); ++i)
		if (!recompose_polygon(in_poly[i], out_poly[i]))
			return false;
	return true;
}

// Unlike the other clippers, the polygon2 clipper has to convert from polygon to general-polygon format,
// clip, then put the world together again.

bool clip_polygon(const vector<Polygon2>& in_pwh, vector<vector<Polygon2> >& out_pwh_list, const Bbox2& box)
{
	Bbox2	bounds;
	bbox_for_any_vector2(in_pwh,bounds);
	if(box.contains(bounds))
	{
		out_pwh_list.push_back(in_pwh);
		return true;
	}

	vector<vector<Segment2> > pwh;
	vector<vector<vector<Segment2> > > pwh_list;
	
	decompose_polygon_with_holes(in_pwh, pwh);
		
	if(!clip_general_polygon(pwh,pwh_list,box))
		return false;
		
	out_pwh_list.resize(pwh_list.size());
	for(int i = 0; i < pwh_list.size(); ++i)
		if(!recompose_polygon_with_holes(pwh_list[i], out_pwh_list[i]))
			return false;
	
	return true;
}

bool clip_polygon(const vector<Polygon2p>& in_pwh, vector<vector<Polygon2p> >& out_pwh_list, const Bbox2& box)
{
	return clip_general_polygon(in_pwh,out_pwh_list,box);
}

bool	clip_polygon(const vector<BezierPolygon2>& in_pwh, vector<vector<BezierPolygon2> >& out_pwh_list, const Bbox2& box)
{
	return clip_general_polygon(in_pwh,out_pwh_list,box);
}

bool	clip_polygon(const vector<BezierPolygon2p>& in_pwh, vector<vector<BezierPolygon2p> >& out_pwh_list, const Bbox2& box)
{
	return clip_general_polygon(in_pwh,out_pwh_list,box);
}
