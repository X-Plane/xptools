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

