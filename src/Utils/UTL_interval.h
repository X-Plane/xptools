/*
 *  Copyright 2011 Laminar Research.
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

#ifndef UTL_interval_H
#define UTL_interval_H

/*

	UTL_interval - THEORY OF OPERATION
	
	UTL_interval contains a zero or more ranges of numbers, e.g. 0...3 6..10, etc.  The type can be templated.  This is useful when we want to track chunks of time, etc.
	Key properties:
	
	- UTL_interval treats a given "range" as being inclusive on the low side and exclusive on the high side.  So given an interval initialized to 3, 6 the number 3 is IN the
	  region and 6 is OUT of the region.
	- Internal storage is a sorted vector of begin/end pairs.  So editing can be slow due to memory allocation but examinatino is relatively quick.
	- Regions can be empty.
	- Regions cannot be "unbounded" - that is, there is no way to say "all time EXCEPT this range" - the region must have FINITE boundaries.
	
	Boolean ops: the interval class contains boolean operations: union, difference, symmetric difference, and intersection.
	
	Currently this class is only partly optimized: pretty much all cases of += are optimized. 
	
	Merge operations (the default for arbitrary boolean ops) takes O(N+M) as well as the cost of memory allocation (which is done incrementally).
	
	The stand alone merge functions will always run in this time complexity and are accessible primarily for debugging.  In the future, I will optimize
	the operators to be more efficient when in-place operation is possible.
	
	Note that we can't actually figure out the truly optimal time for a merge operation...we always have a choice between a linear iteration through both 
	vs. a search in one driven by the other.  So our alternative is O(MlogN) vs O(M+N).  But the actual performance will vary a lot by what is really returned.
	
	WHY INTERVALS (e.g. inclusive on the bottom and exclusive on the top?)
	
	- If this was not true (we had inclusion) then we could have regions of 0 length, e.g. xon [0,3] and [3,6] would be [3,3] whose length is 0.

*/

template <typename T>
class	UTL_interval {
public:

	typedef pair<T,T>	interval;

	UTL_interval();
	UTL_interval(const UTL_interval& rhs);
	UTL_interval(T b, T e);
	UTL_interval(const interval& x);
	
	UTL_interval& operator=(const interval& rhs);
	UTL_interval& operator=(const UTL_interval& rhs);
	bool operator==(const UTL_interval& rhs) const;
	bool operator!=(const UTL_interval& rhs) const;
	
	void	clear(void);
	bool	empty(void) const;								// True if we contain NO time.
	bool	contains(T t) const;
	bool	contains(T b, T e) const;						// 2,3 is contained in the time interval 0,3 (in other words, ends are exclusive)
	bool	overlaps(T b, T e) const;						// 2,3 does not overlap the time interval 0,2 3,4 (ends are exclusive)

	bool	is_simple(void) const;							// True if we have exactly ONE range.  Empty regions are NOT simple.
	T		get_min(void) const;							// These return the bounds of ALL ranges within the interval.
	T		get_max(void) const;

	UTL_interval& operator+=(const interval& rhs);			// union (add rhs in to included time)
	UTL_interval& operator+=(const UTL_interval& rhs);		// union (add rhs in to included time)
	UTL_interval& operator&=(const interval& rhs);			// intersection (only time in both)
	UTL_interval& operator&=(const UTL_interval& rhs);		// intersection (only time in both)
	UTL_interval& operator-=(const interval& rhs);			// difference (cut time out)
	UTL_interval& operator-=(const UTL_interval& rhs);		// difference (cut time out)
	UTL_interval& operator^=(const interval& rhs);			// symmetric difference (invert each region)
	UTL_interval& operator^=(const UTL_interval& rhs);		// symmetric difference (invert each region)

	#if DEV
	void	validate(void) const;
	#endif
	
	class const_iterator;
	
	const_iterator			begin() const;
	const_iterator			end() const;
	const_iterator			empty_begin() const;
	const_iterator			empty_end() const;

	void swap(UTL_interval& other) { schedule.swap(other.schedule); }
	
private:

	int		region_for(T t) const;

public:
	vector<T>		schedule;			// Sorted vector of start, end, start, end)
};

namespace std {
	template <typename T>
	inline void swap(UTL_interval<T>& a, UTL_interval<T>& b)
	{
		a.swap(b);
	}
}



template<typename T, typename F>
void UTL_interval_merge(UTL_interval<T>& out, const UTL_interval<T>& a, const UTL_interval<T>& b, F func);

struct UTL_interval_op_union { bool operator()(bool a, bool b) const { return a || b; } };
struct UTL_interval_op_intersection { bool operator()(bool a, bool b) const { return a && b; } };
struct UTL_interval_op_difference { bool operator()(bool a, bool b) const { return a && !b; } };
struct UTL_interval_op_symmetric_difference { bool operator()(bool a, bool b) const { return a ^ b; } };

template <typename T>
void UTL_interval_union(UTL_interval<T>& out, const UTL_interval<T>& a, const UTL_interval<T>& b)
{
	UTL_interval_merge(out,a,b,UTL_interval_op_union());
}

template <typename T>
void UTL_interval_intersection(UTL_interval<T>& out, const UTL_interval<T>& a, const UTL_interval<T>& b)
{
	UTL_interval_merge(out,a,b,UTL_interval_op_intersection());
}

template <typename T>
void UTL_interval_difference(UTL_interval<T>& out, const UTL_interval<T>& a, const UTL_interval<T>& b)
{
	UTL_interval_merge(out,a,b,UTL_interval_op_difference());
}

template <typename T>
void UTL_interval_symmetric_difference(UTL_interval<T>& out, const UTL_interval<T>& a, const UTL_interval<T>& b)
{
	UTL_interval_merge(out,a,b,UTL_interval_op_symmetric_difference());
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline bool IS_POSITIVE_INTERVAL(int n)
{
	return (n%2) == 0;
}

template<typename T, typename F>
void UTL_interval_merge(UTL_interval<T>& out, const UTL_interval<T>& a, const UTL_interval<T>& b, F func)
{
	out.clear();
//  Early exit is illegal - we do not know what "func" does given an empty set.	
	int na = 0, nb = 0;
	
	bool sa = false, sb = false;
	
	while(na < a.schedule.size() || nb < b.schedule.size())
	{
		if(na == a.schedule.size())
		{
			sb = IS_POSITIVE_INTERVAL(nb);		
			if(IS_POSITIVE_INTERVAL(out.schedule.size()) == func(sa,sb))
				out.schedule.push_back(b.schedule[nb]);
			++nb;
		}
		else if (nb == b.schedule.size())
		{
			sa = IS_POSITIVE_INTERVAL(na);		
			if(IS_POSITIVE_INTERVAL(out.schedule.size()) == func(sa,sb))
				out.schedule.push_back(a.schedule[na]);
			++na;
		}
		else
		{
			if(a.schedule[na] < b.schedule[nb])
			{
				sa = IS_POSITIVE_INTERVAL(na);		
				if(IS_POSITIVE_INTERVAL(out.schedule.size()) == func(sa,sb))
					out.schedule.push_back(a.schedule[na]);
				++na;
			}
			else if(a.schedule[na] > b.schedule[nb])
			{
				sb = IS_POSITIVE_INTERVAL(nb);		
				if(IS_POSITIVE_INTERVAL(out.schedule.size()) == func(sa,sb))
					out.schedule.push_back(b.schedule[nb]);
				++nb;			
			}
			else
			{
				sa = IS_POSITIVE_INTERVAL(na);		
				sb = IS_POSITIVE_INTERVAL(nb);		
				if(IS_POSITIVE_INTERVAL(out.schedule.size()) == func(sa,sb))
					out.schedule.push_back(b.schedule[nb]);
				++na;
				++nb;
			}
		}
	}
	#if DEV
	out.validate();
	#endif
}

template <typename T>
class UTL_interval<T>::const_iterator {
public:
	typedef const_iterator						self;
	typedef	pair<T,T>							value_type;
	typedef const value_type&					reference;
	typedef const value_type*					pointer;

private:	
	value_type									cache;
	int											n;
	const vector<T> *							src;
	
public:

	const_iterator(const vector<T> * ptr, int i) : src(ptr), n(i) { }
	const_iterator(const self& x) : src(x.src), n(x.n) { }
	
	reference operator*() { cache.first = src->at(n); cache.second = src->at(n+1); return cache; }
	pointer operator->() { cache.first = src->at(n); cache.second = src->at(n+1); return &cache; }

	self& operator++() { n += 2; return *this; }
	self operator++(int) { self tmp = *this; n += 2; return tmp; } 
	self& operator--() { n -= 2; return *this; }
	self operator--(int) { self tmp = *this; n -= 2; return tmp; } 

	bool operator==(const self& x) const { return n == x.n && src == x.src; }
	bool operator!=(const self& x) const { return n != x.n || src != x.src; }
};	


template <typename T>
UTL_interval<T>::UTL_interval()
{
}

template <typename T>
UTL_interval<T>::UTL_interval(const UTL_interval<T>& rhs) : schedule(rhs.schedule)
{
	#if DEV
	rhs.validate();
	#endif
}

template <typename T>
UTL_interval<T>::UTL_interval(T b, T e)
{
	DebugAssert(b<e);
	schedule.push_back(b);
	schedule.push_back(e);
	#if DEV
	validate();
	#endif
}

template <typename T>
UTL_interval<T>::UTL_interval(const interval& x)
{
	DebugAssert(x.first < x.second);
	schedule.push_back(x.first);
	schedule.push_back(x.second);
	#if DEV
	validate();
	#endif
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator=(const interval& rhs)
{
	DebugAssert(rhs.first < rhs.second);
	schedule.resize(2);
	schedule[0] = rhs.first;
	schedule[1] = rhs.second;
	return  *this;
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator=(const UTL_interval<T>& rhs)
{
	rhs.validate();
	schedule = rhs.schedule;
	return  *this;
}


template <typename T>
bool UTL_interval<T>::operator==(const UTL_interval<T>& rhs) const
{
	return schedule == rhs.schedule;
}

template <typename T>
bool UTL_interval<T>::operator!=(const UTL_interval<T>& rhs) const
{
	return schedule != rhs.schedule;
}

template <typename T>
void	UTL_interval<T>::clear(void)
{
	schedule.clear();
}

template <typename T>
bool	UTL_interval<T>::empty(void) const
{
	return schedule.empty();
}

template <typename T>
bool	UTL_interval<T>::contains(T t) const
{
	int idx = region_for(t);
	return (idx % 2 == 0 && idx < schedule.size());
}

template <typename T>
bool	UTL_interval<T>::contains(T b, T e) const
{
	DebugAssert(b < e);
	DebugAssert(schedule.size() % 2 == 0);		
	if(schedule.empty()) return false;
	int idx = region_for(b);
	if(idx == -1) return false;
	if (idx >= schedule.size()-1) return false;	// b is off the end, clearly this is not contained
	if(idx % 2) return false;					// the pt b is NOT contained, it is in or at an empty region.
	
	return e <= schedule[idx+1];				// if e is at or before the end of the region B is in, we are contained.
}

template <typename T>
bool	UTL_interval<T>::overlaps(T b, T e) const
{
	DebugAssert(b < e);
	DebugAssert(schedule.size() % 2 == 0);		
	if(schedule.empty())	return false;
	int idx = region_for(b);
	if(idx == -1) return e > schedule[0];
	if (idx >= schedule.size()-1) return false;	// b is off the end, clearly this cannot overlap.  Also if B direct-hits the last "time out", no overlap.
	if(idx % 2)
	{
		// b starts in a gap.  if e is past the end of the gap, we must have an overlap (not fully contained in space)
		return e > schedule[idx+1];
	}
	else
		return true;							// b starts inside a block, good enough.
}

template <typename T>
bool	UTL_interval<T>::is_simple(void) const { return schedule.size() == 2; }

template <typename T>
T		UTL_interval<T>::get_min(void) const { return schedule.front(); }

template <typename T>
T		UTL_interval<T>::get_max(void) const { return schedule.back(); }


template <typename T>
UTL_interval<T>& UTL_interval<T>::operator+=(const UTL_interval<T>& rhs)
{
	if(rhs.empty())															// union with empty set is a no-op.
		return *this;
	if(rhs.is_simple())														// if the set is a simple interval, shunt off to the 
	{
		this->operator+=(interval(rhs.get_min(),rhs.get_max()));			// "interval" addition op, which is further optimized.
	}
	else if(is_simple())
	{
		interval me(get_min(), get_max());
		schedule = rhs.schedule;
		this->operator+=(me);
	}			
	else if(this->empty())														// If we are empty, we are simply the rhs.
	{
		schedule = rhs.schedule; 
	}
	else if(this->get_max() < rhs.get_min())								// Entire range is after us, simple concat
	{
		schedule.insert(schedule.end(), rhs.schedule.begin(), rhs.schedule.end());
	}
	else if (this->get_max() == rhs.get_min())								// Entire range is after us, but touching.
	{
		schedule.pop_back();
		schedule.insert(schedule.end(), rhs.schedule.begin()+1, rhs.schedule.end());
	}
	else if(rhs.get_max() < this->get_min())								// Entire range is before us, simple concat
	{
		schedule.insert(schedule.begin(), rhs.schedule.begin(), rhs.schedule.end());
	}
	else if (rhs.get_max() == this->get_min())								// Entire range is before us, but touching.
	{
		vector<T> tmp;														// Use a temporary vector to assemble in place, then swap.
		tmp.reserve(schedule.size() + rhs.schedule.size() - 2);				// This is the path of minimal copying and allocation.
		tmp.insert(tmp.end(), rhs.schedule.begin(), rhs.schedule.end()-1);
		tmp.insert(tmp.end(), schedule.begin()+1,schedule.end());
		schedule.swap(tmp);
	}
	else if(this->contains(rhs.get_min(),rhs.get_max()))
	{
		// no-op - if we can identify a fully contained case, we can prevent a complete copy with one bounded searches.
	}
	else
	{
		UTL_interval<T> tmp;
		UTL_interval_union(tmp, *this, rhs);
		tmp.swap(*this);
	}
	#if DEV
		validate();
	#endif
	return *this;	
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator+=(const interval& rhs)
{
//	vector<T>	orig(schedule);									// For debugging assert failures!

	if(this->empty())											// empty region?  Just use the interval
	{
		schedule.resize(2);
		schedule[0]=rhs.first;
		schedule[1]=rhs.second;
	}
	else if(this->get_min() > rhs.second)						// interval is entirely in front of us.
	{
		T arr[2] = { rhs.first, rhs.second };
		schedule.insert(schedule.begin(),arr,arr+2);
	}
	else if(this->get_min() == rhs.second)						// interval is immediately in front of us, pull region a bit.
	{
		schedule.front() = rhs.first;
	}
	else if(this->get_max() < rhs.first)						// interval is fully after us
	{
		schedule.push_back(rhs.first);
		schedule.push_back(rhs.second);
	}
	else if(this->get_max() == rhs.first)						// interval is exactly after us
	{
		schedule.back() = rhs.second;
	}
	else if (this->is_simple())									// we must overlap, otherwise we would have hit a disjoint case.  Simply take the bounds of both.
	{
		schedule[0] = min(schedule[0],rhs.first);
		schedule[1] = max(schedule[1],rhs.second);
	}
	else if(rhs.first <= this->get_min() && rhs.second >= this->get_max())	// complex region is FULLY enveloped.
	{
		schedule.resize(2);
		schedule[0] = rhs.first;
		schedule[1] = rhs.second;
	}
	else														// Finally the slow case: a complex interaction between the two.
	{
		// This is an in-place union for a single interval.  We use region_for (logN time) to find where we will edit, and then we
		// at most erase some of the region in one operation, and edit the interals slightly.  For very large regions, this should
		// be a faster operation than a merge.
		
		// p1 will point to the first region we nuke, and p2 will point to the first region we don't nuke.  To achieve this, we 
		// will find the time region that contains each one, and then massage our regions and time ptrs a bit.  Note that cannonically
		// we merge two positive regions R1 and R2 by nuking R1's end and R2's beignning, as well as anything else that has the bad luck
		// of being in the way.  So p1 will be R1.end and p2 will be R2.end (since we use STL-style deletion semantics).  If R1 == R2
		// then p1 == p2 and there is nothing to do.
		int p1 = region_for(rhs.first);
		int p2 = region_for(rhs.second);

		if(p1 == p2)
		{
			// Special case: if we are in the SAME region, our rule of "mod and erase" isn't going to work.  This is the only case 
			// where we miiight need to ADD a region.  If p1 (our start point) is at or in a full region, we know p2 is in the same
			// region and this is a no-op.  So we only need to cope with p1 & p2 being in ... an empty region.
			// We also know that p2 can't be "dead on" in the empty region because the time region we are adding has to have non-zero
			// length.
			if(p1 % 2)
			{
				DebugAssert(p1 >= 0);				// we should not be "in front" because this is the "all in front" case and is special-cased.
				DebugAssert(p1 < schedule.size());	// same with "all in back", also special cased.
				if (schedule[p1] == rhs.first)
				{
					// This region "extends" an existing region by exactly sitting at its end.  
					schedule[p1] = rhs.second;
				}
				else
				{
					// This region sits in the middle of an empty region.  Insert here, otherwise we will get a criss-crossed 
					// array.
					T arr[2] = { rhs.first, rhs.second };
					schedule.insert(schedule.begin() + p1 + 1, arr,arr+2);
				}
			}
		}
		else
		{
			if(p1 % 2)
			{
				// P1 is at or in an empty region.  This is where we start nuking.  However if we are mid-empty-region we need to
				// pull the NEXT region backward and actually nuke after that,e.g.
				//            V
				// |------*     |--------|
				// |------|   |----------*
				// (This works correctly even if we are before the beginning of the entire region, which can happen.
				if(p1 < 0 || schedule[p1] != rhs.first)
				{
					// P1 is in the middle of an empty region.  Move our next count backward (now we are direct hit in full region)
					// and increment.
					++p1;						// This moves us to the end of our empty region.
					schedule[p1] = rhs.first;
					++p1;						// This moves us to the end of the full region we moved over.
				}
			}
			else
			{
				// P1 is in the beginning or middle of a full region.  Move forward to indicate that we clip the region's end.
				++p1;
			}
			
			if(p2 % 2)
			{
				// P2 is at the end of a full region (beginning of an empty).  
				if(schedule[p2] != rhs.second)
				{
					// If the full region ended too early, simply stretch its end to where it should be.
					//            V
					// |------*      |-------|
					// |----------*  |-------|
					schedule[p2] = rhs.second;
				}
			} else {
				if(p2 == schedule.size())
				{
					// P2 is off the end.  Take the region before us and make it end later, then use it.
					--p2;
					schedule[p2] = rhs.second;
				}
				else
				{
					// P2 is at the beginning or middle of a full region.  So this region's end is what we want.
					++p2;
				}
			}
			
			if(p1 < p2)
			{
				DebugAssert(((p2 - p1) % 2) == 0);
				schedule.erase(schedule.begin()+p1,schedule.begin()+p2);
			}
		}
	}
	#if DEV
		validate();
	#endif
	return *this;	
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator&=(const UTL_interval<T>& rhs)
{
	if(empty())														// Empty cases mean we are disjoint and clear.
		return *this;
	if(rhs.empty())
		clear();
	else if (rhs.is_simple())										// Simple region?  Use simple &=.  Note that since we are symmetric, we can
		operator &=(interval(rhs.get_min(), rhs.get_max()));		// do this no matter who is simple. (Just kidding - rhs is immutable. Doh!
	else if(is_simple())
	{
		interval me(get_min(), get_max());
		schedule = rhs.schedule;
		operator &= (me);
	}
	else if(get_max() <= rhs.get_min() ||							// Disjoint ranges?  Zero intersection.
			rhs.get_max() <= get_min())
	{
		clear();
	}
	else
	{
		UTL_interval<T> tmp;
		UTL_interval_intersection(tmp, *this, rhs);
		tmp.swap(*this);
	}
	return *this;	
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator&=(const interval& rhs)
{
	// if we are empty, this is a no-op.
	if(empty()) return *this;
	
	// Quick disjoint test.
	if(rhs.second <= get_min() || rhs.first >= get_max())
	{
		clear();
		return *this;
	}


	if(is_simple())
	{
		schedule[0] = max(schedule[0],rhs.first);
		schedule[1] = min(schedule[1],rhs.second);
		return *this;
	}

	// We are going to keep everything from p1 inclusive to p2 exclusive.  Or in other words, p2 is the first dead thing, p1 is kept.

	int p1 = region_for(rhs.first);
	int p2 = region_for(rhs.second);

	if(p1 == p2)
	{
		// Special case when the entire clip zone is in ONE region.  In this case we are simply going to get zero or one region.
	
		DebugAssert(p1 >= 0);	// can't be before the beginning AND not spanning a region, or we would have special-cased that case.
		if(p1 % 2)
		{
			// We are clipping to a point entirely in empty space.  Clear and done.
			clear();
		}
		else
		{
			// We are clipping entirely in one full region.  We should be the region.
			DebugAssert(schedule[p1  ] <= rhs.first);
			DebugAssert(schedule[p1+1] >  rhs.second);
			schedule.resize(2);
			schedule[0] = rhs.first;
			schedule[1] = rhs.second;			
		}
	}
	else
	{
		// General case.  Set up p1 and p2 to the first keep and first nuke zone.
		DebugAssert(p1 < (int) schedule.size());		// If we were disjoint, we are special cased
		
		if(p1 >= 0)								// p1 < 0 means beginning of clip is SMALLER than us.  no-op in front.
		{
			if(p1 % 2)
			{
				// P1 is in an empty space.  next full region is the FIRST thing we keep.
				++p1;		
			}
			else if(schedule[p1] != rhs.first)
			{
				// P1 is in the middle of a full region.  Pull the region forward, and keep the region.
				schedule[p1] = rhs.first;
			}
		}

		DebugAssert(p2 >= 0);				// Otherwise we were disjoint, should have been special-cased.
		if(p2 < schedule.size())			// p2 == schedule.size() means the end of the clip is BIGGER than us.  no-op in back.
		{
			if(p2 % 2)
			{
				// If p2 is in or at an empty region, everything after p2 gets nuked.
				++p2;
			}
			else if(schedule[p2] != rhs.second)
			{
				// P2 is in a full region.  This region ends at rhs and we nuke the next.
				++p2;
				schedule[p2] = rhs.second;
				++p2;
			}
		}
		
		DebugAssert(p1 <= p2);

		if(p2 < schedule.size())
			schedule.erase(schedule.begin()+p2,schedule.end());
		if(p1 > 0)
			schedule.erase(schedule.begin(),schedule.begin() + p1);

	}

	#if DEV
		validate();
	#endif
	return *this;	
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator-=(const UTL_interval<T>& rhs)
{
	if(rhs.empty() ||						// Subtract empty is a no-op
	       empty() ||						// If we are empty, we are already done
		rhs.get_max() <= get_min() ||		// Also if sets are disjoint, subtraction does nothing.
		rhs.get_min() >= get_max())
	{
		return *this;
	}
	
	if(rhs.is_simple())
	{
		operator -=(interval(rhs.get_min(), rhs.get_max()));		
	}
	// -= is the one non-symmetrical operation...so we can't subtract ourselves
	// from RHS if we are simple.
	else
	{	
		UTL_interval<T> tmp;
		UTL_interval_difference(tmp, *this, rhs);
		tmp.swap(*this);
	}
	#if DEV
		validate();
	#endif
	return *this;	
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator-=(const interval& rhs)
{
	if(    empty() ||						// If we are empty, we are already done
		rhs.second <= get_min() ||		// Also if sets are disjoint, subtraction does nothing.
		rhs.first >= get_max())
	{
		return *this;
	}
	
	// complete clearing case.
	if(rhs.first <= get_min() && rhs.second >= get_max())
	{
		clear();
		return *this;
	}
	
	if(is_simple())
	{
		// We have already eliminated disjoint cases (no-op) and "surround" cases (complete replace).
		// We have two other possibilities: a 'hole' or a clip.
		
		if(rhs.first > get_min() && rhs.second < get_max())
		{
			schedule.resize(4);
			schedule[3] = schedule[1];
			schedule[1] = rhs.first;
			schedule[2] = rhs.second;
		}
		else if(rhs.first <= schedule[0])
		{
			schedule[0] = max(schedule[0],rhs.second);
			DebugAssert (schedule[1] > schedule[0]);			// If this was not true, rhs second > schedule 1.  But since rhs first already < 0, that would have been the CLEAR case!
		}
		else if(rhs.second >= schedule[1])
		{
			schedule[1] = min(rhs.first, schedule[1]);
			DebugAssert (schedule[1] > schedule[0]);
		}
		else
		{
			DebugAssert(!"should not hit his case.\n");
		}
		
		#if DEV
			validate();
		#endif
		return *this;
	}
	
	int p1 = region_for(rhs.first);
	int p2 = region_for(rhs.second);
	
	if(p1 == p2)
	{
		DebugAssert(p1 >= 0);				// Since we are all in ONE region, we know that we have to be in a real region, not off an end, which is a disjoint case
		DebugAssert(p1 < schedule.size());	// handled above.
		
		if((p1 % 2) == 0)					// Only care if we are in a FULL region!
		{
			if(schedule[p1] == rhs.first)
			{
				// We are subtracting from a region that STARTS at our cut point.  Trim the front.
				DebugAssert(schedule[p1+1] > rhs.second);
				schedule[p1] = rhs.second;
			}
			else
			{
				// We are cutting a HOLE in an existing region.  Insert the "hole".
				DebugAssert(schedule[p1] < rhs.first);
				DebugAssert(schedule[p1+1] > rhs.second);
				T arr[2] = { rhs.first, rhs.second };
				schedule.insert(schedule.begin()+p1+1,arr,arr+2);
			}
		}		
	}
	else
	{
		DebugAssert(p1 < (int) schedule.size());
		DebugAssert(p2 >= 0);
		
		if((p1 >= 0) && (p1 % 2))
		{
			// We hit an empty region.  Keep it - first thing to nuke is the NEXT region.
			++p1;
		}
		else if(p1 < 0)
		{
			DebugAssert(p1 == -1);
			// We are cutting before the beginning.  Nuke the first thing.
			++p1;
		} 
		else if(schedule[p1] != rhs.first)
		{
			// We are starting mid-full block.  Make the block end early at our cut point.  This block lives, the next one dies.
			++p1;
			schedule[p1] = rhs.first;
			++p1;
		}

		if(p2 == schedule.size())
		{
			// Last block is off the end...that's okay.  Means we nuke to the end, no biggie.
		}
		else if(p2 % 2)
		{
			// We are in an empty block.  First thing we keep is the next one.
			++p2;
		}
		else if(schedule[p2] != rhs.second)
		{
			// We end in a full block.  Cut this guy's front, but keep the fornt.
			schedule[p2] = rhs.second;
		}
		
		DebugAssert(p1 <= p2);
		if(p1 < p2)
		{
			DebugAssert(((p2 - p1) % 2) == 0);
			schedule.erase(schedule.begin()+p1,schedule.begin()+p2);
		}
		
	}
	#if DEV
		validate();
	#endif
	return *this;	
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator^=(const UTL_interval<T>& rhs)
{
	if(rhs.empty()) return *this;	
	
	if(empty())
	{
		schedule = rhs.schedule;
	}
	else if(rhs.is_simple())
	{
		operator ^=(interval(rhs.get_min(), rhs.get_max()));
	}
	else if(is_simple())
	{
		interval me(get_min(), get_max());
		schedule = rhs.schedule;
		operator ^=(me);
	}
	// This is a bunch of disjoint operations, same as +=, because we are disjoint, ^= and += are the same.	
	else if(this->get_max() < rhs.get_min())								// Entire range is after us, simple concat
	{
		schedule.insert(schedule.end(), rhs.schedule.begin(), rhs.schedule.end());
	}
	else if (this->get_max() == rhs.get_min())								// Entire range is after us, but touching.
	{
		schedule.pop_back();
		schedule.insert(schedule.end(), rhs.schedule.begin()+1, rhs.schedule.end());
	}
	else if(rhs.get_max() < this->get_min())								// Entire range is before us, simple concat
	{
		schedule.insert(schedule.begin(), rhs.schedule.begin(), rhs.schedule.end());
	}
	else if (rhs.get_max() == this->get_min())								// Entire range is before us, but touching.
	{
		vector<T> tmp;														// Use a temporary vector to assemble in place, then swap.
		tmp.reserve(schedule.size() + rhs.schedule.size() - 2);				// This is the path of minimal copying and allocation.
		tmp.insert(tmp.end(), rhs.schedule.begin(), rhs.schedule.end()-1);
		tmp.insert(tmp.end(), schedule.begin()+1,schedule.end());
		schedule.swap(tmp);
	}
	else
	{
		UTL_interval<T> tmp;
		UTL_interval_symmetric_difference(tmp, *this, rhs);
		tmp.swap(*this);
	}
	#if DEV
		validate();
	#endif
	return *this;	
}

template <typename T>
UTL_interval<T>& UTL_interval<T>::operator^=(const interval& rhs)
{
	// A few special cases where we know the exact structure of the invert in advance.  This can save us having to do a ton of copying...
	// while algorithmically an inversion is trivial, inserting into the middle of a vector sucks.
	if(empty())
	{
		schedule.resize(2);
		schedule[0] = rhs.first;
		schedule[1] = rhs.second;		
	}
	else if (rhs.second < get_min())
	{
		T arr[2] = { rhs.first, rhs.second };
		schedule.insert(schedule.begin(), arr,arr+2);
	}
	else if (rhs.first > get_max())
	{
		schedule.push_back(rhs.first);
		schedule.push_back(rhs.second);
	}
	else if(rhs.second == get_min())
	{
		schedule.front() = rhs.first;
	}
	else if(rhs.first == get_max())
	{
		schedule.back() = rhs.second;
	}
	else
	{
		// Inversion of a range is actually REALLY easy.  As it turns out, simply toggling the presence of our 
		// values in the number sequence IS an inversion.  Thus if we find ourselves, we nuke ourselves, and if we nuke
		// ourselves, we find ourselves.

		// We didn't NEED to special case the above, because this works in all cases, but the above avoid doing up to 2
		// mid-array inserts with reallocation.
		typename vector<T>::iterator i = lower_bound(schedule.begin(), schedule.end(), rhs.first);
		if(i != schedule.end() && *i == rhs.first)
			schedule.erase(i);
		else
			schedule.insert(i,rhs.first);

		i = lower_bound(schedule.begin(), schedule.end(), rhs.second);
		if(i != schedule.end() && *i == rhs.second)
			schedule.erase(i);
		else
			schedule.insert(i,rhs.second);
	}
	#if DEV
		validate();
	#endif
	return *this;	
}

template <typename T>
int		UTL_interval<T>::region_for(T t) const
{
	typename vector<T>::const_iterator i = lower_bound(schedule.begin(), schedule.end(), t);
	if(i == schedule.end()) return schedule.size();
	int n = distance(schedule.begin(), i);
	
	if(*i == t)
		return n;
	return n-1;
}

template <typename T>
typename UTL_interval<T>::const_iterator	UTL_interval<T>::begin() const
{
	return const_iterator(&schedule, 0);
}

template <typename T>
typename UTL_interval<T>::const_iterator	UTL_interval<T>::end() const
{
	return const_iterator(&schedule, schedule.size());
}

template <typename T>
typename UTL_interval<T>::const_iterator	UTL_interval<T>::empty_begin() const
{
	return const_iterator(&schedule, 1);
}

template <typename T>
typename UTL_interval<T>::const_iterator	UTL_interval<T>::empty_end() const
{
	return const_iterator(&schedule, (schedule.size() > 2) ? schedule.size()-1 : 1);
}

#if DEV
template <typename T>
void	UTL_interval<T>::validate(void) const
{
	DebugAssert(schedule.size() % 2 == 0);
	for(int n = 1; n < schedule.size(); ++n)
	{
		DebugAssert(schedule[n] > schedule[n-1]);
	}
}
#endif

#endif /* UTL_interval_H */
