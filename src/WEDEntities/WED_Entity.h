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

#ifndef WED_ENTITY_H
#define WED_ENTITY_H

/*
	WED_Entity - THEROY OF OPERATION

	WED_Entity provides the implementation base for all spatial WED_thigns, that is, everything you can see on the map.
	Generally:

	Any final derivative of WED_Entity can be caste to IGISEntity.  (But note that since WED_Entity itself doesn't
	do this, this support is not automatic.)

	All children of WED_Entities are WED_Entities, that is, we don't have random stuff jammed into the map.

	This class provides the "locked" and "hidden" property provided to anything on the map.

	CACHING

	Because GIS entities are typically defined in a hierarchial tree structure, a straight implementation of things
	like bounding boxes can be extraordinarily slow.   (For example, a simple depth-first search with bounding box
	on N items would produce approximately NlogN or N^2 accesses since the parent will access all of the children when
	asked for their bounding box.

	Since we could have logN performance with good culling primitives, all WED entity derivatives have a cache management
	feature.  It works as follows:

	- The cache is automatically invalidated under some circumstances:

		1. A child is added or removed from the object.
		2. The object is read in from DB or undo-memory (since we can't know what changed).
		3. Any child or child's child's cache is invalidated (that is, cache invalidation goes up-stream.

	- The cache is rebuilt on demand by expensive-access routines.  Typically these include "GetBounds" and expensive
	  iterators over children.  CacheRebuild is called to mark it as good and find out if real work must be done.

	CORRECT CACHING BEHAVIORS:

	- Classes that use a cache should invalidate it if their internal state changes in a way that would change cached data,
	  and force a rebuild any time it is accessed.  Exmaple: GIS Chain

	- Classes that do not cache but pas through conventionally cached data should rebuild their caches (a no-op but call
	  BuildCache()) so that the next inval is passed to all parents.  Example: GIS Line-Width.

	- Classes that do not cache but affect others should invalidate and immediately revalidate their caches (the second
	  so that future invals are passed up).  Example: GIS Points.

	CACHE AND UNDO

	One might note that at the time of redo, the object's parent ID is invalid, thus the cache invalidate on redo might not
	travel up the chain.  However this is a non-issue:

	- If an object X was deleted as part of an operation, we know that all of X's children were modified (to set their parent
	  ptr to something else) and X's parent was modified (to remove X from its children).

	- Therefore logically, any objects X' above X that might not get invalidated (because X's child was invalidated BEFORE X was
	  reincarnated via undo) must have been modified, and thus X' will be "undone" as well and will thus be marked invalid
	  directly.

*/

#include "WED_Thing.h"

class	WED_Entity : public WED_Thing {

DECLARE_INTERMEDIATE(WED_Entity)

public:

			int		GetLocked(void) const;
			int		GetHidden(void) const;

	virtual	void 	ReadFrom(IOReader * reader);
	virtual void	FromDB(sqlite3 * db, const map<int,int>& mapping);

protected:

			void	CacheInval(void);			// Invalidate the cache.
			bool	CacheBuild(void) const;		// Set cache to valid.  Returns true if cache needed rebuilding

	virtual	void	AddChild(int id, int n);
	virtual	void	RemoveChild(int id);

private:

	mutable bool				cache_valid;

	WED_PropBoolText			locked;
	WED_PropBoolText			hidden;

};


#endif /* WED_ENTITY_H */
