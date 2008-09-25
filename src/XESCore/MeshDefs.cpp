/*
 * Copyright (c) 2004, Laminar Research.
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

#include "MeshDefs.h"
#include "WED_Globals.h"

/*
 * CACHED LOCATES - THEORY OF OPERATION
 *
 * CGAL's triangulation locate routine is march-based, e.g. from a starting point
 * we keep jumping to the triangle next to us that gets us closer.  This means it's
 * linear to the number of triangles we cross...real good for localized searches and
 * real bad for random searches.
 *
 * Therefore any code with locality gains a huge advantage by 'hinting' its last
 * triangle.  But...calling code has to use a static face handle.  If the next
 * map we get isn't the same map, hinting with a face from a different map will hose
 * the program.
 *
 * This system partially alleviates this:
 * (1) It allows "hints", or the last found triangle, to be automatically cached
 *     inside the map.  This means that given multiple maps, we always get back
 *	   the hint for this map.
 * (2) It allows for global once-used int "cache keys".  A given part of the program
 *     grabs one once the first time it runs and that way its slot is protected.
 *
 * This code does not address the problem of the mesh being emptied out and the face
 * being no longer valid.
 *
 */
int CDT::sKeyGen = 1;

int	CDT::gen_cache_key(void)
{
	return sKeyGen++;
}

CDT::Face_handle CDT::locate_cache(const Point& p, Locate_type& lt, int& li, int cache_key) const
{
	HintMap::iterator i = mHintMap.find(cache_key);
	CDT::Face_handle	hint = NULL;
	if (i != mHintMap.end())
		hint = i->second;

	hint = locate(p, lt, li, hint);

	if (i != mHintMap.end())
		i->second = hint;
	else
		mHintMap.insert(HintMap::value_type(cache_key, hint));

	return hint;
}

void CDT::cache_reset(void)
{
	mHintMap.clear();
}

void CDT::clear(void)
{
	cache_reset();
	CDTBase::clear();
}

inline int sign_of(double x) { return x > 0.0 ? 1 : (x < 0.0 ? -1 : 0); }


#if DEV
CDT::Vertex_handle	CDT::safe_insert(const Point& p, Face_handle hint)
{
	int			li;
	Locate_type	lt;
	Face_handle	who = locate(p, lt, li, hint);
	if (lt == FACE && oriented_side(who, p) != CGAL::ON_POSITIVE_SIDE)
	{
		if(lt == FACE && oriented_side(who, p) != CGAL::ON_POSITIVE_SIDE)
		{

			Point	p0(who->vertex(0)->point());
			Point	p1(who->vertex(1)->point());
			Point	p2(who->vertex(2)->point());

			CGAL_triangulation_precondition( orientation(p0, p1, p2) != CGAL::COLLINEAR);


			CGAL::Orientation 	o2 = orientation(p0, p1, p),
								o0 = orientation(p1, p2, p),
								o1 = orientation(p2, p0, p),
								o2b= orientation(p1, p0, p),
								o0b= orientation(p2, p1, p),
								o1b= orientation(p0, p2, p);


//			if (o1 == CGAL::COLLINEAR && collinear_between(p0, p, p2)) { li = 1; lt = EDGE; }
//			if (o2 == CGAL::COLLINEAR && collinear_between(p1, p, p0)) { li = 2; lt = EDGE; }
//			if (o0 == CGAL::COLLINEAR && collinear_between(p2, p, p1)) { li = 0; lt = EDGE; }

			// Collinear witih TWO sides?  Hrmm...should be a vertex.
			if (o1 == CGAL::COLLINEAR && o2 == CGAL::COLLINEAR) { li = 0; lt = VERTEX; }
			if (o2 == CGAL::COLLINEAR && o0 == CGAL::COLLINEAR) { li = 1; lt = VERTEX; }
			if (o0 == CGAL::COLLINEAR && o1 == CGAL::COLLINEAR) { li = 2; lt = VERTEX; }

			// Colinear with a side and positive on the other two - should be on that edge.
			if (o1 == CGAL::COLLINEAR && o2 == CGAL::POSITIVE && o0 == CGAL::POSITIVE) 				{ li = 1; lt = EDGE; }
			if (o2 == CGAL::COLLINEAR && o0 == CGAL::POSITIVE && o1 == CGAL::POSITIVE) 				{ li = 2; lt = EDGE; }
			if (o0 == CGAL::COLLINEAR && o1 == CGAL::POSITIVE && o2 == CGAL::POSITIVE) 				{ li = 0; lt = EDGE; }

			// On negative of a side AND its opposite?  We've got a rounding error.  Call it the edge and go home.
			if (o0 == CGAL::NEGATIVE && o0b == CGAL::NEGATIVE) { li = 0; lt = EDGE; }
			if (o1 == CGAL::NEGATIVE && o1b == CGAL::NEGATIVE) { li = 1; lt = EDGE; }
			if (o2 == CGAL::NEGATIVE && o2b == CGAL::NEGATIVE) { li = 2; lt = EDGE; }

/*
			if (o0 == CGAL::NEGATIVE && o1 == CGAL::POSITIVE && o2 == CGAL::POSITIVE && o0b == CGAL::POSITIVE)
			{
				lt = FACE;
//				li = ccw(who->neighbor(0)->index(who->vertex(ccw(0))));
				who = who->neighbor(0);
			}
			if (o1 == CGAL::NEGATIVE && o2 == CGAL::POSITIVE && o0 == CGAL::POSITIVE && o1b == CGAL::POSITIVE)
			{
				lt = FACE;
//				li = ccw(who->neighbor(1)->index(who->vertex(ccw(1))));
				who = who->neighbor(1);
			}
			if (o2 == CGAL::NEGATIVE && o0 == CGAL::POSITIVE && o1 == CGAL::POSITIVE && o2b == CGAL::POSITIVE)
			{
				lt = FACE;
//				li = ccw(who->neighbor(2)->index(who->vertex(ccw(2))));
				who = who->neighbor(2);
			}
*/
			if (lt == FACE && oriented_side(who, p) != CGAL::ON_POSITIVE_SIDE)
			{
				Point	p0_(who->vertex(0)->point());
				Point	p1_(who->vertex(1)->point());
				Point	p2_(who->vertex(2)->point());

				CGAL_triangulation_precondition( orientation(p0, p1, p2) != CGAL::COLLINEAR);


				CGAL::Orientation 	_o2 = orientation(p0_, p1_, p),
									_o0 = orientation(p1_, p2_, p),
									_o1 = orientation(p2_, p0_, p);
/*
				gMeshPoints.push_back(pair<Point2,Point3>(Point2(p.x(),p.y()),Point3(1,1,1)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p0.x(),p0.y()),Point3(0,1,1)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p1.x(),p1.y()),Point3(0,1,1)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p1.x(),p1.y()),Point3(0,1,1)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p2.x(),p2.y()),Point3(0,1,1)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p2.x(),p2.y()),Point3(0,1,1)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p0.x(),p0.y()),Point3(0,1,1)));

				gMeshLines.push_back(pair<Point2,Point3>(Point2(p0_.x(),p0_.y()),Point3(0,1,0)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p1_.x(),p1_.y()),Point3(0,1,0)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p1_.x(),p1_.y()),Point3(0,1,0)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p2_.x(),p2_.y()),Point3(0,1,0)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p2_.x(),p2_.y()),Point3(0,1,0)));
				gMeshLines.push_back(pair<Point2,Point3>(Point2(p0_.x(),p0_.y()),Point3(0,1,0)));
*/
				AssertPrintf("Unable to resolve bad locate.");
			}
		}
	}
	return CDTBase::insert(p, lt, who, li);
}
#endif