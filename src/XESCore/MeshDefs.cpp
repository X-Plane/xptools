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

void 
CDT::my_propagating_flip(Face_handle& f,int i, set<Face_handle>& all)
// similar to the corresponding function in Delaunay_triangulation_2.h 
{ 
  if (!is_flipable(f,i)) return;
  Face_handle ni = f->neighbor(i); 
  flip(f, i); // flip for constrained triangulations
  all.insert(f);
  all.insert(ni);
  my_propagating_flip(f,i, all); 
  i = ni->index(f->vertex(i)); 
  my_propagating_flip(ni,i, all); 
} 


// The standard CGAL delauney triangulation doesn't have a way to collect the set of faces affected by an insert
// operation; this set can be larger than just the neighboring triangles, because the flip recursively spans out in
// all directions.  So this is a simple re-paste of the CGAL insert code, except it collects all of the affected
// faces so we can reprocess them if desired.
//
// One note: unlike regular insert this assumes that the 'hint' face is actually a face that OWNS p.  So not only do
// we assert that (1) our locate is sane and (2) we're on or in the tri but we also check at the end that the face
// we inserted into was affected by the insert.  If it wasn't, we'd have a logic error in our collection of flipped faces.
CDT::Vertex_handle	CDT::insert_collect_flips(const Point& p, Face_handle hint, set<Face_handle>& all)
{
	Face_handle loc;
	int li;
	Locate_type lt;


	loc = Ctr::locate(p, lt, li, hint);
	DebugAssert(Ctr::oriented_side(hint, p) != CGAL::ON_NEGATIVE_SIDE);
	DebugAssert(lt == CDT::FACE || lt == CDT::EDGE);
	Vertex_handle va= Ctr::insert(p,lt,loc,li);
	if (dimension() > 1)
	{
		Face_handle f=va->face();
		Face_handle next;    
		Face_handle start(f);
		int i;
		do {
			all.insert(f);
			i = f->index(va); // FRAGILE : DIM 1
			next = f->neighbor(ccw(i));  // turns ccw around a
			my_propagating_flip(f,i, all);
			f=next;
		} while(next != start);
	}

	DebugAssert(all.count(hint) > 0);


  return va;
}

/*
static char * print_edge(CDT::Edge& e)
{
	static char s[256];
	if(e.first == CDT::Face_handle())
	sprintf(s,"<null>");
	else
	sprintf(s,"%p/%d (%p -> %p)",
		&*e.first,e.second,
		&*CDT_he_source(e),
		&*CDT_he_target(e));
	return s;
}

*/