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
#ifndef XCULL_H
#define XCULL_H

#include "XBuckets.h"

/***************************************************
	An X-Haiku
	
	X-Cull checks objects
	Some are not visible now
	They will not be drawn
 ***************************************************/


/*
	XCull - THEORY OF OPERATION
	
	XCull is an add-on to XBuckets...it calculates visibility 
	information for a number of buckets.  A few important things
	to note here:
	
	 o XCull requires a camera type and the culling operations - 
	   it is just a book-keeping class.
	 
	 o XCull is separate from XBuckets - this way we can make many
	   XBuckets for different types of objects and have one XCull
	   that stores the visibility for all buckets.
	   
	 o XCull doesn't have any geometric extent...you are required
	   to figure out where each bucket is when you do the camera 
	   operations.  The easiest way to do this is to ask XBuckets
	   where a bucket is, but it might be desirable to do something
	   else.
	   
	XCull uses the concept of layer level of detail cutoffs...here's
	how it works:
	
	1. XCull always culls every single bucket in an area.  This is 
	   because one cull structure is used for many sets of buckets,
	   making it hard to know which buckets are useful and which
	   are not.  In practice, culling buckets is very rapid (both
	   because it is computationally inexpensive if your camera check
	   is good and because the pattern of recursive visible buckets
	   is usually pretty efficient.
	   
	2. You then iterate, getting a callback to the visible buckets.
	
	3. You provide two filtering parameters for the callbacks:
	
	   - A max level of detail distance beyond which nothing is visible.
	   This prevents XCull from iterating over buckets that are within
	   the view frustum but only contain objects too small to be
	   seen when far away.
	   
	   - A max bucketing level, beyond which the buckets are empty
	   because all objects are too big.
	   
	   Between these two checks we tend to eliminate buckets both
	   for layers that have big objects (by not iterating on small
	   buckets) and small objects (by not iterating on far buckets).
	   This produces a consistant amount of work over varying distance.
		
	
/*

/*

	Traits - since XCull is a template class, you provide a traits class
	that both defines datatypes and a few algorithms.  See XBuckets.h for
	more info; the traits class for XCull is almost the same.

	Scalar - a numeric type used for calculations.  Double or xdob recommended.
	Camera - your camera datatype.  A pointer type is strongly recommended.

class	Traits {
public:

	typedef	double				Scalar;
	typedef x_camera *			Camera;
	typedef void *				Reference;
	
	// Is the given bucket visible to the given camera?
	static bool		IsVisible(XBucketID id, Camera camera, Reference reference);

	// How far is this bucket from the camera?  Return a SQUARED
	// distance...there's no point in ever doing a square route
	// for LOD.
	static Scalar	DistanceSquared(XBucketID id, Camera camera, Reference reference);
	
};

*/

#if BENTODO
FIX THIS
#endif
// I am working on a version of XCull that does not have to make a function call
// to itself to traverse the bucket structure...the theory is that this will
// save a ton of function calls.  My version right now does not work, so leave
// this #define 0!!
#define NO_RECURSION 1

template <typename __Traits>
class XCull {
public:

	typedef __Traits					Traits;
	typedef typename Traits::Scalar		Scalar;
	typedef typename Traits::Camera		Camera;
	typedef typename Traits::Reference	Reference;
	
				XCull();
				XCull(unsigned int 	inLayers);
				~XCull();
	void		Reset(unsigned int 	inLayers);

	// For a given layer, set the LOD cutoff, e.g. for all
	// of the buckets in this layer, none of them is visible
	// any farther than this distance squared.  It is crucial
	// to set these numbers before culling!

	// Go through the entire set of buckets and calculate both
	// which are visible and how far away they are.  Do this
	// any time the camera has moved.
	int			Cull(
						Camera			inCamera,
						Reference		reference);
	
	// Iterate - you pass in a function that is then called
	// for every visible bucket.  The distance of the bucket from
	// the camera is passed as well as the bucket's ID.
	template <class __Ref>
	inline void	Iterate(
						unsigned int	inMaxLevel,
						Scalar			inMaxLOD,		
						Scalar			inBucketSize,				
						void (*)(XBucketID id, Scalar lod, __Ref ref),
						__Ref			inRef);

private:

	void		CullBucket(
						XBucketID		bucket,
						Camera			inCamera,
						Reference		reference);

#if !NO_RECURSION
	template <class __Ref>
	void		IterateBucket(
						XBucketID		id,
						unsigned int	inMaxLevel,
						Scalar			inMaxLOD,		
						Scalar			inBucketSize,				
						void (*)(XBucketID id, Scalar lod, __Ref ref),
						__Ref			inRef);
#endif

	typedef	vector<bool>			VisibleVector;
	typedef vector<Scalar>			DistanceVector;
	
	// Each of these vectors creates an array for the number of layers.
	vector<VisibleVector>			mVisible;
	vector<DistanceVector>			mDistances;
	int								mTotalVisible;

};

// Inline functions sold separately
// #include "XCull_inline.h"
// Templated member functions included here.

#if NO_RECURSION

#define STACK_DEPTH 32

#define STEP_DOWN	\
		id[level+1] = XBucket_GetChild(id[level], step[level]);			\
		step[level+1] = 0;												\
		step[level]++;													\
		level++;

#define UNROLL		\
	--level;															\
	while(level >= 0 && step[level] > 3) --level;						\
	if (level >= 0 && step[level] < 4)									\
	{																	\
		STEP_DOWN														\
	}


template <typename __Traits>
template <class __Ref>
inline void	
XCull<__Traits>::Iterate(
						unsigned int	inMaxLevel,
						Scalar			inMaxLOD,
						Scalar			inBucketSize,
						void (* func)(XBucketID id,Scalar lod, __Ref ref),
						__Ref			inRef)
{
	int		step[STACK_DEPTH];			// "STEP" is which of my children we must visit next!
	int		id[STACK_DEPTH];				// "ID" is the bucket we are on for this level.
	Scalar	bucket_sizes[STACK_DEPTH];
	
	// inMaxLevel is the _inclusive_ max depth to check, use it to make sure
	// we don't blow past the array end.
	if (inMaxLevel > (mVisible.size()-1)) inMaxLevel = mVisible.size()-1;
	
	int level = 0;
	id[level] = 0;
	step[level] = 0;
	bucket_sizes[0] = inBucketSize;
	
	// Precompute our bucekt sizes once...almost guaranteed to be faster than doing it
	// per iteration step.
	for (int i = 1; i <= inMaxLevel; ++i)
		bucket_sizes[i] = 0.5 * bucket_sizes[i-1];

	while(1)
	{
//		deverr << " Checking visible for level " << level << " id " << id[level]
		if (mVisible[level][XBucket_GetIndex(id[level])])
		{
			Scalar	d_sqrd = mDistances[level][XBucket_GetIndex(id[level])];

			if (inMaxLOD == -1.0 || d_sqrd <= sqr(inMaxLOD+bucket_sizes[level]))
			{
				func(id[level],d_sqrd,inRef);
				
				if (level < (inMaxLevel))
				{
					if (step[level] > 3)  {
						UNROLL
					} else {
						STEP_DOWN
					}
				} else {
					UNROLL
				}
			} else {
				UNROLL
			}		
		} else {
			UNROLL
		}	
		if (level < 0) break;
	}	
}


#else /* NO_RECURSION */

template <typename __Traits>
template <class __Ref>
void	XCull<__Traits>::Iterate(
						unsigned int	inMaxLevel,
						Scalar			inMaxLOD,
						Scalar			inBucketSize,
						void (* func)(XBucketID id, Scalar lod, __Ref ref),
						__Ref			inRef)
{
	if (mVisible[0][0])
		IterateBucket(0, inMaxLevel, inMaxLOD, inBucketSize, func, inRef);
}						

template <typename __Traits>
template <class __Ref>
void	XCull<__Traits>::IterateBucket(
						XBucketID		id,
						unsigned int	inMaxLevel,
						Scalar			inMaxLOD,
						Scalar			inBucketSize,						
						void (* func)(XBucketID id, Scalar lod, __Ref ref),
						__Ref			inRef)
{
	// First call the iterator on our bucket...pass in our LOD.
	unsigned int level = XBucket_GetLevel(id);
	Scalar	d_sqrd = mDistances[level][XBucket_GetIndex(id)];
//	if (inMaxLODSquared != -1.0)
//		deverr << " Our level: " << level << " max level " << inMaxLevel << " our dist " << d_sqrd << " max dist " << inMaxLODSquared << "\n";
	if (inMaxLOD == -1.0 || d_sqrd <= sqr(inMaxLOD+inBucketSize))
	{
	func(id,d_sqrd,inRef);
		
	// Now if we're not already on the last level, iterate the children.
		// Also, only go on if our level is not too high.
		if (level < inMaxLevel && level < (mVisible.size()-1))
	{
		for (unsigned int n = 0; n < 4; ++n)
		{
			XBucketID child = XBucket_GetChild(id, n);
			if (mVisible[XBucket_GetLevel(child)][XBucket_GetIndex(child)])
					IterateBucket(child, inMaxLevel, inMaxLOD, 0.5 * inBucketSize, func, inRef);
			}
		}
	}
}

#endif /* NO_RECURSION */

#endif /* XCULL_H */
