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

#include "BezierApprox.h"
#include "AssertUtils.h"
#include "MathUtils.h"
#include "GISTool_Globals.h"
#include "XESConstants.h"
#include "PlatformUtils.h"

#define DEBUG_CURVE_FIT_TRIALS 0
#define DEBUG_CURVE_FIT_SOLUTION 0
#define DEBUG_MERGE 0
#define DEBUG_START_END 0

inline bool ray_intersect(const Point2& p1, const Vector2& v1, const Point2& p2, const Vector2& v2, double& t1, double& t2)
{
	double det = v2.dx * v1.dy - v2.dy * v1.dx;
	if(det < 0.0) 
		return false;

	double dx = p2.x() - p1.x();
	double dy = p2.y() - p1.y();
	t1 = (dy * v2.dx - dx * v2.dy) / det;
	t2 = (dy * v1.dx - dx * v1.dy) / det;
	return true;
}

template<typename __Iter>
void visualize_bezier_seq(__Iter first, __Iter last, int r, int g, int b)
{
	while(first != last)
	{
		debug_mesh_point(*first,r,g,b);
		DebugAssert(!first->c);
		__Iter stop = first;
		++stop;
		while(stop->c) ++stop;
		
		int d = distance(first,stop);
		switch(d) {
		case 1:
			debug_mesh_line(*first, *stop, r,g,b,r,g,b);
			break;
		case 2:
			debug_mesh_bezier(*first, *nth_from(first,1), *nth_from(first,2), r,g,b,r,g,b);
			break;
		case 3: 
			debug_mesh_bezier(*first, *nth_from(first,1), *nth_from(first,2),*nth_from(first,3), r,g,b,r,g,b);
			break;
		}
		first = stop;
		
	}
	debug_mesh_point(*last,r,g,b);
	
}

template <class __Seq>
double squared_distance_pt_seq(__Seq seq, const Point2& p)
{
	if(seq()) return 0.0;
	Segment2 s;
	s.p1 = *seq;
	++seq;
	if(seq()) return 0.0;
	s.p2 = *seq;
	++seq;
		
	double worst = s.squared_distance(p);
	while(!seq())
	{
		s.p1 = s.p2;
		s.p2 = *seq;
		++seq;
		worst = min(worst,s.squared_distance(p));		
	}
	return worst;
}

template <class __Seq1, class __Seq2>
double squared_distance_seq_seq(__Seq1 s1, __Seq2 s2)
{
	double t = 0.0;
	double v = 0.0;
	while(!s1())
	{
		Point2 p = *s1;
		++s1;
//		worst = max(worst, squared_distance_pt_seq<__Seq2>(s2, p));
		v += squared_distance_pt_seq<__Seq2>(s2, p);
		++t;
	}
	return sqrt(v) / t;
}


template <class __Seq>
struct bezier_approx_seq {

	bezier_approx_seq(__Seq in_seq, bool in_want_last) : t(0.0), want_last(in_want_last), s(in_seq), done(false)
	{
		b.p2 = *s;
		++s;	
		if(s())
		{
			b.c1 = b.c2 = b.p1 = b.p2;
			done = true;
			t = 0.0;
		}
		else
			advance();
	}

	Point2 operator*()
	{
		return b.midpoint(t);
	}
	
	bool operator()(void)
	{
		if(!done) return false;
		if(!s())	  return false;
		if(want_last) return false;
		return true;
	}
	bezier_approx_seq& operator++(void)
	{		
		if(done && want_last)		// We're done, but last point isn't consumed?  Mark as consumed
			want_last = false;
		else if (t < 1.0)				// else we have a bezier....advance the T if possible
		{
			t += 0.125;
		}
		else						// else go to next in pt seq
		{
			if(s())					// Note: if sequence is tapped out, go to "done" state!
			{
				b.c1 = b.c2 = b.p1 = b.p2;
				done = true;
				t = 0.0;
			}
			else
				advance();
		}
		return *this;
	}

private:

	void advance(void)
	{
		Point2c np1 = *s;
		++s;
		if(np1.c)
		{
			Point2c np2 = *s;
			++s;
			if(np2.c)
			{
				Point2c np3 = *s;
				++s;
				b = Bezier2(b.p2,np1,np2,np3);
			}
			else
			{
				b = Bezier2(b.p2,np1,np2);
			}
		}
		else
		{
			b.p1 = b.p2;
			b.p2 = np1;
			b.c1 = b.p1;
			b.c2 = b.p2;
		}
		
		t = 0.0;
	}


	Bezier2		b;
	double		t;
	__Seq		s;

	bool		want_last;
	bool		done;			// This indicates that we have already emitted the last real curve
								// of the bezier and we are sitting "on" the end-point.

};

template <class Iter>
struct seq_for_container {

	Iter begin;
	Iter end;
	seq_for_container(Iter b, Iter e) : begin(b), end(e) { }
	
	bool operator()(void) { return begin == end; }
	Point2c operator*() { return *begin; }
	seq_for_container& operator++(void) { ++begin; return *this; }
};

template <typename S1, typename S2>
struct seq_concat {
	S1	s1;
	S2	s2;
	seq_concat(const S1& is1, const S2& is2) : s1(is1), s2(is2) { }
	bool operator()(void) { return s1() && s2(); }
	Point2c operator*() { return s1() ? *s2 : *s1; }
	seq_concat& operator++(void) { if(s1()) ++s2; else ++s1; return *this; } 
};

typedef list<Point2c>	bez_list;

template <typename T1, typename T2>
double error_for_approx(T1 s1_begin, T1 s1_end, T2 s2_begin, T2 s2_end)
{
	typedef seq_for_container<T1>	T1S;
	typedef seq_for_container<T2>	T2S;
	typedef bezier_approx_seq<T1S>	T1AS;
	typedef bezier_approx_seq<T2S>	T2AS;
		
	T1S t1s(s1_begin,s1_end);
	T2S	t2s(s2_begin,s2_end);
	T1AS	t1as(t1s,true);
	T2AS	t2as(t2s,true);

		

	double err = squared_distance_seq_seq(t1as, t2as);
	return err;

//	typedef bezier_approx_seq<T1S>	APVIS;
//	typedef bezier_approx_seq<T2RS> APVRIS;
//
//	APVIS apvis(t1s,true);
//	APVRIS apvris(t2rs,true);
//
//	float s = 0.0;
//	while(!apvis())
//	{
//		debug_mesh_point(*apvis,1,1,s - floorf(s));
//		++apvis;
//		s += 0.06125;
//	}
//	s = 0.0;
//	while(!apvris())
//	{
//		debug_mesh_point(*apvris,1,0,s);
//		++apvris;
//		s += 0.06125;
//	}
//	
//	return signed_area_pt_from_seq(my_seq);
}

double best_bezier_approx(
					list<Point2c>::iterator orig_first,
					list<Point2c>::iterator orig_last,
					Point2c			approx[4],
					double&			 t1_best,
					double&			 t2_best,
					double			 frac_ratio,
					int				 step_start,
					int				 step_stop)
{
	DebugAssert(orig_last != orig_first);
	list<Point2c>::iterator orig_c1(orig_first); ++orig_c1;
	DebugAssert(orig_c1 != orig_last);
	list<Point2c>::iterator orig_c2(orig_last); --orig_c2;
	DebugAssert(orig_c2 != orig_first);
	DebugAssert(!orig_first->c);
	DebugAssert(!orig_last->c);
	DebugAssert(orig_c1->c);
	DebugAssert(orig_c2->c);
	list<Point2c>::iterator orig_end(orig_last);
	++orig_end;
		
	DebugAssert(*orig_first != *orig_c1);	
	DebugAssert(*orig_last != *orig_c2);	
	
	Vector2	c1v(approx[0],approx[1]);
	Vector2	c2v(approx[3],approx[2]);
		
//	TODO
	
	double err = 0.0;
	bool has_err = false;
		
	Point2c	this_approx[4];
	
	double t1_orig(t1_best);
	double t2_orig(t2_best);
	
	for(int s1 = step_start; s1 <= step_stop; ++s1)	
	for(int s2 = step_start; s2 <= step_stop; ++s2)	
	{
		double t1 = t1_orig * pow(frac_ratio, s1);
		double t2 = t2_orig * pow(frac_ratio, s2);
//		double t1 = double_interp(0,t1_lo,steps-1,t1_hi,s1);
//		double t2 = double_interp(0,t2_lo,steps-1,t2_hi,s2);
		this_approx[0] = *orig_first;
		this_approx[1] = Point2c(*orig_first + c1v * t1,true);
		this_approx[2] = Point2c(*orig_last + c2v * t2,true);
		this_approx[3] = *orig_last;		

		double my_err = fabs(error_for_approx<list<Point2c>::iterator,Point2c*>(orig_first,orig_end, this_approx,this_approx+4));

		#if DEBUG_CURVE_FIT_TRIALS
		gMeshBeziers.clear();
		gMeshPoints.clear();

		visualize_bezier_seq(orig_first, orig_last,0,1,0);
		if(!has_err || my_err < err)
			visualize_bezier_seq(this_approx,this_approx+3, 1,1,0);
		else {
			visualize_bezier_seq(this_approx,this_approx+3, 1,0,0);
			visualize_bezier_seq(approx,approx+3, 1,1,0);
		}
		debug_mesh_point(this_approx[1],1,0,0);
		debug_mesh_point(this_approx[2],0,1,0);

		printf("trial with t1=%lf, t2=%lf, err=%lf\n", t1, t2, my_err*DEG_TO_MTR_LAT);
		DoUserAlert("Trial");
		#endif
		if(!has_err || my_err < err)
		{
			has_err = true;
			err = my_err;
			approx[0] = this_approx[0];
			approx[1] = this_approx[1];
			approx[2] = this_approx[2];
			approx[3] = this_approx[3];
			t1_best = t1;
			t2_best = t2;
		}
	}
	
	#if DEBUG_CURVE_FIT_SOLUTION
	
	gMeshBeziers.clear();
	gMeshPoints.clear();
	visualize_bezier_seq(orig_first, orig_last,0,1,0);
	debug_mesh_point(approx[1],1,0,0);
	debug_mesh_point(approx[2],0,1,0);
	debug_mesh_bezier(approx[0],approx[1],approx[2],approx[3],1,1,1, 1,1,1);
	printf("Best err: %lf with t1=%lf,t2=%lf\n",err*DEG_TO_MTR_LAT, t1_best,t2_best);
	DoUserAlert("Best");
	#endif
	return err;
}

struct possible_approx_t;

struct	approx_t {
	approx_t *				prev;			// Links to prev/next approx in our chain
	approx_t *				next;
	bez_list::iterator		orig_first;		// List iterator to original span of nodes, INCLUSIVE! REALLY!
	bez_list::iterator		orig_last;
	Point2c					approx[4];		// Four bezier nodes approximate the curve

	possible_approx_t *		merge_left;		// ptr into future approximations, so that if
	possible_approx_t *		merge_right;	// we are merged out, we can "find" ourselves.
};

typedef multimap<double, possible_approx_t *>	possible_approx_q;

struct	possible_approx_t {
	approx_t *					left;
	approx_t *					right;
	Point2c						approx[4];
	double						err;
	possible_approx_q::iterator	self;
};

void setup_approx(approx_t * l, approx_t * r, possible_approx_t * who, possible_approx_q * q)
{
	DebugAssert(l->next == r);
	DebugAssert(r->prev == l);
	l->merge_right = who;
	r->merge_left = who;
	who->left = l;
	who->right = r;
	
	double t1 = 1.0, t2 = 1.0;
	who->approx[0] = l->approx[0];
	who->approx[1] = l->approx[1];
	who->approx[2] = r->approx[2];
	who->approx[3] = r->approx[3];
	
	who->err = best_bezier_approx(l->orig_first, r->orig_last, who->approx, 
						t1, t2, 2.0, -1, 3);

	// We have to RESET the approx - our big win is that our t1/t2 are now LOOSELY calibrated.
	// So the bezier approx must start over or we double-apply the approx.
	who->approx[0] = l->approx[0];
	who->approx[1] = l->approx[1];
	who->approx[2] = r->approx[2];
	who->approx[3] = r->approx[3];

	who->err = best_bezier_approx(l->orig_first, r->orig_last, who->approx, 
						t1, t2, 1.22, -2, 2);

	if(q)
		who->self = q->insert(possible_approx_q::value_type(who->err, who));
}


// Apply the merge described by "who" - when done, the right edge (and who)
// are gone, and the new edge is returned.
approx_t * merge_approx(possible_approx_t * who, possible_approx_q * q)
{
	approx_t * l = who->left;
	approx_t * r = who->right;
	DebugAssert(l->next == r);
	DebugAssert(r->prev == l);
	DebugAssert(l->merge_right == who);
	DebugAssert(r->merge_left == who);
	
	l->next = r->next;
	if(l->next)
		l->next->prev = l;
	l->orig_last = r->orig_last;
	l->merge_right = r->merge_right;
	if(l->merge_right)
		l->merge_right->left = l;
	
	for(int n = 0; n < 4; ++n)
		l->approx[n] = who->approx[n];
	
	if(q && who->self != q->end())
		q->erase(who->self);
	
	delete r;
	delete who;

	if(l->merge_left)
	{
		DebugAssert(l->prev);
		if(q && l->merge_left->self != q->end())
			q->erase(l->merge_left->self);		
		setup_approx(l->prev, l, l->merge_left, q);
			
	}
	if(l->merge_right)
	{
		DebugAssert(l->next);
		if(q && l->merge_right->self != q->end())
			q->erase(l->merge_right->self);		
		setup_approx(l, l->next, l->merge_right, q);
	}
	
	#if DEBUG_MERGE
	gMeshBeziers.clear();
	gMeshPoints.clear();
	visualize_bezier_seq(l->orig_first,l->orig_last,0,1,0);
	visualize_bezier_seq(l->approx,l->approx+3,1,1,0);
	DoUserAlert("Merged");
	#endif
	
	return l;
	
	
}

void bezier_multi_simplify(
					list<Point2c>::iterator	first,
					list<Point2c>::iterator	last,
					list<Point2c>&			simplified,
					double					max_err)
{
	#if DEBUG_START_END
	gMeshBeziers.clear();
	gMeshPoints.clear();
	visualize_bezier_seq(first, last, 0,1,0);
	DoUserAlert("Will simplify this curve.");
	#endif
	approx_t *				orig = NULL;
	possible_approx_q 		q;
	list<Point2c>::iterator	start, stop;
	/* STEP 1 - build an approx list for each bezier curve in the original sequence. */

	start = first;
	approx_t * prev = NULL;
	DebugAssert(!last->c);
	while(start != last)
	{
		DebugAssert(!start->c);
		stop = nth_from(start,1);
		while(stop->c) ++stop;
		
		approx_t * seg = new approx_t;
		seg->prev = prev;
		if(prev) prev->next = seg;
		else orig = seg;
		seg->next = NULL;
		seg->orig_first = start;
		seg->orig_last = stop;
		seg->merge_left = seg->merge_right = NULL;
		
		int dist = distance(start,stop);
		DebugAssert(dist > 1);
		DebugAssert(dist < 4);
		
		if(dist == 2)
		{
			Bezier2 app(*start, *nth_from(start,1),*stop);
			seg->approx[0] = app.p1;
			seg->approx[1] = app.c1;
			seg->approx[2] = app.c2;
			seg->approx[3] = app.p2;
		}
		else if(dist == 3)
		{
			seg->approx[0] = *start;
			seg->approx[1] = *nth_from(start,1);
			seg->approx[2] = *nth_from(stop,-1);
			seg->approx[3] = *stop;
		}
		start = stop;
		prev = seg;
	}
	
	/* Step 2 - build a possible approx for each adjacent PAIR of approximations. */
	
	approx_t * seg;
	DebugAssert(orig);
	for(seg = orig; seg->next; seg = seg->next)
	{
		possible_approx_t * app = new possible_approx_t;
		setup_approx(seg, seg->next, app, &q);		
	}
	
	/* Step 3 - run the Q to do the actual merges. */
	while(!q.empty())
	{
		if(q.begin()->first > max_err)	
			break;

		possible_approx_t * who = q.begin()->second;
		merge_approx(who, &q);						
	}
	
	/* Step 4 - output the final approxiamte sequence! */
	while(!q.empty())
	{
		delete q.begin()->second;
		q.erase(q.begin());
	}

	simplified.clear();
	while(orig)
	{
		simplified.insert(simplified.end(),orig->approx, orig->approx+3);
		if(orig->next == NULL)
			simplified.push_back(orig->approx[3]);
		approx_t * k = orig;
		orig = orig->next;
		delete k;
	}
	
	#if DEBUG_START_END
	gMeshBeziers.clear();
	gMeshPoints.clear();
	visualize_bezier_seq(first, last, 0,1,0);
	visualize_bezier_seq(simplified.begin(),nth_from(simplified.end(),-1),1,1,1);
	DoUserAlert("End result.");
	#endif
	
}

void bezier_multi_simplify_straight_ok(
					list<Point2c>&		seq,
					double				max_err)
{
	list<Point2c>::iterator start(seq.begin()), stop, last(seq.end());
	--last;

	DebugAssert(!last->c);
	while(start != last)
	{
		DebugAssert(!start->c);
		stop = start;
		int ctr = 1;
		int curves = 1;
		++stop;
		if(stop->c)
		{
			while(stop->c) ++stop, ++ctr;
			while(stop != last && nth_from(stop,1)->c)
			{
				DebugAssert(!stop->c);
				++stop;
				++curves;
				while(stop->c) ++stop, ++ctr;
			}
			DebugAssert(!stop->c);
			DebugAssert(stop == last || !nth_from(stop,1)->c);
			
			if(curves > 1)
			{
				list<Point2c>	better;
				bezier_multi_simplify(start,stop,better,max_err);
				
				if(ctr >= better.size())				// old has to be SMALLER since it isn't counting its end node!
				{
					seq.erase(nth_from(start,1),stop);	// erase between start & stop
					seq.splice(nth_from(start,1),better,nth_from(better.begin(),1),nth_from(better.end(),-1));
				}
			}
		}
		start = stop;
	}
}
