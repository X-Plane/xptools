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
#ifndef XBUCKETS_H
#define XBUCKETS_H

#include <vector>

/***************************************************
	An X-Limerick

	There once was a sim from Nantucket
	Whose objects were stored in an XBucket
	It seemed smart at inception
	But when it threw an exception
	It made Austin call and go 'fuck it'.
***************************************************/


/*
	XBUCKETS - THEORY OF OPERATION
	
	XBuckets implements a nested set of buckets (e.g. 1x1, 2x2, etc.).
	They are useful for storing objects by their 2-d coordinates and
	then rapidly finding them.
	
	XBuckets is templated - you provide a traits object that defines
	your datatypes and then make an XBuckets for them.  So you
	can use XBuckest many times with different kinds of objects, 
	coordinates, and algorithms.  You must plug in a few basic
	algorithms to make XBuckets understand your basic objects.
	
	GEOMETRY NOTE:
	
	You can search for all objects within a rectangle.  All comparisons
	include the lower bound but not the upper bound.  For example, if
	you search for all objects in the area 0, 0 to 1, 1, a point-sized
	object at 0,0 will be returned, but a point-sized object at 1, 1
	will not!  When you search from 1,1 to 2,2 you then get the objet
	at 1,1 and not at 2,2.  So you always get each point once.
	
	Because of this, when you implement "ObjectTouchesRect" make sure
	you register if your left edge touches the left edge of the bounding
	rectangle, but not your right edge!
	
	LOOSE VS TIGHT BUCKETING
	
	X-Buckets has two bucketing methods:
	
	In the tight bucketing scheme, every object is within the smallest
	bucket that fully contains it (in a 2-d sense).  The advantage of 
	this is perfectly accurate point-hit and containment tests...there
	will never be an object in the space you search that is not returned.
	The disadvantage of this is speed; an object that falls on the crack
	between buckets will move to the next higher bucket; an object in the
	center of the screen will be in the zero bucket.  This can mean many
	more objects returned from a spatial search than is desired.
	
	In the loose bucketing scheme, every object is put within the smallest
	bucket that _could_ contain it if the object were translated in any
	way.  Of the buckets of that size, the one that contains the center of
	the object is chosen.  This guarantees that small objects will be
	in small buckets and the zero bucket will not contain a ton of far
	away geometry.  It also means that a search of an area may not return
	objects that are half a bucket away from the edge (where the bucket
	size is the one they're filed in).  Loose bucketing is useful when
	buckets are used for storage and another technique (such as XCull)
	is used to find the items.

*/

/*

	Traits pattern: to use XBucket, you must provide a 'traits' class.
	The traits class is a class that defines the datatypes for this bucket,
	and implements some algorithms.  The typedefs in the traits class
	define the types that you will use...you can use anything for these.

	You then use XBuckets with a traits type, e.g. XBuckets<mytraits> foo;

	The functions provide the implementation of common operations for the
	buckets.  Here are the traits:
	
	Point2 - This should be a two-dimensional point type.  The Point2
	from CompGeomDefs2d.h works nicely, but you can use anything you want.
	
	Scalar - some kind of floating point numeric type that will be used for
	all math.  xdob or double strongly recommended.
	
	Object - This is the kind of object that will be in the bucket.  These
	values will be copied, so it is strongly recommended that this be a 
	pointer type, e.g. a pointer to a struct.

class	Traits {
public:

	// Define these types with typedef to define what types your buckets
	// will use.

	typedef	Point2				Point2;
	typedef	double				Scalar;
	typedef	Object				Object;
	
	// These accessors turn scalar coordinates into Point2s and vice versa.
	// They should probably be inlined!
	
	static	Scalar	X(const Point2& p);
	static	Scalar	Y(const Point2& p);
	static	void	MakePoint(Scalar x, Scalar y, Point2& p);
	
	// This function returns the bounding box of a give object.
	// The first point its the lowerleft (southwset), the second is the
	// upper right (northeast).
	
	static	void	GetObjectBounds(Object o, Point2& p1, Point2& p2);
	
	// Touch functions...given rects and points, figure out if we're touching
	// the object.
	
	static	bool	ObjectTouchesPoint(Object, const Point2& p);
	static	bool	ObjectTouchesRect(Object o, const Point2& p1, const Point2& p2);
	static	bool	ObjectFullyInRect(Object o, const Point2& p1, const Point2& p2);
	
	// This function does cleanup on an object - useful to rapidly depopulate the buckets.
	static	void	DestroyObject(Object o);
};


*/

/*	
	BUCKET IDs
	
	Each bucket has a 32-bit ID.  This ID is specially coded to allow
	identification of the bucket's exact location from the ID and vice
	versa.  For use of XBuckets you usually don't need to care about
	IDs, but when working with XCull an XBuckets together, these IDs
	can be important.
	
	A bucket's level is the size of the grid, e.g. 0 = 1x1, 1 = 2x2, 2 = 4x4, etc.
	
	A bucket's index is the bucket number starting from the upper left.  A bucket's
	parent ist he bucket in the next smaller grid that fully contains it.  So
	a bucket a 5, 3 in the 8x8 grid's parent is bucket 2x1 in the 4x4 grid.
	
	A bucket's 4 children are the 4 buckets in the next bigger grid which it
	fully contains.  So in the 4x4 grid, bucket 2x1 has 4 children (in the 8x8)
	grid, those are: 4x2, 5x2, 4x3, 5x3.  They are iterated in that order.
	
*/	

typedef unsigned int XBucketID;

#pragma dont_inline off

inline unsigned int XBucket_GetLevel(XBucketID id);
inline unsigned int XBucket_GetIndex(XBucketID id);
inline XBucketID 	XBucket_MakeBucketID(unsigned int level, unsigned int index);
inline XBucketID 	XBucket_Compose(unsigned int level, unsigned int x, unsigned int y);
inline void			XBucket_Decompose(XBucketID id, unsigned int& level, unsigned int& x, unsigned int& y);
inline XBucketID 	XBucket_GetParent(XBucketID id);
inline XBucketID 	XBucket_GetChild(XBucketID id, unsigned int child);

#pragma dont_inline reset

/*
	XBuckets
	
	This is the actual buckets class.  When you create XBuckets, you must have a number of
	layers (at least 1), and two points indicating the bounds of the buckets.  You can
	change these later with the 'rest' function.
	
*/	

enum {
	bucket_Organize_Draw,
	bucket_Organize_Test
};

template <typename __Object, typename __Traits>
class XBuckets {
public:

	// These typedefs are to make the C++ more readable.

	typedef __Traits					Traits;
	typedef typename Traits::Point2		Point2;
	typedef typename Traits::Scalar		Scalar;
	typedef typename Traits::Object		Object;

	// Create buckets, or reset the mtarix.

				XBuckets();
				XBuckets(unsigned int inLayers, const Point2& inMin, const Point2& inMax, int mode);
				~XBuckets();
	void		Reset(unsigned int inLayers, const Point2& inMin, const Point2& inMax, int mode);

	// These let you add and remove buckets.
	XBucketID	Insert(Object inObject);
	void		Remove(Object inObject);
	void		RemoveAllAndDestroy(void);
	void		Move(Object inObject, XBucketID inOldBucket);

	// Searching for items in an area.  This first set of functions lets you fill a vector
	// with the result.  The second set lets you pass in a function pointer in the form
	// of MyFunc(Object o, Ref ref) where Ref is any type you want.	
	void		FindTouchesPt(const Point2&, vector<Object>& outIDs) const;
	void		FindTouchesRect(const Point2&, const Point2&, vector<Object>& outIDs) const;
	void		FindFullyInRect(const Point2&, const Point2&, vector<Object>& outIDs) const;

	template <class __Ref>
	inline 	void		FindTouchesPt(const Point2&, void (*)(Object, __Ref), __Ref) const;
	template <class __Ref>
	inline 	void		FindTouchesRect(const Point2&, const Point2&, void (*)(Object, __Ref), __Ref) const;
	template <class __Ref>
	inline 	void		FindFullyInRect(const Point2&, const Point2&, void (*)(Object, __Ref), __Ref) const;

	// Utilities and other stuff using bucket IDs...you can find out which bucket
	// an object is in or all of the objects in that bucket.
	inline  XBucketID	FindBucket(Object inObject) const;
	void		GetOneBucket(XBucketID inBucket, vector<Object>& outIDs) const;
	inline	void		GetOneBucketUnsafe(XBucketID inBucket, Object ** ioBegin, Object ** ioEnd);
	inline	void		GetBucketDimensions(XBucketID inBucket, Point2&, Point2&) const;
	inline	int			GetLayerCount(void) const { return mItems.size(); } 
	inline	bool		Empty(void) const;

	template <class __Ref>
	void		GetOneBucket(XBucketID inBucket, void (*)(Object, __Ref), __Ref) const;
	
	template <class __Ref1, class __Ref2>
	inline	void		IterateBucketLayer(int layer, void (*)(XBucketID, __Ref1, __Ref2), __Ref1, __Ref2) const;
	
	// Debug output	
	void		Dump(void);
	
private:

	static void	InsertIntoVector(Object, vector<Object> *);
	void		GetIndicesForLevel(
							const Point2&		p1, 
							const Point2&		p2,
							unsigned int 		level,
							unsigned int&		x1,
							unsigned int&		y1,
							unsigned int&		x2,
							unsigned int&		y2) const;

	typedef	vector<Object>									ObjectVector;

#if __GNUC__
	typedef	hash_map<XBucketID,  ObjectVector>	BucketMap;
#endif
#if __MWERKS__
	typedef	Metrowerks::hash_map<XBucketID, ObjectVector>	BucketMap;
#endif
	
		int						mMode;
		vector<BucketMap>		mItems;
		Point2					mMin;
		Point2					mMax;
		Point2					mSize;
		int						mTotal;
	
};


// These functions are non-templeated inline functions.  They are
// in the header because we really really want them to be fast...
// they're just bit twiddling.

#pragma dont_inline off

inline unsigned int XBucket_GetLevel(XBucketID id) 								{ return id >> 24; }
inline unsigned int XBucket_GetIndex(XBucketID id)								{ return id & 0x00ffffff; }
inline XBucketID XBucket_MakeBucketID(unsigned int level, unsigned int index)	{ return (level << 24) | index; }
inline XBucketID XBucket_Compose(unsigned int level, unsigned int x, unsigned int y)
{
	return (level << 24) | (x + y * (1 << level));
}

inline void		XBucket_Decompose(XBucketID id, unsigned int& level, unsigned int& x, unsigned int& y)
{
	level = id >> 24;
	unsigned int index = id & 0x00ffffff;
	unsigned int mask = (1 << level) - 1;
	x = index & mask;
	y = index >> level;	
}

inline XBucketID	XBucket_GetParent(XBucketID id)
{
	unsigned int level, x, y;
	level = id >> 24;
	unsigned int index = id & 0x00ffffff;
	unsigned int mask = (1 << level) - 1;
	x = index & mask;
	y = index >> level;	
	if (level == 0) return id;
	x >>= 1;
	y >>= 1;
	level--;
	return (level << 24) | (x + y * (1 << level));
}

inline XBucketID	XBucket_GetChild(XBucketID id, unsigned int child)
{
	unsigned int level, x, y;
	level = id >> 24;
	unsigned int index = id & 0x00ffffff;
	unsigned int mask = (1 << level) - 1;
	x = index & mask;
	y = index >> level;	
	x <<= 1;
	y <<= 1;
	x |= (child & 1);
	y |= (child >> 1);
	level++;
	return (level << 24) | (x + y * (1 << level));
}

#pragma dont_inline reset

// The rest of the defs are in XBuckets_inline.h.  We don't want
// to have the entire stupid implementation in every header that
// uses buckets.
//
// This means we have to explcitily 'instantiate' the template once 
// in X-Plane to make the code for the other functions doing something
// like:
//
// template XBuckets<MyTraitsClass>;
//
// #include "XBuckets_inline.h"

// The templated functions cannot be explicitly instantiated, so we have to
// include them in this header too.

template <typename __Object, typename __Traits>
template <class __Ref>
inline void 
XBuckets<__Object, __Traits>::FindTouchesPt(const Point2& p, void (* f)(Object, __Ref), __Ref r) const
{
	for (int level = 0; level < mItems.size(); ++level)
	{
		const BucketMap& bucket_map = mItems[level];
		unsigned int x1, y1, x2, y2;
		GetIndicesForLevel(p, p, level, x1, y1, x2, y2);
		for (unsigned int y = y1; y <= y2; ++y)
		for (unsigned int x = x1; x <= x2; ++x)
		{		
			XBucketID index = XBucket_GetIndex(XBucket_Compose(level, x, y));
			typename BucketMap::const_iterator bucket = bucket_map.find(index);
			if (bucket != bucket_map.end())
			{
				for (typename ObjectVector::const_iterator object = bucket->second.begin(); object != bucket->second.end(); ++object)
				{
					if (Traits::ObjectTouchesPoint(*object, p))
						f(*object, r);
				}
			}
		}
	}	
}

template <typename __Object, typename __Traits>
template <class __Ref>
inline void 
XBuckets<__Object, __Traits>::FindTouchesRect(const Point2& p1, const Point2& p2, void (* f)(Object, __Ref), __Ref r) const
{
	for (int level = 0; level < mItems.size(); ++level)
	{
		const BucketMap& bucket_map = mItems[level];
		unsigned int x1, y1, x2, y2;
		GetIndicesForLevel(p1, p2, level, x1, y1, x2, y2);
		for (unsigned int y = y1; y <= y2; ++y)
		for (unsigned int x = x1; x <= x2; ++x)
		{
			XBucketID index = XBucket_GetIndex(XBucket_Compose(level, x, y));
			typename BucketMap::const_iterator bucket = bucket_map.find(index);
			if (bucket != bucket_map.end())
			{
				for (typename ObjectVector::const_iterator object = bucket->second.begin(); object != bucket->second.end(); ++object)
				{
					if (Traits::ObjectTouchesRect(*object, p1, p2))
						f(*object, r);
				}
			}
		}
	}
}

template <typename __Object, typename __Traits>
template <class __Ref>
inline void 
XBuckets<__Object, __Traits>::FindFullyInRect(const Point2& p1, const Point2& p2, void (* f)(Object, __Ref), __Ref r) const
{
	for (int level = 0; level < mItems.size(); ++level)
	{
		const BucketMap& bucket_map = mItems[level];
		unsigned int x1, y1, x2, y2;
		GetIndicesForLevel(p1, p2, level, x1, y1, x2, y2);
		for (unsigned int y = y1; y <= y2; ++y)
		for (unsigned int x = x1; x <= x2; ++x)
		{
			XBucketID index = XBucket_GetIndex(XBucket_Compose(level, x, y));
			typename BucketMap::const_iterator bucket = bucket_map.find(index);
			if (bucket != bucket_map.end())
			{
				for (typename ObjectVector::const_iterator object = bucket->second.begin(); object != bucket->second.end(); ++object)
				{
					if (Traits::ObjectFullyInRect(*object, p1, p2))
						f(*object, r);
				}
			}
		}
	}
}

template <typename __Object, typename __Traits>
template <class __Ref>
void		
XBuckets<__Object, __Traits>::GetOneBucket(XBucketID id, void (* f)(Object, __Ref), __Ref r) const
{
	unsigned int level = XBucket_GetLevel(id);
	unsigned int index = XBucket_GetIndex(id);

	const BucketMap& bucket_map = mItems[level];
	typename BucketMap::const_iterator bucket = bucket_map.find(index);
	if (bucket != bucket_map.end())
	{
		for (typename ObjectVector::const_iterator object = bucket->second.begin(); object != bucket->second.end(); ++object)
		{
			f(*object, r);
		}
	}
}

template <typename __Object, typename __Traits>
template <class __Ref1, class __Ref2>
inline void		
XBuckets<__Object, __Traits>::IterateBucketLayer(int layer, void (* f)(XBucketID, __Ref1, __Ref2), __Ref1 ref1, __Ref2 ref2) const
{
	const BucketMap& bucket_map = mItems[layer];
	for (typename BucketMap::const_iterator iter = bucket_map.begin(); iter != bucket_map.end(); ++iter)
	{
		f(XBucket_MakeBucketID(layer,iter->first), ref1, ref2);
	}
}

#pragma dont_inline reset

#endif /* XBUCKET_H_ */
